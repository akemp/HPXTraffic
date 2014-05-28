#include <physics/geometry.hpp>


#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point_xy.hpp>

using namespace boost; 
using namespace glm;
using namespace std;


vec2 intersect(vec2 p1, vec2 p2, vec2 p3, vec2 p4 )
{
    // Store the values for fast access and easy
    // equations-to-code conversion
    float x1 = p1.x, x2 = p2.x, x3 = p3.x, x4 = p4.x;
    float y1 = p1.y, y2 = p2.y, y3 = p3.y, y4 = p4.y;
 
    float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    // If d is zero, there is no intersection
    if (d == 0) return vec2();
 
    // Get the x and y
    float pre = (x1*y2 - y1*x2), post = (x3*y4 - y3*x4);
    float x = ( pre * (x3 - x4) - (x1 - x2) * post ) / d;
    float y = ( pre * (y3 - y4) - (y1 - y2) * post ) / d;
 
 
    return vec2(x,y);
}

void splitLine(vector<Line> spotsl, vector<Line>& roadsegs)
{
     for (int i = 0; i < spotsl.size(); ++i)
    {
        Line line1 = spotsl[i];
        Line seg;
        vector<vector<Point>> pts;
        for (int j = 0; j < spotsl.size(); ++j)
        {
            if (j == i)
                continue;
            Line line2 = spotsl[j];
            vector<Point> ints = GetLineLineIntersections(line1,line2);
            if (ints.size() > 0)
            {
                vector<Point> temp;
                temp.push_back(ints[0]);
                temp.push_back(spotsl[i].front());
                pts.push_back(temp);
            }
        }
        sort(pts.begin(), pts.end(), [](const vector<Point>& p1, const vector<Point>& p2)
        {return boost::geometry::distance(p1.front(), p1.back()) < boost::geometry::distance(p2.front(), p2.back());});
        for (int j = 0; j < pts.size(); ++j)
        {
            seg.push_back(pts[j][0]);
        }
        if (seg.size() > 0)
            roadsegs.push_back(seg);
    }
     return;
}

LineMesh generateRoads(int dim, int offset, float size)
{
    vector<Line> spotsl;
    for (int i = 0; i < dim*offset; ++i)
    {
        Line l;
        vec2 p1(i*size, 0);
        vec2 p2(i*size, dim * size * offset );


        l.push_back(Point(p1.x, p1.y));
        l.push_back(Point(p2.x, p2.y));
        spotsl.push_back(l);
    }
    for (int i = 0; i < dim; ++i)
    {
        Line l;
        vec2 p1(0, i * size * offset);
        vec2 p2(dim * size * offset, i * size * offset);


        l.push_back(Point(p1.x, p1.y));
        l.push_back(Point(p2.x, p2.y));
        spotsl.push_back(l);
    }
    vector<Line> roadsegs;
    splitLine(spotsl, roadsegs);
    
    LineMesh lm;

    for (int i = 0; i < roadsegs.size(); ++i)
    {
        for (int j = 0; j < roadsegs[i].size()-1; ++j)
        {
            bool contains = false;
            vec2 segf = vec2(roadsegs[i][j].x(),roadsegs[i][j].y());
            int ind = 0;
            Edge e;
            for (int k = 0; k < lm.first.size(); ++k)
            {
                if (glm::distance(lm.first[k], segf) < 0.0001f)
                {
                    contains = true;
                    break;
                }
                ++ind;
            }
            if (!contains)
                lm.first.push_back(segf);
            e.first = ind;
            vec2 segb = vec2(roadsegs[i][j+1].x(),roadsegs[i][j+1].y());
            ind = 0;
            contains = false;
            for (int k = 0; k < lm.first.size(); ++k)
            {
                if (glm::distance(lm.first[k], segb) < 0.0001f)
                {
                    contains = true;
                    break;
                }
                ++ind;
            }
            if (!contains)
                lm.first.push_back(segb);
            e.second = ind;
            lm.second.push_back(e);

            int temp = e.first;
            e.first = e.second;
            e.second = temp;
            lm.second.push_back(e);
        }
    }

	return lm;
}


