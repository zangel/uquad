#include "UQuad.h"
#include "../base/ChronoIO.h"
#include "../utility/EigenMatrixSerialization.h"

namespace uquad
{
namespace hal
{
    template<class Archive>
    void UQuadSensorsData::serialize(Archive &ar, const unsigned int)
    {
        ar & time;
        ar & velocityRate;
        ar & rotationRate;
        ar & magneticField;
        ar & baroTemperature;
        ar & baroPressure;
    }
    
    UQuad::UQuad()
    {

    }

    UQuad::~UQuad()
    {

    }

    Device::Type UQuad::type() const
    {
        return typeid(UQuad);
    }


} //namespace hal
} //namespace uquad


UQUAD_BASE_SERIALIZATION_EXPORT_IMPLEMENT(uquad::hal::UQuadSensorsData)