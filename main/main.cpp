#include <iostream>

// THIS IS OPTIONAL AND NOT REQUIRED, ONLY USE THIS IF YOU DON'T WANT GLAD TO INCLUDE windows.h
// GLAD will include windows.h for APIENTRY if it was not previously defined.
// Make sure you have the correct definition for APIENTRY for platforms which define _WIN32 but don't use __stdcall
#ifdef _WIN32
    #define APIENTRY __stdcall
#endif

// GLEW
//#define GLEW_STATIC
//#include <glew/glew.h>

#include <glad/glad.h>

// GLFW
#include <glfw/glfw3.h>

// we load the GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// confirm that GLAD didn't include windows.h
#ifdef _WINDOWS_
    #error windows.h was included!
#endif


#include<utils/shader_v1.h>
#include<utils/model_v1.h>
#include<utils/camera.h>

// parameters for time calculation (for animations)
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

bool keys[1024];

// boolean to activate/deactivate wireframe rendering
GLboolean wireframe = GL_FALSE;

// Uniforms to pass to shaders
// color to be passed to Fullcolor and Flatten shaders
GLfloat myColor[] = {1.0f,0.0f,0.0f};
// weight and velocity for the animation of Wave shader
GLfloat weight = 0.2f;
GLfloat speed = 5.0f;

// if one of the WASD keys is pressed, we call the corresponding method of the Camera class
void apply_camera_movements();


GLfloat cameraX = 0.0f;
GLfloat cameraY = 0.0f;
GLfloat cameraZ = 7.0f;
// we create a camera. We pass the initial position as a parameter to the constructor. The last boolean tells that we want a camera "anchored" to the ground
Camera camera(glm::vec3(cameraX, cameraY, cameraZ), GL_TRUE);

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

// The MAIN function, from here we start the application and run the game loop
int main()
{
    std::cout << "Starting GLFW context" << std::endl;
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    float screenWidth = 800.0f;
    float screenHeight = 600.0f;

    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "main", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);

    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    //glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    //glewInit();

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    // Define the viewport dimensions
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0f, 1.0f, 1.0f, 1.0f);

    Shader shader("base.vert", "base.frag");

    Model warriorModel("../models/robot.obj");

    shader.Use();

    // we set projection and view matrices
    // N.B.) in this case, the camera is fixed -> we set it up outside the rendering loop
    // Projection matrix: FOV angle, aspect ratio, near and far planes
    glm::mat4 projection = glm::perspective(45.0f, screenWidth/screenHeight, 0.1f, 10000.0f);

    // View matrix: the camera moves, so we just set to indentity now
    glm::mat4 view = glm::mat4(1.0f);

    // Model and Normal transformation matrices for the objects in the scene: we set to identity
    glm::mat4 warriorModelMatrix = glm::mat4(1.0f);
    glm::mat3 warriorNormalMatrix = glm::mat3(1.0f);

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // we determine the time passed from the beginning
        // and we calculate time difference between current frame rendering and the previous one
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();

        apply_camera_movements();
        // View matrix (=camera): position, view direction, camera "up" vector
        view = camera.GetViewMatrix();

        // Render
        // Clear the colorbuffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        GLint fragColorLocation = glGetUniformLocation(shader.Program, "colorIn");
        glUniform3fv(fragColorLocation, 1, myColor);

        GLint weightLocation = glGetUniformLocation(shader.Program, "weight");
        glUniform1f(weightLocation, weight);

        GLint timerLocation = glGetUniformLocation(shader.Program, "timer");
        glUniform1f(timerLocation, currentFrame * speed);

        glUniformMatrix4fv(
            glGetUniformLocation(shader.Program, "projectionMatrix"),
            1,
            GL_FALSE,
            glm::value_ptr(projection)
        );

        glUniformMatrix4fv(
            glGetUniformLocation(shader.Program, "viewMatrix"),
            1,
            GL_FALSE,
            glm::value_ptr(view)
        );

        warriorModelMatrix = glm::mat4(1.0f);
        warriorNormalMatrix = glm::mat3(1.0f);
        GLfloat diffX = camera.Position.x - cameraX;
        GLfloat diffY = camera.Position.y - cameraY;
        GLfloat diffZ = camera.Position.z - cameraZ;
        warriorModelMatrix = glm::translate(warriorModelMatrix, glm::vec3(0.0f+diffX, -2.0f+diffY, 4.0f+diffZ));
        warriorModelMatrix = glm::rotate(warriorModelMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        warriorModelMatrix = glm::scale(warriorModelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));	// It's a bit too big for our scene, so scale it down
        // if we cast a mat4 to a mat3, we are automatically considering the upper left 3x3 submatrix
        warriorNormalMatrix = glm::inverseTranspose(glm::mat3(view*warriorModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(warriorModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(warriorNormalMatrix));

        // we render the warrior
        warriorModel.Draw();

        // Swap the screen buffers
        glfwSwapBuffers(window);
    }

    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    shader.Delete();
    return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if(key == GLFW_KEY_P  && action == GLFW_PRESS) {
        wireframe = !wireframe;
    }

    if(action == GLFW_PRESS)
        keys[key] = true;
    else if(action == GLFW_RELEASE)
        keys[key] = false;
}

//////////////////////////////////////////
// If one of the WASD keys is pressed, the camera is moved accordingly (the code is in utils/camera.h)
void apply_camera_movements()
{
    if(keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if(keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if(keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if(keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
}
