#include "AlgorithmLibrary.h"
#include "PoseTracker.h"

namespace uquad
{
namespace cv
{

    AlgorithmLibrary::AlgorithmLibrary()
    	: m_Sync()
    	, m_Registries()
	{
        Algorithm::registerAlgorithms<PoseTracker>(*this);
	}

	AlgorithmLibrary::~AlgorithmLibrary()
	{

	}
    
    bool AlgorithmLibrary::registerAlgorithm(intrusive_ptr<Algorithm::Registry> const &r)
    {
        lock_guard<mutex> lock(m_Sync);
		return m_Registries.push_back(r).second;
    }
    
    bool AlgorithmLibrary::deregisterAlgorithm(intrusive_ptr<Algorithm::Registry> const &r)
    {
        lock_guard<mutex> lock(m_Sync);
		registries_by_identity_t &registries_by_identity = m_Registries.get<1>();
		registries_by_identity_t::iterator itr = registries_by_identity.find(r);
		if(itr == registries_by_identity.end())
		{
			return false;
		}
		registries_by_identity.erase(itr);
		return true;
    }
    
    std::pair<
    	AlgorithmLibrary::registries_by_name_t::const_iterator,
    	AlgorithmLibrary::registries_by_name_t::const_iterator
	> AlgorithmLibrary::findAlgorithmsByName(std::string const &n) const
    {
    	registries_by_name_t const &registries_by_name = m_Registries.get<2>();
    	return registries_by_name.equal_range(n);
    }
    

    std::pair<
        AlgorithmLibrary::registries_by_type_t::const_iterator,
        AlgorithmLibrary::registries_by_type_t::const_iterator
    > AlgorithmLibrary::findAlgorithmsByType(Algorithm::Type const &type) const
    {
        registries_by_type_t const &registries_by_type = m_Registries.get<3>();
		return registries_by_type.equal_range(type);
    }
    
    intrusive_ptr<PoseTracker> AlgorithmLibrary::createPoseTracker(std::string const &name) const
    {
        intrusive_ptr<cv::PoseTracker::Registry> match;
        enumerateAlgorithms<PoseTracker>([&match, &name](intrusive_ptr<cv::PoseTracker::Registry> const &registry) -> bool
        {
            if(registry->name == name)
            {
                match = registry;
                return false;
            }
            return true;
        });
        
        if(match)
            return match->createAlgorithm<PoseTracker>();

        return intrusive_ptr<PoseTracker>();
    }
    
} //namespace cv
} //namespace uquad

