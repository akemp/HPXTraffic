#include "headers.hpp"

struct vehicle
{
    int destination;
    Edge turn;
    vec2 start;
    vec2 dir;
    int index;

    vector<int> edgers;
    int pos;
    vec2 vel;

    float progress;
    float dist;
    bool waiting;
    bool turning;
};

 
struct street
{
    vec2 v1;
    vec2 v2;

    vector<int> neighbors;
    vector<vector<Edge>> intersects;

    street(){};
    street(edger edge)
    {
        v1 = edge.v1;
        v2 = edge.v2;
        neighbors = edge.neighbors;
    };

};

void processCars(vector<Mesh>& cars, vector<vehicle>& pathers,
    const vector<street>& streets, const float scaler, const float elapsed)
{
    vector<vec2> places;
    vector<vec2> vels;

    float carsize = 0.6;

    for (int i = 0; i < pathers.size(); ++i)
    {
        if (pathers[i].pos >= pathers[i].edgers.size())
        {
            pathers.erase(pathers.begin() + i);
            cars.erase(cars.begin() + i);
            --i;
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
        cars[i].rot.y = atan2(pathers[i].dir.x,pathers[i].dir.y)*180.0f/3.141592f;
        places.push_back(v); 
        vels.push_back(pathers[i].dir*prog);
    }
    for (int i = 0; i < pathers.size(); ++i)
    {
        bool move = true;
        if (!pathers[i].turning)
        {
            vec2 vel = vels[i];
            vec2 place = places[i];
            vec2 start = pathers[i].start;
            if (!pathers[i].waiting)
            {
                for (int j = 0; j < pathers.size(); ++j)
                {
                    if (i == j || pathers[j].index != pathers[i].index)
                        continue;
                    else
                    {
                        vec2 place2 = places[j];
                        if (glm::distance(places[j], places[i]) > carsize || glm::distance(places[i], start) >= glm::distance(places[j], start))
                            continue;
                        move = false;
                        break;
                    }
                }
            }
            else
            {
                for (int j = 0; j < pathers.size(); ++j)
                {
                    if (i == j || pathers[j].waiting || pathers[j].index != pathers[i].edgers[pathers[i].pos])
                        continue;
                    else if (pathers[j].turning)
                    {
                        int index = -1;
                        for (int k = 0; k < streets[pathers[i].turn.first].neighbors.size(); ++k)
                        {
                            if(pathers[i].turn.second == streets[pathers[i].turn.first].neighbors[k])
                            {
                                index = k;
                                break;
                            }
                        }
                        for (int k = 0; k < streets[pathers[i].turn.first].intersects[index].size(); ++k)
                        {
                            if (streets[pathers[i].turn.first].intersects[index][k].first == pathers[j].turn.first
                                && streets[pathers[i].turn.first].intersects[index][k].second == pathers[j].turn.second)
                                {
                                    move = false;
                                    break;
                                }
                        }
                    }
                    else
                    {
                        vec2 place2 = places[j];
                        if (glm::distance(places[j], pathers[j].start) > carsize)
                            continue;
                        move = false;
                        break;
                    }
                }
            }
        }
        if (move)
        {
            pathers[i].progress += elapsed;
            if (pathers[i].progress >= pathers[i].dist)
            {
                pathers[i].progress = 0;
                if (pathers[i].turning)
                {
                    pathers[i].turning = false;
                    if (pathers[i].pos < pathers[i].edgers.size())
                    {
                        pathers[i].start = streets[pathers[i].index].v1;
                        pathers[i].dir = normalize(streets[pathers[i].index].v2-pathers[i].start);
                        pathers[i].dist = glm::distance(pathers[i].start,streets[pathers[i].index].v2);
                    }
                }
                else if (pathers[i].waiting)
                {
                    pathers[i].waiting = false;
                    pathers[i].index = pathers[i].edgers[pathers[i].pos];
                    pathers[i].dir = normalize(streets[pathers[i].index].v1-pathers[i].start);
                    pathers[i].dist = glm::distance(streets[pathers[i].index].v1,pathers[i].start)+0.001;
                    pathers[i].turning = true;
                }
                else
                {
                    pathers[i].waiting = true;
                    pathers[i].dist = 0.5;
                    pathers[i].start = streets[pathers[i].index].v2;
                    //pathers[i].dir = normalize(edges[pathers[i].edgers[pathers[i].pos+1]].v1-pathers[i].start);
                    pathers[i].pos += 1;
                    pathers[i].turn = Edge(pathers[i].index,pathers[i].edgers[pathers[i].pos]);
                }
            }
        }
    }
    return;
}

int main()
{
    float len = 200;
	vector<Line> roadsegs = generateRoads(6,4, len);
    
    vector<pair<int,int>> egs;
    vector<vec2> inputted;
    
    vector<edger> edges;
    
    vector<vector<vec3>> quads;

    generateRoadModels(roadsegs, inputted, egs, quads, edges);
    
    int count = 0;
    
    vector<unsigned int> indices;
    vector<VertexData> vertex_data;
    
    
    float scaler = 0.0105;
    for (int i = 0; i < edges.size(); ++i)
    {
        edges[i].v1 *= scaler;
        edges[i].v2 *= scaler;
        edges[i].v1 += normalize(edges[i].v2-edges[i].v1)*0.2f;
        edges[i].v2 += normalize(edges[i].v1-edges[i].v2)*0.2f;
    }

    generateQuads(quads, egs, inputted, count, scaler, indices, vertex_data);
    
    int code = startup(1200,600);

    if (code != 0)
        return code;
    
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);


    Mesh terrain(vertex_data,indices, LoadShaders( "vert.glsl", "frag.glsl" ), loadDDS("asphalt.dds"));
    terrain.move.y = -3.0;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile("car.obj", aiProcess_FlipUVs);
    
    
    double lastTime = glfwGetTime();
    int nbFrames = 0;
    double last = glfwGetTime();
    float speed = 0.15;
    double totalTime = 0;

    vector<vehicle> vehicles;
    vector<Mesh> cars;

    indices = vector<unsigned int>();
    vertex_data = vector<VertexData>();
    
    loadAssImp(scene->mMeshes[0],indices, vertex_data, 0.008*scaler/0.0225);
    
    Mesh car(vertex_data,indices, LoadShaders( "vert.glsl", "frag.glsl" ), loadDDS("delorean.dds"));
    
    car.move.y = -3.0;

    
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

    vector<street> streets;
    for (int i = 0; i < edges.size(); ++i)
    {   
        street temp(edges[i]);
        vec2 o = temp.v2;
        vector<vector<Edge>> intersects;
        for (int j = 0; j < temp.neighbors.size(); ++j)
        {
            vec2 dir = (edges[temp.neighbors[j]].v1) + normalize(edges[temp.neighbors[j]].v1-o)*0.01f;

            Line line1;
            line1.push_back(Point(o[0],o[1]));
            line1.push_back(Point(dir[0],dir[1]));
            vector<Edge> innersects;

            for (int k = 0; k < edge_vector.size(); ++k)
            {
                Edge edge = edge_vector[k];
                if (edge.first == i)
                    continue;
                vec2 o2 = edges[edge_vector[k].first].v2;
                if (glm::distance(o,o2) > 4.0)
                    continue;
                vec2 dir2 = edges[edge_vector[k].second].v1 + normalize(edges[edge_vector[k].second].v1-o2)*0.01f;
                
                Line line2;
                line2.push_back(Point(o2[0],o2[1]));
                line2.push_back(Point(dir2[0],dir2[1]));
                if (GetLineLineIntersections(line1,line2).size() > 0)
                {
                    innersects.push_back(edge_vector[k]);
                }
            }
            intersects.push_back(innersects);
        }
        temp.intersects = intersects;
        streets.push_back(temp);

    }

    for (int i = 0; i < 1000; ++i)
    {
        vehicle pather;
        vector<int> edger = generatePath((i)%edges.size(),0,g,weightmap,p,d,i);

        pather.pos = 0;
        pather.destination = 0;
        pather.progress = (i%5)*0.4f;
        pather.start = streets[edger.front()].v1;
        pather.dir = normalize(streets[edger.front()].v2-pather.start);
        pather.dist = glm::distance(streets[edger.front()].v2,pather.start);
        pather.waiting = false;
        pather.edgers = edger;
        pather.index = edger.front();
        pather.turning = false;
        pather.turn = Edge(edger.front(), edger[1]);

        vehicles.push_back(pather);
        cars.push_back(car);
    }
    do{	
        double elapsed = fps(nbFrames, totalTime, lastTime, last)*speed;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        computeMatricesFromInputs();

        terrain.draw();
        for (int i = 0; i < cars.size(); ++i)
            cars[i].draw();

        processCars(cars, vehicles, streets, scaler, elapsed/100.0f);

        glfwSwapBuffers();

    } while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS &&
                glfwGetWindowParam( GLFW_OPENED ) );

     glfwTerminate();

    return 0;
}