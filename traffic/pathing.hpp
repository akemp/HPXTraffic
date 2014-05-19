
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point_xy.hpp>


#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <boost/graph/astar_search.hpp>
#include <boost/property_map/property_map.hpp>


using namespace boost; 
using namespace glm;
using namespace std;

typedef boost::geometry::model::d2::point_xy<double> Point;
typedef boost::geometry::model::linestring<Point> Line;
typedef boost::bgl_named_params<boost::iterator_property_map<std::_Vector_iterator<std::_Vector_val<double,std::allocator<double> > >,boost::vec_adj_list_vertex_id_map<boost::no_property,unsigned __int64>,double,double &>,enum boost::vertex_distance_t,boost::bgl_named_params<boost::iterator_property_map<std::_Vector_iterator<std::_Vector_val<unsigned __int64,std::allocator<unsigned __int64> > >,boost::vec_adj_list_vertex_id_map<boost::no_property,unsigned __int64>,unsigned __int64,unsigned __int64 &>,enum boost::vertex_predecessor_t,boost::no_property> >
    pred_map;

typedef adjacency_list < listS, vecS, directedS,
no_property, property < edge_weight_t, double > > graph_t;
typedef graph_traits < graph_t >::vertex_descriptor vertex_descriptor;
typedef std::pair<int, int> Edge;


// euclidean distance heuristic
template <class Graph, class CostType, class LocMap>
class distance_heuristic : public astar_heuristic<Graph, CostType>
{
public:
  typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
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



struct edger
{
    pair<int,int> edge;
    vec2 v1;
    vec2 v2;
    vector<int> neighbors;

    edger(){};
    edger(pair<int,int> v)
    {
        edge = v;
    }
};

typedef struct street;

struct vehicle
{
    int destination;
    Edge turn;
    vec2 start;
    vec2 dir;

    street* streetloc;

    int index;
    int license;

    int path;
    float vel;
    vec2 place;

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
    vec2 v1;
    vec2 v2;
    vec2 dir;
    float dist;
    float caradj;

    vector<int> neighbors;
    vector<vector<Edge>> intersects;

    vector<pair<vehicle*,int>> cars;

    street(){};
    street(edger edge)
    {
        v1 = edge.v1;
        v2 = edge.v2;
        dist = glm::distance(v1,v2);
        neighbors = edge.neighbors;
        dir = normalize(v2-v1);
        caradj = 10.0f;
    };
    float totaltime()
    {
        float caradjust = cars.size() * caradj;
        return cars.size() * caradj + dist;
    }
    void removevehicle(const int car)
    {
        vector<pair<vehicle*,int>>::iterator it = remove_if(cars.begin(), cars.end(),
                                                  [car](const pair<vehicle*,int> pather) {return(pather.second == car);});
        cars.erase(it, cars.end());
    }
    void addvehicle(vehicle* car)
    {
        cars.push_back(pair<vehicle*,int>(car,car->license));
    }
    void erasecars()
    {
        cars = vector<pair<vehicle*, int>>();
    }
};

int generatePath(int start, int end,
    const graph_t &g, 
    const property_map<graph_t, edge_weight_t>::type &weightmap, const vector<vertex_descriptor> &p,
    const vector<double>& d, const vector<street*>& streets,
    const pred_map& pd)
{
    vertex_descriptor s = vertex(start%num_vertices(g), g);
    vertex_descriptor t = vertex(end%num_vertices(g), g);
    vector<int> shortest_path;
    /*dijkstra_shortest_paths(g, s,
                            pd);
    */
      try {
    // call astar named parameter interface
    astar_search
      (g, s,
       distance_heuristic<graph_t, float, vector<street*>>
        (streets, t),
       pd.visitor(astar_goal_visitor<vertex_descriptor>(t)));
      } catch(found_goal fg) { // found a path to the goal
        for(vertex_descriptor v = t;; v = p[v]) {
          if(p[v] == v)
            break;
          shortest_path.push_back(v);
        }
        //reverse(shortest_path.begin(), shortest_path.end());
      }
    if (!shortest_path.empty())
        return shortest_path.back();
    return -1;
}

