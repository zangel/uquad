#ifndef __CONTEXT_H
#define __CONTEXT_H

#include <TooN/TooN.h>

#define NUMTRACKERCAMPARAMETERS 5

typedef enum
{
    kMEstimatorTukey,
    kMEstimatorCauchy,
    kMEstimatorHuber
} eMEstimator;

typedef enum
{
    kCornerDetectorFAST9 = 0,
    kCornerDetectorFAST10,
    kCornerDetectorFAST9_nonmax,
    kCornerDetectorAGAST12d,
    kCornerDetectorOAST16,
    kCornerDetectorCount
} eCornerDetector;



class Context
{
public:
    TooN::Vector<NUMTRACKERCAMPARAMETERS> ptamCameraIntrinsicParameters;
    
    //KeyFrame
    double ptamKeyFrameCandidateMinShiTomasiScore;
    eCornerDetector ptamKeyFrameCornerDetector;
    
    
    //Bundle
    eMEstimator ptamBundleMEstimator;
    double ptamBundleMinTukeySigma;
    double ptamBundleUpdateConvergenceLimit;
    int ptamBundleMaxIterations;
    
    //MapMaker
    double ptamMapMakerWiggleScale;
    double ptamMapMakerMaxKFDistWiggleMult;
    int ptamMapMakerPlaneAlignerRansacs;
    
    //Relocaliser
    double ptamRelocaliserMaxScore;
    
    //Tracker
    double ptamTrackerRotationEstimatorBlur;
    int ptamTrackerUseRotationEstimator;
    int ptamTrackerMiniPatchMaxSSD;
    int ptamTrackerMaxInitialTrails;
    unsigned int ptamTrackerCoarseMin;
    unsigned int ptamTrackerCoarseMax;
    unsigned int ptamTrackerCoarseRange;
    int ptamTrackerCoarseSubPixIts;
    int ptamTrackerDisableCoarse;
    double ptamTrackerCoarseMinVelocity;
    int ptamTrackerMaxPatchesPerFrame;
    double ptamTrackerTrackingQualityGood;
    double ptamTrackerTrackingQualityLost;
    eMEstimator ptamTrackerMEstimator;
    int ptamTrackerInitLevel;
    bool ptamTrackerAutoInit;
    int ptamTrackerAutoInitPixel;
    
    //Calibrator
    double ptamCalibratorBlurSigma;
    int ptamCalibratorMeanGate;
    int ptamCalibratorMinCornersForGrabbedImage;
    int ptamCalibratorCornerPatchPixelSize;
    double ptamCalibratorExpandByStepMaxDistFrac;
};

#endif //__CONTEXT_H
