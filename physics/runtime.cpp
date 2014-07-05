#include <physics\graphics.hpp>
#include <physics\pathing.hpp>
#include <physics\runtime.hpp>

void processCars(std::vector<Mesh>& cars, std::vector<vehicle>& pathers,
    std::vector<road>& streets, const float scaler, const float elapsed,
    const graph_t &g, 
    const boost::property_map<graph_t, boost::edge_weight_t>::type &weightmap, const std::vector<vertex_descriptor> &p,
    const std::vector<double>& d, const std::vector<road*>& streetsp,
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
                dist += pathers[i].streetloc->totaltime();
            float diff = dist-pathers[i].progress;
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
                {
                    pathers[i].start = pathers[i].streetloc->v0;
                    pathers[i].dir = pathers[i].streetloc->dir;
                    pathers[i].dist = pathers[i].streetloc->totaltime();
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

                pathers[i].dir = normalize(pathers[i].streetloc->v0-pathers[i].start);
                pathers[i].dist = glm::distance(pathers[i].streetloc->v0,pathers[i].start)+0.001f;

                pathers[i].turning = true;
            }
            else
            {

                pathers[i].waiting = true;
                pathers[i].waited = 0;
                pathers[i].dist = elapsed*10.0f;
                pathers[i].vel = 0;
                pathers[i].start = pathers[i].streetloc->v1;
                {   
                    pathers[i].path = generatePath(pathers[i].streetloc->index,pathers[i].destination,g,weightmap,p,d,streetsp,pd);
                    pathers[i].turn = Edge(pathers[i].streetloc->index,pathers[i].path);
                }
            }
        }
        
    }
    return;
}
