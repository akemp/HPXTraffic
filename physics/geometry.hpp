#ifndef GEOMETRY
#define GEOMETRY

#include <vector>
#include <queue>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>
#include <boost/geometry/multi/geometries/multi_linestring.hpp>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GL/glfw.h>

// Include GLM
#include <glm/glm.hpp>


#include <common/objloader.hpp>


typedef std::pair<int,int> Edge;
typedef boost::geometry::model::d2::point_xy<double> Point;
typedef boost::geometry::model::linestring<Point> Line;
typedef boost::geometry::model::multi_linestring <Line> multilinestring;

typedef std::pair<std::vector<glm::vec2>, std::vector<Edge>> LineMesh;
typedef boost::geometry::model::polygon<Point> polygon;
typedef boost::geometry::model::multi_polygon <polygon> multipolygon;


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

glm::vec2 intersect(glm::vec2 v1, glm::vec2 v2);

struct Node
{
    std::vector<std::pair<std::pair<Node*,Node*>,bool>*> intersects;
    Point center;
    bool start;
    int index;
    Node(){start=false;};
};

LineMesh generateRoads(int dim, int offset, float size);

void generateRoadModels(const LineMesh& roadsegs, std::vector<unsigned int>& indices, std::vector<VertexData>& vertex_data);

#endif