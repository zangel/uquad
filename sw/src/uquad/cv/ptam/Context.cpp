#include "Context.h"
#include <ptam/ATANCamera.h>

namespace uquad
{
namespace cv
{
namespace ptam
{
    Context::Context()
        : ::Context()
    {
        ptamCameraIntrinsicParameters = ::ATANCamera::mvDefaultParams;
        
        ptamKeyFrameCandidateMinShiTomasiScore = 500;
        ptamKeyFrameCornerDetector = ::kCornerDetectorFAST9_nonmax;
        ptamBundleMEstimator = ::kMEstimatorTukey;
        ptamBundleMinTukeySigma = 0.4;
        ptamBundleUpdateConvergenceLimit = 1e-06;
        ptamBundleMaxIterations = 20;
        ptamMapMakerWiggleScale = 0.1;
        ptamMapMakerMaxKFDistWiggleMult = 1.0;
        ptamMapMakerPlaneAlignerRansacs = 100;
        ptamRelocaliserMaxScore = 9e6;
        
        //Tracker
        ptamTrackerRotationEstimatorBlur = 0.75;
        ptamTrackerUseRotationEstimator = 1;
        ptamTrackerMiniPatchMaxSSD = 100000;
        ptamTrackerMaxInitialTrails = 1000;
        ptamTrackerCoarseMin = 20;
        ptamTrackerCoarseMax = 60;
        ptamTrackerCoarseRange = 30;
        ptamTrackerCoarseSubPixIts = 8;
        ptamTrackerDisableCoarse = 0;
        ptamTrackerCoarseMinVelocity = 0.006;
        ptamTrackerMaxPatchesPerFrame = 1000;
        ptamTrackerTrackingQualityGood = 0.3;
        ptamTrackerTrackingQualityLost = 0.13;
        ptamTrackerMEstimator = ::kMEstimatorTukey;
        ptamTrackerInitLevel = 2;
        ptamTrackerAutoInit = false;
        ptamTrackerAutoInitPixel = 100;
        
        ptamCalibratorBlurSigma = 1.0;
        ptamCalibratorMeanGate = 10;
        ptamCalibratorMinCornersForGrabbedImage = 20;
        ptamCalibratorCornerPatchPixelSize = 20;
        ptamCalibratorExpandByStepMaxDistFrac = 0.3;
    }
    
    Context::~Context()
    {
    }
    
} //namespace ptam
} //namespace cv
} //namespace uquad
