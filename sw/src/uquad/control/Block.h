#ifndef UQUAD_CONTROL_BLOCK_H
#define UQUAD_CONTROL_BLOCK_H

#include "Config.h"

namespace uquad
{
namespace control
{
    class System;
    class BlockLibrary;
    class InputSignal;
    class OutputSignal;
    
    template <typename T> class TypedInputSignal;
    template <typename T> class TypedOutputSignal;
        
    
    class Block
    	: public base::Object
        , public base::Graph::Node
    {
    public:
        friend class System;
        
    	Block();
    	~Block();
        
        inline intrusive_ptr<InputSignal> inputSignal(std::size_t index) const { return dynamic_pointer_cast<InputSignal>(inputPort(index)); }
        inline intrusive_ptr<InputSignal> inputSignal(std::string const &n) const { return dynamic_pointer_cast<InputSignal>(inputPort(n)); }
        inline intrusive_ptr<OutputSignal> outputSignal(std::size_t index) const { return dynamic_pointer_cast<OutputSignal>(outputPort(index)); }
        inline intrusive_ptr<OutputSignal> outputSignal(std::string const &n) const { return dynamic_pointer_cast<OutputSignal>(outputPort(n)); }
        
        template <typename T>
        inline intrusive_ptr< TypedInputSignal<T> > typedInputSignal(std::string const &n) const
        {
            return dynamic_pointer_cast< TypedInputSignal<T> >(inputPort(n));
        }
        
        template <typename T>
        inline intrusive_ptr< TypedOutputSignal<T> > typedOutputSignal(std::string const &n) const
        {
            return dynamic_pointer_cast< TypedOutputSignal<T> >(outputPort(n));
        }
        
        intrusive_ptr<System> system() const;
        
        void requestUpdate();
        
    protected:
        virtual system::error_code prepare(asio::yield_context yctx);
        bool isPrepared() const;
        virtual void unprepare(asio::yield_context yctx);
        
        virtual system::error_code update(asio::yield_context yctx);
        
    private:
    };

} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_BLOCK_H
