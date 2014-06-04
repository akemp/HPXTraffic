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

pair<int,Node*> getInd(Node* n1, Node* n2)
{
    int ind;
    Point p = n2->center;
    double phi = atan2(-n1->center.y()+p.y(),-n1->center.x()+p.x());

    sort(n2->intersects.begin(), n2->intersects.end(),
        [phi,p](const pair<pair<Node*,Node*>,bool>* n1z, const pair<pair<Node*,Node*>,bool>* n2z) -> bool
    {
        double phi1 = atan2(n1z->first.second->center.y()-p.y(),n1z->first.second->center.x()-p.x());
        double phi2 = atan2(n2z->first.second->center.y()-p.y(),n2z->first.second->center.x()-p.x());
       
        phi1 -= phi;
        phi2 -= phi;
       
        phi1 = atan2(sin(phi1),cos(phi1));
        phi2 = atan2(sin(phi2),cos(phi2));
        return (phi1 > phi2);
    } );
    for (int i = 0; i < n2->intersects.size()-1; ++i)
    {
        if (boost::geometry::distance(n2->intersects[i]->first.second->center,n1->center) < 0.001)
        {
            n2->intersects.push_back(n2->intersects[i]);
            n2->intersects.erase(n2->intersects.begin() + i);
            break;
        }
    }
    for (int i = 0; i < n2->intersects.size(); ++i)
    {
        if (n2->intersects[i]->second)
            continue;
        n2->intersects[i]->second = true;
        ind = n2->intersects[i]->first.second->index;
        n2 = n2->intersects[i]->first.second;
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
    ring.push_back(ind);
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
        for (int k = 0; k < seg.size()-1; ++k)
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

        }
    }

	return lm;
}


void generateRoadModels(const LineMesh& linemesh,  vector<unsigned int>& indices, vector<VertexData>& vertex_data)
{
    LineMesh roadsegs = linemesh;

    for (int i = 0; i < roadsegs.first.size(); ++i)
    {
        roadsegs.first[i].x += rand()%190;
        roadsegs.first[i].y += rand()%190;
    }

    vector<Edge> edges;

    for (int i = 0; i < roadsegs.second.size(); ++i)
    {
        bool contains = false;
        Edge test = roadsegs.second[i];
        int i1 = test.first;
        int i2 = test.second;
        
        if (i1 == i2)
            continue;

        for (int j = i+1; j < roadsegs.second.size(); ++j)
        {
            if (roadsegs.second[j].first == test.first && roadsegs.second[j].second == test.second ||
                roadsegs.second[j].first == test.second && roadsegs.second[j].second == test.first)
            {
                contains = true;
                break;
            }

        }
        if (!contains)
            edges.push_back(test);
    }
    roadsegs.second = edges;

    vector<Node> nodes;

    for (int i = 0; i < roadsegs.first.size(); ++i)
    {
        Node temp;
        temp.center = Point(roadsegs.first[i].x, roadsegs.first[i].y);
        temp.index = i;
        nodes.push_back(temp);
    }
    
    list<pair<pair<Node*,Node*>,bool>> intersects;
    
    for (int i = 0; i < edges.size(); ++i)
    {
        int i1 = edges[i].first;
        int i2 = edges[i].second;
        
        intersects.push_back(pair<pair<Node*,Node*>,bool>(pair<Node*,Node*>(&nodes[i1],&nodes[i2]),false));
        nodes[i1].intersects.push_back(&intersects.back());
        intersects.push_back(pair<pair<Node*,Node*>,bool>(pair<Node*,Node*>(&nodes[i2],&nodes[i1]),false));
        nodes[i2].intersects.push_back(&intersects.back());
    }

    multipolygon polys;
    multilinestring lines;
    
    vector<vector<int>> rings;
    
    
    for (list<pair<pair<Node*,Node*>,bool>>::iterator it = intersects.begin(); it != intersects.end(); ++it)
    {
        if (!it->second)
        {
            Node* n1 = it->first.first;
            Node* n2 = it->first.second;
            vector<int> ring = getRing(n1,n2);
            if (ring.size() > 0)
                rings.push_back(ring);
        }
    }


    float scaler = 0.0105f;
    int count = 0;
    VertexData temp;
    temp.texInd[0] = 1;
    for (int i = 0; i < rings.size(); ++i)
    {

        polygon p;
        for (int j = 0; j < rings[i].size(); ++j)
        {
            boost::geometry::append(p,nodes[rings[i][j]].center);
        }
        
        boost::geometry::append(p,nodes[rings[i][0]].center);
        boost::geometry::correct(p);
        //for (int l = 0; l < p.inners().size(); ++l)
        {
            temp.texInd[0] += 0.55;
            if (temp.texInd[0] > 10.01)
                temp.texInd[0] -= 9.005;
            //vector<Point> inners = p.inners()[l];
            vector<Point> inners = p.outer();
            Point vn;
            //boost::geometry::centroid(p.inners()[l],vn);
            boost::geometry::centroid(p.outer(),vn);
            for (int k = 0; k < inners.size(); ++k)
            {
                vector<vec2> p2s;
                vec2 vs = vec2(vn.x(),vn.y());
                p2s.push_back(vec2(inners[k].x(),inners[k].y())*scaler);
                p2s.push_back(vec2(inners[(k+1)%inners.size()].x(),inners[(k+1)%inners.size()].y())*scaler);
                p2s.push_back(vs*scaler);
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
                count += 3;
            }
        }
    }

    return;
}