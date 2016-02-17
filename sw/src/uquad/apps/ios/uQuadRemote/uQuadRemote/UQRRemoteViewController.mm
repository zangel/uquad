//
//  UQRRemoteViewController.mm
//  uQuadRemote
//
//  Created by Zoran Angelov on 11/3/15.
//  Copyright © 2015 uQuad. All rights reserved.
//

#import "UQRRemoteViewController.h"

#import <Metal/Metal.h>
#import <simd/simd.h>
#import <MetalKit/MetalKit.h>
#import <CoreVideo/CVMetalTextureCache.h>
#include "UQRClient.h"
#include <uquad/hal/av/CVImagePixelSampleBuffer.h>
#include <uquad/media/Playout.h>


namespace
{
    static NSUInteger const g_MaxInflightBuffers = 1;
    
    static float const g_QuadVertexData[16] =
    {
        -1.0, -1.0,  0.0, 1.0,
         1.0, -1.0,  1.0, 1.0,
        -1.0,  1.0,  0.0, 0.0,
         1.0,  1.0,  1.0, 0.0
    };

    typedef struct
    {
        matrix_float3x3 matrix;
        vector_float3 offset;
    } ColorConversion;
    
    class Callbacks
        : public uquad::media::PlayoutDelegate
        , public uquad::comm::CameraClientDelegate
    {
    public:
        Callbacks(UQRRemoteViewController *vc);
        ~Callbacks();
        
        void onPlayoutVideoResized(uquad::Vec2i const &size);
        void onPlayoutVideoFrame(uquad::intrusive_ptr<uquad::base::PixelSampleBuffer> const &buffer);
        void onCameraClientDisconnected();
        
    private:
        __weak UQRRemoteViewController *m_VC;
    };
    
    
} //namespace anonymous


@interface UQRRemoteViewController ()
{
    CAMetalLayer *m_MetalLayer;
    id<CAMetalDrawable> m_CurrentDrawable;
    BOOL m_LayerSizeDidUpdate;
    MTLRenderPassDescriptor *m_MetalRenderPassDescriptor;
    
    CADisplayLink *m_DisplayLink;
    dispatch_semaphore_t m_InflightSemaphore;
    
    id<MTLDevice> m_MetalDevice;
    id<MTLCommandQueue> m_MetalCommandQueue;
    id<MTLLibrary> m_MetalDefaultLibrary;
    id<MTLRenderPipelineState> m_MetalPipelineState;
    id<MTLBuffer> m_MetalVertexBuffer;
    id<MTLDepthStencilState> m_MetalDepthState;
	id<MTLTexture> m_MetalTextureY;
	id<MTLTexture> m_MetalTextureCbCr;
	id<MTLBuffer> m_MetalColorConversionBuffer;
    
    CVMetalTextureCacheRef m_MetalTextureCache;
    
    Callbacks *m_Callbacks;
}

@property IBOutlet UIActivityIndicatorView *connectActivity;

- (id)initWithCoder:(NSCoder*)aDecoder;
- (void)dealloc;
- (void)setupMetal;
- (void)setupMetalTextureCache;
- (void)loadAssets;

- (void)displayLinkCallback;
- (void)setupRenderPassDescriptorForTexture:(id <MTLTexture>) texture;
- (void)render;

- (void)playoutDidRecieveRemoteCameraBuffer:(uquad::intrusive_ptr<uquad::base::PixelSampleBuffer> const &) buffer;
- (void)cameraClientDidDisconnect;

- (void)connectRemoteCamera;
- (void)disconnectRemoteCamera;

- (id<CAMetalDrawable>)currentDrawable;

- (IBAction)navigateBack:(id)sender;

@end

@implementation UQRRemoteViewController

@synthesize viewControllerDelegate;
@synthesize connectActivity;


- (id)initWithCoder:(NSCoder*)aDecoder
{
    self = [super initWithCoder:aDecoder];
    m_Callbacks = new Callbacks(self);
    return self;
    
}

