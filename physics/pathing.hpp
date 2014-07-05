#ifndef PATHING
#define PATHING

#include <algorithm>
#include <vector>
#include <queue>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point_xy.hpp>


#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <boost/graph/astar_search.hpp>
#include <boost/property_map/property_map.hpp>


// Include GLM
#include <glm/glm.hpp>
#include <physics/geometry.hpp>
#include <physics/graphics.hpp>

typedef boost::bgl_named_params<boost::iterator_property_map<std::_Vector_iterator<std::_Vector_val<double,std::allocator<double> > >,boost::vec_adj_list_vertex_id_map<boost::no_property,unsigned __int64>,double,double &>,enum boost::vertex_distance_t,boost::bgl_named_params<boost::iterator_property_map<std::_Vector_iterator<std::_Vector_val<unsigned __int64,std::allocator<unsigned __int64> > >,boost::vec_adj_list_vertex_id_map<boost::no_property,unsigned __int64>,unsigned __int64,unsigned __int64 &>,enum boost::vertex_predecessor_t,boost::no_property> >
    pred_map;

typedef boost::adjacency_list < boost::listS, boost::vecS, boost::directedS,
boost::no_property, boost::property < boost::edge_weight_t, double > > graph_t;
typedef boost::graph_traits < graph_t >::vertex_descriptor vertex_descriptor;


// euclidean distance heuristic
template <class Graph, class CostType, class LocMap>
class distance_heuristic : public boost::astar_heuristic<Graph, CostType>
{
public:
  typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
  distance_heuristic(LocMap l, Vertex goal)
    : m_location(l), m_goal(goal) {}
  CostType operator()(Vertex u)
  {
    return (m_location[u]->totaltime());
  }
private:
  LocMap m_location;
  Vertex m_goal;
};

struct found_goal {}; // exception for termination

// visitor that terminates when we find the goal
template <class Vertex>
class astar_goal_visitor : public boost::default_astar_visitor
{
public:
  astar_goal_visitor(Vertex goal) : m_goal(goal) {}
  template <class Graph>
  void examine_vertex(Vertex u, Graph& g) {
    if(u == m_goal)
      throw found_goal();
  }
private:
  Vertex m_goal;
};


int generatePath(int start, int end,
    const graph_t &g, 
    const boost::property_map<graph_t, boost::edge_weight_t>::type &weightmap, const std::vector<vertex_descriptor> &p,
    const std::vector<double>& d, const std::vector<road*>& streets,
    const pred_map& pd);

void processCars(std::vector<Mesh>& cars, std::vector<vehicle>& pathers,
    std::vector<road>& streets, const float scaler, const float elapsed,
    const graph_t &g, 
    const boost::property_map<graph_t, boost::edge_weight_t>::type &weightmap, const std::vector<vertex_descriptor> &p,
    const std::vector<double>& d, const std::vector<road*>& streetsp,
    const pred_map& pd, 
    float carsize,
    bool deleteafter);
#endif