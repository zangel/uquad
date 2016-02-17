#include <boost/test/unit_test.hpp>
#include <uquad/base/Serialization.h>
#include <uquad/base/Error.h>
#include <fstream>

#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <uquad/utility/EigenMatrixSerialization.h>

using namespace uquad;

template <typename T>
void saveObject(T const &obj, std::string const &fn)
{
    std::ofstream file(fn);
    BOOST_TEST_REQUIRE(file.is_open());
    
    base::OArchive archive(file);
    archive & obj;
}

template <typename T>
void loadObject(T &obj, std::string const &fn)
{
    std::ifstream file(fn);
    BOOST_TEST_REQUIRE(file.is_open());
    
    base::IArchive archive(file);
    archive & obj;
}

BOOST_AUTO_TEST_CASE(IOArchive)
{
    #if 1
    { std::vector<bool> obj(100, true); saveObject(obj, "std_vector_of_bool.archive"); }
    { std::vector<Vec2i> obj(100, Vec2i(1,2)); saveObject(obj, "std_vector_of_Vec2i.archive"); }
    { std::vector<Vec2f> obj(100, Vec2f(100.0f,200.0f)); saveObject(obj, "std_vector_of_Vec2f.archive"); }
    { std::vector<Vec2d> obj(100, Vec2d(1000.0,2000.0)); saveObject(obj, "std_vector_of_Vec2d.archive"); }
    #else
    { std::vector<bool> obj; loadObject(obj, "std_vector_of_bool.archive"); }
    { std::vector<Vec2i> obj; loadObject(obj, "std_vector_of_Vec2i.archive"); }
    { std::vector<Vec2f> obj; loadObject(obj, "std_vector_of_Vec2f.archive"); }
    { std::vector<Vec2d> obj; loadObject(obj, "std_vector_of_Vec2d.archive"); }
    #endif
}