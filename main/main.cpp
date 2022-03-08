/*
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
*/

/*
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
*/

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

// dimensions of application's window
GLuint screenWidth = 800, screenHeight = 600;

// callback function for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// index of the current shader subroutine (= 0 in the beginning)
GLuint current_subroutine = 0;
// a vector for all the shader subroutines names used and swapped in the application
vector<std::string> shaders;

// the name of the subroutines are searched in the shaders, and placed in the shaders vector (to allow shaders swapping)
void SetupShader(int shader_program);

// print on console the name of current shader subroutine
void PrintCurrentShader(int subroutine);

// parameters for time computation
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

vector<GLint> textureId;

//////////////////////////////////////////
// we load the image from disk and we create an OpenGL texture
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

/////////////////// MAIN function ///////////////////////
int main()
{
    /*  INIT GLFW */
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    /*  INIT WINDOW */
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

    /*  INIT CONTEXT */
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return false;
    }

    /*  INIT VIEWPORT */
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.26f, 0.46f, 0.98f, 1.0f);

    /*  INIT SHADERS */
    Shader shader = Shader("base.vert", "base.frag");
    SetupShader(shader.Program);
    PrintCurrentShader(current_subroutine);
    
    /*  LOAD MODELS */
    Model femaleModel("../models/female.obj");
    Model sphereModel("../models/sphere.obj");
    Model bunnyModel("../models/bunny_lp.obj");
    Model planeModel("../models/plane.obj");

    /* LOAD TEXTURES */
    textureId.push_back(LoadTexture("../textures/floor_wood.jpg"));

    /*  INIT CAMERA */
    glm::mat4 projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 10000.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 7.0f), glm::vec3(0.0f, 0.0f, -7.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    /*  INIT MATRICES */
    glm::mat4 sphereModelMatrix = glm::mat4(1.0f);
    glm::mat3 sphereNormalMatrix = glm::mat3(1.0f);
    glm::mat4 femaleModelMatrix = glm::mat4(1.0f);
    glm::mat3 femaleNormalMatrix = glm::mat3(1.0f);
    glm::mat4 bunnyModelMatrix = glm::mat4(1.0f);
    glm::mat3 bunnyNormalMatrix = glm::mat3(1.0f);
    glm::mat4 planeModelMatrix = glm::mat4(1.0f);

    while(!glfwWindowShouldClose(window))
    {
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        /*  UPDATE TIME */
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        /*  CHECK INPUT EVENTS */
        glfwPollEvents();

        /*  CLEAR */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* USE SHADER */
        shader.Use();

        /* SHADER LOCATIONS */
        GLint projectionMatrixLocation = glGetUniformLocation(shader.Program, "projectionMatrix");
        GLint viewMatrixLocation = glGetUniformLocation(shader.Program, "viewMatrix");
        GLint textureLocation = glGetUniformLocation(shader.Program, "texture");
        GLint repeatLocation = glGetUniformLocation(shader.Program, "repeat");
        GLint modelMatrixLocation = glGetUniformLocation(shader.Program, "modelMatrix");

        /* SET PLANE TEXTURE */
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureId[0]);

        /* PASS VALUES TO SHADER */
        glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(view));
        glUniform1i(textureLocation, 1);
        glUniform1f(repeatLocation, 80.0);

        /*  SET PLANE MATRICES */
        planeModelMatrix = glm::mat4(1.0f);
        planeModelMatrix = glm::translate(planeModelMatrix, glm::vec3(0.0f, -1.0f, 0.0f));
        planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(10.0f, 1.0f, 10.0f));
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(planeModelMatrix));

        /*  DRAW PLANE */
        planeModel.Draw();

        /* SWAP BUFFERS */
        glfwSwapBuffers(window);

        continue;


        /////////////////// OBJECTS ////////////////////////////////////////////////
        // We "install" the noise_shader Shader Program as part of the current rendering process
        shader.Use();
        // we search inside the Shader Program the name of the subroutine currently selected, and we get the numerical index
        GLuint index = glGetSubroutineIndex(shader.Program, GL_FRAGMENT_SHADER, "redColor");
        // we activate the subroutine using the index (this is where shaders swapping happens)
        glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &index);

        // we pass projection and view matrices to the Shader Program
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));

        // SPHERE
        /*
          we create the transformation matrix

          N.B.) the last defined is the first applied

          We need also the matrix for normals transformation, which is the inverse of the transpose of the 3x3 submatrix (upper left) of the modelview. We do not consider the 4th column because we do not need translations for normals.
          An explanation (where XT means the transpose of X, etc):
            "Two column vectors X and Y are perpendicular if and only if XT.Y=0. If We're going to transform X by a matrix M, we need to transform Y by some matrix N so that (M.X)T.(N.Y)=0. Using the identity (A.B)T=BT.AT, this becomes (XT.MT).(N.Y)=0 => XT.(MT.N).Y=0. If MT.N is the identity matrix then this reduces to XT.Y=0. And MT.N is the identity matrix if and only if N=(MT)-1, i.e. N is the inverse of the transpose of M.

        */
        // we reset to identity at each frame
        sphereModelMatrix = glm::mat4(1.0f);
        sphereNormalMatrix = glm::mat3(1.0f);
        sphereModelMatrix = glm::translate(sphereModelMatrix, glm::vec3(-3.0f, 0.0f, 0.0f));
        sphereModelMatrix = glm::rotate(sphereModelMatrix, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        sphereModelMatrix = glm::scale(sphereModelMatrix, glm::vec3(0.8f, 0.8f, 0.8f));
        // if we cast a mat4 to a mat3, we are automatically considering the upper left 3x3 submatrix
        sphereNormalMatrix = glm::inverseTranspose(glm::mat3(view*sphereModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(sphereModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(sphereNormalMatrix));

        // we render the model
        sphereModel.Draw();


        index = glGetSubroutineIndex(shader.Program, GL_FRAGMENT_SHADER, "greenColor");
        // we activate the subroutine using the index (this is where shaders swapping happens)
        glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &index);

        //female
        // we create the transformation matrix and the normals transformation matrix
        // we reset to identity at each frame
        femaleModelMatrix = glm::mat4(1.0f);
        femaleNormalMatrix = glm::mat3(1.0f);
        femaleModelMatrix = glm::translate(femaleModelMatrix, glm::vec3(0.0f, -0.75f, 5.5f));
        femaleModelMatrix = glm::rotate(femaleModelMatrix, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        femaleModelMatrix = glm::scale(femaleModelMatrix, glm::vec3(0.3f, 0.3f, 0.3f));	// It's a bit too big for our scene, so scale it down
        femaleNormalMatrix = glm::inverseTranspose(glm::mat3(view*femaleModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(femaleModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(femaleNormalMatrix));

        // we render the female
        femaleModel.Draw();


        index = glGetSubroutineIndex(shader.Program, GL_FRAGMENT_SHADER, "redColor");
        // we activate the subroutine using the index (this is where shaders swapping happens)
        glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &index);

        //BUNNY
        // we create the transformation matrix and the normals transformation matrix
        // we reset to identity at each frame
        bunnyModelMatrix = glm::mat4(1.0f);
        bunnyNormalMatrix = glm::mat3(1.0f);
        bunnyModelMatrix = glm::translate(bunnyModelMatrix, glm::vec3(3.0f, 0.0f, 0.0f));
        bunnyModelMatrix = glm::rotate(bunnyModelMatrix, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        bunnyModelMatrix = glm::scale(bunnyModelMatrix, glm::vec3(0.3f, 0.3f, 0.3f));	// It's a bit too big for our scene, so scale it down
        bunnyNormalMatrix = glm::inverseTranspose(glm::mat3(view*bunnyModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(bunnyModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(bunnyNormalMatrix));

        // Swapping back and front buffers
        bunnyModel.Draw();
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
// we print on console the name of the currently used shader subroutine
void PrintCurrentShader(int subroutine)
{
    std::cout << "Current shader subroutine: " << shaders[subroutine]  << std::endl;
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
}