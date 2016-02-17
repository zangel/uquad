#include "Camera.h"
#include "../DeviceManager.h"
#include "../../base/Error.h"
#include "CVImagePixelSampleBuffer.h"

@interface uquad_hal_av_Camera : NSObject < AVCaptureVideoDataOutputSampleBufferDelegate >
{
@public
    
    
@private
    uquad::hal::av::Camera *m_Camera;
    AVCaptureDevice *m_Device;
    AVCaptureDeviceInput *m_DeviceInput;
    AVCaptureVideoDataOutput *m_PreviewDataOutput;
    AVCaptureSession *m_Session;
    AVCaptureConnection *m_PreviewConnection;
    uquad::atomic<bool> m_bRecording;
    std::vector<AVCaptureDeviceFormat*> m_SupportedFormats;
    
    uquad::base::eImageFormat m_PictureFormat;
    uquad::Vec2i m_PictureSize;
}

-(uquad_hal_av_Camera*) initWithCamera:(uquad::hal::av::Camera*) camera device:(AVCaptureDevice*)device;

-(uquad::system::error_code) open;
-(bool) isOpen;
-(uquad::system::error_code) close;

-(uquad::system::error_code) beginConfiguration;
-(uquad::system::error_code) endConfiguration;

-(void)fillSupportedPreviewFormats:(uquad::unordered_set<uquad::base::ePixelFormat>&) formats;
-(void)fillSupportedPreviewSizes:(uquad::unordered_set<uquad::Vec2i>&) sizes;
-(void)fillSupportedPreviewFPS:(uquad::unordered_set<float>&) fps;
-(uquad::base::ePixelFormat) previewFormat;
-(uquad::system::error_code) setPreviewFormat:(uquad::base::ePixelFormat) fmt;
-(uquad::Vec2i) previewSize;
-(uquad::system::error_code) setPreviewSize:(uquad::Vec2i const &) sz;
-(float) previewFPS;
-(uquad::system::error_code) setPreviewFPS:(float) fps;

-(void)fillSupportedPictureFormats:(uquad::unordered_set<uquad::base::eImageFormat>&) formats;
-(void)fillSupportedPictureSizes:(uquad::unordered_set<uquad::Vec2i>&) sizes;
-(uquad::base::eImageFormat) pictureFormat;
-(uquad::system::error_code) setPictureFormat:(uquad::base::eImageFormat) fmt;
-(uquad::Vec2i) pictureSize;
-(uquad::system::error_code) setPictureSize:(uquad::Vec2i const &) sz;

-(uquad::system::error_code) startPreview;
-(bool) isPreviewStarted;
-(uquad::system::error_code) stopPreview;

-(uquad::system::error_code) startRecording;
-(bool) isRecordingStarted;
-(uquad::system::error_code) stopRecording;

-(void) captureOutput : (AVCaptureOutput*) captureOutput didOutputSampleBuffer : (CMSampleBufferRef) sampleBuffer fromConnection : (AVCaptureConnection*) connection;

@end



namespace uquad
{
namespace hal
{
namespace av
{
    namespace
    {
        std::string stdStringFromNSString(NSString *str)
        {
            NSData* asData = [str dataUsingEncoding : NSUTF8StringEncoding];
            return std::string(
                reinterpret_cast<std::string::value_type const *>([asData bytes]),
                [asData length] / sizeof(std::string::value_type)
            );
        }
    }
    
    Camera::Camera(AVCaptureDevice *dev)
        : hal::Camera()
        , m_Camera(0)
        , m_bConfigurationBegun(false)
    
    {
        m_Camera = [[uquad_hal_av_Camera alloc] initWithCamera:this device:dev];
        m_Name = stdStringFromNSString(dev.localizedName);
        resetParameters();
    }
    
    Camera::~Camera()
    {
        [m_Camera release];
    }
    
    std::string const& Camera::name() const
    {
        return m_Name;
    }

    bool Camera::isOpen() const
    {
        return [m_Camera isOpen];
    }
    
