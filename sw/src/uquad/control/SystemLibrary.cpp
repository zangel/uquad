#include "SystemLibrary.h"
#include "PoseEstimation.h"
#include "../base/Error.h"

namespace uquad
{
namespace control
{

    SystemLibrary::SystemLibrary()
    	: base::ObjectFactory()
	{
	}

	SystemLibrary::~SystemLibrary()
	{

	}
    
    intrusive_ptr<PoseEstimation> SystemLibrary::createPoseEstimation(std::string const &name) const
    {
        intrusive_ptr<PoseEstimation::Registry> match;
        enumerateRegistries<PoseEstimation>([&match, &name](intrusive_ptr<PoseEstimation::Registry> const &registry) -> bool
        {
            if(registry->name == name)
            {
                match = registry;
                return false;
            }
            return true;
        });
        
        if(match)
            return match->createObject<PoseEstimation>();
        
        return intrusive_ptr<PoseEstimation>();
    }
    
} //namespace control
} //namespace uquad

