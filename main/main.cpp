/***
Es02b: Procedural shaders 1 (random patterns)
- procedural shaders for noise patterns, pressing keys from 1 to 7

N.B. 1) 
In this example we use Shaders Subroutines to do shader swapping:
http://www.geeks3d.com/20140701/opengl-4-shader-subroutines-introduction-3d-programming-tutorial/
https://www.lighthouse3d.com/tutorials/glsl-tutorial/subroutines/
https://www.khronos.org/opengl/wiki/Shader_Subroutine

In other cases, an alternative could be to consider Separate Shader Objects:
https://www.informit.com/articles/article.aspx?p=2731929&seqNum=7
https://www.khronos.org/opengl/wiki/Shader_Compilation#Separate_programs
https://riptutorial.com/opengl/example/26979/load-separable-shader-in-cplusplus

N.B. 2) no texturing in this version of the classes

N.B. 3) to test different parameters of the shaders, it is convenient to use some GUI library, like e.g. Dear ImGui (https://github.com/ocornut/imgui)

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2020/2021
Master degree in Computer Science
Universita' degli Studi di Milano
***/

/***
OpenGL coordinate system (right-handed)
positive X axis points right
positive Y axis points up
positive Z axis points "outside" the screen


                              Y
                              |
                              |
                              |________X
                             /
                            /
                           /
                          Z
***/

// Std. Includes
#include <string>

// Loader estensions OpenGL
// http://glad.dav1d.de/
// THIS IS OPTIONAL AND NOT REQUIRED, ONLY USE THIS IF YOU DON'T WANT GLAD TO INCLUDE windows.h
// GLAD will include windows.h for APIENTRY if it was not previously defined.
// Make sure you have the correct definition for APIENTRY for platforms which define _WIN32 but don't use __stdcall
#ifdef _WIN32
    #define APIENTRY __stdcall
#endif

#include <glad/glad.h>

// GLFW library to create window and to manage I/O
#include <glfw/glfw3.h>

// another check related to OpenGL loader
// confirm that GLAD didn't include windows.h
#ifdef _WINDOWS_
    #error windows.h was included!
#endif

// classes developed during lab lectures to manage shaders and to load models
#include <utils/shader_v1.h>
#include <utils/model_v1.h>

// we load the GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include<stb_image/stb_image.h>

/*** APPLICATION WINDOW ***/
GLuint screenWidth = 800, screenHeight = 600;

/*** INPUT KEY CALLBACK ***/
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
//void mouse_position_callback(GLFWwindow* window, double xpos, double ypos);
void process_keys(GLFWwindow* window);
bool keys[1024];
double mouseXPos;

/*** SHADERS SUBROUTINES ***/
GLuint current_subroutine = 0;
vector<std::string> shaders;
void SetupShader(int shader_program);

/*** TIME ***/
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

/*** TEXTURES ***/
vector<GLint> textureId;
GLint LoadTexture(const char* path);

/*** MOVEMENT ***/
GLfloat deltaZ = 0.0f;
GLfloat deltaX = 0.0f;
GLfloat speed = 2.0f;
GLfloat rotationY = 180.0f;
GLfloat rotationSpeed = 2.0f;

