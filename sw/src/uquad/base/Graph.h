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
        class InputPort;
        class OutputPort;
        
        
        typedef adjacency_list
        <
            vecS,
            vecS,
            bidirectionalS,
            intrusive_ptr<Node>,
            intrusive_ptr<Connection>
        > container_type;
        
        class Port
            : public RefCounted
        {
            friend class Node;
            
        public:
            Port();
            ~Port();
            
            inline intrusive_ptr<Node> node() const { return intrusive_ptr<Node>(m_Node); }
            
            virtual std::string const& name() const = 0;
            
        protected:
            mutable Node *m_Node;
        };
        
        class InputPort
            : public virtual Port
        {
            friend class Node;
            friend class Graph;
        public:
            InputPort();
            ~InputPort();
            
            virtual bool acceptsPort(intrusive_ptr<OutputPort> op) const = 0;
            
            inline intrusive_ptr<Connection> sourceConnection() const { return m_SourceConnection; }
            
        protected:
            intrusive_ptr<Connection> m_SourceConnection;
        };
        
        class OutputPort
            : public virtual Port
        {
            friend class Node;
            friend class Graph;
        
        public:
            OutputPort();
            ~OutputPort();
            
            inline unordered_set< intrusive_ptr<Connection> > const& destinationConnections() const { return m_DestinationConnections; }
            
        protected:
            unordered_set< intrusive_ptr<Connection> > m_DestinationConnections;
        };
        
        class Node
            : public virtual RefCounted
        {
            friend class Graph;
        public:
            
            typedef mi::multi_index_container
            <
                intrusive_ptr<InputPort>,
                mi::indexed_by
                <
                    mi::random_access<>,
                    mi::hashed_unique< mi::identity< intrusive_ptr<InputPort> > >,
                    mi::hashed_unique< mi::const_mem_fun<Port, std::string const&, &Port::name> >
                >
            > input_ports_type;
            
            typedef mi::nth_index<input_ports_type, 1>::type input_ports_by_identity_type;
            typedef mi::nth_index<input_ports_type, 2>::type input_ports_by_name_type;
            
            
            typedef mi::multi_index_container
            <
                intrusive_ptr<OutputPort>,
                mi::indexed_by
                <
                    mi::random_access<>,
                    mi::hashed_unique< mi::identity< intrusive_ptr<OutputPort> > >,
                    mi::hashed_unique< mi::const_mem_fun<Port, std::string const&, &Port::name> >
                >
            > output_ports_type;
        
            typedef mi::nth_index<output_ports_type, 1>::type output_ports_by_identity_type;
            typedef mi::nth_index<output_ports_type, 2>::type output_ports_by_name_type;
            
        
            typedef container_type::vertex_descriptor handle_type;
            
            Node();
            ~Node();
            
            inline intrusive_ptr<Graph> graph() const { return intrusive_ptr<Graph>(m_Graph); }
            
            inline input_ports_type const& inputPorts() const { return m_InputPorts; }
            inline intrusive_ptr<InputPort> inputPort(std::size_t index) const { return m_InputPorts[index]; }
            intrusive_ptr<InputPort> inputPort(std::string const &n) const;
            bool addInputPort(intrusive_ptr<InputPort> ip);
            
            inline output_ports_type const& outputPorts() const { return m_OutputPorts; }
            inline intrusive_ptr<OutputPort> outputPort(std::size_t index) const { return m_OutputPorts[index]; }
            intrusive_ptr<OutputPort> outputPort(std::string const &n) const;
            bool addOutputPort(intrusive_ptr<OutputPort> op);
            
        protected:
            input_ports_type m_InputPorts;
            output_ports_type m_OutputPorts;
            mutable Graph *m_Graph;
            mutable handle_type m_Handle;
            mutable std::size_t m_Order;
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
            
            inline intrusive_ptr<OutputPort> sourcePort() const { return intrusive_ptr<OutputPort>(m_SourcePort); }
            inline intrusive_ptr<InputPort> destinationPort() const { return intrusive_ptr<InputPort>(m_DestinationPort); }
            
        protected:
            mutable Graph *m_Graph;
            mutable handle_type m_Handle;
            mutable OutputPort *m_SourcePort;
            mutable InputPort *m_DestinationPort;
        };
        
        Graph();
        ~Graph();
        
        typedef function<system::error_code (intrusive_ptr<Node>)> NodeVisitor;
        typedef function<system::error_code (intrusive_ptr<Connection>)> ConnectionVisitor;
        
        
        system::error_code addNode(intrusive_ptr<Node> node);
        system::error_code removeNode(intrusive_ptr<Node> node);
        
        system::error_code connectPorts(intrusive_ptr<OutputPort> src, intrusive_ptr<InputPort> dst);
        system::error_code disconnectPorts(intrusive_ptr<OutputPort> src, intrusive_ptr<InputPort> dst);
        system::error_code connectNodes(intrusive_ptr<Node> src, intrusive_ptr<Node> dst);
        
        
        system::error_code visitAllNodes(NodeVisitor ndov, NodeVisitor nundov);
        system::error_code visit(unordered_set< intrusive_ptr<Node> > const &src, NodeVisitor nv, ConnectionVisitor cv);
        
        virtual bool isMutable() const { return true; }
        
    private:
        void updateNodeHandles();
        void updateVisitOrder();
        
    protected:
        container_type m_Container;
        std::vector<Node::handle_type> m_NodesVisitOrder;
    };

} //namespace base
} //namespace uquad

#endif //UQUAD_BASE_GRAPH_H
