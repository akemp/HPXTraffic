#include <physics\graphics.hpp>
#include <physics\pathing.hpp>

using namespace boost;
using namespace std;

int generatePath(int start, int end,
    const graph_t &g, 
    const property_map<graph_t, edge_weight_t>::type &weightmap, const vector<vertex_descriptor> &p,
    const vector<double>& d, const vector<road*>& streets,
    const pred_map& pd)
{
    vertex_descriptor s = vertex(start%num_vertices(g), g);
    vertex_descriptor t = vertex(end%num_vertices(g), g);
    int shortest_path = -1;
    /*dijkstra_shortest_paths(g, s,
                            pd);
    */
      try {
    // call astar named parameter interface
    astar_search
      (g, s,
       distance_heuristic<graph_t, float, vector<road*>>
        (streets, t),
       pd.visitor(astar_goal_visitor<vertex_descriptor>(t)));
      } catch(found_goal fg) { // found a path to the goal
        for(vertex_descriptor v = t;; v = p[v]) {
          if(p[v] == v)
            break;
          shortest_path = v;
        }
        //reverse(shortest_path.begin(), shortest_path.end());
      }
        return shortest_path;
}