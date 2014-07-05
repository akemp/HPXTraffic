#include <physics/geometry.hpp>


#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point_xy.hpp>

#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_real.hpp>


// Include GLM
#include <glm/glm.hpp>

using namespace boost; 
using namespace glm;
using namespace std;
typedef boost::minstd_rand base_generator_type;

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
            roadsegs.push_back(seg);
    }
     return;
}
vec2 intersection(vec2 p1, vec2 p2, vec2 p3, vec2 p4) {
    // Store the values for fast access and easy
    // equations-to-code conversion
    float x1 = p1.x, x2 = p2.x, x3 = p3.x, x4 = p4.x;
    float y1 = p1.y, y2 = p2.y, y3 = p3.y, y4 = p4.y;
 
    float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    // If d is zero, there is no intersection
    if ((d == 0) || (abs(d) < 0.001f))
    {
        return p1;
    }
 
    // Get the x and y
    float pre = (x1*y2 - y1*x2), post = (x3*y4 - y3*x4);
    float x = ( pre * (x3 - x4) - (x1 - x2) * post ) / d;
    float y = ( pre * (y3 - y4) - (y1 - y2) * post ) / d;
    // Return the point of intersection
    vec2 ret = vec2(x,y);
    return ret;
}
pair<int,Node*> getInd(Node* n1, Node* n2)
{
    int ind;
    Point p = n2->center;
    double phi = atan2(-n1->center.y()+p.y(),-n1->center.x()+p.x());

    sort(n2->intersects.begin(), n2->intersects.end(),
        [phi,p](const boost::tuple<std::pair<Node*,Node*>,bool,int>* n1z, const boost::tuple<std::pair<Node*,Node*>,bool,int>* n2z) -> bool
    {
        double phi1 = atan2(n1z->get<0>().second->center.y()-p.y(),n1z->get<0>().second->center.x()-p.x());
        double phi2 = atan2(n2z->get<0>().second->center.y()-p.y(),n2z->get<0>().second->center.x()-p.x());
       
        phi1 -= phi;
        phi2 -= phi;
       
        phi1 = atan2(sin(phi1),cos(phi1));
        phi2 = atan2(sin(phi2),cos(phi2));
        return (phi1 > phi2);
    } );
    for (int i = 0; i < n2->intersects.size()-1; ++i)
    {
        if (boost::geometry::distance(n2->intersects[i]->get<0>().second->center,n1->center) < 0.001)
        {
            n2->intersects.push_back(n2->intersects[i]);
            n2->intersects.erase(n2->intersects.begin() + i);
            break;
        }
    }
    for (int i = 0; i < n2->intersects.size(); ++i)
    {
        if (n2->intersects[i]->get<1>())
            continue;
        n2->intersects[i]->get<1>() = true;
        ind = n2->intersects[i]->get<0>().second->index;
        n2 = n2->intersects[i]->get<0>().second;
        return pair<int,Node*>(ind,n2);
    }
    return pair<int,Node*>(-1,n2);
}

vector<int> getRing(Node* n1, Node* n2)
{
    
    int target = n2->index;
    vector<int> ring;
    int ind = n2->index;
    while ((ind != target || ring.size() <= 0) && ind != -1)
    {
        ring.push_back(ind);
        pair<int,Node*> val = getInd(n1,n2);
        ind = val.first;
        n1 = n2;
        n2 = val.second;
    }
    if (ind == -1)
        return vector<int>();
    //ring.push_back(ind);
    return ring;
}

