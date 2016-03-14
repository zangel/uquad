#ifndef UQUAD_BASE_GRAPH_H
#define UQUAD_BASE_GRAPH_H

#include "Config.h"
#include "RefCounted.h"

namespace uquad
{
namespace base
{
    class Graph
        : public RefCounted
    {
    public:
        class Node;
        class Connection;
        class Port;
        
        typedef adjacency_list
        <
            listS,
            listS,
            bidirectionalS,
            intrusive_ptr<Node>,
            intrusive_ptr<Connection>
        > container_type;
        
        class Node
            : public RefCounted
        {
            friend class Graph;
        public:
            typedef container_type::vertex_descriptor handle_type;
            
            Node();
            ~Node();
            
            inline intrusive_ptr<Graph> graph() const { return intrusive_ptr<Graph>(m_Graph); }
            inline uint32_t index() const { return m_Index; }
            
            virtual std::size_t numInputPorts() const = 0;
            virtual intrusive_ptr<Port> inputPort(std::size_t index) const = 0;
            
            virtual std::size_t numOutputPorts() const = 0;
            virtual intrusive_ptr<Port> outputPort(std::size_t index) const = 0;
            
        protected:
            mutable Graph *m_Graph;
            mutable handle_type m_Handle;
            mutable uint32_t m_Index;
        };
        
        
        class Connection
            : public RefCounted
        {
            friend class Graph;
        public:
            typedef container_type::edge_descriptor handle_type;
            
            Connection();
            ~Connection();
            
            inline intrusive_ptr<Graph> graph() const { return intrusive_ptr<Graph>(m_Graph); }
            
            inline intrusive_ptr<Port> sourcePort() const { return intrusive_ptr<Port>(m_SourcePort); }
            inline intrusive_ptr<Port> destinationPort() const { return intrusive_ptr<Port>(m_DestinationPort); }
            
        protected:
            mutable Graph *m_Graph;
            mutable handle_type m_Handle;
            mutable Port *m_SourcePort;
            mutable Port *m_DestinationPort;
        };
        
        class Port
            : public RefCounted
        {
            friend class Node;
        public:
            Port();
            ~Port();
            
            virtual intrusive_ptr<Node> node() const = 0;
            virtual bool acceptsPort(intrusive_ptr<Port> input) const = 0;
        };
        
        typedef function<void (intrusive_ptr<Node>)> NodeVisitPredicate;
        
        Graph();
        ~Graph();
        
        system::error_code addNode(intrusive_ptr<Node> node);
        system::error_code removeNode(intrusive_ptr<Node> node);
        
        system::error_code connectPorts(intrusive_ptr<Port> src, intrusive_ptr<Port> dst);
        system::error_code disconnectPorts(intrusive_ptr<Port> src, intrusive_ptr<Port> dst);
        
        
        system::error_code visitNodes(unordered_set< intrusive_ptr<Node> > const &src, NodeVisitPredicate pred);
    
    protected:
        container_type m_Container;
    };

} //namespace base
} //namespace uquad

#endif //UQUAD_BASE_GRAPH_H
