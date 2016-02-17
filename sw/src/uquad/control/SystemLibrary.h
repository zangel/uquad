#ifndef UQUAD_CONTROL_SYSTEM_LIBRARY_H
#define UQUAD_CONTROL_SYSTEM_LIBRARY_H

#include "Config.h"
#include "../base/ObjectFactory.h"
#include "../base/RefCounted.h"
#include "System.h"

namespace uquad
{
namespace control
{
    class PoseEstimation;
    
    class SystemLibrary
        : public base::ObjectFactory
    {
	public:
        
    protected:
        SystemLibrary();

    public:
        ~SystemLibrary();

        intrusive_ptr<PoseEstimation> createPoseEstimation(std::string const &name) const;
	};

} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_SYSTEM_LIBRARY_H