void generateRoadModels(const LineMesh& roadsegs,  vector<unsigned int>& indices, vector<VertexData>& vertex_data)
{
    vector<vector<vec3>> quads;

    vector<Node> nodes;
    for (int i = 0; i < roadsegs.first.size(); ++i)
    {
        Node n;
        n.index = i;
        n.center = roadsegs.first[i] + vec2(rand()%100-50,rand()%150-75);
        for (int j = 0; j < roadsegs.second.size(); ++j)
        {
            if (i == roadsegs.second[j].first)
            {
                n.intersects.push_back(pair<vec2,int>(vec2(0.0f,0.0f),roadsegs.second[j].second));
            }
        }
        nodes.push_back(n);
    }
    
    float scaler = 0.0105f;

    int count = 0;

    for (int i = 0; i < nodes.size(); ++i)
    {
        vec2 v1 = nodes[i].center;

        vector<vec2> dirs;
        

        for (int j = 0; j < nodes[i].intersects.size(); ++j)
        {
            vec2 v2 = nodes[nodes[i].intersects[j].second].center;
        
            nodes[i].intersects[j].first = v2-v1;
            
            vec2 ov1 = v1;
            vec2 ov2 = v2;
        

            vec2 norm = normalize(v2-v1);
            
            ov1 += norm * 20.0f;
            ov2 -= norm * 20.0f;

            float phi = atan2(norm.y, norm.x);
            phi += 3.141592f/2.0f;
            float x = 10.0f*cos(phi);
            float y = 10.0f*sin(phi);
            vec2 adder(x,y);
        
            vec2 nv1 = ov1;
            vec2 nv2 = ov2;


            {
                ov1 += adder*2.0f;
                ov2 += adder*2.0f;
            
                vector<vec3> quad;
                quad.push_back(vec3(nv1[0],0, nv1[1]));
                quad.push_back(vec3(nv2[0],0, nv2[1]));
                quad.push_back(vec3(ov1[0],0, ov1[1]));
                quad.push_back(vec3(ov2[0],0, ov2[1]));
                quads.push_back(quad);
            }
            
        }


        {
            sort(nodes[i].intersects.begin(), nodes[i].intersects.end(),
                [](const pair<vec2,int>& v3, const pair<vec2,int>& v2){return (atan2(v3.first.y,v3.first.x) > atan2(v2.first.y,v2.first.x));});
            
            //nodes[i].intersects = dirs;
            vector<vec2> pts;

            vector<vec2> aim;
            for (int j = 0; j < nodes[i].intersects.size(); ++j)
            {
                aim.push_back(nodes[i].intersects[j].first);
            }

            
            if (aim.size() == 3)
            {
                vec2 avg;
                for (int j = 0; j < aim.size(); ++j)
                {
                    avg += normalize(aim[j]);
                }
                vec2 neg = normalize(avg);
                neg = -neg;
                aim.push_back(neg);
            }
            sort(aim.begin(), aim.end(), [](const vec2& v3, const vec2& v2){return (atan2(v3.y,v3.x) > atan2(v2.y,v2.x));});

            for (int j = 0; j < aim.size(); ++j)
            {
                vec2 p1 = aim[j];
                vec2 p2 = aim[(j+1)%aim.size()];

                vec2 p3 = vec2(0,0);
                vec2 p4 = vec2(0,0);

                {
                    vec2 norm = p1;
                    float phi = atan2(norm.y, norm.x);
                    phi += 3.141592f/2.0f;
                    float x = 20.0f*cos(phi);
                    float y = 20.0f*sin(phi);
                    vec2 adder(x,y);

                    p1 += adder;
                    p3 += adder;
                }
                {
                    vec2 norm = p2;
                    float phi = atan2(norm.y, norm.x);
                    phi -= 3.141592f/2.0f;
                    float x = 20.0f*cos(phi);
                    float y = 20.0f*sin(phi);
                    vec2 adder(x,y);

                    p2 += adder;
                    p4 += adder;
                }

                

                vec2 p = intersect(p1,p3,p2,p4);
                pts.push_back(p);
            }
            
            sort(pts.begin(), pts.end(), [](const vec2& v3, const vec2& v2){return (atan2(v3.y,v3.x) > atan2(v2.y,v2.x));});
            
            for (int k = 0; k < nodes[i].intersects.size(); ++k)
            {
                bool toggled = false;
                vec2 dir = nodes[i].intersects[k].first;
                float phi = atan2(dir.y,dir.x);
                for (int l = 0; l < pts.size()-1; ++l)
                {
                    pair<vec2,vec2> pt;
                    pt.first = pts[l];
                    pt.second = pts[l+1];
                    float phi1 = atan2(pt.first.y, pt.first.x);
                    float phi2 = atan2(pt.second.y, pt.second.x);
                    if (phi1 >= phi && phi2 <= phi)
                    {
                        nodes[i].corners.push_back(pt);
                        toggled = true;
                        break;
                    }
                }
                if (!toggled)
                {
                    nodes[i].corners.push_back(pair<vec2,vec2>(pts.back(),pts.front()));
                }
            
            }
        }
    }
    VertexData temp;
    temp.texInd[0] = 1;
    for (int i = 0; i < nodes.size(); ++i)
    {
        temp.texInd[0] += 1.2;
        if (temp.texInd[0] > 10)
            temp.texInd[0] -= 9.0;
        {
            temp.position[0] = 0;
            temp.position[1] = 0;
            temp.position[2] = 0;
            temp.normal[0] = 0;
            temp.normal[1] = 1;
            temp.normal[2] = 0;
            if (nodes[i].intersects.size() > 2)
                for (int j = 0; j < nodes[i].corners.size(); j++)
                {
                    vector<vec2> p2s;
                    vec2 v1 = nodes[i].center;
                    p2s.push_back((nodes[i].corners[j].first+v1)*scaler);
                    p2s.push_back((nodes[i].corners[j].second+v1)*scaler);
                    p2s.push_back(v1*scaler);
                    for (int k = 0; k < p2s.size(); ++k)
                    {
                        vec2 pt = p2s[k];
                        temp.position[0] = pt[0];
                        temp.position[2] = pt[1];
                        temp.textureCoord[0] = pt[0];
                        temp.textureCoord[1] = pt[1];
                        vertex_data.push_back(temp);
                    }
                    indices.push_back(count+2);
                    indices.push_back(count+0);
                    indices.push_back(count+1);
                    count += 3;
                }
            
            for (int j = 0; j < nodes[i].intersects.size(); ++j)
            {
                vector<vec3> quad;
                int ele = j;
                vec2 v = nodes[i].intersects[ele].first/2.0f;
                vec2 p1 = nodes[i].corners[j].first;
                vec2 v1 = nodes[i].center;
                int index = 0;
                
                vec2 p2 = nodes[i].corners[j].second;
                bool triggered = false;

                


                int indexer = nodes[i].intersects[ele].second;

                vec2 p3, p4;
                bool togged = false;
                for (int k = 0; k < nodes[indexer].intersects.size(); ++k)
                {
                    if (nodes[indexer].intersects[k].second == nodes[i].index)
                    {
                        p3 = nodes[indexer].corners[k].first;
                        p4 = nodes[indexer].corners[k].second;
                        togged = true;
                        break;
                    }
                }
                if(!togged)
                    exit(1);



                vec2 dir = nodes[indexer].center;
                
                vec2 t1 = (p1) + v1;
                vec2 t2 = (p2) + v1;
                
                vec2 t3 = (p3) + dir;
                vec2 t4 = (p4) + dir;

                Line l1;
                l1.push_back(Point(t2.x,t2.y));
                l1.push_back(Point(t4.x,t4.y));
                Line l2;
                l2.push_back(Point(t1.x,t1.y));
                l2.push_back(Point(t3.x,t3.y));

                if (GetLineLineIntersections(l1,l2).size() > 0)
                    swap(p3,p4);

                p1 += v1;
                p2 += v1;
                p3 += dir;
                p4 += dir;

                t1 = (p1+p2)/2.0f;
                t2 = (p3+p4)/2.0f;



                p1 = t1;
                p3 = t2;

                vector<vec2> pts;
                pts.push_back(p1);
                pts.push_back(p3);
                pts.push_back(p4);
                pts.push_back(p2);




                for (int k = 0; k < pts.size(); ++k)
                {
                    quad.push_back(vec3(pts[k].x, 0.0f, pts[k].y));
                }
                for (int k = 0; k < quad.size(); k++)
                {
                    vec3 pt = quad[k]*scaler;
                    temp.position[0] = pt[0];
                    temp.position[2] = pt[2];
                    temp.textureCoord[0] = pt[0];
                    temp.textureCoord[1] = pt[2];
                    vertex_data.push_back(temp);
                }
                indices.push_back(count+0);
                indices.push_back(count+1);
                indices.push_back(count+2);
                indices.push_back(count+0);
                indices.push_back(count+3);
                indices.push_back(count+2);
                count += 4;
            }
        }

    }
    return;
}