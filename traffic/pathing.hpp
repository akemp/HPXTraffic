
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

typedef adjacency_list < listS, vecS, directedS,
no_property, property < edge_weight_t, double > > graph_t;
typedef graph_traits < graph_t >::vertex_descriptor vertex_descriptor;
typedef std::pair<int, int> Edge;


vector<Edge> shortest_path(vertex_descriptor target, vertex_descriptor s, graph_t g, const vector<double>& d, const vector<vertex_descriptor>& p)
{
    vector<Edge> nodes;
    int test = 0;
    int ltarget = target;
    {
        do{
            ltarget = target;
            target = p[target];
            nodes.push_back(Edge(target, ltarget));
            ++test;
        }while(target != s && test < 10000000);
    }
    if (target != s)
    {
        cout << "ERROR! STUCK IN LOOP!\n";
        exit(1);
    }
    reverse(nodes.begin(), nodes.end());
    return nodes;
}

vector<Edge> generate_path(const graph_t &g, vertex_descriptor s, vertex_descriptor t, vector<vertex_descriptor> p, vector<double> d)
{

    //cout << "\nGenerating paths\n";

    dijkstra_shortest_paths(g, s,
                            predecessor_map(boost::make_iterator_property_map(p.begin(), get(boost::vertex_index, g))).
                            distance_map(boost::make_iterator_property_map(d.begin(), get(boost::vertex_index, g))));
    
    //cout << "\nGenerated. Outputting path\n";

    return shortest_path(t,s,g,d,p);
}

vector<Edge> generatePath(int start, int end,vector<edger> edges)
{
    vector<Edge> edge_vector;
    vector<double> weight_array;

    for (int i = 0; i < edges.size(); ++i)
    {
        for (int j = 0; j < edges[i].neighbors.size(); ++j)
        {
            edge_vector.push_back(Edge(i, edges[i].neighbors[j]));
            edger temp = edges[edges[i].neighbors[j]];
            weight_array.push_back(glm::distance(temp.v1, temp.v2));
        }
    }
    
    graph_t g(edge_vector.begin(), edge_vector.end(), weight_array.begin(), edges.size());

    property_map<graph_t, edge_weight_t>::type weightmap = get(edge_weight, g);
    vector<vertex_descriptor> p(num_vertices(g));
    vector<double> d(num_vertices(g));

    time_t timev;

    vertex_descriptor s = vertex(start%num_vertices(g), g);
    vertex_descriptor t = vertex(end%num_vertices(g), g);

    return generate_path(g,s,t, p, d);
}