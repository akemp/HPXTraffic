#include "headers.hpp"

typedef adjacency_list < listS, vecS, directedS,
no_property, property < edge_weight_t, double > > graph_t;
typedef graph_traits < graph_t >::vertex_descriptor vertex_descriptor;
typedef std::pair<int, int> Edge;

struct node
{
	vec2 v;
	vector<int> edges;
	int index;
    int start;
    int end;
	node();
	node(vec2 tv, int tindex)
	{
		v.x = tv.x;
		v.y = tv.y;
		index = tindex;
	};
};

bool vec2s (vec2 i, vec2 j) {
    return (glm::distance(i,j) < 0.0001);
}

void outputPath(vector<Edge> nodes, graph_t g, vector<string> name,  property_map<graph_t, edge_weight_t>::type weightmap, string path)
{
  ofstream dot_file(path + ".dot");

  dot_file << "digraph D {\n"
    << "  rankdir=LR\n"
    << "  size=\"25,25\"\n"
    << "  ratio=\"fill\"\n"
    << "  edge[style=\"bold\"]\n" << "  node[shape=\"circle\"]\n";
  int indexer = 0;
  graph_traits < graph_t >::edge_iterator ei, ei_end;
  for (boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
    graph_traits < graph_t >::edge_descriptor e = *ei;
    graph_traits < graph_t >::vertex_descriptor
      u = source(e, g), v = target(e, g);
    dot_file << name[u] << " -> " << name[v]
      << "[label=\"" << get(weightmap, e) << "\"";
      bool contains = false;
      if (indexer < nodes.size() &&  (nodes[indexer].first == u && nodes[indexer].second == v))
      {
        dot_file << ", color=\"black\"";
        indexer++;
      }
    else
      dot_file << ", color=\"grey\"";
    dot_file << "]";
  }
  dot_file << "}";
  return;
}

