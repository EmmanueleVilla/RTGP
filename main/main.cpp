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

// parameters for time calculation (for animations)
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// rotation angle on Y axis
GLfloat orientationY = 0.0f;
// rotation speed on Y axis
GLfloat spin_speed = 30.0f;
// boolean to start/stop animated rotation on Y angle
GLboolean spinning = GL_TRUE;

// boolean to activate/deactivate wireframe rendering
GLboolean wireframe = GL_FALSE;

// Uniforms to pass to shaders
// color to be passed to Fullcolor and Flatten shaders
GLfloat myColor[] = {1.0f,0.0f,0.0f};
// weight and velocity for the animation of Wave shader
GLfloat weight = 0.2f;
GLfloat speed = 5.0f;

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
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Work01", nullptr, nullptr);
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

    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

    Shader shader("base.vert", "base.frag");

    Model cubeModel("../models/cube.obj");
    Model sphereModel("../models/sphere.obj");
    Model bunnyModel("../models/bunny_lp.obj");

    shader.Use();

    // we set projection and view matrices
    // N.B.) in this case, the camera is fixed -> we set it up outside the rendering loop
    // Projection matrix: FOV angle, aspect ratio, near and far planes
    glm::mat4 projection = glm::perspective(45.0f, screenWidth/screenHeight, 0.1f, 10000.0f);
    // View matrix (=camera): position, view direction, camera "up" vector
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 7.0f), glm::vec3(0.0f, 0.0f, -7.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // Model and Normal transformation matrices for the objects in the scene: we set to identity
    glm::mat4 sphereModelMatrix = glm::mat4(1.0f);
    glm::mat3 sphereNormalMatrix = glm::mat3(1.0f);
    glm::mat4 cubeModelMatrix = glm::mat4(1.0f);
    glm::mat3 cubeNormalMatrix = glm::mat3(1.0f);
    glm::mat4 bunnyModelMatrix = glm::mat4(1.0f);
    glm::mat3 bunnyNormalMatrix = glm::mat3(1.0f);

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

        // Render
        // Clear the colorbuffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        if(spinning) {
            orientationY += deltaTime * spin_speed;
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

        sphereModelMatrix = glm::mat4(1.0f);
        sphereNormalMatrix = glm::mat3(1.0f);
        sphereModelMatrix = glm::translate(sphereModelMatrix, glm::vec3(-3.0f, 0.0f, 0.0f));
        sphereModelMatrix = glm::rotate(sphereModelMatrix, glm::radians(orientationY), glm::vec3(0.0f, 1.0f, 0.0f));
        sphereModelMatrix = glm::scale(sphereModelMatrix, glm::vec3(0.8f, 0.8f, 0.8f));	// It's a bit too big for our scene, so scale it down
        // if we cast a mat4 to a mat3, we are automatically considering the upper left 3x3 submatrix
        sphereNormalMatrix = glm::inverseTranspose(glm::mat3(view*sphereModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(sphereModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(sphereNormalMatrix));

        // we render the sphere
        sphereModel.Draw();

        cubeModelMatrix = glm::mat4(1.0f);
        cubeNormalMatrix = glm::mat3(1.0f);
        cubeModelMatrix = glm::translate(cubeModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
        cubeModelMatrix = glm::rotate(cubeModelMatrix, glm::radians(orientationY), glm::vec3(0.0f, 1.0f, 0.0f));
        cubeModelMatrix = glm::scale(cubeModelMatrix, glm::vec3(0.8f, 0.8f, 0.8f));
        cubeNormalMatrix = glm::inverseTranspose(glm::mat3(view * cubeModelMatrix));
        glUniformMatrix4fv(
            glGetUniformLocation(shader.Program, "modelMatrix"),
            1,
            GL_FALSE,
            glm::value_ptr(cubeModelMatrix)
        );
        glUniformMatrix4fv(
            glGetUniformLocation(shader.Program, "normalMatrix"),
            1,
            GL_FALSE,
            glm::value_ptr(cubeNormalMatrix)
        );
        cubeModel.Draw();

        bunnyModelMatrix = glm::mat4(1.0f);
        bunnyNormalMatrix = glm::mat3(1.0f);
        bunnyModelMatrix = glm::translate(bunnyModelMatrix, glm::vec3(3.0f, 0.0f, 0.0f));
        bunnyModelMatrix = glm::rotate(bunnyModelMatrix, glm::radians(orientationY), glm::vec3(0.0f, 1.0f, 0.0f));
        bunnyModelMatrix = glm::scale(bunnyModelMatrix, glm::vec3(0.3f, 0.3f, 0.3f));
        bunnyNormalMatrix = glm::inverseTranspose(glm::mat3(view * bunnyModelMatrix));
        glUniformMatrix4fv(
            glGetUniformLocation(shader.Program, "modelMatrix"),
            1,
            GL_FALSE,
            glm::value_ptr(bunnyModelMatrix)
        );
        glUniformMatrix4fv(
            glGetUniformLocation(shader.Program, "normalMatrix"),
            1,
            GL_FALSE,
            glm::value_ptr(bunnyNormalMatrix)
        );
        bunnyModel.Draw();

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

    if(key == GLFW_KEY_S  && action == GLFW_PRESS) {
        spinning = !spinning;
    }

    if(key == GLFW_KEY_W  && action == GLFW_PRESS) {
        wireframe = !wireframe;
    }
}
