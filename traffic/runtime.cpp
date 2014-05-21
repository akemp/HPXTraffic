#include <traffic\graphics.hpp>
#include <traffic\pathing.hpp>
#include <traffic\runtime.hpp>

void processCars(std::vector<Mesh>& cars, std::vector<vehicle>& pathers,
    std::vector<street>& streets, const float scaler, const float elapsed,
    const graph_t &g, 
    const boost::property_map<graph_t, boost::edge_weight_t>::type &weightmap, const std::vector<vertex_descriptor> &p,
    const std::vector<double>& d, const std::vector<street*>& streetsp,
    const pred_map& pd, 
    float carsize,
    bool deleteafter)
{
    using namespace std;
    using namespace glm;
    if (!deleteafter)
    {
        for (int i = 0; i < pathers.size(); ++i)
        {
            int count = i*341;
            while (pathers[i].streetloc->index == pathers[i].destination)
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
            auto it = remove_if(pathers.begin(), pathers.end(),
                [](const vehicle& pather) {return(pather.streetloc->index == pather.destination);});
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
        float total = 0;
        for (float j = pathers[i].vel; j > 0; j -= elapsed*0.1f)
            total += j;
        //if (!pathers[i].turning)
        {
            vec2 start = pathers[i].start;
            float dist1 = glm::distance(pathers[i].place, start);
            float bufferzone = carsize+total*0.1f;
            if (!pathers[i].waiting)
            {
                for (auto it = pathers[i].streetloc->cars.begin(); it < pathers[i].streetloc->cars.end(); ++it)
                {
                    if ((*it).second == pathers[i].license)
                        continue;
                    else
                    {
                        if (glm::distance(it->first->place, pathers[i].place) > bufferzone-it->first->vel ||
                            dist1 >= glm::distance(it->first->place, start))
                            continue;
                        pathers[i].vel -= elapsed * (0.1f);
                        if (pathers[i].vel < 0.00f)
                            pathers[i].vel = 0.00f;
                        move = false;
                        break;
                    }
                }
                if (!pathers[i].turning && !pathers[i].waiting && bufferzone+pathers[i].progress > pathers[i].dist)
                {
                    if (pathers[i].path < 0)
                        pathers[i].path = generatePath(pathers[i].streetloc->index,pathers[i].destination,g,weightmap,p,d,streetsp,pd);
                    float diff = (bufferzone+pathers[i].progress)-pathers[i].dist;
                    for (auto it = streets[pathers[i].path].cars.begin();
                        it < streets[pathers[i].path].cars.end(); ++it)
                    {
                        if (!it->first->turning || it->first->progress > diff || it->first->turn.first != pathers[i].streetloc->index)
                        {
                            continue;
                        }
                        pathers[i].vel -= elapsed * (0.1f);
                        if (pathers[i].vel < 0.00f)
                            pathers[i].vel = 0.00f;
                        move = false;
                        break;
                    }
                }
            }
            else
            {
                for (auto it = streets[pathers[i].turn.second].cars.begin();
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
                for (int k = 0; k < pathers[i].streetloc->neighbors.size(); ++k)
                {
                    if (pathers[i].turn.second == pathers[i].streetloc->neighbors[k])
                    {
                        index = k;
                        break;
                    }
                }
                if (index < 0)
                    exit(1);

                vector<Edge>* intersects = &pathers[i].streetloc->intersects[index];
                
                for (int k = 0; k < intersects->size() && move; ++k)
                {
                    for (auto it = streets[(*intersects)[k].second].cars.begin();
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
            if (!move)
                pathers[i].waited += 1.0f;
            if (pathers[i].waited > 1000.0f)
            {
                pathers[i].path = generatePath(pathers[i].streetloc->index,pathers[i].destination,g,weightmap,p,d,streetsp,pd);
                pathers[i].turn = Edge(pathers[i].streetloc->index,pathers[i].path);
                pathers[i].waited = 0;
            }
            if (move)
            {
                pathers[i].progress += elapsed;
            }
        }
        if (move)
        {
            float dist = pathers[i].dist;
            if (pathers[i].turning)
                dist += pathers[i].streetloc->dist;
            float diff = dist-pathers[i].progress;
            //if (pathers[i].progress+pow(pathers[i].vel,2.0f) > dist)
            {
                pathers[i].vel = glm::min(2.0f,pathers[i].vel);
                if (diff < total*elapsed)
                {
                    pathers[i].vel -= elapsed*0.1f;
                }
                else
                    pathers[i].vel += elapsed*0.1f;
                pathers[i].vel = glm::max(0.0f,pathers[i].vel);
            }
        }
        if (!pathers[i].waiting)
            pathers[i].progress += pathers[i].vel * elapsed;
        if (pathers[i].progress >= pathers[i].dist)
        {
            pathers[i].progress = 0;
            if (pathers[i].turning)
            {
                pathers[i].turning = false;
                pathers[i].waiting = false;
                //if (pathers[i].pos < pathers[i].edgers.size())
                {
                    pathers[i].start = pathers[i].streetloc->v1;
                    pathers[i].dir = pathers[i].streetloc->dir;
                    pathers[i].dist = pathers[i].streetloc->dist;
                }
                pathers[i].path = generatePath(pathers[i].streetloc->index,pathers[i].destination,g,weightmap,p,d,streetsp,pd);
            }
            else if (pathers[i].waiting)
            {
                pathers[i].waiting = false;
                pathers[i].waited = 0;

                pathers[i].streetloc->removevehicle(pathers[i].license);
                pathers[i].streetloc = &streets[pathers[i].path];
                pathers[i].streetloc->addvehicle(&pathers[i]);

                pathers[i].dir = normalize(pathers[i].streetloc->v1-pathers[i].start);
                pathers[i].dist = glm::distance(pathers[i].streetloc->v1,pathers[i].start)+0.001f;

                pathers[i].turning = true;
            }
            else
            {

                pathers[i].waiting = true;
                pathers[i].waited = 0;
                pathers[i].dist = elapsed*10.0f;
                pathers[i].vel = 0;
                pathers[i].start = pathers[i].streetloc->v2;
                //
                {   
                    pathers[i].path = generatePath(pathers[i].streetloc->index,pathers[i].destination,g,weightmap,p,d,streetsp,pd);
                    pathers[i].turn = Edge(pathers[i].streetloc->index,pathers[i].path);
                }
            }
        }
        
    }
    return;
}
