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

typedef struct street;

struct vehicle
{
    int destination;
    Edge turn;
    glm::vec2 start;
    glm::vec2 dir;

    street* streetloc;

    int license;

    int path;

    float vel;

    glm::vec2 place;

    float progress;
    float waited;
    float dist;
    float last;
    float avgtime;
    bool waiting;
    bool turning;

    vehicle(int ind, street* loc)
    {
        waiting = false;
        turning = false;
        avgtime = 0;
        license = ind;
        streetloc = loc;
        waited = 0;
        vel = 0;
    };
    vehicle(){};
    float gettime()
    {
        float avgret = avgtime;
        avgtime = 0;
        return avgret;
    }
    void increment(float elapsed)
    {
        avgtime += elapsed;
    }
};

struct street
{
    glm::vec2 v1;
    glm::vec2 v2;
    glm::vec2 dir;
    float dist;
    float caradj;
    
    int index;
    std::vector<int> neighbors;
    std::vector<std::vector<Edge>> intersects;

    std::vector<std::pair<vehicle*,int>> cars;

    street(){};
    street(edger edge, int ind)
    {
        v1 = edge.v1;
        v2 = edge.v2;
        dist = glm::distance(v1,v2);
        neighbors = edge.neighbors;
        dir = glm::normalize(v2-v1);
        caradj = 20.0f;
        index = ind;
    };
    float totaltime()
    {
        float caradjust = cars.size() * caradj;
        return cars.size() * caradj + dist + 50.0f;
    }
    void removevehicle(const int car)
    {
        auto it = std::remove_if(cars.begin(), cars.end(),
                                                  [car](const std::pair<vehicle*,int> pather) {return(pather.second == car);});
        cars.erase(it, cars.end());
    }
    void addvehicle(vehicle* car)
    {
        cars.push_back(std::pair<vehicle*,int>(car,car->license));
    }
    void erasecars()
    {
        cars = std::vector<std::pair<vehicle*, int>>();
    }
};



void generateQuads(std::vector<std::vector<glm::vec3>> quads, std::vector<Edge> egs, std::vector<glm::vec2> inputted, int& count, float scaler,
                   std::vector<unsigned int>& indices, std::vector<VertexData>& vertex_data);


//This section of code and parts of its implementation are copied from http://www.richelbilderbeek.nl

bool sorter(std::vector<Point> &p1, std::vector<Point> &p2);

std::vector<Line> generateRoads(int dim, int offset, float size);

void generateRoadModels(const std::vector<Line>& roadsegs, std::vector<glm::vec2>& inputted, std::vector<Edge>& egs,
    std::vector<std::vector<glm::vec3>>& quads, std::vector<edger>& edges);


#endif