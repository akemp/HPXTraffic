#ifndef HEADERS
#define HEADERS

//#include <hpx/hpx_init.hpp>

#include <traffic/graphics.hpp>
#include <traffic/geometry.hpp>
#include <traffic/pathing.hpp>
#include <traffic/runtime.hpp>

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include <queue>
#include <fstream>
/*
#include <boost/random/normal_distribution.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/mersenne_twister.hpp>
*/




void generateQuads(vector<vector<vec3>> quads, vector<Edge> egs, vector<vec2> inputted, int& count, float scaler,
                   vector<unsigned int>& indices, vector<VertexData>& vertex_data)
{
    
    vector<pair<vec3, vector<vec3>>> inputs;

    for (int i = 0; i < inputted.size(); ++i)
    {
        vec2 spot = inputted[i];
        pair<vec3, vector<vec3>> temp;
        temp.first = vec3(spot[0],0,spot[1]);
        vector<vec3> places;
        for (int j = 0; j < egs.size(); ++j)
        {
            if (glm::distance(spot, inputted[egs[j].first]) < 0.01)
            {
                vec2 p = inputted[egs[j].second];
                places.push_back(vec3(p[0],0,p[1]));
            }
        }
        temp.second = places;
        inputs.push_back(temp);
    }


    for (int i = 0; i < inputs.size(); ++i)
    {
        vec3 cent = inputs[i].first;
        vector<vec3> newinputs;
        for (int j = 0; j < inputs[i].second.size(); ++j)
        {
            vec3 norm = normalize(inputs[i].second[j]-cent);
            {
                float phi = atan2(norm.z, norm.x);
                phi += 3.141592f/2.0f;
                float x = 20.0f*cos(phi);
                float y = 20.0f*sin(phi);
                vec2 adder(x,y);
                newinputs.push_back(norm * 20.0f + cent + vec3(adder[0],0,adder[1]));
            }
            {
                float phi = atan2(norm.z, norm.x);
                phi -= 3.141592f/2.0f;
                float x = 20.0f*cos(phi);
                float y = 20.0f*sin(phi);
                vec2 adder(x,y);
                newinputs.push_back(norm * 20.0f + cent + vec3(adder[0],0,adder[1]));
            }
        }    
        inputs[i].second = newinputs;    
    }

    for (int i = 0; i < inputs.size(); ++i)
    {
        vec3 pt = inputs[i].first*scaler;
        VertexData temp;
        temp.position[0] = pt[0];
        temp.position[1] = pt[1];
        temp.position[2] = pt[2];
        temp.normal[0] = 0;
        temp.normal[1] = 1;
        temp.normal[2] = 0;
        temp.texInd[0] = 0;
        temp.textureCoord[0] = pt[0];
        temp.textureCoord[1] = pt[2];
        vertex_data.push_back(temp);
        int current = count;
        ++count;
        for (int j = 0; j < inputs[i].second.size(); j += 2)
        {
            indices.push_back(current);
            for (int k = 0; k < 2; ++k)
            {
                pt = inputs[i].second[j+k]*scaler;
                temp.position[0] = pt[0];
                temp.position[2] = pt[2];
                temp.textureCoord[0] = pt[0];
                temp.textureCoord[1] = pt[2];
                vertex_data.push_back(temp);
                indices.push_back(count);
                ++count;
            }
        }
    }

    for (int i = 0; i < quads.size(); ++i)
    {
        VertexData temp;
        temp.position[0] = 0;
        temp.position[1] = 0;
        temp.position[2] = 0;
        temp.normal[0] = 0;
        temp.normal[1] = 1;
        temp.normal[2] = 0;
        temp.texInd[0] = 0;
        for (int j = 0; j < quads[i].size(); j ++)
        {
            vec3 pt = quads[i][j]*scaler;
            temp.position[0] = pt[0];
            temp.position[2] = pt[2];
            temp.textureCoord[0] = pt[0];
            temp.textureCoord[1] = pt[2];
            vertex_data.push_back(temp);
        }
        indices.push_back(count+2);
        indices.push_back(count+3);
        indices.push_back(count+1);
        indices.push_back(count+2);
        indices.push_back(count);
        indices.push_back(count+1);
        count += 4;
    }
    return;

}



//This section of code and parts of its implementation are copied from http://www.richelbilderbeek.nl

bool sorter(vector<Point> &p1, vector<Point> &p2)
{
    return (boost::geometry::distance(p1.front(), p1.back()) < boost::geometry::distance(p2.front(), p2.back()));
}