vector<Edge> shortest_path(vertex_descriptor target, vertex_descriptor s, graph_t g, const vector<double>& d, const vector<vertex_descriptor>& p)
{
    
    vector<Edge> nodes;
    int ltarget = target;
    {
        do{
            ltarget = target;
            target = p[target];
            nodes.push_back(Edge(target, ltarget));
        }while(target != s);
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

int makePath(vector<node> spots)
{   

    vector<string> name;

    ofstream fout("out.html");
    fout << "<svg width=\"24000\" height=\"24000\">\n";
    
    vector<node> places;

    vector<Edge> edge_vector;
    vector<double> weight_array;
    vec2 adder = vec2(100.0,100.0);
	for (int i = 0; i < spots.size(); ++i)
	{
		vec2 st = spots[i].v+adder;
        fout << "<circle cx=\"";
        fout << st.x;
        fout << "\" cy=\"";
        fout << st.y;
        fout << "\" r=\"2\" stroke=\"black\" fill=\"red\" z-index=\"0\" />\n";
		for (int j = 0; j < spots[i].edges.size(); ++j)
		{
			edge_vector.push_back(Edge(i,spots[i].edges[j]));
			vec2 e = spots[spots[i].edges[j]].v+adder;
			weight_array.push_back(glm::distance(st,e));
            vec2 s = spots[i].v+adder;

            fout << "<line x1=\"" << s.x << "\" y1=\"" << s.y << "\"";
            fout << "x2=\"" << e.x << "\" y2=\"" << e.y << "\" stroke=\"black\" stroke-width=\"1\" z-index=\"1\"\/>\n";
		}
	}
    
    graph_t g(edge_vector.begin(), edge_vector.end(), weight_array.begin(), name.size());

    property_map<graph_t, edge_weight_t>::type weightmap = get(edge_weight, g);
    vector<vertex_descriptor> p(num_vertices(g));
    vector<double> d(num_vertices(g));
	
	for (int l = 0; l < 10; ++l)
	{

		vertex_descriptor s = vertex(rand()%num_vertices(g), g);
		vertex_descriptor t = vertex(rand()%num_vertices(g), g);
	
		vector<Edge> path = generate_path(g,s,t, p, d);


		for (int i = 0; i < path.size(); ++i)
		{
			vec2 s = spots[path[i].first].v+adder;
			vec2 e = spots[path[i].second].v+adder;
        
			fout << "<line x1=\"" << s.x << "\" y1=\"" << s.y << "\"";
			fout << "x2=\"" << e.x << "\" y2=\"" << e.y << "\" stroke=\"green\" stroke-width=\"2\" z-index=\"1\"\/>\n";

			fout << "<circle cx=\"";
			fout << s.x;
			fout << "\" cy=\"";
			fout << s.y;
			if (i == 0)
				fout << "\" r=\"4\" stroke=\"black\" fill=\"blue\" z-index=\"0\" />\n";
			else
				fout << "\" r=\"3\" stroke=\"black\" fill=\"green\" z-index=\"0\" />\n";
			fout << "<circle cx=\"";
			fout << e.x;
			fout << "\" cy=\"";
			fout << e.y;
			if (i == path.size() - 1)
				fout << "\" r=\"4\" stroke=\"black\" fill=\"yellow\" z-index=\"0\" />\n";
			else
				fout << "\" r=\"3\" stroke=\"black\" fill=\"green\" z-index=\"0\" />\n";

		}
	}

    fout << "\n</svg>";
    fout.close();
    
    return 0;
}

bool sorter(vector<Point> &p1, vector<Point> &p2)
{
    return (boost::geometry::distance(p1.front(), p1.back()) < boost::geometry::distance(p2.front(), p2.back()));
}

int main()
{
    vector<Line> spotsl;
	int dim = 4;

    for (int i = 0; i < dim*10; ++i)
    {
        Line l;
        l.push_back(Point(i*200, 0));
        l.push_back(Point(i*200, dim * 2000));
        spotsl.push_back(l);
    }
    vector<Line> spotsw;
    for (int i = 0; i < dim; ++i)
    {
        Line l;
        l.push_back(Point(0, i*2000));
        l.push_back(Point(dim * 2000, i * 2000));
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
    vector<pair<vec2,vec2>>  pts;
	vector<node> nodes;
    for (int i = 0; i < roadsegs.size(); ++i)
    {
        if (roadsegs[i].size() > 0)
        {
            for (int j = 0; j < roadsegs[i].size()-1; ++j)
            {
                pair<vec2,vec2> pt(vec2(roadsegs[i][j].x(),roadsegs[i][j].y()),vec2(roadsegs[i][j+1].x(),roadsegs[i][j+1].y()));
                pts.push_back(pt);
            }
        }
    }
    PointCloud p;
    p.pts = pts;
    kd_tree tree(2 /*dim*/, p, KDTreeSingleIndexAdaptorParams(10 /* max leaf */) );

    tree.buildIndex();

    vector<pair<vec2,vector<int>>> spots;
    
    vector<vec2> visited;
    vector<vec2> inputted;
    for (int i = 0; i < pts.size(); ++i)
    {
        
        {
            vector<vec2> temp;
            temp.push_back(pts[i].first);
            vector<vec2>::iterator it = search(visited.begin(), visited.end(), temp.begin(), temp.end(), vec2s);
            if (it != visited.end())
                continue;
        }

        visited.push_back(pts[i].first);
        
        vector<vec2>::iterator it;
        {
            vector<vec2> temp;
            temp.push_back(pts[i].first);
            it = search(inputted.begin(), inputted.end(), temp.begin(), temp.end(), vec2s);
        }
        int d = std::distance(inputted.begin(), it);
        if (it == inputted.end())
        {
            inputted.push_back(pts[i].first);
            spots.push_back(pair<vec2,vector<int>>(pts[i].first,vector<int>()));
            d = spots.size()-1;
        }

        const float query_pt[2] = { pts[i].first.x, pts[i].first.y};

        const float search_radius = static_cast<float>(0.001);
        vector<pair<size_t,float> >   ret_matches;

        SearchParams params;
        //params.sorted = false;

        const size_t nMatches = tree.radiusSearch(&query_pt[0],search_radius, ret_matches, params);
        
        for (size_t j=0;j<nMatches;++j)
        {
            int ind = ret_matches[j].first;
            vec2 pt = pts[ind].second;
            vector<vec2> temp;
            temp.push_back(pt);
            vector<vec2>::iterator it2 = search(inputted.begin(), inputted.end(), temp.begin(), temp.end(), vec2s);
            int d2;
            if (it2 == inputted.end())
            {
                inputted.push_back(pt);
                spots.push_back(pair<vec2,vector<int>>(pt,vector<int>()));
                d2 = spots.size()-1;
            }
            else
                d2 = std::distance(inputted.begin(), it2);
            spots[d].second.push_back(d2);
            spots[d2].second.push_back(d);
        }
    }

    for (int i = 0; i < spots.size(); ++i)
    {
        node n(spots[i].first, i);
        n.edges = spots[i].second;
        nodes.push_back(n);
    }

    {
        vector<node> newnodes;
        
	    int count = 0;
        for (int i = 0; i < nodes.size(); ++i)
        {
            node n = nodes[i];
            vec2 v = n.v;

            vector<int> indexes;
            nodes[i].start = count;
            for (int j = 0; j < n.edges.size(); ++j)
            {
                node samp = nodes[n.edges[j]];
                vec2 tv = samp.v;
                vec2 norm = normalize(tv-v)*10.0f;
                vec2 v1 = tv - norm;
                vec2 v2 = v + norm;

                double phi = atan2(norm.y, norm.x);
                phi += 3.141592/2.0;
                double x = 10.0*cos(phi);
                double y = 10.0*sin(phi);
                vec2 adder(x,y);
                v1 += adder;
                v2 += adder;
                
                {
                    node temp(v1, i);
                    //temp.edges.push_back(count + 1);
                    newnodes.push_back(temp);
                }
                {
                    node temp(v2, i);
                    temp.edges.push_back(count);
                    newnodes.push_back(temp);

                    indexes.push_back(count+1);
                }
                count += 2;
            }
            nodes[i].end = count;
        }
        for (int i = 0; i < nodes.size(); ++i)
        {
            node n = nodes[i];
            for (int l = n.start; l < n.end; ++l)
            {
                vec2 v = newnodes[l].v;
                for (int j = 0; j < n.edges.size(); ++j)
                {
                    node temp = nodes[n.edges[j]];
                    for (int k = temp.start; k < temp.end; ++k)
                    {
						if (glm::distance(v, newnodes[k].v) < 80.0 && temp.index != n.index)
                        newnodes[l].edges.push_back(k);
                    }
                }
            }
        }
        nodes = newnodes;
    }

	makePath(nodes);
	return 0;
}