void splitLine(LineMesh& roadsegs)
{
    LineMesh mesh;
    int count = 0;
    for (int i = 0; i < roadsegs.second.size(); ++i)
    {
        Line line1;
        line1.push_back(Point(roadsegs.first[roadsegs.second[i].first].x,roadsegs.first[roadsegs.second[i].first].y));
        line1.push_back(Point(roadsegs.first[roadsegs.second[i].second].x,roadsegs.first[roadsegs.second[i].second].y));
        Line seg;
        vector<vector<Point>> pts;
        for (int j = 0; j < roadsegs.second.size(); ++j)
        {
            if (j == i)
                continue;
            Line line2;
            line2.push_back(Point(roadsegs.first[roadsegs.second[i].first].x,roadsegs.first[roadsegs.second[i].first].y));
            line2.push_back(Point(roadsegs.first[roadsegs.second[i].second].x,roadsegs.first[roadsegs.second[i].second].y));
            vector<Point> ints = GetLineLineIntersections(line1,line2);
            if (ints.size() > 0)
            {
                vector<Point> temp;
                temp.push_back(ints[0]);
                temp.push_back(line1.front());
                pts.push_back(temp);
            }
        }
        sort(pts.begin(), pts.end(), [](const vector<Point>& p1, const vector<Point>& p2)
        {return boost::geometry::distance(p1.front(), p1.back()) < boost::geometry::distance(p2.front(), p2.back());});
        for (int j = 0; j < pts.size(); ++j)
        {
            seg.push_back(pts[j][0]);
        }
        for (int k = 0; k < seg.size(); ++k)
        {
            mesh.first.push_back(vec2(seg[k].x(),seg[k].y()));
            mesh.second.push_back(Edge(count,count+1));
            ++count;
        }
        ++count;
    }
     return;
}

LineMesh generateRoads(int dim, int offset, float size)
{
    vector<Line> spotsl;
    
    for (float i = 0; i < dim*offset; ++i)
    {
        Line l;
        vec2 p1(i*size, 0);
        vec2 p2(i*size, dim * size * offset );

        l.push_back(Point(p1.x, p1.y));
        l.push_back(Point(p2.x, p2.y));
        spotsl.push_back(l);
    }
    for (float i = 0; i < dim; ++i)
    {
        Line l;
        vec2 p1(0.0f, i * size * offset);
        vec2 p2(dim * size * offset, i * size * offset);


        l.push_back(Point(p1.x, p1.y));
        l.push_back(Point(p2.x, p2.y));
        spotsl.push_back(l);
    }
    vector<Line> roadsegs;
    splitLine(spotsl, roadsegs);
    
    LineMesh lm;
    int count = 0;
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
                if (glm::distance(lm.first[k], segf) < 0.001f)
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
                if (glm::distance(lm.first[k], segb) < 0.001f)
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

        }
    }
    
	return lm;
}


