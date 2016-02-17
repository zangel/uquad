#include "PoseTracker.h"
#if defined(UQUAD_CV_HAS_SVO)
#include "svo/PoseTracker.h"
#endif
#include "ptam/PoseTracker.h"
#include "AlgorithmLibrary.h"
#include "../base/Error.h"

namespace uquad
{
namespace cv
{
    template<>
	system::error_code Algorithm::registerAlgorithms<PoseTracker>(AlgorithmLibrary &al)
    {
        al.registerAlgorithm(new ptam::PoseTracker::Registry());
        
#if defined(UQUAD_CV_HAS_SVO)
        al.registerAlgorithm(new svo::PoseTracker::Registry());
#endif
        return base::makeErrorCode(base::kENoError);
    }
    
    PoseTracker::Registry::Registry(std::string const &n)
        : Algorithm::Registry(n, typeid(PoseTracker))
    {
    }
    
    PoseTracker::Registry::~Registry()
    {
    }
        
    PoseTracker::PoseTracker()
        : Algorithm()
    {
    }
    
    PoseTracker::~PoseTracker()
    {
    }

} //namespace cv
} //namespace uquad