- (void)dealloc
{
    if(m_Callbacks)
    {
        delete m_Callbacks;
        m_Callbacks = nullptr;
    }
    [m_DisplayLink invalidate];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    m_InflightSemaphore = dispatch_semaphore_create(g_MaxInflightBuffers);
    
    [self setupMetal];
    [self setupMetalTextureCache];
    [self loadAssets];
    
    m_DisplayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(displayLinkCallback)];
    [m_DisplayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
    m_DisplayLink.paused = YES;
    
    [self connectRemoteCamera];
}

- (void)viewDidUnload
{
    [super viewDidUnload];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (BOOL)shouldAutorotate
{
    return NO;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return (UIInterfaceOrientationMaskLandscapeRight);
}

- (NSUInteger)supportedInterfaceOrientations
{
    return UIInterfaceOrientationMaskLandscapeRight;
}

- (UIInterfaceOrientation)preferredInterfaceOrientationForPresentation
{
    return UIInterfaceOrientationLandscapeRight;
}

- (NSUInteger)application:(UIApplication *)application supportedInterfaceOrientationsForWindow:(UIWindow *)window
{
    return UIInterfaceOrientationLandscapeRight;
}

- (void)viewWillAppear:(BOOL)animated
{
    UQRClient::instance()->cameraClient()->cameraClientDelegate() += m_Callbacks;
    UQRClient::instance()->cameraClient()->getPlayout()->playoutDelegate() += m_Callbacks;
    m_DisplayLink.paused = NO;
    [super viewWillAppear:animated];
}

- (void)viewDidDisappear:(BOOL)animated
{
    m_DisplayLink.paused = YES;
    UQRClient::instance()->cameraClient()->getPlayout()->playoutDelegate() -= m_Callbacks;
    UQRClient::instance()->cameraClient()->cameraClientDelegate() -= m_Callbacks;
    [super viewDidDisappear:animated];
}


- (void)setupMetal
{
    m_MetalDevice = MTLCreateSystemDefaultDevice();
    
    // Create a new command queue
    m_MetalCommandQueue = [m_MetalDevice newCommandQueue];
    
    // Load all the shader files with a metal file extension in the project
    m_MetalDefaultLibrary = [m_MetalDevice newDefaultLibrary];
    
    // Setup metal layer and add as sub layer to view
    m_MetalLayer = [CAMetalLayer layer];
    m_MetalLayer.device = m_MetalDevice;
    m_MetalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    
    // Change this to NO if the compute encoder is used as the last pass on the drawable texture
    m_MetalLayer.framebufferOnly = YES;
    
    // Add metal layer to the views layer hierarchy
    [m_MetalLayer setFrame:self.view.layer.frame];
    //[self.view.layer addSublayer:m_MetalLayer];
    [self.view.layer insertSublayer:m_MetalLayer atIndex:0];

    
    self.view.opaque = YES;
    self.view.backgroundColor = nil;
    self.view.contentScaleFactor = [UIScreen mainScreen].scale;
}

- (void)setupMetalTextureCache
{
    CVMetalTextureCacheCreate(NULL, NULL, m_MetalDevice, NULL, &m_MetalTextureCache);
	
	ColorConversion colorConversion =
    {
		.matrix =
        {
			.columns[0] = { 1.164,  1.164, 1.164, },
			.columns[1] = { 0.000, -0.392, 2.017, },
			.columns[2] = { 1.596, -0.813, 0.000, },
		},
		.offset = { -(16.0/255.0), -0.5, -0.5 },
	};
	
	m_MetalColorConversionBuffer = [m_MetalDevice newBufferWithBytes:&colorConversion length:sizeof(colorConversion) options:MTLResourceOptionCPUCacheModeDefault];
}

- (void)loadAssets
{
    // Load the fragment program into the library
    id <MTLFunction> fragmentProgram = [m_MetalDefaultLibrary newFunctionWithName:@"videoFragColorConversion"];
    
    // Load the vertex program into the library
    id <MTLFunction> vertexProgram = [m_MetalDefaultLibrary newFunctionWithName:@"videoVertPassthrough"];
    
    // Setup the vertex buffers
    m_MetalVertexBuffer = [m_MetalDevice newBufferWithBytes:g_QuadVertexData length:sizeof(g_QuadVertexData) options:MTLResourceOptionCPUCacheModeDefault];
    m_MetalVertexBuffer.label = @"videoVertices";
    
    // Create a reusable pipeline state
    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.label = @"videoPipeline";
    [pipelineStateDescriptor setSampleCount: 1];
    [pipelineStateDescriptor setVertexFunction:vertexProgram];
    [pipelineStateDescriptor setFragmentFunction:fragmentProgram];
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    pipelineStateDescriptor.depthAttachmentPixelFormat = MTLPixelFormatInvalid;
    
    NSError* error = NULL;
    m_MetalPipelineState = [m_MetalDevice newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
    if(!m_MetalPipelineState)
    {
        NSLog(@"Failed to created pipeline state, error %@", error);
    }
    
    MTLDepthStencilDescriptor *depthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
    depthStateDesc.depthCompareFunction = MTLCompareFunctionAlways;
    depthStateDesc.depthWriteEnabled = NO;
    m_MetalDepthState = [m_MetalDevice newDepthStencilStateWithDescriptor:depthStateDesc];
}

- (void)displayLinkCallback
{
    @autoreleasepool
    {
        if(m_LayerSizeDidUpdate)
        {
            CGSize drawableSize = self.view.bounds.size;
            drawableSize.width *= self.view.contentScaleFactor;
            drawableSize.height *= self.view.contentScaleFactor;
            m_MetalLayer.drawableSize = drawableSize;
            
            m_LayerSizeDidUpdate = NO;
        }
        
        // draw
        [self render];
        
        m_CurrentDrawable = nil;
    }
}

- (void)setupRenderPassDescriptorForTexture:(id <MTLTexture>) texture
{
    if (m_MetalRenderPassDescriptor == nil)
        m_MetalRenderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    
    m_MetalRenderPassDescriptor.colorAttachments[0].texture = texture;
    m_MetalRenderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    m_MetalRenderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.23f, 0.23f, 0.23f, 1.0f);
    m_MetalRenderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
}


- (void)render
{
    dispatch_semaphore_wait(m_InflightSemaphore, DISPATCH_TIME_FOREVER);
    
    // Create a new command buffer for each renderpass to the current drawable
    id <MTLCommandBuffer> commandBuffer = [m_MetalCommandQueue commandBuffer];
    commandBuffer.label = @"videoCommand";
    
    // obtain a drawable texture for this render pass and set up the renderpass descriptor for the command encoder to render into
    id <CAMetalDrawable> drawable = [self currentDrawable];
    [self setupRenderPassDescriptorForTexture:drawable.texture];
    
    // Create a render command encoder so we can render into something
    id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:m_MetalRenderPassDescriptor];
    renderEncoder.label = @"videoRenderEncoder";
    [renderEncoder setDepthStencilState:m_MetalDepthState];
    
    // Set context state
	if(m_MetalTextureY != nil && m_MetalTextureCbCr != nil)
	{
		[renderEncoder pushDebugGroup:@"drawVideoQuad"];
		[renderEncoder setRenderPipelineState:m_MetalPipelineState];
		[renderEncoder setVertexBuffer:m_MetalVertexBuffer offset:0 atIndex:0];
		[renderEncoder setFragmentTexture:m_MetalTextureY atIndex:0];
		[renderEncoder setFragmentTexture:m_MetalTextureCbCr atIndex:1];
		[renderEncoder setFragmentBuffer:m_MetalColorConversionBuffer offset:0 atIndex:0];
		
		// Tell the render context we want to draw our primitives
		[renderEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4 instanceCount:1];
		[renderEncoder popDebugGroup];
	}
	
    // We're done encoding commands
    [renderEncoder endEncoding];
    
    // Call the view's completion handler which is required by the view since it will signal its semaphore and set up the next buffer
    __block dispatch_semaphore_t block_sema = m_InflightSemaphore;
    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer)
    {
        dispatch_semaphore_signal(block_sema);
    }];
	
    // Schedule a present once the framebuffer is complete
    [commandBuffer presentDrawable:drawable];
    
    // Finalize rendering here & push the command buffer to the GPU
    [commandBuffer commit];
}

