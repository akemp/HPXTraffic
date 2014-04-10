#include "headers.hpp"

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

struct timestep
{
    vec2 e;
    float time;
    float prog;
    timestep(vec2 te, float ttime)
    {
        e = te;
        time = ttime;
        prog = 0.0f;
    };
};

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

    time_t  timev;
	
	vertex_descriptor s = vertex(start%num_vertices(g), g);
	vertex_descriptor t = vertex(end%num_vertices(g), g);

	return generate_path(g,s,t, p, d);
}

void outputEdges(vector<edger> edges, vector<vector<timestep>> paths)
{

    
    ofstream fout("out.html");
    fout << "<!DOCTYPE html> \n<html>\n<body>\n";
    fout << "<canvas id=\"myCanvas\" width=\"4200\" height=\"3400\"></canvas>\n";
    fout << "<script>\n";
    fout << "window.requestAnimFrame = (function(callback) {\n";
    fout << "return window.requestAnimationFrame || window.webkitRequestAnimationFrame || window.mozRequestAnimationFrame || window.oRequestAnimationFrame || window.msRequestAnimationFrame || \
        function(callback) { \n \
          window.setTimeout(callback, 1000 / 60); \n \
        }; \n \
      })();";
    fout << "var canvas = document.getElementById('myCanvas');\n";
    fout << "var context = canvas.getContext('2d');\n";
    fout << "var date = new Date()\n";
    fout << "var start = date.getTime();\n";
    
    string roadstyle = "context.lineWidth = 1;\ncontext.strokeStyle = '#000000';\ncontext.stroke();\n";
    string edgestyle = "context.lineWidth = 0.5;\ncontext.strokeStyle = '#00ff00';\ncontext.closePath();\ncontext.fillStyle = 'green';\ncontext.fill();\ncontext.stroke();\n";
    string pathstyle = "context.lineWidth = 2;\ncontext.strokeStyle = '#ff0000';\ncontext.fillStyle = '#ff0000';\ncontext.stroke();\n";
    
    {
        
        vec2 adder = vec2(100.0,100.0);
        fout << "\n\nvar paths = [\n";

    
        for (int l = 0; l < paths.size(); ++l)
        {
            vector<timestep> path = paths[l];
            fout << "[\n";
		    for (int i = 0; i < path.size(); ++i)
		    {
                {
				    vec2 v1 = path[i].e+adder;
                    
                    fout << "[" << v1.x << ", " << v1.y << ", " << path[i].time;
                 
                    if (i < path.size() - 1)
                        fout << "],\n";
                    else
                        fout << "]\n";   
			    }
                /*
                {
				    vec2 v1 = path[i].s+adder;
				    vec2 v2 = path[i].e+adder;
        
                    fout << "[" << v1.x << ", " << v1.y << "],";
                    fout << "[" << v2.x << ", " << v2.y;
                    if (i < path.size() - 1)
                        fout << "],\n";
                    else
                        fout << "]\n";
			    }*/
            }
            if (l < paths.size() - 1)
                fout << "],\n";
            else
                fout << "]\n";
        }

        fout << "];\n\n";
    }

    {
        vec2 adder = vec2(100.0,100.0);

        fout << "\nfunction drawcars(i,time){\n";

        fout << "var difference = (time-start)/2000.0;\n";
        fout << "if (Math.ceil(difference) < paths[i].length){\n";
        
        {
            int count = 0;

            fout << "var low = Math.floor(difference);\n";
            fout << "var high = Math.ceil(difference);\n";
            fout << "var diff = difference - low;\n";
            fout << "var vl = paths[i][low];\n";
            fout << "var vh = paths[i][high];\n";
            fout << "var x = vl[0]*(1-diff)+vh[0]*diff;";
            fout << "var y = vl[1]*(1-diff)+vh[1]*diff;";
            
            fout << "context.beginPath();\n";
            fout << "context.arc(x, y, 10, 0, 2 * Math.PI, false);\n";
            fout << "context.fill();";
        }

        fout << "}\n";
        fout << "};\n";
    }

    // set line color
    {
        vec2 adder = vec2(100.0,100.0);
        fout << "function drawroads(){\n";
	    for (int i = 0; i < edges.size(); ++i)
	    {
		    {
			    vec2 s = edges[i].v1;
                vec2 e = edges[i].v2;
			    double dist = glm::distance(s,e);

                s += adder;
                e += adder;
                fout << "context.beginPath();\n";
                fout << "context.moveTo(" << s.x << ", " << s.y << ");\n";
                fout << "context.lineTo(" << e.x << ", " << e.y << ");\n";
                fout << roadstyle;
		    }
        
            for (int j = 0; j < edges[i].neighbors.size(); ++j)
            {
                vec2 s = edges[i].v2;
                vec2 e = edges[edges[i].neighbors[j]].v1;
			    double dist = glm::distance(s,e);
            
                s += adder;
                e += adder;

                fout << "context.beginPath();";
                vec2 norm = e-s;
                float phi = atan2(norm.y, norm.x);
                vec2 v1 = s;
                vec2 v2 = s;
                phi += 3.141592f/2.0f;
                float x = 2.0*cos(phi);
                float y = 2.0*sin(phi);
                vec2 addme(x,y);
                v1 += addme;
                v2 -= addme;
            
                fout << "context.moveTo(" << e.x << ", " << e.y << ");\n";
                fout << "context.lineTo(" << v1.x << ", " << v1.y << ");\n";
                fout << "context.lineTo(" << v2.x << ", " << v2.y << ");\n";

                fout << edgestyle;
            }
        
	    }
    
        fout << "};\n";
    }
    fout << "function animate() { \n \
        var canvas = document.getElementById('myCanvas'); \n \
        var context = canvas.getContext('2d'); \n \
        context.clearRect(0, 0, canvas.width, canvas.height); \n \
        //draw items \n \
        drawroads(); \n \
        var d = new Date(); \n \
        for (var i = 0; i < paths.length; ++i) \n \
        drawcars(i,d.getTime()); \n \
        //call next frame \n \
        requestAnimFrame(function() { \n \
          animate(); \n \
        }); \n \
      } \n \
      animate();";

    fout << "\n</script>\n</body>\n</html>";
    fout.close();
}

