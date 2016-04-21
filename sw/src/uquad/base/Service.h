#ifndef UQUAD_BASE_SERVICE_H
#define UQUAD_BASE_SERVICE_H

#include "Config.h"

namespace uquad
{
namespace base
{
    template <typename Derived>
    class Service
    {
    public:
        typedef function<void (system::error_code)> handler_t;
        
    protected:
        struct FStarted {};
        
        struct SIdle : msm::front::state<>
        {
        };
        
        struct SStarted : msm::front::state<>
        {
            typedef mpl::vector<FStarted> flag_list;
        };
        
        struct EStart
        {
            EStart(handler_t h, asio::yield_context yc) : handler(h), yctx(yc) {}
            
            mutable handler_t handler;
            mutable asio::yield_context yctx;
        };
        
        struct EStop
        {
            EStop(handler_t h, asio::yield_context yc) : handler(h), yctx(yc) {}
            
            mutable handler_t handler;
            mutable asio::yield_context yctx;
        };
    
        struct Guards
        {
            template <class FSM>
            bool operator()(EStart const &e, FSM &fsm, SIdle &, SStarted &)
            {
                system::error_code startErr = fsm.derived().doStart(e.yctx);
                if(e.handler) e.handler(startErr);
                return !startErr;
            }
            
            template <class FSM>
            bool operator()(EStop const &e, FSM &fsm, SStarted &, SIdle &)
            {
                system::error_code stopErr = fsm.derived().doStop(e.yctx);
                if(e.handler) e.handler(stopErr);
                return !stopErr;
            }
        };

        typedef SIdle InitialState;
    
        typedef mpl::vector
        <
            msm::front::Row< SIdle        ,EStart           ,SStarted     ,msm::front::none         ,Guards        >,
            msm::front::Row< SStarted     ,EStop            ,SIdle        ,msm::front::none         ,Guards        >
        > StateTranitionsTable;
    
        inline Derived& derived() const { return *static_cast<Derived*>(const_cast<Service*>(this)); }
    
    public:
        Service()
        {
        }
        
        ~Service()
        {
        }
        
        bool isStarted() const
        {
            return derived().sm().template is_flag_active<FStarted>();
        }
        
        
        void start()
        {
            asio::spawn(
                derived().iosStrand(),
                [this](asio::yield_context yctx)
                {
                    derived().sm().process_event(EStart(handler_t(), yctx));
                }
            );
        }
        
        void start(handler_t handler)
        {
            asio::spawn(
                derived().iosStrand(),
                [this, handler](asio::yield_context yctx)
                {
                    derived().sm().process_event(EStart(handler, yctx));
                }
            );
        }
        
        void stop()
        {
            asio::spawn(
                derived().iosStrand(),
                [this](asio::yield_context yctx)
                {
                    derived().sm().process_event(EStop(handler_t(), yctx));
                }
            );
        }
        
        void stop(handler_t handler)
        {
            asio::spawn(
                derived().iosStrand(),
                [this, handler](asio::yield_context yctx)
                {
                    derived().sm().process_event(EStop(handler, yctx));
                }
            );
        }
    };

} //namespace base
} //namespace uquad

#endif //UQUAD_BASE_SERVICE_H
