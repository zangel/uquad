#include <boost/test/unit_test.hpp>
#include <uquad/control/System.h>
#include <uquad/control/Block.h>
#include <uquad/control/Signal.h>

using namespace uquad;
namespace tt = boost::test_tools;

class TestBlock
    : public control::Block
{
public:
    TestBlock()
        : control::Block()
        , m_Input(s_IOSignalName)
        , m_Output(s_IOSignalName)
    {
        intrusive_ptr_add_ref(&m_Input); addInputPort(&m_Input);
        intrusive_ptr_add_ref(&m_Output); addOutputPort(&m_Output);
    }
    
    bool isValid() const
    {
        return true;
    }
    
    system::error_code update(asio::yield_context yctx)
    {
        m_Output.m_Value = m_Input.m_Value + 1;
        return base::makeErrorCode(base::kENoError);
    }
    
    static std::string s_IOSignalName;
    mutable control::DefaultInputSignal<float> m_Input;
    mutable control::DefaultOutputSignal<float> m_Output;
};

std::string TestBlock::s_IOSignalName = "value";

#define TEST_SYSTEM_RUN_METHOD(method)          \
    done = false;                               \
    system->method([&done](system::error_code)  \
    {                                           \
        done = true;                            \
    });                                         \
    runloop->runUntil([&done]() -> bool         \
    {                                           \
        return !done;                           \
    });


BOOST_AUTO_TEST_CASE(System)
{
    intrusive_ptr<base::Runloop> runloop(new base::Runloop());
    intrusive_ptr<control::System> system(new control::System(runloop));
    
    intrusive_ptr<TestBlock> block1(new TestBlock());
    BOOST_TEST_REQUIRE(!system->addBlock(block1));
    
    intrusive_ptr<TestBlock> block2(new TestBlock());
    BOOST_TEST_REQUIRE(!system->addBlock(block2));
    
    BOOST_TEST_REQUIRE(!system->connectBlocks(block1, block2));
    
    BOOST_TEST_REQUIRE(!system->isStarted());
    BOOST_TEST_REQUIRE(!system->isUpdating());
    
    bool done;
    
    TEST_SYSTEM_RUN_METHOD(start)
    BOOST_TEST_REQUIRE(system->isStarted());
    
    
    BOOST_TEST_REQUIRE(!system->isUpdating());
    block1->m_Input.m_Value = 10;
    block1->requestUpdate();
    
    TEST_SYSTEM_RUN_METHOD(update)
    BOOST_REQUIRE_EQUAL(block2->m_Output.m_Value, 12);
    BOOST_TEST_REQUIRE(!system->isUpdating());
    
    TEST_SYSTEM_RUN_METHOD(stop)
    BOOST_TEST_REQUIRE(!system->isStarted());
}