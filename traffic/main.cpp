#include "headers.hpp"

struct vehicle
{
    int destination;
    int index;
    vec2 start;
    vec2 dir;

    vec2 vel;

    float progress;
    float dist;
};
 
// Given three colinear points p, q, r, the function checks if
// point q lies on line segment 'pr'
bool onSegment(vec2 p, vec2 q, vec2 r)
{
    if (q.x <= glm::max(p.x, r.x) && q.x >= glm::min(p.x, r.x) &&
        q.y <= glm::max(p.y, r.y) && q.y >= glm::min(p.y, r.y))
       return true;
 
    return false;
}
 
// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are colinear
// 1 --> Clockwise
// 2 --> Counterclockwise
int orientation(vec2 p, vec2 q, vec2 r)
{
    // See 10th slides from following link for derivation of the formula
    // http://www.dcs.gla.ac.uk/~pat/52233/slides/Geometry1x1.pdf
    float val = (q.y - p.y) * (r.x - q.x) -
              (q.x - p.x) * (r.y - q.y);
 
    if (val == 0) return 0;  // colinear
 
    return (val > 0)? 1: 2; // clock or counterclock wise
}
 
// The main function that returns true if line segment 'p1q1'
// and 'p2q2' intersect.
bool doIntersect(vec2 p1, vec2 q1, vec2 p2, vec2 q2)
{
    // Find the four orientations needed for general and
    // special cases
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);
 
    // General case
    if (o1 != o2 && o3 != o4)
        return true;
 
    // Special Cases
    // p1, q1 and p2 are colinear and p2 lies on segment p1q1
    if (o1 == 0 && onSegment(p1, p2, q1)) return true;
 
    // p1, q1 and p2 are colinear and q2 lies on segment p1q1
    if (o2 == 0 && onSegment(p1, q2, q1)) return true;
 
    // p2, q2 and p1 are colinear and p1 lies on segment p2q2
    if (o3 == 0 && onSegment(p2, p1, q2)) return true;
 
     // p2, q2 and q1 are colinear and q1 lies on segment p2q2
    if (o4 == 0 && onSegment(p2, q1, q2)) return true;
 
    return false; // Doesn't fall in any of the above cases
}

void processCars(vector<Mesh>& cars, vector<vehicle>& pathers,
    const vector<edger>& edges, const float scaler, const float elapsed)
{
    vector<vec2> places;
    vector<vec2> vels;
    for (int i = 0; i < pathers.size(); ++i)
    {
        vec2 v = pathers[i].start + pathers[i].dir * pathers[i].progress;
        cars[i].move.x = v.x;
        cars[i].move.z = v.y;
        places.push_back(v); 
        vels.push_back(pathers[i].dir*elapsed);
    }
    for (int i = 0; i < pathers.size(); ++i)
    {
        bool move = true;
        vec2 vel = vels[i];
        vec2 place = places[i];
        for (int j = 0; j < pathers.size(); ++j)
        {
            if (i == j)
                continue;
            vec2 vel2 = vels[j];
            vec2 place2 = places[j];
            if (glm::distance(vel + place, vel2 + place2) < 0.8f)
            {
                if (doIntersect(place, vel, place2, vel2))
                {
                    if (glm::distance(place + vel, place2) < glm::distance(place2 + vel2, place))
                    {
                        move = false;
                        break;
                    }
                }
            }
        }
        if (move)
        {
            pathers[i].progress += elapsed;
            if (pathers[i].progress > pathers[i].dist)
            {

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

    for (int i = 0; i < 3; ++i)
    {
        vehicle pather;
        vector<Edge> edger = generatePath(0,i,edges);

        pather.index = 0;
        pather.destination = i;
        pather.progress = i/2.0f;
        pather.start = edges[0].v1;
        pather.dir = normalize(edges[0].v2-edges[0].v1);
        pather.dist = glm::distance(edges[0].v2,edges[0].v1);

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