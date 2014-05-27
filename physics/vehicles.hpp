#ifndef VEHICLES
#define VEHICLES

#include "headers.hpp"

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/adapted/boost_tuple.hpp>

using namespace boost::geometry;

BOOST_GEOMETRY_REGISTER_BOOST_TUPLE_CS(cs::cartesian)
typedef model::point<double, 2, cs::cartesian> pointxy;

struct Car
{
    Car(Mesh m, Mesh s, double ts, vector<pair<double,double>> p, int tid, vec3 postMove)
    {
        progress = 0.01;
        timescale = ts;
        resttime = 0;
        rot = 0;
        
        m.postMove = vec3(0,0,0);
        s.postMove = vec3(0,0,0);
        dest = vec2(0,0);

        for (int i = 0; i < p.size(); ++i)
        {
            vec3 moving = vec3(p[i].first+0.5,0,p[i].second+0.5);
            
            int high = i+1;
            int low = i;
            if (high > p.size()-1)
                high = p.size()-1;

            vec2 dir1 = vec2(p[high].first-p[low].first, p[high].second - p[low].second);
            double rot1 = atan2(dir1[0],dir1[1])*180.0/3.141592;

            glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0), moving) * glm::rotate(glm::mat4(1.0f), float(rot1), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::translate(glm::mat4(1.0), postMove);
	    
            vec4 spot = ModelMatrix * vec4(0, 0, 0,1.0);

            colpath.push_back(pair<double,double>(spot[0],spot[2]));
        }
        //path = p;
        mesh = m;
        shadow = s;
        loc = vec2(colpath[0].first,colpath[0].second);
        finished = false;
        id = tid;
        waittime = 0;

    };
    Car(){};
    // The main function that returns true if line segment 'p1q1'
    // and 'p2q2' intersect.


    void draw(double elapsed, kd_tree& index, vector<Car>& cars)
    {
        bool colliding = false;
        bool removewait = true;
        if (resttime <= 0)
        {
            double query_pt[2] = {mesh.move[0],mesh.move[2]};
        
		    const double radius = 20.0;

		    std::vector<std::pair<size_t,double> > ret_matches;
		    RadiusResultSet<double,size_t> resultSet(radius,ret_matches);

		    const size_t nMatches = index.radiusSearch(&query_pt[0],radius, ret_matches, nanoflann::SearchParams());
            
            model::linestring<pointxy> l1;
            {
                int lele = colpath.size()-1;
                int low = std::min<int>(floor(progress), lele);
                int high = std::min<int>(ceil(progress), lele);
                int vhigh = std::min<int>(high+1,lele);
                double diff = std::min<double>(progress-floor(progress), 1.0);
                vec2 moving = vec2(colpath[low].first*(1.0-diff)+colpath[high].first*diff,colpath[low].second*(1.0-diff)+colpath[high].second*diff);
            
                l1.push_back(pointxy(moving[0],moving[1]));
                l1.push_back(pointxy(colpath[high].first,colpath[high].second));
                l1.push_back(pointxy(colpath[vhigh].first,colpath[vhigh].second));
            }

            for (size_t k=0;k<nMatches;k++)
            {
                if (id != ret_matches[k].first && !cars[ret_matches[k].first].finished)
                {
                    Car* t = &cars[ret_matches[k].first];
                    model::linestring<pointxy> l2;
                    {
                        int lele = t->colpath.size()-1;
                        int low = std::min<int>(floor(progress), lele);
                        int high = std::min<int>(ceil(progress), lele);
                        int vhigh = std::min<int>(high+1,lele);
                        double diff = std::min<double>(progress-floor(progress), 1.0);
                        vec2 moving = vec2(t->colpath[low].first*(1.0-diff)+t->colpath[high].first*diff,t->colpath[low].second*(1.0-diff)+t->colpath[high].second*diff);
            
                        l2.push_back(pointxy(moving[0],moving[1]));
                        l2.push_back(pointxy(t->colpath[high].first,t->colpath[high].second));
                        l2.push_back(pointxy(t->colpath[vhigh].first,t->colpath[vhigh].second));
                    }
                    bool b = boost::geometry::intersects(l1, l2);
                    if (b)
                        colliding = true;


                }
            }
        }
        //if (!colliding)
            progress += elapsed/timescale;
        
        if (!finished)
        {
            
            int lele =colpath.size()-1;
            int low = std::min<int>(floor(progress), lele);
            int high = std::min<int>(low+1, lele);
            int vhigh = std::min<int>(high+1,lele);
            double diff = std::min<double>(progress-floor(progress), 1.0);
            
            if (low == lele)
                finished = true;
            vec2 moving;
            if (low != high || low <= 0)
                moving = vec2(colpath[low].first*(1.0-diff)+colpath[high].first*diff,colpath[low].second*(1.0-diff)+colpath[high].second*diff);
            else
                moving = vec2(colpath[high-1].first*(1.0-diff)+colpath[high].first*diff,colpath[high-1].second*(1.0-diff)+colpath[high].second*diff);


            if (vhigh > high)
            {
                vec2 dir1 = vec2(colpath[high].first-colpath[low].first,colpath[high].second -colpath[low].second);
                double rot1 = atan2(dir1[0],dir1[1])*180.0/3.141592;
                vec2 dir2 = vec2(colpath[vhigh].first-colpath[high].first,colpath[vhigh].second -colpath[high].second);
                double rot2 = atan2(dir2[0],dir2[1])*180.0/3.141592;
                mesh.rot[1] = rot1*(1.0-diff)+rot2*diff;
                dest =  dir1*float(1.0f-diff);
                dest += rot2*float(diff);
                //dest = vec2(path[vhigh].first,path[vhigh].second);

            }
            else
            {
                //vec2 dir1 = vec2(colpath[high].first-moving[high-1],colpath[high].second-moving[1]);
                //double rot1 = atan2(dir1[0],dir1[1])*180.0/3.141592;
                //mesh.rot[1] = rot1;
                //dest = dir1;
                //dest = vec2(path[vhigh].first,path[vhigh].second);
            }


            mesh.move = vec3(moving[0], -3.999, moving[1]);

            loc = vec2(mesh.move[0],mesh.move[2]);

            rot = mesh.rot[1];

            shadow.move = mesh.move;
            shadow.rot = mesh.rot;
        
        }    
        shadow.draw();
        if (!finished)
        mesh.draw();
    }
    double progress, timescale, waittime;
    vec2 loc, dest;
    //vector<pair<double,double>>colpath;
    vector<pair<double,double>> colpath;
    Mesh mesh;
    Mesh shadow;
    bool finished;
    int id;
    double resttime;
    double rot;
};


#endif