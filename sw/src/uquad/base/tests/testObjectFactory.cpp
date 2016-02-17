#include <boost/test/unit_test.hpp>
#include <uquad/base/ObjectFactory.h>

using namespace uquad;

class TestObject
    : public base::Object
{
public:
    class Registry
        : public base::Object::Registry
    {
    public:
        Registry()
            : base::Object::Registry("testObject", typeid(TestObject))
        {
        }
        
        intrusive_ptr<base::Object> createObject() const { return new TestObject(); }
    };
    
    TestObject()
        : base::Object()
    {
    }
    
    bool isValid() const { return true; }
};


class TestObjectFactory
    : public base::ObjectFactory
{
protected:
    TestObjectFactory()
    {
    }

public:
    intrusive_ptr<TestObject> createTestObject()
    {
        intrusive_ptr<TestObject::Registry> match;
        enumerateRegistries<TestObject>([&match](intrusive_ptr<TestObject::Registry> const &registry) -> bool
        {
            if(registry->name == "testObject")
            {
                match = registry;
                return false;
            }
            return true;
        });
        
        if(match)
            return match->base::Object::Registry::createObject<TestObject>();
        
        return intrusive_ptr<TestObject>();
    }
    
};

UQUAD_BASE_REGISTER_OBJECT(TestObject, TestObjectFactory)


BOOST_AUTO_TEST_CASE(ObjectFactory)
{
    BOOST_TEST_REQUIRE(SI(TestObjectFactory).createTestObject());
}