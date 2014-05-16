
struct vehicle
{
    int destination;
    Edge turn;
    vec2 start;
    vec2 dir;
    int index;

    vector<int> edgers;
    int pos;
    vec2 vel;

    float progress;
    float dist;
    bool waiting;
    bool turning;
};

 
struct street
{
    vec2 v1;
    vec2 v2;

    vector<int> neighbors;
    vector<vector<Edge>> intersects;

    street(){};
    street(edger edge)
    {
        v1 = edge.v1;
        v2 = edge.v2;
        neighbors = edge.neighbors;
    };

};

void processCars(vector<Mesh>& cars, vector<vehicle>& pathers,
    const vector<street>& streets, const float scaler, const float elapsed)
{
    vector<vec2> places;
    vector<vec2> vels;

    float carsize = 0.45;

    for (int i = 0; i < pathers.size(); ++i)
    {
        if (pathers[i].pos >= pathers[i].edgers.size())
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
                for (int j = 0; j < pathers.size(); ++j)
                {
                    if (i == j || pathers[j].index != pathers[i].index)
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
                for (int j = 0; j < pathers.size(); ++j)
                {
                    if (i == j || pathers[j].waiting || (pathers[j].index != pathers[i].edgers[pathers[i].pos] && !pathers[j].turning))
                        continue;
                    else if (pathers[j].turning)
                    {
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
                            if (streets[pathers[i].turn.first].intersects[index][k].first == pathers[j].turn.first
                                && streets[pathers[i].turn.first].intersects[index][k].second == pathers[j].turn.second)
                                {
                                    move = false;
                                    break;
                                }
                        }
                    }
                    else
                    {
                        vec2 place2 = places[j];
                        if (glm::distance(places[j], pathers[j].start) > carsize)
                            continue;
                        move = false;
                        break;
                    }
                }
            }
        }
        if (move)
        {
            pathers[i].progress += elapsed;
            if (pathers[i].progress >= pathers[i].dist)
            {
                pathers[i].progress = 0;
                if (pathers[i].turning)
                {
                    pathers[i].turning = false;
                    if (pathers[i].pos < pathers[i].edgers.size())
                    {
                        pathers[i].start = streets[pathers[i].index].v1;
                        pathers[i].dir = normalize(streets[pathers[i].index].v2-pathers[i].start);
                        pathers[i].dist = glm::distance(pathers[i].start,streets[pathers[i].index].v2);
                    }
                }
                else if (pathers[i].waiting)
                {
                    pathers[i].waiting = false;
                    pathers[i].index = pathers[i].edgers[pathers[i].pos];
                    pathers[i].dir = normalize(streets[pathers[i].index].v1-pathers[i].start);
                    pathers[i].dist = glm::distance(streets[pathers[i].index].v1,pathers[i].start)+0.001;
                    pathers[i].turning = true;
                }
                else
                {
                    pathers[i].waiting = true;
                    pathers[i].dist = 0.5;
                    pathers[i].start = streets[pathers[i].index].v2;
                    //pathers[i].dir = normalize(edges[pathers[i].edgers[pathers[i].pos+1]].v1-pathers[i].start);
                    pathers[i].pos += 1;
                    pathers[i].turn = Edge(pathers[i].index,pathers[i].edgers[pathers[i].pos]);
                }
            }
        }
    }
    return;
}
