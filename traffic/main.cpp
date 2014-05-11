#include "headers.hpp"

struct vehicle
{
    vector<vec2> path;
    float current;
    float progress;
    float total;
    float maximum;
    int index;
};

void processCars(vector<Mesh>& cars, vector<vector<Edge>>& paths, vector<vehicle>& pathers,
    const vector<edger>& edges, const float scaler, const float elapsed)
{
    for (int i = 0; i < pathers.size(); ++i)
    {
        {
            float mov = (pathers[i].progress + 0.2)/pathers[i].current;
            int ind = pathers[i].index;
            vec2 v;
            float cur = pathers[i].current;
            float lastcur = cur;
            while (mov > 1 && ind < pathers[i].path.size()-1)
            {
                ind += 1;
                vec2 vl = pathers[i].path[ind-1];
                vec2 vh = pathers[i].path[ind];

                mov -= 1.0f;
                lastcur = cur;
                cur = glm::max((glm::distance(vh,vl)),0.00001f);
                mov *= lastcur/cur;
            }
            {
                vec2 vl = pathers[i].path[ind-1];
                vec2 vh = pathers[i].path[ind];
        
                v = vl*(1.0f-mov)+vh*mov;
        
            }
            mov = pathers[i].progress/pathers[i].current;
            ind = pathers[i].index;
            {
                vec2 vl = pathers[i].path[ind-1];
                vec2 vh = pathers[i].path[ind];
                
                vec2 v2 = (vl*(1.0f-mov)+vh*mov);

                vec2 dir = normalize(v-v2);
                cars[i].move.x = v.x;
                cars[i].move.z = v.y;
                
                cars[i].rot.y = atan2(dir.x,dir.y)*180.0f/3.141592f;
            }

        }


        if (pathers[i].total < pathers[i].maximum)
        {
            pathers[i].total += elapsed/100.0f;
            pathers[i].progress += elapsed/100.0f;
        }
        else
        {
            pathers[i].total = pathers[i].maximum;
            pathers[i].progress = pathers[i].current;
        }
        while (pathers[i].current < pathers[i].progress && pathers[i].index < pathers[i].path.size() - 1)
        {
            pathers[i].index += 1;
            pathers[i].progress -= pathers[i].current;
            
            vec2 vl = pathers[i].path[pathers[i].index-1];
            vec2 vh = pathers[i].path[pathers[i].index];

            pathers[i].current = glm::distance(vl,vh);
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
    generateQuads(quads, egs, inputted, count, scaler, indices, vertex_data);
    
    vector<vector<Edge>> paths;

    
    for (int i = 0; i < 10; ++i)
    {
        paths.push_back(generatePath(0,i,edges));
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
    
    loadAssImp(scene->mMeshes[0],indices, vertex_data, 0.008*scaler/0.0225);
    
    Mesh car(vertex_data,indices, LoadShaders( "vert.glsl", "frag.glsl" ), loadDDS("delorean.dds"));
    
    car.move.y = -3.0;
    vector<Mesh> cars;
    for (int i = 0; i < paths.size(); ++i)
    {
        cars.push_back(car);
    }
    
    for (int i = 0; i < edges.size(); ++i)
    {
        edges[i].v1 *= scaler;
        edges[i].v2 *= scaler;
    }

    double lastTime = glfwGetTime();
    int nbFrames = 0;
    double last = glfwGetTime();
    float speed = 0.15;
    double totalTime = 0;
    vector<vehicle> vehicles;
    for (int i = 0; i < paths.size(); ++i)
    {
        vehicle pather;
        vec2 start = edges[paths[i][0].first].v1;
        //vec2 norm = normalize(edges[paths[i].first[0].first].v2-start);
        pather.current = glm::distance(edges[paths[i][0].first].v2,start);
        pather.progress = 0.1f+i/2.0f;
        pather.index = 1;
        pather.total =  pather.progress;
        pather.maximum = 0;
        for (int j = 0; j < paths[i].size(); ++j)
        {
            pather.path.push_back(edges[paths[i][j].first].v1);
            pather.path.push_back(edges[paths[i][j].first].v2);
        }
        for (int j = 1; j < pather.path.size(); ++j)
        {
            pather.maximum += glm::distance(pather.path[j-1],pather.path[j]);
        }
        vehicles.push_back(pather);
    }
    do{	
        double elapsed = fps(nbFrames, totalTime, lastTime, last)*speed;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        computeMatricesFromInputs();

        terrain.draw();
        for (int i = 0; i < cars.size(); ++i)
            cars[i].draw();

        processCars(cars, paths, vehicles, edges, scaler, elapsed);

        glfwSwapBuffers();

    } while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS &&
                glfwGetWindowParam( GLFW_OPENED ) );

     glfwTerminate();

    return 0;
}