void generateRoadModels(const LineMesh& linemesh,  vector<unsigned int>& indices, vector<VertexData>& vertex_data, vector<road>& roads)
{
    LineMesh roadsegs = linemesh;
    
    for (int i = 0; i < roadsegs.first.size(); ++i)
    {
        roadsegs.first[i].x += rand()%120;
        roadsegs.first[i].y += rand()%120;
    }

    vector<Edge> edges = roadsegs.second;
    
    vector<Node> nodes;

    for (int i = 0; i < roadsegs.first.size(); ++i)
    {
        Node temp;
        temp.center = Point(roadsegs.first[i].x, roadsegs.first[i].y);
        temp.index = i;
        nodes.push_back(temp);
    }
    
    list<boost::tuple<pair<Node*,Node*>,bool,int>> intersects;
    roads.resize(edges.size() * 2);
    
    for (int i = 0; i < edges.size(); ++i)
    {
        int i1 = edges[i].first;
        int i2 = edges[i].second;
        
        intersects.push_back(boost::make_tuple(pair<Node*,Node*>(&nodes[i1],&nodes[i2]),false,i*2));
        nodes[i1].intersects.push_back(&intersects.back());

        intersects.push_back(boost::make_tuple(pair<Node*,Node*>(&nodes[i2],&nodes[i1]),false,i*2+1));
        nodes[i2].intersects.push_back(&intersects.back());
        
    }


    vector<vector<int>> rings;

    float scaler = 0.0105f;
    for (auto it = intersects.begin(); it != intersects.end(); ++it)
    {
        
        Node* n1 = it->get<0>().first;
        Node* n2 = it->get<0>().second;
        int ind = it->get<2>();
        roads[ind].v0 = vec2(n1->center.x(),n1->center.y());
        roads[ind].v1 = vec2(n2->center.x(),n2->center.y());
        roads[ind].index = ind;
        vec2 dir = glm::normalize(roads[ind].v1-roads[ind].v0);
        roads[ind].dir = dir;
        float phi = atan2(dir.y,dir.x) + HALF_PI;
        
        vec2 adder = normalize(vec2(cos(phi),sin(phi)));
                
        roads[ind].v0 += adder*15.0f/2.0f;
        roads[ind].v1 += adder*15.0f/2.0f;
        
        roads[ind].v0 *= scaler;
        roads[ind].v1 *= scaler;

        for (int i = 0; i < n2->intersects.size(); ++i)
        {
            int ind2 = n2->intersects[i]->get<2>();
            roads[ind].links.push_back(&roads[ind2]);
        }

        if (!it->get<1>())
        {
            vector<int> ring = getRing(n1,n2);
            
            if (ring.size() > 0)
                rings.push_back(ring);
        }
    }
    vector<vector<vector<vec2>>> quads;

    for (int i = 0; i < rings.size(); ++i)
    {
        vector<vector<vec2>> quad;
        
        vector<Node> inners;
        for (int j = 0; j < rings[i].size(); ++j)
        {
            Node temp = nodes[rings[i][j]];
            inners.push_back(temp);
        }

        vector<vec2> innerring;
        for (int j = 0; j < inners.size(); ++j)
        {
            int ind1 = j;
            int ind2 = (j+1)%inners.size();
            int ind3 = (j+2)%inners.size();
            int ind4 = (j+3)%inners.size();
            vec2 e1(inners[ind1].center.x(),inners[ind1].center.y());
            vec2 e2(inners[ind2].center.x(),inners[ind2].center.y());
            vec2 e3(inners[ind3].center.x(),inners[ind3].center.y());
            vec2 e4(inners[ind4].center.x(),inners[ind4].center.y());
                
            vec2 l0 = normalize(e3-e2);
            vec2 l1 = normalize(e2-e1);
            vec2 l2 = normalize(e4-e3);
            
            vec2 v1 = e2;
            vec2 v2 = e3;

            float phi = atan2(l0.y,l0.x) + HALF_PI;
            float phi1 = atan2(l1.y,l1.x) + HALF_PI;
            float phi2 = atan2(l2.y,l2.x) + HALF_PI;

            vec2 adder = normalize(vec2(cos(phi),sin(phi)));
                
            vec2 v3 = v2 + adder*15.0f;
            vec2 v4 = v1 + adder*15.0f;
            vec2 adder1 = normalize(vec2(cos(phi1),sin(phi1)));
            vec2 adder2 = normalize(vec2(cos(phi2),sin(phi2)));
            e1 += adder1*15.0f;
            e4 += adder2*15.0f;

            v3 = intersection(v3,v3+l0,e4-l2,e4+l2);
            v4 = intersection(v4,v4+l0,e1-l1,e1+l1);

            {
                vector<vec2> p2s;

                p2s.push_back(v1 * scaler);
                p2s.push_back(v2 * scaler);
                p2s.push_back(v3 * scaler);
                p2s.push_back(v4 * scaler);

                std::for_each(p2s.begin(), p2s.end(), [innerring](vec2& current){
                    for (auto it = innerring.begin(); it < innerring.end(); ++it)
                    {
                        if (glm::distance(*it,current) < 0.1f)
                        {
                            current = *it;
                            break;
                        }
                    }
                });

                innerring.insert(innerring.end(), p2s.begin(), p2s.end());

                quad.push_back(p2s);
            }
        }
        quads.push_back(quad);
    }
    
    int count = 0;
    VertexData temp;
    temp.texInd[0] = 1;

    for (int i = 0; i < quads.size(); ++i)
    {
        for (int j = 0; j < quads[i].size(); ++j)
        {
            vector<vec2> p2s = quads[i][j];
            
            for (int k = 0; k < p2s.size(); ++k)
            {
                vec2 pt = p2s[k];
                temp.position[0] = pt[0];
                temp.position[1] = 0;
                temp.position[2] = pt[1];
                temp.textureCoord[0] = pt[0];
                temp.textureCoord[1] = pt[1];
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

    return;
}