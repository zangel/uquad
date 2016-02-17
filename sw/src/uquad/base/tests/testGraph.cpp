#include <boost/test/unit_test.hpp>
#include <uquad/base/Graph.h>
#include <uquad/base/Error.h>

using namespace uquad;

BOOST_AUTO_TEST_CASE(Graph)
{
    intrusive_ptr<base::Graph> graph(new base::Graph());
    
    intrusive_ptr<base::Graph::Node> node1(new base::Graph::Node());
    intrusive_ptr<base::Graph::Node> node2(new base::Graph::Node());
    
    BOOST_TEST_REQUIRE(!graph->addNode(node1));
    BOOST_TEST_REQUIRE(!graph->addNode(node2));
    
    intrusive_ptr<base::Graph::Port> node1Port(new base::Graph::Port());
    intrusive_ptr<base::Graph::Port> node2Port(new base::Graph::Port());
    
    BOOST_TEST_REQUIRE(!node1->addOutputPort(node1Port));
    BOOST_TEST_REQUIRE(!node2->addInputPort(node2Port));
    
    BOOST_TEST_REQUIRE(!graph->connectPorts(node1Port, node2Port));
    
    unordered_set< intrusive_ptr<base::Graph::Node> > sources;
    sources.insert(node1);
    
    graph->visitNodes(sources, [](intrusive_ptr<base::Graph::Node> node)
    {
        BOOST_TEST_REQUIRE(node);
    });
}