void outputStreets(vector<edger> edges)
{
    ofstream fout("out.obj");
    
    int count = 1;

    fout << "g g1\n";
    
	for (int i = 0; i < edges.size(); ++i)
	{
		{
			vec2 s = edges[i].v1;
            vec2 e = edges[i].v2;
			double dist = glm::distance(s,e);

            fout << "v " << s.x << " " << s.y << " 0\n";
            fout << "v " << e.x << " " << e.y << " 0\n";

            fout << "l " << count << " " << (count + 1) << endl;
            count += 2;
		}
        
        for (int j = 0; j < edges[i].neighbors.size(); ++j)
        {
            vec2 s = edges[i].v2;
            vec2 e = edges[edges[i].neighbors[j]].v1;
			double dist = glm::distance(s,e);
            
            
            fout << "v " << e.x << " " << e.y << " 0\n";
            fout << "v " << s.x << " " << s.y << " 0\n";
            
            fout << "l " << count << " " << (count + 1) << endl;
            count += 2;
        }
        
	}
    

    return;
}

int main()
{
    float len = 80;
	vector<Line> roadsegs = generateRoads(3, len);
    
    vector<pair<int,int>> egs;
    vector<vec2> inputted;

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
    vector<edger> edges;

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
        vec2 norm = normalize(v2-v1);

        v1 += norm * 10.0f;
        v2 -= norm * 10.0f;

        float phi = atan2(norm.y, norm.x);
        phi += 3.141592/2.0;
        float x = 10.0*cos(phi);
        float y = 10.0*sin(phi);
        vec2 adder(x,y);
        v1 += adder;
        v2 += adder;

        e.v1 = v1;
        e.v2 = v2;

        edges.push_back(e);
    }
    
    vector<vector<Edge>> paths;

    vector<vector<timestep>> steps;
    if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
    {
        std::cout << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_Quit();
    /*
    for (int i = 0; i < 50; ++i)
    {
        paths.push_back(generatePath(0,i*5000,edges));
        vector<timestep> step;
        vector<vec2> holders;
        for (int j = 0; j < paths[i].size(); ++j)
        {
            if (paths[i][j].first != paths[i][j].second)
            {
                holders.push_back(edges[paths[i][j].first].v1);
                holders.push_back(edges[paths[i][j].first].v2);
                holders.push_back(edges[paths[i][j].second].v1);
            }
        }
        if (holders.size() > 0)
        {
            step.push_back(timestep(holders[0], 0));
            for (int j = 0; j < holders.size(); ++j)
            {
                float dist = glm::distance(step.back().e,holders[j]);
                if (dist > 0.001f)
                {
                    step.push_back(timestep(holders[j], dist));
                }
            }
        }
        steps.push_back(step);
    }*/

    //outputStreets(edges);

    //outputEdges(edges,steps);

    return 0;
}