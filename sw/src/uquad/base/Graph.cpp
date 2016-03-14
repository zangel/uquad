#include "Graph.h"
#include "Error.h"

namespace
{
    class GraphNodeIndexMap
    {
    public:
        typedef boost::readable_property_map_tag category;
        typedef uint32_t value_type;
        typedef value_type reference;
        typedef uquad::base::Graph::Node::handle_type key_type;
        
        GraphNodeIndexMap(const uquad::base::Graph::container_type &c)
            : container(&c)
        {
        }

        const uquad::base::Graph::container_type *container;
    };
    
    GraphNodeIndexMap get(boost::vertex_index_t, const uquad::base::Graph::container_type &g)
    {
        return GraphNodeIndexMap(g);
    }

    GraphNodeIndexMap::value_type get(GraphNodeIndexMap map, GraphNodeIndexMap::key_type node)
    {
        return (*map.container)[node]->index();
    }
    
    
    struct GraphNodeVisitor
        : public boost::default_bfs_visitor
    {
        //typedef typename property_traits < TimeMap >::value_type T;
    public:
        GraphNodeVisitor(uquad::base::Graph::NodeVisitPredicate p)
            : predicate(p)
        {
        }
        
        template < typename Vertex, typename Graph >
        void discover_vertex(Vertex u, const Graph & g) const
        {
            predicate(g[u]);
        }
        
        uquad::base::Graph::NodeVisitPredicate predicate;
    };
    
} //namespace anonymous


namespace boost
{
    template<>
    struct property_map<uquad::base::Graph::container_type, vertex_index_t>
    {
        typedef GraphNodeIndexMap const_type;
    };

} //namespace boost


namespace uquad
{
namespace base
{
    Graph::Node::Node()
        : RefCounted()
        , m_Graph(0)
        , m_Handle()
        , m_Index(0)
    {
    }
    
    Graph::Node::~Node()
    {
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
    
    Graph::Port::Port()
        : RefCounted()
    {
    }
    
    Graph::Port::~Port()
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
        if(node->graph())
            return makeErrorCode(kEAlreadyExists);
        
        uint32_t const nodeIndex = num_vertices(m_Container);
        Node::handle_type hNode = add_vertex(m_Container);
        node->m_Graph = this;
        node->m_Handle = hNode;
        node->m_Index = nodeIndex;
        m_Container[hNode] = node;
        
        return makeErrorCode(kENoError);
    }
    
    system::error_code Graph::removeNode(intrusive_ptr<Graph::Node> node)
    {
        intrusive_ptr<Graph> graph = node->graph();
    
        if(!graph || graph != this)
            return makeErrorCode(kENotFound);
       
        return makeErrorCode(kENotFound);
    }
    
    system::error_code Graph::connectPorts(intrusive_ptr<Graph::Port> src, intrusive_ptr<Graph::Port> dst)
    {
        if(!src || !dst)
            return makeErrorCode(kENotFound);
        
        intrusive_ptr<Node> srcNode = src->node();
        if(!srcNode)
            return makeErrorCode(kENotFound);
        
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
        m_Container[hConn.first] = conn;
        
        return makeErrorCode(kENoError);
    }
    
    system::error_code Graph::disconnectPorts(intrusive_ptr<Graph::Port> src, intrusive_ptr<Graph::Port> dst)
    {
        return makeErrorCode(kENotFound);
    }
    
    system::error_code Graph::visitNodes(unordered_set< intrusive_ptr<Node> > const &src, NodeVisitPredicate pred)
    {
        std::list<Node::handle_type> vertices;
        std::for_each(src.begin(), src.end(), [&vertices](intrusive_ptr<Node> node) -> void
        {
            vertices.push_back(node->m_Handle);
        });
        
        GraphNodeIndexMap nodeIndexMap = get(vertex_index, m_Container);
        std::vector<default_color_type> nodeColors(num_vertices(m_Container));
        
        breadth_first_search(
            m_Container,
            *(boost::vertices(m_Container).first),
            visitor(GraphNodeVisitor(pred))
                .color_map(make_iterator_property_map(nodeColors.begin(), nodeIndexMap))
        );
        return makeErrorCode(kENoError);
    }

} //namespace base
} //namespace uquad