    system::error_code Camera::open()
    {
        if([m_Camera isOpen])
			return base::makeErrorCode(base::kEAlreadyOpened);
        
        if(system::error_code oe = [m_Camera open])
            return oe;
        
        [m_Camera fillSupportedPreviewFormats:m_SupportedPreviewFormats];
        [m_Camera fillSupportedPreviewSizes:m_SupportedPreviewSizes];
        [m_Camera fillSupportedPreviewFPS:m_SupportedPreviewFPS];
        m_PreviewFormat = [m_Camera previewFormat];
        m_PreviewSize = [m_Camera previewSize];
        m_PreviewFPS = [m_Camera previewFPS];
        

        
        [m_Camera fillSupportedPictureFormats:m_SupportedPictureFormats];
        [m_Camera fillSupportedPictureSizes:m_SupportedPictureSizes];
        m_PictureFormat = [m_Camera pictureFormat];
        m_PictureSize = [m_Camera pictureSize];
        
        notifyOnDeviceOpened();
        
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code Camera::close()
    {
        if(![m_Camera isOpen])
			return base::makeErrorCode(base::kENotOpened);
        
        if(system::error_code oe = [m_Camera close])
            return oe;
        resetParameters();
        notifyOnDeviceClosed();
        
        return base::makeErrorCode(base::kENoError);
    }
    
    void Camera::resetParameters()
    {
        m_SupportedPreviewFormats.clear();
        m_SupportedPreviewSizes.clear();
        m_SupportedPreviewFPS.clear();
        m_PreviewFormat = base::kPixelFormatInvalid;
        m_PreviewSize = Vec2i(0,0);
        m_PreviewFPS = 0.0f;
        m_SupportedPictureFormats.clear();
        m_SupportedPictureSizes.clear();
        m_PictureFormat = base::kImageFormatInvalid;
        m_PictureSize = Vec2i(0,0);
    }
    
    system::error_code Camera::beginConfiguration()
    {
        if(!isOpen())
			return base::makeErrorCode(base::kENotOpened);

		if(m_bConfigurationBegun)
			return base::makeErrorCode(base::kEAlreadyStarted);

        if(system::error_code bce = [m_Camera beginConfiguration])
            return bce;
        
        m_bConfigurationBegun = true;
		return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code Camera::endConfiguration()
    {
        if(!isOpen())
			return base::makeErrorCode(base::kENotOpened);

		if(!m_bConfigurationBegun)
			return base::makeErrorCode(base::kENotStarted);
        
        if(system::error_code bce = [m_Camera endConfiguration])
            return bce;

		m_bConfigurationBegun = false;
        notifyOnCameraParametersChanged();
		return base::makeErrorCode(base::kENoError);
    }
    
    unordered_set<base::ePixelFormat> const& Camera::getSupportedPreviewFormats() const
    {
        return m_SupportedPreviewFormats;
    }
    
    unordered_set<Vec2i> const& Camera::getSupportedPreviewSizes() const
    {
        return m_SupportedPreviewSizes;
    }
    
    unordered_set<float> const& Camera::getSupportedPreviewFPS() const
    {
        return m_SupportedPreviewFPS;
    }
    
    base::ePixelFormat Camera::getPreviewFormat() const
    {
        return m_PreviewFormat;
    }
    
    system::error_code Camera::setPreviewFormat(base::ePixelFormat fmt)
    {
        if(!m_bConfigurationBegun)
			return base::makeErrorCode(base::kENotApplicable);

		if(isPreviewStarted())
			return base::makeErrorCode(base::kEAlreadyStarted);

        if(fmt == m_PreviewFormat)
        	return base::makeErrorCode(base::kENoError);

		if(!m_SupportedPreviewFormats.count(fmt))
			return base::makeErrorCode(base::kEInvalidArgument);

        if(system::error_code e = [m_Camera setPreviewFormat:fmt])
            return e;
        
		m_PreviewFormat = fmt;
        return base::makeErrorCode(base::kENoError);
    }
    
    Vec2i Camera::getPreviewSize() const
    {
        return m_PreviewSize;
    }
    
    system::error_code Camera::setPreviewSize(Vec2i const &sz)
    {
        if(!m_bConfigurationBegun)
			return base::makeErrorCode(base::kENotApplicable);

		if(isPreviewStarted())
			return base::makeErrorCode(base::kEAlreadyStarted);

        if(sz == m_PreviewSize)
        	return base::makeErrorCode(base::kENoError);

		if(!m_SupportedPreviewSizes.count(sz))
			return base::makeErrorCode(base::kEInvalidArgument);
        
        if(system::error_code e = [m_Camera setPreviewSize:sz])
            return e;
        
        [m_Camera fillSupportedPreviewFPS:m_SupportedPreviewFPS];
        
		m_PreviewSize = sz;
        return base::makeErrorCode(base::kENoError);
    }
    
    float Camera::getPreviewFPS() const
    {
        return m_PreviewFPS;
    }
    
    system::error_code Camera::setPreviewFPS(float fps)
    {
        if(!m_bConfigurationBegun)
			return base::makeErrorCode(base::kENotApplicable);

		if(isPreviewStarted())
			return base::makeErrorCode(base::kEAlreadyStarted);

        if(fps == m_PreviewFPS)
        	return base::makeErrorCode(base::kENoError);

		if(!m_SupportedPreviewFPS.count(fps))
			return base::makeErrorCode(base::kEInvalidArgument);
        
        if(system::error_code e = [m_Camera setPreviewFPS:fps])
            return e;

		m_PreviewFPS = fps;
        return base::makeErrorCode(base::kENoError);
    }
    
    unordered_set<base::ePixelFormat> const& Camera::getSupportedVideoFormats() const
    {
        return m_SupportedVideoFormats;
    }
    
    unordered_set<Vec2i> const& Camera::getSupportedVideoSizes() const
    {
        return m_SupportedVideoSizes;
    }
    
    base::ePixelFormat Camera::getVideoFormat() const
    {
        return m_PreviewFormat;
    }
    
    system::error_code Camera::setVideoFormat(base::ePixelFormat fmt)
    {
        return base::makeErrorCode(base::kENotApplicable);
    }
    
    Vec2i Camera::getVideoSize() const
    {
        return m_PreviewSize;
    }
    
    system::error_code Camera::setVideoSize(Vec2i const &sz)
    {
        return base::makeErrorCode(base::kENotApplicable);
    }
    
    unordered_set<base::eImageFormat> const& Camera::getSupportedPictureFormats() const
    {
        return m_SupportedPictureFormats;
    }
    
    unordered_set<Vec2i> const& Camera::getSupportedPictureSizes() const
    {
        return m_SupportedPictureSizes;
    }
    
	base::eImageFormat Camera::getPictureFormat() const
    {
        return m_PictureFormat;
    }
    
	system::error_code Camera::setPictureFormat(base::eImageFormat fmt)
    {
        if(!m_bConfigurationBegun)
			return base::makeErrorCode(base::kENotApplicable);

		if(fmt == m_PictureFormat)
			return base::makeErrorCode(base::kENoError);

		if(!m_SupportedPictureFormats.count(fmt))
			return base::makeErrorCode(base::kEInvalidArgument);
        
        if(system::error_code e = [m_Camera setPictureFormat:fmt])
            return e;

		m_PictureFormat = fmt;
        return base::makeErrorCode(base::kENoError);
    }
    
	Vec2i Camera::getPictureSize() const
    {
        return m_PictureSize;
    }
    
	system::error_code Camera::setPictureSize(Vec2i const &sz)
    {
        if(!m_bConfigurationBegun)
			return base::makeErrorCode(base::kENotApplicable);

		if(sz == m_PictureSize)
			return base::makeErrorCode(base::kENoError);

		if(!m_SupportedPictureSizes.count(sz))
			return base::makeErrorCode(base::kEInvalidArgument);
        
        if(system::error_code e = [m_Camera setPictureSize:sz])
            return e;
		
        m_PictureSize = sz;
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code Camera::startPreview()
    {
        if(!isOpen())
			return base::makeErrorCode(base::kENotOpened);

		if(isPreviewStarted())
			return base::makeErrorCode(base::kEAlreadyStarted);
        
        if(system::error_code e = [m_Camera startPreview])
            return e;
        
        return base::makeErrorCode(base::kENoError);
    }
    
    bool Camera::isPreviewStarted() const
    {
        if(!isOpen())
            return false;
        
        return [m_Camera isPreviewStarted];
    }
    
	system::error_code Camera::stopPreview()
    {
        if(!isOpen())
			return base::makeErrorCode(base::kENotOpened);

        if(!isPreviewStarted())
			return base::makeErrorCode(base::kENotStarted);
        
        if(system::error_code e = [m_Camera stopPreview])
            return e;
        
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code Camera::startRecording()
    {
        if(!isOpen())
			return base::makeErrorCode(base::kENotOpened);

        if(!isPreviewStarted())
            return base::makeErrorCode(base::kENotStarted);
        
		if(isRecordingStarted())
			return base::makeErrorCode(base::kEAlreadyStarted);
        
        if(system::error_code e = [m_Camera startRecording])
            return e;
        
        return base::makeErrorCode(base::kENoError);
    }
    
    bool Camera::isRecordingStarted() const
    {
        if(!isOpen())
            return false;
        
        return [m_Camera isRecordingStarted];
    }
    
	system::error_code Camera::stopRecording()
    {
        if(!isOpen())
			return base::makeErrorCode(base::kENotOpened);

		if(!isRecordingStarted())
			return base::makeErrorCode(base::kENotStarted);

        if(system::error_code e = [m_Camera stopRecording])
            return e;
        
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code Camera::registerCameras(DeviceManager &dm)
    {
        NSArray *captureDevices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
        
        if(captureDevices == nil || ![captureDevices count])
            return base::makeErrorCode(base::kENoSuchDevice);
        
        for(AVCaptureDevice *device in captureDevices)
        {
            dm.registerDevice(
                intrusive_ptr<hal::Camera>(new Camera(device))
            );
        }
        return base::makeErrorCode(base::kENoError);
    }
    
} //namespace av
} //namespace hal
} //namespace uquad

using namespace uquad;

@implementation uquad_hal_av_Camera
-(uquad_hal_av_Camera*) initWithCamera:(hal::av::Camera*) camera device:(AVCaptureDevice*)device
{
    self=[super init];
    m_Camera = camera;
    m_Device = device;
    m_DeviceInput = nil;
    m_PreviewDataOutput = nil;
    m_Session = nil;
    m_bRecording = false;
    m_PreviewConnection = nil;
    return self;
}

- (void) dealloc
{
    [super dealloc];
}

-(system::error_code) open
{
    #if defined(UQUAD_PLATFORM_Mac)
    if(m_Device.isInUseByAnotherApplication)
        return base::makeErrorCode(base::kEDeviceBusy);
    #endif
    
    for(AVCaptureDeviceFormat *fmt in m_Device.formats)
    {
        CMVideoDimensions formatDims = CMVideoFormatDescriptionGetDimensions(fmt.formatDescription);
        if(formatDims.width > 640 || formatDims.height > 480)
            break;
        
        bool supports30fps = false;
        for(AVFrameRateRange *fr in fmt.videoSupportedFrameRateRanges)
        {
            if(fr.maxFrameRate >= 30)
            {
                supports30fps = true;
                break;
            }
        }
        
        if(supports30fps)
        {
            m_SupportedFormats.push_back(fmt);
        }
    }
    
    if(m_SupportedFormats.empty())
    {
        return base::makeErrorCode(base::kEInvalidContent);
    }
    
    NSError *err;
    m_DeviceInput = [[AVCaptureDeviceInput
        deviceInputWithDevice : m_Device
        error : &err
    ] retain ];
    
    if(err)
    {
        return base::makeErrorCode(base::kEDeviceBusy);
    }
    
    m_PreviewDataOutput = [[AVCaptureVideoDataOutput alloc] init];
    m_PreviewDataOutput.alwaysDiscardsLateVideoFrames = YES;
    
    dispatch_queue_t queue = dispatch_queue_create("uquad_hal_av_camera_queue", NULL);
    [m_PreviewDataOutput setSampleBufferDelegate : self queue : queue];
    dispatch_release(queue);
    
    m_Session = [[AVCaptureSession alloc] init];
    [m_Session addInput : m_DeviceInput];
    [m_Session addOutput : m_PreviewDataOutput];
    m_PreviewConnection = [[m_PreviewDataOutput connectionWithMediaType : AVMediaTypeVideo] retain];
    
    [self beginConfiguration];
    
    [self setPreviewFormat:base::kPixelFormatYUV420sp];
    
    CMVideoDimensions formatDims = CMVideoFormatDescriptionGetDimensions(m_SupportedFormats.back().formatDescription);
    [self setPreviewSize:Vec2i(formatDims.width, formatDims.height)];
    
    m_PictureFormat = base::kImageFormatJPEG;
    m_PictureSize = Vec2i(formatDims.width, formatDims.height);
    
    /*if([m_Device isExposureModeSupported:AVCaptureExposureModeContinuousAutoExposure])
    {
        m_Device.exposureMode = AVCaptureExposureModeContinuousAutoExposure;
    }*/
    
    if([m_Device isFocusModeSupported:AVCaptureFocusModeLocked])
    {
        [m_Device setFocusMode:AVCaptureFocusModeLocked];
    }
    
    [self endConfiguration];
    
    return base::makeErrorCode(base::kENoError);
}

-(bool) isOpen
{
    return m_Session != nil;
}

-(system::error_code) close
{
    m_PictureFormat = base::kImageFormatInvalid;
    m_PictureSize = Vec2i(0,0);
    m_SupportedFormats.clear();
    
    [m_PreviewConnection release];
    m_PreviewConnection = nil;
    
    [m_Session removeOutput : m_PreviewDataOutput];
    [m_PreviewDataOutput release];
    m_PreviewDataOutput = nil;
    
    [m_Session removeInput : m_DeviceInput];
    [m_DeviceInput release];
    m_DeviceInput = nil;
    
    [m_Session release];
    m_Session = nil;
    
    return base::makeErrorCode(base::kENoError);
}

-(system::error_code) beginConfiguration
{
    if(![m_Device lockForConfiguration:nil])
        return base::makeErrorCode(base::kENotApplicable);
    
    return base::makeErrorCode(base::kENoError);
}

-(system::error_code) endConfiguration
{
    [m_Device unlockForConfiguration];
    return base::makeErrorCode(base::kENoError);
}

-(void)fillSupportedPreviewFormats:(unordered_set<base::ePixelFormat>&) formats
{
    formats.clear();
    formats.insert(base::kPixelFormatBGRA8);
    formats.insert(base::kPixelFormatYUV420sp);
}

-(void)fillSupportedPreviewSizes:(uquad::unordered_set<uquad::Vec2i>&) sizes
{
    sizes.clear();
    std::for_each(m_SupportedFormats.begin(), m_SupportedFormats.end(), [&sizes](AVCaptureDeviceFormat *fmt)
    {
        CMFormatDescriptionRef formatDesc = fmt.formatDescription;
        if(formatDesc != nil)
        {
            CMVideoDimensions formatDims = CMVideoFormatDescriptionGetDimensions(formatDesc);
        
            sizes.insert(Vec2i(formatDims.width, formatDims.height));
        }
    });
}

-(void)fillSupportedPreviewFPS:(uquad::unordered_set<float>&) fps
{
    fps.clear();
    
    
    for(AVFrameRateRange *fr in m_Device.activeFormat.videoSupportedFrameRateRanges)
    {
        fps.insert(fr.maxFrameRate);
    }
}


-(base::ePixelFormat) previewFormat
{
    if(!m_PreviewDataOutput.videoSettings)
        return base::kPixelFormatInvalid;
    
    NSNumber *formatValue = [m_PreviewDataOutput.videoSettings valueForKey:(id)kCVPixelBufferPixelFormatTypeKey];
    if(!formatValue)
        return base::kPixelFormatInvalid;
    
    return hal::av::CVImagePixelSampleBuffer::uquadFromOSXPixelFormat([formatValue intValue]);
}

-(system::error_code) setPreviewFormat:(uquad::base::ePixelFormat) fmt
{
    int osxFormat = hal::av::CVImagePixelSampleBuffer::osxFromUQuadPixelFormat(fmt);
    if(osxFormat < 0)
        return base::makeErrorCode(base::kEInvalidArgument);
    
    [m_PreviewDataOutput setVideoSettings:
        [NSDictionary
            dictionaryWithObject:[NSNumber numberWithInt:osxFormat]
                forKey:(id)kCVPixelBufferPixelFormatTypeKey
        ]
    ];
    
    return base::makeErrorCode(base::kENoError);
}

-(Vec2i) previewSize
{
    CMFormatDescriptionRef formatDesc = m_Device.activeFormat.formatDescription;
    if(formatDesc != nil)
    {
        CMVideoDimensions formatDims = CMVideoFormatDescriptionGetDimensions(formatDesc);
        return Vec2i(formatDims.width, formatDims.height);
    }
    return Vec2i(0,0);
}

-(system::error_code) setPreviewSize:(uquad::Vec2i const &) sz
{
    std::size_t f;
    for(f = 0; f < m_SupportedFormats.size();++f)
    {
        CMFormatDescriptionRef formatDesc = m_SupportedFormats[f].formatDescription;
        if(!formatDesc)
            continue;
        
        CMVideoDimensions formatDims = CMVideoFormatDescriptionGetDimensions(formatDesc);
        if(formatDims.width == sz[0] && formatDims.height == sz[1])
            break;
    }
    
    if(f >= m_SupportedFormats.size())
        return base::makeErrorCode(base::kEInvalidArgument);
    
    [m_Device setActiveFormat:m_SupportedFormats[f]];
    return base::makeErrorCode(base::kENoError);
}

-(float) previewFPS
{
    return static_cast<float>(1.0/CMTimeGetSeconds(m_Device.activeVideoMaxFrameDuration));
}

-(system::error_code) setPreviewFPS:(float) fps
{
    #if 1
    for(AVFrameRateRange *fr in m_Device.activeFormat.videoSupportedFrameRateRanges)
    {
        if(std::fabs(fr.maxFrameRate - fps) < 1.0e-1)
        {
            #if 1
            CMTime minFrameDuration = fr.minFrameDuration;
            //CMTime maxFrameDuration = fr.maxFrameDuration;
            //CMTime maxFrameDuration = fr.maxFrameDuration;
            #else
            CMTime frameDuration = CMTimeMake(1000000, 1000000*fps);
            //CMTime maxFrameDuration = CMTimeMake(1000000, 1000000*fps);
            #endif
            
            [m_Device setActiveVideoMinFrameDuration:minFrameDuration];
            [m_Device setActiveVideoMaxFrameDuration:minFrameDuration];
            
            if(m_PreviewConnection.supportsVideoMinFrameDuration)
            {
                m_PreviewConnection.videoMinFrameDuration = minFrameDuration;
            }
            if(m_PreviewConnection.supportsVideoMaxFrameDuration)
            {
                m_PreviewConnection.videoMaxFrameDuration = minFrameDuration;
            }
            break;
        }
        
    }
    #else
    //[m_Device setActiveVideoMaxFrameDuration:CMTimeMakeWithSeconds(1.0f/fps, 1000)];
    //[m_Device setActiveVideoMinFrameDuration:CMTimeMakeWithSeconds(1.0f/fps, 1000)];
    
    #endif
    return base::makeErrorCode(base::kENoError);
}

-(void)fillSupportedVideoFormats:(unordered_set<base::ePixelFormat>&) formats
{
    formats.clear();
    formats.insert(base::kPixelFormatBGRA8);
    formats.insert(base::kPixelFormatYUV420sp);
}

-(void)fillSupportedPictureFormats:(uquad::unordered_set<uquad::base::eImageFormat>&) formats
{
    formats.clear();
    formats.insert(base::kImageFormatJPEG);
    formats.insert(base::kImageFormatRAW);
}

-(void)fillSupportedPictureSizes:(uquad::unordered_set<uquad::Vec2i>&) sizes
{
    sizes.clear();
    std::for_each(m_SupportedFormats.begin(), m_SupportedFormats.end(), [&sizes](AVCaptureDeviceFormat *fmt)
    {
        CMFormatDescriptionRef formatDesc = fmt.formatDescription;
        if(formatDesc != nil)
        {
            CMVideoDimensions formatDims = CMVideoFormatDescriptionGetDimensions(formatDesc);
        
            sizes.insert(Vec2i(formatDims.width, formatDims.height));
        }
    });
}

-(base::eImageFormat) pictureFormat
{
    return m_PictureFormat;
}

-(system::error_code) setPictureFormat:(base::eImageFormat) fmt
{
    m_PictureFormat = fmt;
    return base::makeErrorCode(base::kENoError);
}

-(Vec2i) pictureSize
{
    return m_PictureSize;
}

-(system::error_code) setPictureSize:(Vec2i const &) sz
{
    m_PictureSize = sz;
    return base::makeErrorCode(base::kENoError);
}

-(system::error_code) startPreview
{
    [m_Device lockForConfiguration:nil];
    [m_Session startRunning];
    return base::makeErrorCode(base::kENoError);
}

-(bool) isPreviewStarted
{
    return [m_Session isRunning];
}

-(system::error_code) stopPreview
{
    [self stopRecording];
    
    [m_Session stopRunning];
    while([m_Session isRunning])
        boost::this_thread::yield();
    [m_Device unlockForConfiguration];
    
    return base::makeErrorCode(base::kENoError);
}

-(system::error_code) startRecording
{
    m_bRecording = true;
    return base::makeErrorCode(base::kENoError);
}

-(bool) isRecordingStarted
{
    return m_bRecording;
}

-(system::error_code) stopRecording
{
    m_bRecording = false;
    return base::makeErrorCode(base::kENoError);
}

-(void) captureOutput : (AVCaptureOutput*) captureOutput didOutputSampleBuffer : (CMSampleBufferRef) sampleBuffer fromConnection : (AVCaptureConnection*) connection
{
    if(![m_Session isRunning])
        return;
    
    CVImageBufferRef pixelPuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    intrusive_ptr<base::PixelSampleBuffer> buffer(new hal::av::CVImagePixelSampleBuffer(pixelPuffer));
    
    m_Camera->notifyOnCameraPreviewFrame(buffer);
    
    if(m_bRecording)
    {
        m_Camera->notifyOnCameraVideoFrame(buffer);
    }
}




@end
