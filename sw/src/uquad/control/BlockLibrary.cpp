#include "BlockLibrary.h"
#include "blocks/PoseEstimation.h"
#include "../base/Error.h"

namespace uquad
{
namespace control
{

    BlockLibrary::BlockLibrary()
    	: base::ObjectFactory()
	{
	}

	BlockLibrary::~BlockLibrary()
	{

	}
    
    intrusive_ptr<blocks::PoseEstimation> BlockLibrary::createPoseEstimation(std::string const &name) const
    {
        intrusive_ptr<blocks::PoseEstimation::Registry> match;
        enumerateRegistries<blocks::PoseEstimation>([&match, &name](intrusive_ptr<blocks::PoseEstimation::Registry> const &registry) -> bool
        {
            if(registry->name == name)
            {
                match = registry;
                return false;
            }
            return true;
        });
        
        if(match)
            return match->createObject<blocks::PoseEstimation>();
        
        return intrusive_ptr<blocks::PoseEstimation>();
    }
    
} //namespace control
} //namespace uquad
