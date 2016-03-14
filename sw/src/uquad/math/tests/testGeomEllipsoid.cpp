#include <boost/test/unit_test.hpp>
#include <uquad/math/geom/Ellipsoid.h>

#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

using namespace uquad;

typedef boost::minstd_rand base_generator_type;

static std::size_t const NUM_PTS = 20;

BOOST_AUTO_TEST_CASE(GeomEllipsoid)
{
    boost::minstd_rand generator(static_cast<unsigned int>(std::time(0)));
    boost::uniform_real<> uni_dist(-1,1);
    boost::variate_generator<boost::minstd_rand&, boost::uniform_real<> > uni(generator, uni_dist);
    
    //Vec3f center = 10.0f*Vec3f(uni(), uni(), uni());
    Vec3f center = Vec3f(1, 2, 3);
    Vec3f radii = Vec3f(5, 6, 7);

    //float const radius = 10 * (1 + uni());
    
    std::vector<uquad::math::geom::Ellipsoidf::Vector3> ellipsoidPoints(NUM_PTS);
    for(std::size_t p = 0; p < NUM_PTS; p++)
    {
        Vec3f pt(uni(), uni(), uni());
        pt.normalize();
        
        float dev = (10 + uni())/10.0f;
        
        ellipsoidPoints[p] = center + dev * Vec3f(
            radii.x() * pt.x(),
            radii.y() * pt.y(),
            radii.z() * pt.z()
        );
    }
    
    //uquad::math::geom::Ellipsoidf ellipsoid(Vec3f::Zero(), Vec3f::Ones(), Mat3x3f::Identity());
    uquad::math::geom::Ellipsoidf ellipsoid = uquad::math::geom::Ellipsoidf::inertiaTensorFit(ellipsoidPoints);
    
    Vec3f pt1 = Vec3f::Zero();
    ellipsoid.map(pt1);
    Vec3f pt1i = pt1;
    ellipsoid.unmap(pt1i);
    
    Vec3f pt2(1.0f, 0.0f, 0.0f);
    ellipsoid.map(pt2);
    Vec3f pt2i = pt2;
    ellipsoid.unmap(pt2i);
    
}

