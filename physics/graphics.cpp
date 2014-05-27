#include <physics/graphics.hpp>

int startup(int W, int H, bool use2d)
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	if( !glfwOpenWindow( W, H, 0,0,0,0, 32,0, GLFW_WINDOW ) )
	{
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		return -1;
	}

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	if (!use2d)
	{
		glfwDisable( GLFW_MOUSE_CURSOR );
		glfwSetMousePos(W/2, H/2);
	}


	glfwSetWindowTitle( "physics sim" );

	// Ensure we can capture the escape key being pressed below
	glfwEnable( GLFW_STICKY_KEYS );

	// Dark blue background
	glClearColor(0.95f, 0.95f, 0.95f, 0.0f);


	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 
    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);       // Blending Function For Translucency Based On Source Alpha 
    glEnable(GL_BLEND);     // Turn Blending On
    glEnable(GL_ALPHA_TEST);
	// Cull triangles which normal is not towards the camera
	//glEnable(GL_CULL_FACE);
    

    glMatrixMode( GL_PROJECTION ); //Switch to setting the camera perspective

    //Set the camera perspective

    glLoadIdentity(); //reset the camera

    gluPerspective( 45.0f,                      //camera angle

                (GLfloat)W/(GLfloat)H, //The width to height ratio

                 0.001f,                          //The near z clipping coordinate

                110.0f );    
	// For speed computation

    return 0;
}

double fps(int& nbFrames, double& totalTime, double& lastTime, double &last)
{
	// Measure speed
	double currentTime = glfwGetTime();
	nbFrames++;
    double diff = currentTime-last;
    totalTime += diff;
        
	if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1sec ago
		// printf and reset
		printf("%f ms/frame\n", 1000.0/double(nbFrames));
		nbFrames = 0;
		lastTime += 1.0;
	}
    return diff; 
}