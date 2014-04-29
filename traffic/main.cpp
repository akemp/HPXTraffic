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

    time_t timev;

vertex_descriptor s = vertex(start%num_vertices(g), g);
vertex_descriptor t = vertex(end%num_vertices(g), g);

return generate_path(g,s,t, p, d);
}

void foutRoads(const vector<vector<vec3>> &quads, const vector<pair<vec3,vector<vec3>>> &inputs, double diver)
{
    int count = 1;
    ofstream fout("out.obj");
    for (int i = 0; i < quads.size(); ++i)
    {
        for (int j = 0; j < 4; ++j)
            fout << "v " << quads[i][j][0]/diver << " " << quads[i][j][1]/diver << " " << quads[i][j][2]/diver << endl;
        fout << "f " << count + 2 << " " << count + 3 << " " << count + 1 << " " << count << endl;
        count += 4;
    }

    for (int i = 0; i < inputs.size(); ++i)
    {   
        for (int j = 0; j < inputs[i].second.size(); j += 2)
        {
            fout << "v " << inputs[i].first[0]/diver << " " << inputs[i].first[1]/diver << " " << inputs[i].first[2]/diver << endl;
            fout << "v " << inputs[i].second[j][0]/diver << " " << inputs[i].second[j][1]/diver << " " << inputs[i].second[j][2]/diver << endl;
            fout << "v " << inputs[i].second[j+1][0]/diver << " " << inputs[i].second[j+1][1]/diver << " " << inputs[i].second[j+1][2]/diver << endl;
            fout << "f " << count << " " << count + 1 << " " << count + 2 << endl;
            count += 3;
        }
    }
    fout.close();
    return;
}

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

int main()
{
    float len = 300;
	vector<Line> roadsegs = generateRoads(5,4, len);
    
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
    

    vector<vector<vec3>> quads;

    
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

    
    int count = 0;
    
    vector<unsigned int> indices;
    vector<VertexData> vertex_data;
    
    
    float scaler = 0.0225;
    generateQuads(quads, egs, inputted, count, scaler, indices, vertex_data);
    
    vector<pair<vector<Edge>,float>> paths;
    
    for (int i = 0; i < 3; ++i)
    {
        paths.push_back(pair<vector<Edge>,float>(generatePath(0,i*10,edges),i));
    }


    int code = startup(1200,600);


    if (code != 0)
        return code;
    
    int width = 64, height = width;
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);


    Mesh terrain(vertex_data,indices, LoadShaders( "vert.glsl", "frag.glsl" ), loadDDS("asphalt.dds"));
    terrain.move.y = -3.0;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile("car.obj", aiProcess_FlipUVs);
    
    indices = vector<unsigned int>();
    vertex_data = vector<VertexData>();
    
    loadAssImp(scene->mMeshes[0],indices, vertex_data, 0.008);
    
    Mesh car(vertex_data,indices, LoadShaders( "vert.glsl", "frag.glsl" ), loadDDS("delorean.dds"));
    
    car.move.y = -3.0;
    vector<Mesh> cars;
    for (int i = 0; i < 3; ++i)
    {
        cars.push_back(car);
    }

    double lastTime = glfwGetTime();
    int nbFrames = 0;
    double last = glfwGetTime();
    double totalTime = 0;
    	do{	
            double elapsed = fps(nbFrames, totalTime, lastTime, last);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            computeMatricesFromInputs();

            terrain.draw();
            for (int i = 0; i < cars.size(); ++i)
            {
                int indl = paths[i].first.front().first;
                vec2 vl = edges[indl].v1*scaler;
                vec2 vh = edges[indl].v2*scaler;
                float dist = glm::max<float>(glm::distance(vh,vl),0.00001);
                while (paths[i].second > dist && paths[i].first.size() > 1)
                {
                    paths[i].second -= dist;
                    paths[i].first.erase(paths[i].first.begin());
                    indl = paths[i].first.front().first;
                    vl = edges[indl].v1*scaler;
                    vh = edges[indl].v2*scaler;
                    dist = glm::max<float>(glm::distance(vh,vl),0.00001);
                }
                float mov = paths[i].second/dist;
                if (paths[i].first.size() == 1 && mov > 1.0f)
                    mov = 1.0f;
                vec2 v = vl*(1.0f-mov)+vh*mov;
                cars[i].move.x = v.x;
                cars[i].move.z = v.y;
                cars[i].draw();
                paths[i].second += elapsed/100.0f;
            }
            

            glfwSwapBuffers();

        } while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS &&
                    glfwGetWindowParam( GLFW_OPENED ) );

        	glfwTerminate();

    return 0;
}