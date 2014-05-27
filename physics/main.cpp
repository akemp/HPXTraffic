#include "headers.hpp"

int main()
{
    using namespace std;
    using namespace boost;
    using namespace glm;
    float len = 200;
	LineMesh roadsegs = generateRoads(8,4, len);
    
    vector<Edge> egs;
    vector<vec2> inputted;
    
    vector<edger> edges;
    
    vector<vector<vec3>> quads;

    vector<unsigned int> indices;
    vector<VertexData> vertex_data;

    generateRoadModels(roadsegs, indices, vertex_data);
    
    //return 0;
    
    int code = startup(1200,600);

    if (code != 0)
        return code;
    
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);


    Mesh terrain(vertex_data,indices, LoadShaders( "vert.glsl", "frag.glsl" ), loadDDS("asphalt.dds"));
    terrain.move.y = -3.0;

    
    double lastTime = glfwGetTime();
    int nbFrames = 0;
    double last = glfwGetTime();
    float speed = 1.0;
    double totalTime = 0;


    do{	
        double elapsed = fps(nbFrames, totalTime, lastTime, last)*speed;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        computeMatricesFromInputs();

        terrain.draw();
        //elapsed = 1.0f;//glm::min(10.0,elapsed);
        //elapsed = 100.0f;

        glfwSwapBuffers();

    } while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS &&
                glfwGetWindowParam( GLFW_OPENED ) );
     glfwTerminate();
     //*/

    return 0;
}