
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point_xy.hpp>


#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
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



vector<int> shortest_path(vertex_descriptor target, vertex_descriptor s, graph_t g, const vector<double>& d, const vector<vertex_descriptor>& p)
{
    vector<int> nodes;
    int test = 0;
    int ltarget = target;
    {
        do{ 
            ltarget = target;
            target = p[target];
            nodes.push_back(target);
            ++test;
        } while(target != s && test < 10000000);
    }
    if (target != s)
    {
        cout << "ERROR! STUCK IN LOOP!\n";
        exit(1);
    }
    reverse(nodes.begin(), nodes.end());
    return nodes;
}

vector<int> generatePath(int start, int end, const graph_t &g, 
    const property_map<graph_t, edge_weight_t>::type &weightmap, vector<vertex_descriptor> &p, vector<double>& d, int i)
{
    vertex_descriptor s = vertex(start%num_vertices(g), g);
    vertex_descriptor t = vertex(end%num_vertices(g), g);

    pred_map pd = predecessor_map(boost::make_iterator_property_map(p.begin(), get(boost::vertex_index, g))).
                            distance_map(boost::make_iterator_property_map(d.begin(), get(boost::vertex_index, g)));
    dijkstra_shortest_paths(g, s,
                            pd);
    
    cout << i << endl;

    return shortest_path(t,s,g,d,p);
}

