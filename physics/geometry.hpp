#ifndef GEOMETRY
#define GEOMETRY

#include <vector>
#include <queue>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
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

typedef std::pair<std::vector<glm::vec2>, std::vector<Edge>> LineMesh;


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

struct edger
{
    Edge edge;
    glm::vec2 v1;
    glm::vec2 v2;
    std::vector<int> neighbors;

    edger(){};
    edger(Edge v)
    {
        edge = v;
    }
};

struct Node
{
    int index;
    std::vector<std::pair<glm::vec2,int>> intersects;
    std::vector<std::pair<glm::vec2,glm::vec2>> corners;
    glm::vec2 center;
};

LineMesh generateRoads(int dim, int offset, float size);

//This section of code and parts of its implementation are copied from http://www.richelbilderbeek.nl


void generateRoadModels(const LineMesh& roadsegs, std::vector<unsigned int>& indices, std::vector<VertexData>& vertex_data);


#endif