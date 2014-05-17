
struct vehicle
{
    int destination;
    Edge turn;
    vec2 start;
    vec2 dir;

    vector<pair<vec2,float>> hist;

    int index;

    vector<int> path;
    vec2 vel;

    float progress;
    float dist;
    float last;
    float avgtime;
    bool waiting;
    bool turning;
    vehicle()
    {
        waiting = false;
        turning = false;
        avgtime = 0;
    };
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

 
void processCars(vector<Mesh>& cars, vector<vehicle>& pathers,
    vector<street>& streets, const float scaler, const float elapsed,
    const graph_t &g, 
    const property_map<graph_t, edge_weight_t>::type &weightmap, const vector<vertex_descriptor> &p,
    const vector<double>& d, const vector<street*>& streetsp,
    const pred_map& pd)
{
    vector<vec2> places;
    vector<vec2> vels;

    float carsize = 0.45;

    for (int i = 0; i < pathers.size(); ++i)
    {
        if (0 >= pathers[i].path.size())
        {
            pathers.erase(pathers.begin() + i);
            cars.erase(cars.begin() + i);
            --i;
        }
    }
    for (int i = 0; i < pathers.size(); ++i)
    {
        float prog;
        if (pathers[i].waiting)
            prog = 0.0f;
        else
            prog = pathers[i].progress;
        vec2 v = pathers[i].start + pathers[i].dir * prog;
        
        cars[i].move.x = v.x;
        cars[i].move.z = v.y;

        cars[i].rot.y = atan2(pathers[i].dir.x,pathers[i].dir.y)*180.0f/3.141592f;
        
        places.push_back(v); 
        vels.push_back(pathers[i].dir*prog);
    }
    for (int i = 0; i < pathers.size(); ++i)
    {
        bool move = true;
        if (!pathers[i].turning)
        {
            vec2 vel = vels[i];
            vec2 place = places[i];
            vec2 start = pathers[i].start;
            if (!pathers[i].waiting)
            {
                int j = -1;
                for (vector<vehicle>::iterator it = pathers.begin(); it < pathers.end(); ++it)
                {
                    ++j;
                    if (i == j || it->index != pathers[i].index)
                        continue;
                    else
                    {
                        vec2 place2 = places[j];
                        if (glm::distance(places[j], places[i]) > carsize || glm::distance(places[i], start) >= glm::distance(places[j], start))
                            continue;
                        move = false;
                        break;
                    }
                }
            }
            else
            {
                int j = -1;
                for (vector<vehicle>::iterator it = pathers.begin(); it < pathers.end(); ++it)
                {
                    ++j;
                    if (i == j || pathers[j].waiting || (it->index != pathers[i].path.front() && !it->turning))
                        continue;
                    else if (it->turning)
                    {
                        if (it->path.front() == pathers[i].turn.second)
                        {
                            move = false;
                            break;
                        }
                        int index = -1;
                        for (int k = 0; k < streets[pathers[i].turn.first].neighbors.size(); ++k)
                        {
                            if(pathers[i].turn.second == streets[pathers[i].turn.first].neighbors[k])
                            {
                                index = k;
                                break;
                            }
                        }
                        if (index < 0)
                            exit(1);
                        for (int k = 0; k < streets[pathers[i].turn.first].intersects[index].size(); ++k)
                        {
                            if (streets[pathers[i].turn.first].intersects[index][k].first == it->turn.first
                                && streets[pathers[i].turn.first].intersects[index][k].second == it->turn.second)
                                {
                                    move = false;
                                    break;
                                }
                        }
                    }
                    else
                    {
                        if (glm::distance(places[j], it->start) > carsize)
                            continue;
                        move = false;
                        break;
                    }
                }
            }
        }
        pathers[i].increment(elapsed);
        if (move)
        {
            pathers[i].progress += elapsed;
            if (pathers[i].progress >= pathers[i].dist)
            {
                pathers[i].progress = 0;
                if (pathers[i].turning)
                {
                    pathers[i].turning = false;
                    pathers[i].waiting = false;
                    //if (pathers[i].pos < pathers[i].edgers.size())
                    {
                        pathers[i].start = streets[pathers[i].index].v1;
                        pathers[i].dir = normalize(streets[pathers[i].index].v2-pathers[i].start);
                        pathers[i].dist = glm::distance(pathers[i].start,streets[pathers[i].index].v2);
                    }
                }
                else if (pathers[i].waiting)
                {
                    pathers[i].waiting = false;
                    streets[pathers[i].index].traffic -= 20.0;
                    pathers[i].index = pathers[i].path.front();
                    streets[pathers[i].index].traffic += 20.0;
                    pathers[i].dir = normalize(streets[pathers[i].index].v1-pathers[i].start);
                    pathers[i].dist = glm::distance(streets[pathers[i].index].v1,pathers[i].start)+0.001;
                    pathers[i].turning = true;
                }
                else
                {
                    pathers[i].waiting = true;
                    pathers[i].dist = 0.3;
                    pathers[i].start = streets[pathers[i].index].v2;
                    //pathers[i].dir = normalize(edges[pathers[i].edgers[pathers[i].pos+1]].v1-pathers[i].start);
                    //pathers[i].pos += 1;
                    pathers[i].path.erase(pathers[i].path.begin());
                    if (0 < pathers[i].path.size())
                    {
                        {
                            vertex_descriptor s = vertex(pathers[i].path.front(), g);
                            vertex_descriptor t = vertex(pathers[i].destination, g);
                            vector<int> shortest_path;
                            /*dijkstra_shortest_paths(g, s,
                                                    pd);
                            */
                              try {
                            // call astar named parameter interface
                            astar_search
                              (g, s,
                               distance_heuristic<graph_t, float, vector<street*>>
                                (streetsp, t),
                               pd.visitor(astar_goal_visitor<vertex_descriptor>(t)));
                              } catch(found_goal fg) { // found a path to the goal
                                for(vertex_descriptor v = t;; v = p[v]) {
                                  shortest_path.push_back(v);
                                  if(p[v] == v)
                                    break;
                                }
                                reverse(shortest_path.begin(), shortest_path.end());
                              }
                               pathers[i].path = shortest_path;
                        }
                        pathers[i].turn = Edge(pathers[i].index,pathers[i].path.front());
                    }
                }
            }
        }
    }
    return;
}
