#include "Graph.h"
#include "Error.h"

namespace
{
    struct GraphVisitor
        : public boost::default_bfs_visitor
    {
    public:
        GraphVisitor(uquad::base::Graph::NodeVisitor nv, uquad::base::Graph::ConnectionVisitor cv)
            : m_NodeVisitor(nv)
            , m_ConnectionVisitor(cv)
        {
        }
        
        template <typename Vertex, typename Graph>
        void examine_vertex(Vertex u, const Graph &g) const
        {
            m_NodeVisitor(g[u]);
        }
        
        template <class Edge, class Graph>
        void examine_edge(Edge e, Graph &g) const
        {
            m_ConnectionVisitor(g[e]);
        }
        
        uquad::base::Graph::NodeVisitor m_NodeVisitor;
        uquad::base::Graph::ConnectionVisitor m_ConnectionVisitor;
    
    };
}

namespace uquad
{
namespace base
{
    Graph::Port::Port()
        : RefCounted()
        , m_Node(0)
    {
    }
    
    Graph::Port::~Port()
    {
    }
    
    
    Graph::InputPort::InputPort()
        : Port()
    {
    }
    
    Graph::InputPort::~InputPort()
    {
    }
    
    Graph::OutputPort::OutputPort()
        : Port()
    {
    }
    
    Graph::OutputPort::~OutputPort()
    {
    }

    Graph::Node::Node()
        : RefCounted()
        , m_Graph(0)
        , m_Handle()
        , m_Order()
    {
    }
    
    Graph::Node::~Node()
    {
    }
    
    intrusive_ptr<Graph::InputPort> Graph::Node::inputPort(std::string const &n) const
    {
        input_ports_by_name_type const &input_ports_by_name = m_InputPorts.get<2>();
        input_ports_by_name_type::const_iterator itIP = input_ports_by_name.find(n);
        if(itIP == input_ports_by_name.end())
            return intrusive_ptr<InputPort>();
        return *itIP;
    }
    
    bool Graph::Node::addInputPort(intrusive_ptr<InputPort> ip)
    {
        if(!ip) return false;
        if(ip->m_Node) return false;
        if(!m_InputPorts.push_back(ip).second) return false;
        ip->m_Node = this;
        return true;
    }
    
    intrusive_ptr<Graph::OutputPort> Graph::Node::outputPort(std::string const &n) const
    {
        output_ports_by_name_type const &output_ports_by_name = m_OutputPorts.get<2>();
        output_ports_by_name_type::const_iterator itOP = output_ports_by_name.find(n);
        if(itOP == output_ports_by_name.end())
            return intrusive_ptr<OutputPort>();
        return *itOP;
    }
    
    bool Graph::Node::addOutputPort(intrusive_ptr<OutputPort> op)
    {
        if(!op) return false;
        if(op->m_Node) return false;
        if(!m_OutputPorts.push_back(op).second) return false;
        op->m_Node = this;
        return true;
    }
    
    Graph::Connection::Connection()
        : RefCounted()
        , m_Graph(0)
        , m_Handle()
        , m_SourcePort(0)
        , m_DestinationPort(0)
    {
    }
    
    Graph::Connection::~Connection()
    {
    }
    
    
    Graph::Graph()
        : RefCounted()
        , m_Container()
    {
    }
    
    Graph::~Graph()
    {
    }
    
    system::error_code Graph::addNode(intrusive_ptr<Graph::Node> node)
    {
        if(!isMutable())
            return makeErrorCode(kEInvalidState);
        
        if(node->graph())
            return makeErrorCode(kEAlreadyExists);
        
        Node::handle_type hNode = add_vertex(m_Container);
        node->m_Graph = this;
        node->m_Handle = hNode;
        m_Container[hNode] = node;
        
        updateVisitOrder();
        
        return makeErrorCode(kENoError);
    }
    
    system::error_code Graph::removeNode(intrusive_ptr<Graph::Node> node)
    {
        if(!isMutable())
            return makeErrorCode(kEInvalidState);
        
        intrusive_ptr<Graph> graph = node->graph();
    
        if(!graph || graph != this)
            return makeErrorCode(kENotFound);
       
        updateNodeHandles();
        updateVisitOrder();
        
        return makeErrorCode(kENotFound);
    }
    
    system::error_code Graph::connectPorts(intrusive_ptr<Graph::OutputPort> src, intrusive_ptr<Graph::InputPort> dst)
    {
        if(!isMutable())
            return makeErrorCode(kEInvalidState);
        
        if(!src || !dst)
            return makeErrorCode(kENotFound);
        
        intrusive_ptr<Node> srcNode = src->node();
        if(!srcNode)
            return makeErrorCode(kENotFound);
        
        if(dst->m_SourceConnection)
            return makeErrorCode(kEInvalidArgument);
        
        intrusive_ptr<Graph> srcGraph = srcNode->graph();
        if(!srcGraph || srcGraph != this)
            return makeErrorCode(kENotFound);

        intrusive_ptr<Node> dstNode = dst->node();
        if(!dstNode)
            return makeErrorCode(kENotFound);
            
        intrusive_ptr<Graph> dstGraph = dstNode->graph();
        if(!dstGraph || dstGraph != this)
            return makeErrorCode(kENotFound);
        
        if(!dst->acceptsPort(src))
            return makeErrorCode(kEInvalidArgument);
            
        std::pair
        <
            Connection::handle_type,
            bool
        > hConn = add_edge(srcNode->m_Handle, dstNode->m_Handle, m_Container);
        
        if(!hConn.second)
            return makeErrorCode(kENotFound);
        
        intrusive_ptr<Connection> conn(new Connection());
        conn->m_SourcePort = src.get();
        conn->m_DestinationPort = dst.get();
        conn->m_Graph = this;
        conn->m_Handle = hConn.first;
        dst->m_SourceConnection = conn;
        src->m_DestinationConnections.insert(conn);
        m_Container[hConn.first] = conn;
        
        updateVisitOrder();
        
        return makeErrorCode(kENoError);
    }
    
