#ifndef UQUAD_CONTROL_SIGNAL_H
#define UQUAD_CONTROL_SIGNAL_H

#include "Config.h"

namespace uquad
{
namespace control
{
    class Block;
    class Signal;
    class InputSignal;
    class OutputSignal;
    
    class Signal
        : public virtual base::Graph::Port
    {
    public:
        Signal();
        ~Signal();
        
        typedef std::type_index Type;
        
        virtual Type type() const = 0;
    };
    
    class InputSignal
        : public Signal
        , public base::Graph::InputPort
    {
    public:
        InputSignal();
        ~InputSignal();
        
        virtual system::error_code assign(intrusive_ptr<OutputSignal> os) = 0;
    };
    
    class OutputSignal
        : public Signal
        , public base::Graph::OutputPort
    {
    public:
        OutputSignal();
        ~OutputSignal();
    };
    
    template <typename T>
    class TypedInputSignal
        : public InputSignal
    {
    public:
        TypedInputSignal()
            : InputSignal()
        {
        }
        
        ~TypedInputSignal()
        {
        }
        
        Type type() const { return typeid(T); }
        
        virtual void value(T v) = 0;
    };
    
    template <typename T>
    class TypedOutputSignal
        : public OutputSignal
    {
    public:
        TypedOutputSignal()
            : OutputSignal()
        {
        }
        
        ~TypedOutputSignal()
        {
        }
        
        Type type() const { return typeid(T); }
        
        virtual T value() const = 0;
    };
    
    
    
    template <typename T>
    class DefaultInputSignal
        : public TypedInputSignal<T>
    {
    public:
        DefaultInputSignal(std::string const &n, T v = T())
            : TypedInputSignal<T>()
            , m_Name(n)
            , m_Value(v)
        {
        }
        
        ~DefaultInputSignal()
        {
        }
        
        std::string const& name() const
        {
            return m_Name;
        }
        
        bool acceptsPort(intrusive_ptr<base::Graph::OutputPort> op) const
        {
            intrusive_ptr<OutputSignal> os = dynamic_pointer_cast<OutputSignal>(op);
            return os && os->type() == TypedInputSignal<T>::type();
        }
        
        system::error_code assign(intrusive_ptr<OutputSignal> os)
        {
            intrusive_ptr< TypedOutputSignal<T> > tos = dynamic_pointer_cast< TypedOutputSignal<T> >(os);
            if(!tos)
                return base::makeErrorCode(base::kEInvalidArgument);
            
            m_Value = tos->value();
            return base::makeErrorCode(base::kENoError);
        }
        
        void value(T v) { m_Value = v; }
        
    public:
        std::string const &m_Name;
        T m_Value;
    };
    
    template <typename T>
    class DefaultOutputSignal
        : public TypedOutputSignal<T>
    {
    public:
        DefaultOutputSignal(std::string const &n, T v = T())
            : TypedOutputSignal<T>()
            , m_Name(n)
            , m_Value(v)
        {
        }
        
        ~DefaultOutputSignal()
        {
        }
        
        std::string const& name() const
        {
            return m_Name;
        }
        
        T value() const { return m_Value; }
        
    public:
        std::string const &m_Name;
        T m_Value;
    };

    
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_SIGNAL_H
