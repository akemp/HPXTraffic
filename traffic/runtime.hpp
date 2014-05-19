

 
void processCars(vector<Mesh>& cars, vector<vehicle>& pathers,
    vector<street>& streets, const float scaler, const float elapsed,
    const graph_t &g, 
    const property_map<graph_t, edge_weight_t>::type &weightmap, const vector<vertex_descriptor> &p,
    const vector<double>& d, const vector<street*>& streetsp,
    const pred_map& pd, 
    float carsize = 0.45,
    bool deleteafter = false)
{
    if (!deleteafter)
    {
        for (int i = 0; i < pathers.size(); ++i)
        {
            int count = i*341;
            while (pathers[i].index == pathers[i].destination)
            {
                pathers[i].destination = (pathers[i].destination + count)%streets.size();
                ++count;
            }
        }
    }
    else
    {
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
                streets[i].erasecars();
            }
            for (int i = 0; i < pathers.size(); ++i)
            {
                pathers[i].streetloc->addvehicle(&pathers[i]);
            }
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
        float dister = 0;
        //if (!pathers[i].turning)
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
                        if (glm::distance((*it).first->place, pathers[i].place)-pow(pathers[i].vel,2.0f)*4.0f > carsize ||
                            glm::distance(pathers[i].place, start) >= glm::distance((*it).first->place, start))
                            continue;
                        dister = pow(pathers[i].vel,2.0f)*4.0f+carsize-glm::distance((*it).first->place, pathers[i].place);
                        move = false;
                        break;
                    }
                }
            }
            else
            {
                for (vector<pair<vehicle*,int>>::iterator it = streets[pathers[i].turn.second].cars.begin();
                    it < streets[pathers[i].turn.second].cars.end(); ++it)
                {
                    if ((*it).first->waiting)
                    {
                        if ((*it).first->progress <= pathers[i].progress)
                            continue;
                        move = false;
                        break;
                    }
                    else if ((*it).first->turning)
                    {
                        move = false;
                        break;
                    }
                    else if (glm::distance((*it).first->place, (*it).first->start) < carsize)
                    {
                        move = false;
                        break;
                    }
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

                vector<pair<int,int>>* intersects = &streets[pathers[i].turn.first].intersects[index];
                
                for (int k = 0; k < intersects->size() && move; ++k)
                {
                    for (vector<pair<vehicle*,int>>::iterator it = streets[(*intersects)[k].second].cars.begin();
                        it < streets[(*intersects)[k].second].cars.end(); ++it)
                    {
                        
                        if ((*it).first->turning)
                        {
                            if ( (*intersects)[k].first == (*it).first->turn.first
                                && (*intersects)[k].second == (*it).first->turn.second)
                                {
                                    move = false;
                                    break;
                                }
                        }
                    }
                
                }
            }
        }
        if (pathers[i].waiting)
        {
            pathers[i].waited += elapsed;
            if (pathers[i].waited > 3.0f)
            {
                pathers[i].path = generatePath(pathers[i].index,pathers[i].destination,g,weightmap,p,d,streetsp,pd);
                pathers[i].turn = Edge(pathers[i].index,pathers[i].path);
                pathers[i].waited = 0;
            }
        }
        if (!move)
        {
            if (!pathers[i].waiting)
            {
                pathers[i].vel -= elapsed * (1.0f+dister);
                if (pathers[i].vel < 0)
                    pathers[i].vel = 0;
                if (pathers[i].turning)
                    pathers[i].vel = glm::max(pathers[i].vel, 0.05f);
                pathers[i].progress += pathers[i].vel * elapsed;
            }
        }
        else
        {
            if (pathers[i].waiting)
            {
                pathers[i].progress += elapsed;
            }
            else
            {
                float dist = pathers[i].dist;
                if (pathers[i].turning)
                    dist += pathers[i].streetloc->dist;
                float diff = dist-pathers[i].progress;
                //if (pathers[i].progress+pow(pathers[i].vel,2.0f) > dist)
                {
                    pathers[i].vel += elapsed*glm::min(1.0f-pathers[i].vel,0.5f)/4.0f;
                    pathers[i].vel = glm::min(1.0f,pathers[i].vel);
                    pathers[i].vel -= elapsed * (2.0f*pow(glm::max(pathers[i].vel*4.0f-diff,0.0f),2.0f));
                    pathers[i].vel = glm::max(0.05f,pathers[i].vel);
                }
                pathers[i].progress += pathers[i].vel * elapsed;
            }
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
                    pathers[i].waited = 0;

                    pathers[i].index = pathers[i].path;

                    pathers[i].streetloc->removevehicle(pathers[i].license);
                    pathers[i].streetloc = &streets[pathers[i].index];
                    pathers[i].streetloc->addvehicle(&pathers[i]);

                    pathers[i].dir = normalize(streets[pathers[i].index].v1-pathers[i].start);
                    pathers[i].dist = glm::distance(streets[pathers[i].index].v1,pathers[i].start)+0.001;

                    pathers[i].turning = true;
                }
                else
                {
                    pathers[i].waiting = true;
                    pathers[i].waited = 0;
                    pathers[i].dist = 0.2;
                    pathers[i].vel = 0;
                    pathers[i].start = streets[pathers[i].index].v2;
                    //pathers[i].dir = normalize(edges[pathers[i].edgers[pathers[i].pos+1]].v1-pathers[i].start);
                    //pathers[i].pos += 1;
                    //
                    {   
                        pathers[i].path = generatePath(pathers[i].index,pathers[i].destination,g,weightmap,p,d,streetsp,pd);
                        pathers[i].turn = Edge(pathers[i].index,pathers[i].path);
                    }
                }
            }
        }
    }
    return;
}