vector<Line> generateRoads(int dim, int offset, float size)
{
    vector<Line> spotsl;
    for (int i = 0; i < dim*offset; ++i)
    {
        /*
        boost::random::mt19937 rng;         // produces randomness out of thin air
                                        // see pseudo-random number generators
    
        boost::normal_distribution<double> noise(-1.0,1.0);
        boost::variate_generator<boost::mt19937, 
            boost::normal_distribution<double> > nD(rng, noise);*/
        Line l;
        l.push_back(Point(i*size, 0));
        l.push_back(Point(i*size, dim * size * offset ));
        spotsl.push_back(l);
    }
    vector<Line> spotsw;
    for (int i = 0; i < dim; ++i)
    {
        Line l;
        l.push_back(Point(0, i * size * offset));
        l.push_back(Point(dim * size * offset, i * size * offset));
        spotsw.push_back(l);
    }
    vector<Line> roadsegs;

    for (int i = 0; i < spotsl.size(); ++i)
    {
        Line line1 = spotsl[i];
        Line seg;
        vector<vector<Point>> pts;
        for (int j = 0; j < spotsw.size(); ++j)
        {
            Line line2 = spotsw[j];
            vector<Point> ints = GetLineLineIntersections(line1,line2);
            if (ints.size() > 0)
            {
                vector<Point> temp;
                temp.push_back(ints[0]);
                temp.push_back(spotsl[i].front());
                pts.push_back(temp);
            }
        }
        sort(pts.begin(), pts.end(), sorter);
        for (int j = 0; j < pts.size(); ++j)
        {
            seg.push_back(pts[j][0]);
        }
        roadsegs.push_back(seg);
    }
    
    for (int i = 0; i < spotsw.size(); ++i)
    {
        Line line1 = spotsw[i];
        Line seg;
        vector<vector<Point>> pts;
        for (int j = 0; j < spotsl.size(); ++j)
        {
            Line line2 = spotsl[j];
            vector<Point> ints = GetLineLineIntersections(line1,line2);
            if (ints.size() > 0)
            {
                vector<Point> temp;
                temp.push_back(ints[0]);
                temp.push_back(spotsw[i].front());
                pts.push_back(temp);
            }
        }
        sort(pts.begin(), pts.end(), sorter);
        for (int j = 0; j < pts.size(); ++j)
        {
            seg.push_back(pts[j][0]);
        }
        roadsegs.push_back(seg);
    }
	return roadsegs;
}



void generateRoadModels(const vector<Line>& roadsegs, vector<vec2>& inputted, vector<pair<int,int>>& egs, vector<vector<vec3>>& quads, vector<edger>& edges)
{
    
    for (int i = 0; i < roadsegs.size(); ++i)
    {
        if (roadsegs[i].size() > 0)
        {
            for (int j = 0; j < roadsegs[i].size()-1; ++j)
            {
                pair<vec2,vec2> pt(vec2(roadsegs[i][j].x(),roadsegs[i][j].y()),vec2(roadsegs[i][j+1].x(),roadsegs[i][j+1].y()));
                if (glm::distance(pt.first, pt.second) < 0.0001)
                    continue;
                int indf = 0;
                int indb = 0;
                for (int k = 0; k < inputted.size(); ++k)
                {
                    if (glm::distance(inputted[k], pt.first) < 0.0001)
                    {
                        indf = k;
                        break;
                    }
                    ++indf;
                }
                if (indf >= inputted.size())
                    inputted.push_back(pt.first);

                for (int k = 0; k < inputted.size(); ++k)
                {
                    if (glm::distance(inputted[k], pt.second) < 0.0001)
                    {
                        indb = k;
                        break;
                    }
                    ++indb;
                }
                if (indb >= inputted.size())
                    inputted.push_back(pt.second);
                if (indf != indb)
                {
                    pair<int,int> tester(indb,indf);
                    if (find(egs.begin(), egs.end(), tester) != egs.end())
                        continue;
                    tester = pair<int,int>(indf,indb);
                    if (find(egs.begin(), egs.end(), tester) != egs.end())
                        continue;
                    egs.push_back(tester);
                    egs.push_back(pair<int,int>(indb,indf));
                }
            }
        }
    }
    

    
    for (int i = 0; i < egs.size(); ++i)
    {
        int ind = egs[i].second;
        edger e(egs[i]);
        for (int j = 0; j < egs.size(); ++j)
        {
            if (j == i)
                continue;
            if (ind == egs[j].first && egs[i].first != egs[j].second)
            {
                e.neighbors.push_back(j);
            }
        }

        vec2 v1 = inputted[e.edge.first];
        vec2 v2 = inputted[e.edge.second];
        
        vec2 ov1 = v1;
        vec2 ov2 = v2;
        

        vec2 norm = normalize(v2-v1);
        

        v1 += norm * 10.0f;
        v2 -= norm * 10.0f;


        float phi = atan2(norm.y, norm.x);
        phi += 3.141592/2.0;
        float x = 10.0*cos(phi);
        float y = 10.0*sin(phi);
        vec2 adder(x,y);
        
        vec2 nv1 = ov1;
        vec2 nv2 = ov2;

        v1 += adder;
        v2 += adder;

        e.v1 = v1;
        e.v2 = v2;
        {
            ov1 += adder*2.0f;
            ov2 += adder*2.0f;

            ov1 += norm * 20.0f;
            ov2 -= norm * 20.0f;
            
            nv1 += norm * 20.0f;
            nv2 -= norm * 20.0f;
            
            vector<vec3> quad;
            quad.push_back(vec3(nv1[0],0, nv1[1]));
            quad.push_back(vec3(nv2[0],0, nv2[1]));
            quad.push_back(vec3(ov1[0],0, ov1[1]));
            quad.push_back(vec3(ov2[0],0, ov2[1]));
            quads.push_back(quad);
        }

        edges.push_back(e);
    }

    return;
}

#endif