/////////////////// MAIN function ///////////////////////
int main()
{
    /***  INIT GLFW ***/
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    /***  INIT WINDOW ***/
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "The switcher", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    //glfwSetCursorPosCallback(window, mouse_position_callback);

    /***  INIT CONTEXT ***/
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return false;
    }

    /***  INIT VIEWPORT ***/
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.26f, 0.46f, 0.98f, 1.0f);

    /***  INIT SHADERS ***/
    Shader shader = Shader("base.vert", "base.frag");
    SetupShader(shader.Program);
    
    /***  LOAD MODELS ***/
    Model planeModel("../models/plane.obj");
    Model playerModel("../models/player.obj");
    Model treeModel("../models/tree.obj");

    /*** LOAD TEXTURES ***/
    textureId.push_back(LoadTexture("../textures/plane.jpg"));
    textureId.push_back(LoadTexture("../textures/player.png"));
    textureId.push_back(LoadTexture("../textures/tree.jpg"));

    /***  INIT CAMERA ***/
    glm::mat4 projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 10000.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 2.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    /***  INIT MATRICES ***/
    glm::mat4 planeModelMatrix = glm::mat4(1.0f);
    glm::mat4 playerModelMatrix = glm::mat4(1.0f);
    glm::mat4 treeModelMatrix = glm::mat4(1.0f);    

    while(!glfwWindowShouldClose(window))
    {
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        /***  UPDATE TIME ***/
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        /***  CHECK INPUT EVENTS ***/
        glfwPollEvents();
        process_keys(window);

        /***  CLEAR ***/
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /*** USE SHADER ***/
        shader.Use();

        /*** SHADER LOCATIONS ***/
        GLint projectionMatrixLocation = glGetUniformLocation(shader.Program, "projectionMatrix");
        GLint viewMatrixLocation = glGetUniformLocation(shader.Program, "viewMatrix");
        GLint textureLocation = glGetUniformLocation(shader.Program, "tex");
        GLint repeatLocation = glGetUniformLocation(shader.Program, "repeat");
        GLint modelMatrixLocation = glGetUniformLocation(shader.Program, "modelMatrix");

        /*** SET PLANE TEXTURE ***/
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureId[0]);

        /*** PASS VALUES TO SHADER ***/
        glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(view));
        glUniform1i(textureLocation, 1);
        glUniform1f(repeatLocation, 80.0);

        /***  SET PLANE MATRICES ***/
        planeModelMatrix = glm::mat4(1.0f);
        planeModelMatrix = glm::translate(planeModelMatrix, glm::vec3(0.0f, -1.0f, 0.0f));
        planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(10.0f, 1.0f, 10.0f));
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(planeModelMatrix));

        /***  DRAW PLANE ***/
        planeModel.Draw();

        /*** SET PLAYER TEXTURE ***/
        glBindTexture(GL_TEXTURE_2D, textureId[1]);
        glUniform1f(repeatLocation, 1.0);

        /***  SET PLAYER MATRICES ***/
        playerModelMatrix = glm::mat4(1.0f);
        playerModelMatrix = glm::translate(playerModelMatrix, glm::vec3(deltaX, 0.0f, 2.5f + deltaZ));
        playerModelMatrix = glm::rotate(playerModelMatrix, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
        playerModelMatrix = glm::scale(playerModelMatrix, glm::vec3(0.15f, 0.15f, 0.15f));
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(playerModelMatrix));

        /***  DRAW PLAYER ***/
        playerModel.Draw();

        /*** SET TREE TEXTURE ***/
        glBindTexture(GL_TEXTURE_2D, textureId[2]);
        glUniform1f(repeatLocation, 1.0);

        /***  SET TREE MATRICES ***/
        treeModelMatrix = glm::mat4(1.0f);
        treeModelMatrix = glm::translate(treeModelMatrix, glm::vec3(0.0f, 0.0f, -2.0f));
        treeModelMatrix = glm::rotate(treeModelMatrix, glm::radians(23.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        treeModelMatrix = glm::scale(treeModelMatrix, glm::vec3(1.25f, 1.25f, 1.25f));
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(treeModelMatrix));

        /***  DRAW TREE ***/
        treeModel.Draw();

        /*** SWAP BUFFERS ***/
        glfwSwapBuffers(window);
    }

    // when I exit from the graphics loop, it is because the application is closing
    // we delete the Shader Programs
    shader.Delete();
    shader.Delete();
    // we close and delete the created context
    glfwTerminate();
    return 0;
}


