

 
void processCars(vector<Mesh>& cars, vector<vehicle>& pathers,
    vector<street>& streets, const float scaler, const float elapsed,
    const graph_t &g, 
    const property_map<graph_t, edge_weight_t>::type &weightmap, const vector<vertex_descriptor> &p,
    const vector<double>& d, const vector<street*>& streetsp,
    const pred_map& pd)
{

    float carsize = 0.45;
    int lastsize = pathers.size();
    {
        vector<vehicle>::iterator it = remove_if(pathers.begin(), pathers.end(),
            [](const vehicle& pather) {return(pather.index == pather.destination);});
        pathers.erase(it, pathers.end());
    }
    cars.resize(pathers.size());
    if (pathers.size() != lastsize)
    {
        for (int i = 0; i < streets.size(); ++i)
        {
            streets[i].cars = vector<pair<vehicle*,int>>();
        }
        for (int i = 0; i < pathers.size(); ++i)
        {
            pathers[i].streetloc->addvehicle(&pathers[i]);
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
        pathers[i].place = v;

        cars[i].rot.y = atan2(pathers[i].dir.x,pathers[i].dir.y)*180.0f/3.141592f;
        
    }
    for (int i = 0; i < pathers.size(); ++i)
    {
        bool move = true;
        if (!pathers[i].turning)
        {
            vec2 start = pathers[i].start;
            if (!pathers[i].waiting)
            {
                for (vector<pair<vehicle*,int>>::iterator it = pathers[i].streetloc->cars.begin(); it < pathers[i].streetloc->cars.end(); ++it)
                {
                    if ((*it).second == pathers[i].license)
                        continue;
                    else
                    {
                        if (glm::distance((*it).first->place, pathers[i].place) > carsize ||
                            glm::distance(pathers[i].place, start) >= glm::distance((*it).first->place, start))
                            continue;
                        move = false;
                        break;
                    }
                }
            }
            else
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
                for (vector<vehicle>::iterator it = pathers.begin(); it < pathers.end(); ++it)
                {
                    if (it->license == pathers[i].license || it->waiting || (!it->turning && it->index != pathers[i].path.front()))
                        continue;
                    else if (it->turning)
                    {
                        if (it->path.front() == pathers[i].turn.second)
                        {
                            move = false;
                            break;
                        }
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
                        if (glm::distance(it->place, it->start) > carsize)
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
                    pathers[i].waiting = false;
                    //if (pathers[i].pos < pathers[i].edgers.size())
                    {
                        pathers[i].start = streets[pathers[i].index].v1;
                        pathers[i].dir = streets[pathers[i].index].dir;
                        pathers[i].dist = streets[pathers[i].index].dist;
                    }
                }
                else if (pathers[i].waiting)
                {
                    pathers[i].waiting = false;

                    streets[pathers[i].index].traffic -= 20.0;

                    pathers[i].index = pathers[i].path.front();
                    pathers[i].streetloc->removevehicle(pathers[i].license);
                    pathers[i].streetloc = &streets[pathers[i].index];
                    pathers[i].streetloc->addvehicle(&pathers[i]);

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
                    //
                    {   
                        pathers[i].path = generatePath(pathers[i].index,pathers[i].destination,g,weightmap,p,d,streetsp,pd);
                        pathers[i].path.erase(pathers[i].path.begin());
                        if (0 < pathers[i].path.size())
                            pathers[i].turn = Edge(pathers[i].index,pathers[i].path.front());
                    }
                }
            }
        }
    }
    return;
}