- (void)playoutDidRecieveRemoteCameraBuffer:(uquad::intrusive_ptr<uquad::base::PixelSampleBuffer> const &)buffer
{
    uquad::intrusive_ptr<uquad::hal::av::CVImagePixelSampleBuffer> cvImgSampleBuffer = uquad::dynamic_pointer_cast<uquad::hal::av::CVImagePixelSampleBuffer>(buffer);
    if(cvImgSampleBuffer)
    {
        CVPixelBufferRef pixelBuffer = cvImgSampleBuffer->imageBuffer();
        id<MTLTexture> textureY = nil;
        id<MTLTexture> textureCbCr = nil;
        
        // textureY
        {
            size_t width = CVPixelBufferGetWidthOfPlane(pixelBuffer, 0);
            size_t height = CVPixelBufferGetHeightOfPlane(pixelBuffer, 0);
            MTLPixelFormat pixelFormat = MTLPixelFormatR8Unorm;
            
            CVMetalTextureRef texture = NULL;
            CVReturn status = CVMetalTextureCacheCreateTextureFromImage(NULL, m_MetalTextureCache, pixelBuffer, NULL, pixelFormat, width, height, 0, &texture);
            if(status == kCVReturnSuccess)
            {
                textureY = CVMetalTextureGetTexture(texture);
                CFRelease(texture);
            }
        }
        
        // textureCbCr
        {
            size_t width = CVPixelBufferGetWidthOfPlane(pixelBuffer, 1);
            size_t height = CVPixelBufferGetHeightOfPlane(pixelBuffer, 1);
            MTLPixelFormat pixelFormat = MTLPixelFormatRG8Unorm;
            
            CVMetalTextureRef texture = NULL;
            CVReturn status = CVMetalTextureCacheCreateTextureFromImage(NULL, m_MetalTextureCache, pixelBuffer, NULL, pixelFormat, width, height, 1, &texture);
            if(status == kCVReturnSuccess)
            {
                textureCbCr = CVMetalTextureGetTexture(texture);
                CFRelease(texture);
            }
        }
    
        if(textureY != nil && textureCbCr != nil)
        {
            dispatch_async(dispatch_get_main_queue(), ^
            {
                m_MetalTextureY = textureY;
                m_MetalTextureCbCr = textureCbCr;
            });
        }
    }
}

