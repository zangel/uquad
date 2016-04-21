#ifndef UQUAD_CONTROL_FILTERS_H
#define UQUAD_CONTROL_FILTERS_H

#include "Config.h"

namespace uquad
{
namespace control
{
    template <typename T>
    class FilterBase
    {
    public:
        typedef T Value;
        
        FilterBase()
            : m_bInitialized(false)
            , m_CutoffFrequency(1.0f)
            , m_State()
        {
        }
        
        FilterBase(float cutoff, Value st = Value())
            : m_bInitialized(true)
            , m_CutoffFrequency(cutoff)
            , m_State(st)
        {
        }
        
        inline bool isInitialized() const { return m_bInitialized; }
        
        void reset(Value st = Value())
        {
            m_bInitialized = false;
            m_State = st;
        }
        
        Value state() const
        {
            return m_State;
        }
        
        void state(Value st)
        {
            m_State = st;
            m_bInitialized = true;
        }
        
    protected:
        bool m_bInitialized;
        float m_CutoffFrequency;
        Value m_State;
    };
    
    template <typename T>
    class LowPassFilter
        : public FilterBase<T>
    {
    public:
        typedef FilterBase<T> Base;
        typedef typename Base::Value Value;
        
        
        LowPassFilter()
            : Base()
        {
        }
        
        LowPassFilter(float cutoff, Value st = Value())
            : Base(cutoff, st)
        {
        }
        
        void update(Value input, float dt)
        {
            
            if(!Base::isInitialized()) Base::state(input);
            
            float const b = 2.0f * M_PI * Base::m_CutoffFrequency * dt;
            float const a = b / (1.0f + b);
            
            Base::state(a*input + (1.0f - a)*Base::state());
        }
    };
    
    /*
    template <typename T>
    class HighPassFilter
        : public FilterBase<T>
    {
    public:
        typedef FilterBase<T> BaseType;
        typedef typename BaseType::Value Value;
        
        HighPassFilter()
            : BaseType()
        {
        }
        
        HighPassFilter(float cutoff, Value st = Value())
            : BaseType(cutoff, st)
        {
        }
        
        void update(Value input, float dt)
        {
            if(!isInitialized()) m_PrevInput = input;
            
            float const b = 2 * M_PI * m_CutoffFrequency * dt;
            float const a = b / (1 + b);
            
            state(a * (state() + input - m_PrevInput);
            m_PrevInput = input;
        }

    private:
        Value m_PrevInput;
    };
    */
    
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_FILTERS_H
