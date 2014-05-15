
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point_xy.hpp>

#include <vector>
#include <queue>


using namespace boost; 
using namespace glm;
using namespace std;


template <class T>
const std::vector<
  boost::geometry::model::d2::point_xy<T>
>
GetLineLineIntersections(
  const boost::geometry::model::linestring<
    boost::geometry::model::d2::point_xy<T>
  > line1,
  const boost::geometry::model::linestring<
    boost::geometry::model::d2::point_xy<T>
  > line2)
{
  typedef boost::geometry::model::d2::point_xy<T> Point;
  typedef boost::geometry::model::linestring<Point> Line;
  std::vector<Point> points;
  boost::geometry::intersection(line1,line2,points);
  //assert(points.empty() || points.size() == 1);
  return points;
}

template <class T>
const boost::geometry::model::linestring<boost::geometry::model::d2::point_xy<T>
>
CreateLine(const std::vector<boost::geometry::model::d2::point_xy<T> >& v)
{
  return boost::geometry::model::linestring<
    boost::geometry::model::d2::point_xy<T>
  >(v.begin(),v.end());
}

struct fuzzy_equal_to
  : public std::binary_function<double,double,bool>
{
  fuzzy_equal_to(const double tolerance = std::numeric_limits<double>::epsilon())
    : m_tolerance(tolerance)
  {
    assert(tolerance >= 0.0);
  }
  bool operator()(const double lhs, const double rhs) const
  {
    return rhs > (1.0 - m_tolerance) * lhs
        && rhs < (1.0 + m_tolerance) * lhs;
  }
  const double m_tolerance;
};

