#ifndef UQUAD_CONTROL_SYSTEM_H
#define UQUAD_CONTROL_SYSTEM_H

#include "Config.h"
#include "../base/Service.h"

namespace uquad
{
namespace control
{
    class Block;
    class InputSignal;
    class OutputSignal;
    
    class System
    	: public base::Graph
        , public base::Service<System>
    {
    public:
        typedef function<void (system::error_code)> handler_t;
        
        System(intrusive_ptr<base::Runloop> const &rl);
    	~System();
        
        bool isMutable() const;
        
        
        system::error_code addBlock(intrusive_ptr<Block> const &block);
        system::error_code removeBlock(intrusive_ptr<Block> const &block);
        
        system::error_code connectSignals(intrusive_ptr<OutputSignal> const &os, intrusive_ptr<InputSignal> const &is);
        system::error_code disconnectSignals(intrusive_ptr<OutputSignal> const &os, intrusive_ptr<InputSignal> const &is);
        
        system::error_code connectBlocks(intrusive_ptr<Block> src, intrusive_ptr<Block> dst);
        
        inline intrusive_ptr<base::Runloop> runloop() const { return m_Runloop; }
        
        bool isUpdating() const;
        void update();
        void update(handler_t handler);
    
    protected:
        void requestBlockUpdate(intrusive_ptr<Block> const &block);

    //state machine related
    protected:
        typedef base::Service<System> BaseService;
        template <typename> friend class base::Service;
        
        friend class Block;
        
        struct FUpdating {};
        
        struct SUpdating : msm::front::state<>
        {
            typedef mpl::vector<FUpdating> flag_list;
        };
        
        struct EUpdate
        {
            EUpdate(handler_t h, asio::yield_context yc) : handler(h), yctx(yc) {}
            
            mutable handler_t handler;
            mutable asio::yield_context yctx;
        };
        
        struct Actions
        {
            template <class FSM>
            void operator()(EUpdate const &e, FSM &fsm, SStarted &, SUpdating &)
            {
                system::error_code updateErr = fsm.derived().doUpdate(e.yctx);
                if(e.handler) e.handler(updateErr);
            }
        };
        
        
        typedef mpl::vector
        <
            msm::front::Row< SStarted       ,EUpdate            ,SUpdating      ,Actions            ,msm::front::none       >,
            msm::front::Row< SUpdating      ,msm::front::none   ,SStarted       ,msm::front::none   ,msm::front::none       >
        > StateTranitionsTable;
        
        struct StateMachineDef : msm::front::state_machine_def<StateMachineDef>
        {
            StateMachineDef(System &s)
                : system(s)
            {
            }
            
            inline System& derived() { return system; }
            
            typedef BaseService::InitialState initial_state;
            typedef mpl::insert_range
            <
                BaseService::StateTranitionsTable,
                mpl::end<BaseService::StateTranitionsTable>::type,
                StateTranitionsTable
            >::type transition_table;
            
            System &system;
        };
        
        typedef msm::back::state_machine<StateMachineDef>  StateMachine;
        
        inline StateMachine& sm() { return m_SM; }
        inline asio::io_service::strand& iosStrand() { return m_IOSStrand; }
        
        //actions and guards
    protected:
        system::error_code doStart(asio::yield_context yctx);
        system::error_code doStop(asio::yield_context yctx);
        system::error_code doUpdate(asio::yield_context yctx);
        
    private:
        StateMachine m_SM;
        intrusive_ptr<base::Runloop> m_Runloop;
        asio::io_service::strand m_IOSStrand;
        unordered_set< intrusive_ptr<Block> > m_BlockUpdateRequests;
    };

} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_BLOCK_H