    system::error_code Graph::disconnectPorts(intrusive_ptr<Graph::OutputPort> src, intrusive_ptr<Graph::InputPort> dst)
    {
        if(!isMutable())
            return makeErrorCode(kEInvalidState);
        
        return makeErrorCode(kENotFound);
    }
    
    system::error_code Graph::connectNodes(intrusive_ptr<Node> src, intrusive_ptr<Node> dst)
    {
        if(!isMutable())
            return makeErrorCode(kEInvalidState);
        
        if(!src || !dst)
            return makeErrorCode(kENotFound);
        
        bool anyConnected = false;
        std::for_each(src->outputPorts().begin(), src->outputPorts().end(), [this, dst, &anyConnected](intrusive_ptr<OutputPort> op)
        {
            if(intrusive_ptr<InputPort> ip = dst->inputPort(op->name()))
            {
                anyConnected = !connectPorts(op, ip) || anyConnected;
            }
        });
        
        return anyConnected?makeErrorCode(kENoError):makeErrorCode(kENotFound);
    }
    
    system::error_code Graph::visitAllNodes(NodeVisitor ndov, NodeVisitor nundov)
    {
        for(std::size_t n = 0; n < m_NodesVisitOrder.size(); ++n)
        {
            if(system::error_code ndove = ndov(m_Container[m_NodesVisitOrder[n]]))
            {
                while(n--)
                {
                    nundov(m_Container[m_NodesVisitOrder[n]]);
                }
                return ndove;
            }
        }
        
        return makeErrorCode(kENoError);
    }
    
    system::error_code Graph::visit(unordered_set< intrusive_ptr<Node> > const &src, NodeVisitor nv, ConnectionVisitor cv)
    {
        std::set<std::size_t> nodesOrdered;
        std::for_each(src.begin(), src.end(), [&nodesOrdered](intrusive_ptr<Node> node) -> void
        {
            nodesOrdered.insert(node->m_Order);
        });
        
        std::vector<bool> nodesProcessed(m_NodesVisitOrder.size(), false);
        
        for(auto itN = nodesOrdered.begin(); itN != nodesOrdered.end(); ++itN)
        {
            std::vector<std::size_t> n2p;
            n2p.push_back(*itN);
            
            auto const outCons = out_edges(m_NodesVisitOrder[*itN], m_Container);
            
            for(auto itC = outCons.first; itC != outCons.second; ++itC)
            {
                intrusive_ptr<Connection> conn = m_Container[*itC];
                n2p.push_back(conn->destinationPort()->m_Node->m_Order);
            }

            for(std::size_t in2p = 0; in2p < n2p.size(); ++in2p)
            {
                for(std::size_t n = n2p[in2p] /* *itN */; n < m_NodesVisitOrder.size(); ++n)
                {
                    if(nodesProcessed[n])
                        break;
                    
                    auto const inCons = in_edges(m_NodesVisitOrder[n], m_Container);
                    
                    for(auto itC = inCons.first; itC != inCons.second; ++itC)
                    {
                        intrusive_ptr<Connection> conn = m_Container[*itC];
                        if(system::error_code cve = cv(conn))
                        {
                            return cve;
                        }
                    }
                    
                    intrusive_ptr<Node> node = m_Container[m_NodesVisitOrder[n]];
                    
                    if(system::error_code nve = nv(node))
                    {
                        return nve;
                    }
                    
                    nodesProcessed[n] = true;
                    
                    if(!out_degree(m_NodesVisitOrder[n], m_Container))
                        break;
                }
            }
        }
        return makeErrorCode(kENoError);
    }
    
    void Graph::updateNodeHandles()
    {
        auto const iVB = vertices(m_Container).first;
        auto const iVE = vertices(m_Container).second;
        
        for(auto iV = iVB; iV != iVE; ++iV)
        {
            m_Container[*iV]->m_Handle = *iV;
        }
    }
    
    void Graph::updateVisitOrder()
    {
        m_NodesVisitOrder.resize(num_vertices(m_Container));
        topological_sort(
            m_Container,
            m_NodesVisitOrder.rbegin()
        );
        
        for(std::size_t no = 0; no < m_NodesVisitOrder.size(); ++no)
        {
            m_Container[m_NodesVisitOrder[no]]->m_Order = no;
        }
    }

} //namespace base
} //namespace uquad


