#include <boost/test/unit_test.hpp>
#include <uquad/base/Graph.h>
#include <uquad/base/Error.h>

using namespace uquad;

class TestNode
    : public base::Graph::Node
{
public:
    class IOPort
        : public base::Graph::Port
    {
    public:
        
        IOPort(TestNode &tn)
            : base::Graph::Port()
            , m_TestNode(tn)
        {
        }
        
        intrusive_ptr<base::Graph::Node> node() const
        {
            return intrusive_ptr<base::Graph::Node>(&m_TestNode);
        }
        
        bool acceptsPort(intrusive_ptr<base::Graph::Port> input) const
        {
            return true;
        }
        
        TestNode &m_TestNode;
    };
    
    TestNode()
        : base::Graph::Node()
        , m_IOPort(*this)
    {
        intrusive_ptr_add_ref(&m_IOPort);
    }
    
    std::size_t numInputPorts() const
    {
        return 1;
    }
    
    intrusive_ptr<base::Graph::Port> inputPort(std::size_t index) const
    {
        return intrusive_ptr<base::Graph::Port>(&m_IOPort);
    }
    
    std::size_t numOutputPorts() const
    {
        return 1;
    }
    
    intrusive_ptr<base::Graph::Port> outputPort(std::size_t index) const
    {
        return intrusive_ptr<base::Graph::Port>(&m_IOPort);
    }
    
    
    mutable IOPort m_IOPort;
};


BOOST_AUTO_TEST_CASE(Graph)
{
    intrusive_ptr<base::Graph> graph(new base::Graph());
    
    intrusive_ptr<base::Graph::Node> node1(new TestNode());
    intrusive_ptr<base::Graph::Node> node2(new TestNode());
    
    BOOST_TEST_REQUIRE(!graph->addNode(node1));
    BOOST_TEST_REQUIRE(!graph->addNode(node2));
    
    BOOST_TEST_REQUIRE(!graph->connectPorts(node1->outputPort(0), node1->inputPort(0)));
    
    unordered_set< intrusive_ptr<base::Graph::Node> > sources;
    sources.insert(node1);
    
    graph->visitNodes(sources, [](intrusive_ptr<base::Graph::Node> node)
    {
        BOOST_TEST_REQUIRE(node);
    });
}
