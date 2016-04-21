#ifndef UQUAD_CONTROL_BLOCK_LIBRARY_H
#define UQUAD_CONTROL_BLOCK_LIBRARY_H

#include "Config.h"
#include "Block.h"

namespace uquad
{
namespace control
{
    class PoseEstimation;
    
    class BlockLibrary
        : public base::ObjectFactory
    {
	public:
        
    protected:
        BlockLibrary();

    public:
        ~BlockLibrary();

        intrusive_ptr<PoseEstimation> createPoseEstimation(std::string const &name) const;
	};

} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_BLOCK_LIBRARY_H
