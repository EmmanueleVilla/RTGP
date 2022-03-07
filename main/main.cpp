#include <iostream>

#ifdef _WIN32
    #define APIENTRY __stdcall
#endif

#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifdef _WINDOWS_
    #error windows.h was included!
#endif

#include<utils/shader_v1.h>
#include<utils/model_v1.h>
#include<utils/camera.h>

#pragma region

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);

void setup_shader(int shared_program);

void apply_camera_movements();

#pragma endregion Functions prototypes

#pragma region

void setup_shader(int shader_program) {
    int maxSub. maxSubU, countActiveSU;
    GLchar name[256];
    int len, numCompS;

    glGetIntegerv(GL_MAX_SUBROUTINES, &maxSub);
    glGetInteger(GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS, &maxSubU);
    glGetProgramStageiv(shader_program, GL_FRAGMENT_SHADER, GL_ACTIVE_SUBROUTINE_UNIFORMS, &countActiveSU);

    for (int i = 0; i < countActiveSU; i++) {

        glGetActiveSubroutineUniformName(program, GL_FRAGMENT_SHADER, i, 256, &len, name);
        std::cout << "Subroutine Uniform: " << i << " - name: " << name << std::endl;
        glGetActiveSubroutineUniformiv(program, GL_FRAGMENT_SHADER, i, GL_NUM_COMPATIBLE_SUBROUTINES, &numCompS);
        int *s =  new int[numCompS];
        glGetActiveSubroutineUniformiv(program, GL_FRAGMENT_SHADER, i, GL_COMPATIBLE_SUBROUTINES, s);
        std::cout << "Compatible Subroutines:" << std::endl;
        for (int j=0; j < numCompS; ++j) {
            glGetActiveSubroutineName(program, GL_FRAGMENT_SHADER, s[j], 256, &len, name);
            std::cout << "\t" << s[j] << " - " << name << "\n";
            shaders.push_back(name);
        }
    std::cout << std::endl;
    delete[] s;
}

#pragma endregion implementations

#pragma region

GLuint current_subroutine = 0;

vector<std::string> shaders;

#pragma endregion Shader handling

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


GLfloat cameraX = 0.0f;
GLfloat cameraY = 0.0f;
GLfloat cameraZ = 7.0f;
// we create a camera. We pass the initial position as a parameter to the constructor. The last boolean tells that we want a camera "anchored" to the ground
Camera camera(glm::vec3(cameraX, cameraY, cameraZ), GL_TRUE);

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

// color to be passed as uniform to the shader of the plane
GLfloat planeMaterial[] = {0.0f,0.5f,0.0f};

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

    Model robotModel("../models/robot.obj");
    Model planeModel("../models/plane.obj");

    shader.Use();

    Shaders shaders(shader.Program);

    // we set projection and view matrices
    // N.B.) in this case, the camera is fixed -> we set it up outside the rendering loop
    // Projection matrix: FOV angle, aspect ratio, near and far planes
    glm::mat4 projection = glm::perspective(45.0f, screenWidth/screenHeight, 0.1f, 10000.0f);

    // View matrix: the camera moves, so we just set to indentity now
    glm::mat4 view = glm::mat4(1.0f);

    // Model and Normal transformation matrices for the objects in the scene: we set to identity
    glm::mat4 robotModelMatrix = glm::mat4(1.0f);
    glm::mat3 robotNormalMatrix = glm::mat3(1.0f);

    glm::mat4 planeModelMatrix = glm::mat4(1.0f);
    glm::mat3 planeNormalMatrix = glm::mat3(1.0f);

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

        GLint projectionMatrixLocation = glGetUniformLocation(shader.Program, "projectionMatrix");
        glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projection));

        GLint viewMatrixLocation = glGetUniformLocation(shader.Program, "viewMatrix");

        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(view));

        GLuint index = glGetSubroutineIndex(shader.Program, GL_FRAGMENT_SHADER, "greenColor");
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &index);

        // we create the transformation matrix
        // we reset to identity at each frame
        planeModelMatrix = glm::mat4(1.0f);
        planeNormalMatrix = glm::mat3(1.0f);
        planeModelMatrix = glm::translate(planeModelMatrix, glm::vec3(0.0f, -1.0f, 0.0f));
        planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(10.0f, 1.0f, 10.0f));
        planeNormalMatrix = glm::inverseTranspose(glm::mat3(view*planeModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(planeNormalMatrix));

        // we render the plane
        planeModel.Draw();

        index = glGetSubroutineIndex(shader.Program, GL_FRAGMENT_SHADER, "redColor");
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &index);

        robotModelMatrix = glm::mat4(1.0f);
        robotNormalMatrix = glm::mat3(1.0f);
        GLfloat diffX = camera.Position.x - cameraX;
        GLfloat diffY = camera.Position.y - cameraY;
        GLfloat diffZ = camera.Position.z - cameraZ;
        robotModelMatrix = glm::translate(robotModelMatrix, glm::vec3(0.0f+diffX, -2.0f+diffY, 4.0f+diffZ));
        robotModelMatrix = glm::rotate(robotModelMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        robotModelMatrix = glm::scale(robotModelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));	// It's a bit too big for our scene, so scale it down
        // if we cast a mat4 to a mat3, we are automatically considering the upper left 3x3 submatrix
        robotNormalMatrix = glm::inverseTranspose(glm::mat3(view*robotModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(robotModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(robotNormalMatrix));

        // we render the robot
        robotModel.Draw();

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

