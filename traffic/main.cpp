#include "headers.hpp"

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
        edges[i].v1 += normalize(edges[i].v2-edges[i].v1)*0.4f;
        edges[i].v2 += normalize(edges[i].v1-edges[i].v2)*0.4f;
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
    float speed = 0.5;
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
    pred_map pd = predecessor_map(boost::make_iterator_property_map(p.begin(), get(boost::vertex_index, g))).
                            distance_map(boost::make_iterator_property_map(d.begin(), get(boost::vertex_index, g)));


    vector<street> streets;
    for (int i = 0; i < edges.size(); ++i)
    {   
        street temp(edges[i]);
        vec2 o = temp.v2;
        vector<vector<Edge>> intersects;
        for (int j = 0; j < temp.neighbors.size(); ++j)
        {
            vec2 adder = normalize(edges[temp.neighbors[j]].v1-o)*0.2f;

            vec2 dir = (edges[temp.neighbors[j]].v1);

            dir += adder;
            o -= adder;

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
                if (glm::distance(o,o2) > 3.0)
                    continue;
                vec2 dir2 = edges[edge_vector[k].second].v1;
                adder =  normalize(edges[edge_vector[k].second].v1-o2)*0.2f;
                dir2 += adder;
                o2 -= adder;
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

    vector<street*> streetsp;

    for (int i = 0; i < streets.size(); ++i)
        streetsp.push_back(&streets[i]);
    int ncars = 2000;
    vehicles.reserve(ncars);
    cars.resize(ncars, car);
    for (int i = 0; i < ncars; ++i)
    {
        vehicle pather(i);
        //vector<int> path = generatePath(i/10,0,g,weightmap,p,d,streetsp,pd);
        int start = i/10;
        int end = 0;
        pather.destination = end;
        pather.index = start;
        pather.progress = (i%10)*0.4f+i*0.01f;
        pather.start = streets[pather.index].v1;
        pather.dir = streets[pather.index].dir;
        pather.dist = streets[pather.index].dist;
        //pather.path = path;
        //pather.turn = Edge(path.front(), path[1]);
        vehicles.push_back(pather);
        streets[pather.index].addvehicle(&vehicles[vehicles.size()-1]);
    }
    do{	
        double elapsed = fps(nbFrames, totalTime, lastTime, last)*speed;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        computeMatricesFromInputs();

        terrain.draw();
        for (int i = 0; i < cars.size(); ++i)
            cars[i].draw();
        float iters = 5;
        for (float i = 0; i < iters; ++i)
            processCars(cars, vehicles, streets, scaler, elapsed/(100.0f*iters),g,weightmap,p,d,streetsp,pd);

        glfwSwapBuffers();

    } while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS &&
                glfwGetWindowParam( GLFW_OPENED ) );

     glfwTerminate();

    return 0;
}