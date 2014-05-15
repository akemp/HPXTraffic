#include "headers.hpp"

struct vehicle
{
    int destination;
    int index;
    int heading;
    vec2 start;
    vec2 dir;

    vector<Edge> edgers;
    int pos;
    vec2 vel;

    float progress;
    float dist;
    bool waiting;
    bool turning;
};
 

void processCars(vector<Mesh>& cars, vector<vehicle>& pathers,
    const vector<edger>& edges, const float scaler, const float elapsed)
{
    vector<vec2> places;
    vector<vec2> vels;

    for (int i = 0; i < pathers.size(); ++i)
    {
        if (pathers[i].pos > pathers[i].edgers.size())
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
        places.push_back(v); 
        vels.push_back(pathers[i].dir*prog);
    }
    for (int i = 0; i < pathers.size(); ++i)
    {
        bool move = true;
        /*
        vec2 vel = vels[i];
        vec2 place = places[i];
        Line line1;
        line1.push_back(Point(place[0],place[1]));
        line1.push_back(Point(vel[0],vel[1]));
        for (int j = 0; j < pathers.size(); ++j)
        {
            if (i == j)
                continue;

            vec2 vel2 = vels[j];
            vec2 place2 = places[j];
            if (glm::distance(vel + place, vel2 + place2) < 0.8f)
            {

                Line line2;
                line2.push_back(Point(place2[0],place2[1]));
                line2.push_back(Point(vel2[0],vel2[1]));
                
                vector<Point> ints = GetLineLineIntersections(line1,line2);
                if (ints.size() > 0 && !(pathers[i].waiting && !pathers[j].waiting))
                {
                    vec2 pt = vec2(ints.front().x(),ints.front().y());
                    
                    if (glm::distance(place + vel, place2) < glm::distance(place2 + vel2, place))
                    {
                        if (pathers[i].waiting && pathers[j].waiting)
                        {
                            if (pathers[i].progress < pathers[j].progress)
                            {
                                pathers[i].progress = 0;
                                move = false;
                                break;
                            }
                        }
                        else
                        {
                            move = false;
                            break;
                        }
                    }
                }
            }
        }
        */
        if (move)
        {
            pathers[i].progress += elapsed;
            if (pathers[i].progress > pathers[i].dist)
            {
                pathers[i].progress = 0;
                if (pathers[i].waiting)
                {
                    pathers[i].waiting = false;
                    pathers[i].index = pathers[i].heading;
                    pathers[i].start = edges[pathers[i].heading].v1;
                    pathers[i].dir = normalize(edges[pathers[i].heading].v2-edges[pathers[i].heading].v1);
                    pathers[i].dist = glm::distance(edges[pathers[i].heading].v2,edges[pathers[i].heading].v1);
                    pathers[i].heading = pathers[i].edgers[pathers[i].pos].second;
                    pathers[i].pos += 1;
                }
                else
                {
                    pathers[i].waiting = true;
                    pathers[i].dist = 0.5;
                    pathers[i].start = edges[pathers[i].index].v2;
                    pathers[i].dir = normalize(edges[pathers[i].heading].v1-edges[pathers[i].index].v2);
                }
            }
        }
    }
    return;
}

int main()
{
    float len = 200;
	vector<Line> roadsegs = generateRoads(5,4, len);
    
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
    }
    generateQuads(quads, egs, inputted, count, scaler, indices, vertex_data);
    
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

    for (int i = 0; i < 10; ++i)
    {
        vehicle pather;
        vector<Edge> edger = generatePath(i%edges.size(),0,edges);

        pather.index = edger[0].first;
        pather.heading = edger[0].second;
        pather.destination = 0;
        pather.progress = (i%10)/5.0f;
        pather.start = edges[pather.index].v1;
        pather.dir = normalize(edges[pather.index].v2-edges[pather.index].v1);
        pather.dist = glm::distance(edges[pather.index].v2,edges[pather.index].v1);
        pather.waiting = false;
        pather.turning = false;
        pather.edgers = edger;
        pather.pos = 0;

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

        processCars(cars, vehicles, edges, scaler, elapsed/100.0f);

        glfwSwapBuffers();

    } while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS &&
                glfwGetWindowParam( GLFW_OPENED ) );

     glfwTerminate();

    return 0;
}