//////////////////////////////////////////
// The function parses the content of the Shader Program, searches for the Subroutine type names, 
// the subroutines implemented for each type, print the names of the subroutines on the terminal, and add the names of 
// the subroutines to the shaders vector, which is used for the shaders swapping
void SetupShader(int program)
{
    int maxSub,maxSubU,countActiveSU;
    GLchar name[256]; 
    int len, numCompS;
    
    // global parameters about the Subroutines parameters of the system
    glGetIntegerv(GL_MAX_SUBROUTINES, &maxSub);
    glGetIntegerv(GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS, &maxSubU);
    std::cout << "Max Subroutines:" << maxSub << " - Max Subroutine Uniforms:" << maxSubU << std::endl;

    // get the number of Subroutine uniforms (only for the Fragment shader, due to the nature of the exercise)
    // it is possible to add similar calls also for the Vertex shader
    glGetProgramStageiv(program, GL_FRAGMENT_SHADER, GL_ACTIVE_SUBROUTINE_UNIFORMS, &countActiveSU);
    
    // print info for every Subroutine uniform
    for (int i = 0; i < countActiveSU; i++) {
        
        // get the name of the Subroutine uniform (in this example, we have only one)
        glGetActiveSubroutineUniformName(program, GL_FRAGMENT_SHADER, i, 256, &len, name);
        // print index and name of the Subroutine uniform
        std::cout << "Subroutine Uniform: " << i << " - name: " << name << std::endl;

        // get the number of subroutines
        glGetActiveSubroutineUniformiv(program, GL_FRAGMENT_SHADER, i, GL_NUM_COMPATIBLE_SUBROUTINES, &numCompS);
        
        // get the indices of the active subroutines info and write into the array s
        int *s =  new int[numCompS];
        glGetActiveSubroutineUniformiv(program, GL_FRAGMENT_SHADER, i, GL_COMPATIBLE_SUBROUTINES, s);
        std::cout << "Compatible Subroutines:" << std::endl;
        
        // for each index, get the name of the subroutines, print info, and save the name in the shaders vector
        for (int j=0; j < numCompS; ++j) {
            glGetActiveSubroutineName(program, GL_FRAGMENT_SHADER, s[j], 256, &len, name);
            std::cout << "\t" << s[j] << " - " << name << "\n";
            shaders.push_back(name);
        }
        std::cout << std::endl;
        
        delete[] s;
    }
}

//////////////////////////////////////////
// callback for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
  GLuint new_subroutine;
  
    // if ESC is pressed, we close the application
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if(action == GLFW_PRESS) {
        keys[key] = true;
    }

    if(action == GLFW_RELEASE) {
        keys[key] = false;
    }
}

void process_keys(GLFWwindow* window) {

    if(keys[GLFW_KEY_W]) {
        deltaX += sin(glm::radians(rotationY)) * speed * deltaTime;
        deltaZ += cos(glm::radians(rotationY)) * speed * deltaTime;
    }

    if(keys[GLFW_KEY_S]) {
        deltaX -= sin(glm::radians(rotationY)) * speed * deltaTime;
        deltaZ -= cos(glm::radians(rotationY)) * speed * deltaTime;
    }

    if(keys[GLFW_KEY_A]) {
        deltaX += cos(glm::radians(rotationY)) * speed * deltaTime;
        deltaZ -= sin(glm::radians(rotationY)) * speed * deltaTime;
    }

    if(keys[GLFW_KEY_D]) {
        deltaX -= cos(glm::radians(rotationY)) * speed * deltaTime;
        deltaZ += sin(glm::radians(rotationY)) * speed * deltaTime;
    }

    if(keys[GLFW_MOUSE_BUTTON_RIGHT]) {
        double xPos, unused;
        glfwGetCursorPos(window, &xPos, &unused);
        double diff = mouseXPos - xPos;
        mouseXPos = xPos;
        rotationY += diff * rotationSpeed;
    }
}

GLint LoadTexture(const char* path)
{
    GLuint textureImage;
    int w, h, channels;
    unsigned char* image;
    image = stbi_load(path, &w, &h, &channels, STBI_rgb);

    if (image == nullptr)
        std::cout << "Failed to load texture!" << std::endl;

    glGenTextures(1, &textureImage);
    glBindTexture(GL_TEXTURE_2D, textureImage);
    // 3 channels = RGB ; 4 channel = RGBA
    if (channels==3)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    else if (channels==4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    // we set how to consider UVs outside [0,1] range
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // we set the filtering for minification and magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    // we free the memory once we have created an OpenGL texture
    stbi_image_free(image);

    // we set the binding to 0 once we have finished
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureImage;
}

///////////////////////////////////////
// callback for mouse click
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if(action == GLFW_PRESS) {
        keys[button] = true;
    }
    if(action == GLFW_RELEASE) {
        keys[button] = false;
    }

    if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        double unused;
        glfwGetCursorPos(window, &mouseXPos, &unused);
    }
}
