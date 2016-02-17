//
//  CameraView.m
//  CameraClient
//
//  Created by Zoran Angelov on 10/29/15.
//  Copyright © 2015 uQuad. All rights reserved.
//

#import "CameraView.h"

#include <uquad/comm/CameraClient.h>
#include <uquad/base/Runloop.h>
#include <uquad/hal/av/CVImagePixelSampleBuffer.h>
#include <future>

namespace helper
{
    using namespace uquad;
    
    class CameraClientHelper
        : media::PlayoutDelegate
    {
    public:
        CameraClientHelper(CameraViewController *view);
        ~CameraClientHelper();
        
        void onPlayoutVideoResized(Vec2i const &size);
        void onPlayoutVideoFrame(intrusive_ptr<base::PixelSampleBuffer> const &buffer);
        
    private:
        CameraViewController *m_Controller;
        intrusive_ptr<base::Runloop> m_Runloop;
        comm::CameraClient m_CameraClient;
        thread m_RunloopThread;
        bool m_bStopRunloopThread;
    };
    
} //namespace helper

@interface CameraView ()

- (void)onPlayoutVideoResized:(uquad::Vec2i) size;
- (void)onPlayoutVideoFrame:(uquad::intrusive_ptr<uquad::base::PixelSampleBuffer> const&) buffer;

@end


@implementation CameraView

- (id)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    
    self.videoLayer = [[AVSampleBufferDisplayLayer alloc] init];
    self.videoLayer.bounds = self.bounds;
    self.videoLayer.position = CGPointMake(CGRectGetMidX(self.bounds), CGRectGetMidY(self.bounds));
    self.videoLayer.videoGravity = AVLayerVideoGravityResizeAspect;
    self.videoLayer.backgroundColor = CGColorGetConstantColor(kCGColorBlack);
    
    [self setLayer:self.videoLayer];
    [self setWantsLayer:YES];
    
    return self;
}

- (void)onPlayoutVideoResized:(uquad::Vec2i) size
{
}

- (void)onPlayoutVideoFrame:(uquad::intrusive_ptr<uquad::base::PixelSampleBuffer> const&) buffer
{
    if(uquad::intrusive_ptr<uquad::hal::av::CVImagePixelSampleBuffer> cvPixelBuffer = uquad::dynamic_pointer_cast<uquad::hal::av::CVImagePixelSampleBuffer>(buffer))
    {
        if(self.videoLayer.readyForMoreMediaData)
        {
            [self.videoLayer enqueueSampleBuffer:cvPixelBuffer->imageBuffer()];
        }
    }
}

@end

@interface CameraViewController ()
{
    helper::CameraClientHelper *m_CameraClientHelper;
}

- (id)init;
- (void)dealloc;

@end


@implementation CameraViewController

- (id)init
{
    self = [super init];
    m_CameraClientHelper = 0;
    return self;
}

- (void)dealloc
{
    if(m_CameraClientHelper)
    {
        delete m_CameraClientHelper;
    }
}

- (IBAction) connectOrDisconnect:(id) sender
{
    if(m_CameraClientHelper)
    {
        delete m_CameraClientHelper;
        m_CameraClientHelper = 0;
        connectButton.title = @"Connect";
        
    }
    else
    {
        m_CameraClientHelper = new helper::CameraClientHelper(self);
        connectButton.title = @"Disconnect";
    }
}


@end


namespace helper
{
    CameraClientHelper::CameraClientHelper(CameraViewController *controller)
        : m_Controller(controller)
        , m_Runloop(new base::Runloop())
        , m_CameraClient(m_Runloop)
        , m_RunloopThread()
        , m_bStopRunloopThread(false)
    {
        m_CameraClient.getPlayout()->playoutDelegate() += this;
        m_CameraClient.connect();
        m_RunloopThread = std::move(thread([this]()
        {
            m_Runloop->runUntil([this]() -> bool
            {
                return !m_bStopRunloopThread;
            });
        }));
    }
    
    CameraClientHelper::~CameraClientHelper()
    {
        std::promise<system::error_code> p;
        m_CameraClient.disconnect([&p](system::error_code const &e) { p.set_value(e); });
        p.get_future().wait();
        m_bStopRunloopThread = true;
        m_RunloopThread.join();
        m_CameraClient.getPlayout()->playoutDelegate() -= this;
    }
    
    void CameraClientHelper::onPlayoutVideoResized(Vec2i const &size)
    {
        [m_Controller->cameraView onPlayoutVideoResized:size];
    }
    
    void CameraClientHelper::onPlayoutVideoFrame(intrusive_ptr<base::PixelSampleBuffer> const &buffer)
    {
        [m_Controller->cameraView onPlayoutVideoFrame:buffer];
    }

} //namespace helper

