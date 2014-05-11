#ifndef HEADERS
#define HEADERS

//#include <hpx/hpx_init.hpp>

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include <queue>
#include <fstream>


// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GL/glfw.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <boost/random/normal_distribution.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/mersenne_twister.hpp>


#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/property_map/property_map.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point_xy.hpp>


#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>



using namespace boost; 
using namespace glm;
using namespace std;

struct edger
{
    pair<int,int> edge;
    vec2 v1;
    vec2 v2;
    vector<int> neighbors;

    edger(){};
    edger(pair<int,int> v)
    {
        edge = v;
    }
};


template <class T>
const std::vector<
  boost::geometry::model::d2::point_xy<T>
>
GetLineLineIntersections(
  const boost::geometry::model::linestring<
    boost::geometry::model::d2::point_xy<T>
  > line1,
  const boost::geometry::model::linestring<
    boost::geometry::model::d2::point_xy<T>
  > line2)
{
  typedef boost::geometry::model::d2::point_xy<T> Point;
  typedef boost::geometry::model::linestring<Point> Line;
  std::vector<Point> points;
  boost::geometry::intersection(line1,line2,points);
  assert(points.empty() || points.size() == 1);
  return points;
}

template <class T>
const boost::geometry::model::linestring<boost::geometry::model::d2::point_xy<T>
>
CreateLine(const std::vector<boost::geometry::model::d2::point_xy<T> >& v)
{
  return boost::geometry::model::linestring<
    boost::geometry::model::d2::point_xy<T>
  >(v.begin(),v.end());
}

struct fuzzy_equal_to
  : public std::binary_function<double,double,bool>
{
  fuzzy_equal_to(const double tolerance = std::numeric_limits<double>::epsilon())
    : m_tolerance(tolerance)
  {
    assert(tolerance >= 0.0);
  }
  bool operator()(const double lhs, const double rhs) const
  {
    return rhs > (1.0 - m_tolerance) * lhs
        && rhs < (1.0 + m_tolerance) * lhs;
  }
  const double m_tolerance;
};


typedef boost::geometry::model::d2::point_xy<double> Point;
typedef boost::geometry::model::linestring<Point> Line;

typedef adjacency_list < listS, vecS, directedS,
no_property, property < edge_weight_t, double > > graph_t;
typedef graph_traits < graph_t >::vertex_descriptor vertex_descriptor;
typedef std::pair<int, int> Edge;


#include <traffic/graphics.hpp>


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


vector<Edge> shortest_path(vertex_descriptor target, vertex_descriptor s, graph_t g, const vector<double>& d, const vector<vertex_descriptor>& p)
{
    vector<Edge> nodes;
    int test = 0;
    int ltarget = target;
    {
        do{
            ltarget = target;
            target = p[target];
            nodes.push_back(Edge(target, ltarget));
            ++test;
        }while(target != s && test < 10000000);
    }
    if (target != s)
    {
        cout << "ERROR! STUCK IN LOOP!\n";
        exit(1);
    }
    reverse(nodes.begin(), nodes.end());
    return nodes;
}

vector<Edge> generate_path(const graph_t &g, vertex_descriptor s, vertex_descriptor t, vector<vertex_descriptor> p, vector<double> d)
{

    cout << "\nGenerating paths\n";

    dijkstra_shortest_paths(g, s,
                            predecessor_map(boost::make_iterator_property_map(p.begin(), get(boost::vertex_index, g))).
                            distance_map(boost::make_iterator_property_map(d.begin(), get(boost::vertex_index, g))));
    
    cout << "\nGenerated. Outputting path\n";


    return shortest_path(t,s,g,d,p);
}

vector<Edge> generatePath(int start, int end,vector<edger> edges)
{
    vector<Edge> edge_vector;
    vector<double> weight_array;

    for (int i = 0; i < edges.size(); ++i)
    {
        for (int j = 0; j < edges[i].neighbors.size(); ++j)
        {
            edge_vector.push_back(Edge(i, edges[i].neighbors[j]));
            edger temp = edges[edges[i].neighbors[j]];
            weight_array.push_back(glm::distance(temp.v1, temp.v2));
        }
    }
    
    graph_t g(edge_vector.begin(), edge_vector.end(), weight_array.begin(), edges.size());

    property_map<graph_t, edge_weight_t>::type weightmap = get(edge_weight, g);
    vector<vertex_descriptor> p(num_vertices(g));
    vector<double> d(num_vertices(g));

    time_t timev;

    vertex_descriptor s = vertex(start%num_vertices(g), g);
    vertex_descriptor t = vertex(end%num_vertices(g), g);

    return generate_path(g,s,t, p, d);
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