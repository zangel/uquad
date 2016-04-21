#include <boost/test/unit_test.hpp>
#include <uquad/base/Graph.h>
#include <uquad/base/Error.h>

using namespace uquad;

class TestNode
    : public base::Graph::Node
{
public:
    class InputPort1
        : public base::Graph::InputPort
    {
    public:
        
        InputPort1()
            : base::Graph::InputPort()
        {
        }
        
        std::string const& name() const
        {
            static std::string _name = "in1";
            return _name;
        }
        
        bool acceptsPort(intrusive_ptr<base::Graph::OutputPort> op) const
        {
            return true;
        }
    };
    
    class InputPort2
        : public base::Graph::InputPort
    {
    public:
        
        InputPort2()
            : base::Graph::InputPort()
        {
        }
        
        std::string const& name() const
        {
            static std::string _name = "in2";
            return _name;
        }
        
        bool acceptsPort(intrusive_ptr<base::Graph::OutputPort> op) const
        {
            return true;
        }
    };
    
    class OutputPort1
        : public base::Graph::OutputPort
    {
    public:
        
        OutputPort1()
            : base::Graph::OutputPort()
        {
        }
        
        std::string const& name() const
        {
            static std::string _name = "out1";
            return _name;
        }
    };
    
    TestNode(std::string const &n)
        : base::Graph::Node()
        , m_InputPort1()
        , m_InputPort2()
        , m_OutputPort1()
        , m_Name(n)
    {
        intrusive_ptr_add_ref(&m_InputPort1); addInputPort(&m_InputPort1);
        intrusive_ptr_add_ref(&m_InputPort2); addInputPort(&m_InputPort2);
        intrusive_ptr_add_ref(&m_OutputPort1); addOutputPort(&m_OutputPort1);
    }
    
    InputPort1 m_InputPort1;
    InputPort2 m_InputPort2;
    OutputPort1 m_OutputPort1;
    std::string m_Name;
};


BOOST_AUTO_TEST_CASE(Graph)
{
    intrusive_ptr<base::Graph> graph(new base::Graph());
    
    intrusive_ptr<base::Graph::Node> node0(new TestNode("0"));
    intrusive_ptr<base::Graph::Node> node1(new TestNode("1"));
    intrusive_ptr<base::Graph::Node> node2(new TestNode("2"));
    intrusive_ptr<base::Graph::Node> node3(new TestNode("3"));
    
    BOOST_TEST_REQUIRE(!graph->addNode(node0));
    BOOST_TEST_REQUIRE(!graph->addNode(node1));
    BOOST_TEST_REQUIRE(!graph->addNode(node2));
    BOOST_TEST_REQUIRE(!graph->addNode(node3));

    
    BOOST_TEST_REQUIRE(!graph->connectPorts(node0->outputPort(0), node1->inputPort(0)));
    BOOST_TEST_REQUIRE(!graph->connectPorts(node1->outputPort(0), node2->inputPort(0)));
    BOOST_TEST_REQUIRE(!graph->connectPorts(node3->outputPort(0), node2->inputPort(1)));
    
    unordered_set< intrusive_ptr<base::Graph::Node> > sources;
    sources.insert(node0);
    sources.insert(node3);
    
    graph->visit(
        sources,
        [](intrusive_ptr<base::Graph::Node> node) -> system::error_code
        {
            if(intrusive_ptr<TestNode> tnode = dynamic_pointer_cast<TestNode>(node))
            {
                std::cout << tnode->m_Name << std::endl;
            }
            return base::makeErrorCode(base::kENoError);
        },
        [](intrusive_ptr<base::Graph::Connection> conn) -> system::error_code
        {
            return base::makeErrorCode(base::kENoError);
        }
    );
}