- (void)cameraClientDidDisconnect
{
    
}

- (void)connectRemoteCamera
{
    UQRClient::instance()->cameraClient()->connect([self](uquad::system::error_code e)
    {
        if(e)
        {
            dispatch_async(dispatch_get_main_queue(),^
            {
                [self navigateBack:nil];
            });
        }
        else
        {
            dispatch_async(dispatch_get_main_queue(),^
            {
                [self.connectActivity stopAnimating];
            });
        }
    });
    
}

- (void)disconnectRemoteCamera
{
    UQRClient::instance()->cameraClient()->disconnect();
}

- (id <CAMetalDrawable>)currentDrawable
{
    while (m_CurrentDrawable == nil)
    {
        m_CurrentDrawable = [m_MetalLayer nextDrawable];
        if (!m_CurrentDrawable)
        {
            NSLog(@"CurrentDrawable is nil");
        }
    }
    
    return m_CurrentDrawable;
}


- (IBAction)navigateBack:(id)sender
{
    if(self.viewControllerDelegate)
    {
        [self disconnectRemoteCamera];
        [self.viewControllerDelegate dismissViewController:self animated:YES completion:nil];
    }
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end

namespace
{
    Callbacks::Callbacks(UQRRemoteViewController *vc)
        : m_VC(vc)
    {
    }
    
    Callbacks::~Callbacks()
    {
    }
    
    
    void Callbacks::onPlayoutVideoResized(uquad::Vec2i const &size)
    {
    }
    
    
    void Callbacks::onPlayoutVideoFrame(uquad::intrusive_ptr<uquad::base::PixelSampleBuffer> const &buffer)
    {
        [m_VC playoutDidRecieveRemoteCameraBuffer:buffer];
    }
    
    void Callbacks::onCameraClientDisconnected()
    {
        [m_VC cameraClientDidDisconnect];
    }
    
}
