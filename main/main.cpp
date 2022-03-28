//---  Std. Includes
#include <string>
#define GLM_ENABLE_EXPERIMENTAL
//---  Loader estensions OpenGL
#ifdef _WIN32
    #define APIENTRY __stdcall
#endif

#include <chrono>

#include <glad/glad.h>

//---  GLFW library to create window and to manage I/O
#include <glfw/glfw3.h>

//---  confirm that GLAD didn't include windows.h
#ifdef _WINDOWS_
    #error windows.h was included!
#endif

//---  classes developed during lab lectures to manage shaders and to load models
#include <utils/shader_v1.h>
#include <utils/model_v1.h>
#include <utils/aabb.h>
#include <utils/csv_loader.h>

//---  we load the GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include<stb_image/stb_image.h>

//---  APPLICATION WINDOW 
GLuint screenWidth = 1280, screenHeight = 720;

//---  INPUT KEY CALLBACK 
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
//void mouse_position_callback(GLFWwindow* window, double xpos, double ypos);
void process_keys(GLFWwindow* window);
bool keys[1024];
double mouseXPos;

//---  SHADERS SUBROUTINES 
GLuint current_subroutine = 0;
vector<std::string> shaders;
void SetupShader(int shader_program);

//---  TIME 
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

//---  TEXTURES 
vector<GLint> textureId;
GLint LoadTexture(const char* path);

//---  MOVEMENT 
GLfloat oldDeltaZ = 0.0f;
GLfloat oldDeltaX = 0.0f;
GLfloat deltaZ = 0.0f;
GLfloat deltaX = 0.0f;
GLfloat speed = 5.0f;
GLfloat rotationY = 90.0f;
GLfloat rotationSpeed = 2.0f;

//--- CAMERA
#define MAX_CAMERA_DISTANCE 5.0f
#define MIN_CAMERA_DISTANCE 2.5f
#define MAX_CAMERA_Y_DELTA 1.0f
#define MIN_CAMERA_Y_DELTA 0.0f
GLfloat cameraDistance = MAX_CAMERA_DISTANCE;
GLfloat cameraY = MAX_CAMERA_Y_DELTA;
GLfloat cameraZoomSpeed = 3.0f;

//--- PINCUSHION DISTORSION
#define MIN_DISTORSION -1.00f
#define MAX_DISTORSION 0.0f
GLfloat distorsion = MAX_DISTORSION;
GLfloat distorsionSpeed = 0.75f;

//---  APP_STATE 
enum class AppStates { LoadingMap, LoadingAABBs, CreatingAABBsHierarchy, Loaded };
AppStates appState = AppStates::LoadingMap;

/////////////////// MAIN function ///////////////////////
int main()
{
    cout << "Initializing app" << endl;

    //--- INIT RANDOM SEED
    srand (time(NULL));

    //---  INIT GLFW 
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    //---  INIT WINDOW 
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

    //---  INIT CONTEXT 
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    //---  INIT VIEWPORT 
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //---  INIT SHADERS 
    Shader shader = Shader("base.vert", "base.frag");
    SetupShader(shader.Program);
    
    //---  LOAD MODELS 
    Model coinModel("../models/coin.obj");
    Model planeModel("../models/plane.obj");
    Model playerModel("../models/dog.obj");
    Model treeModel("../models/tree.obj");
    Model cartModel("../models/cart.obj");

    //--- LOAD TEXTURES 
    int coinTextureIndex = 0;
    int planeTextureIndex = 1;
    int playerTextureIndex = 2;
    int treeTextureIndex = 3;
    int cartTextureIndex = 4;
    textureId.push_back(LoadTexture("../textures/coin.jpeg"));
    textureId.push_back(LoadTexture("../textures/plane.jpg"));
    textureId.push_back(LoadTexture("../textures/dog.jpg"));
    textureId.push_back(LoadTexture("../textures/tree.jpg"));
    textureId.push_back(LoadTexture("../textures/cart.jpg"));

    cout << "Loaded textures" << endl;

    //---  INIT MATRICES 
    glm::mat4 coinModelMatrix = glm::mat4(1.0f);
    glm::mat4 playerModelMatrix = glm::mat4(1.0f);
    glm::mat4 cartModelMatrix = glm::mat4(1.0f);
    glm::mat4 planeModelMatrix = glm::mat4(1.0f);

    //--- INIT FIXED PLANE MATRIX
    planeModelMatrix = glm::mat4(1.0f);
    planeModelMatrix = glm::translate(planeModelMatrix, glm::vec3(32.0f, 0.0f, 32.0f));
    planeModelMatrix = glm::rotate(planeModelMatrix, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(2.0f, 2.0f, 2.0f));

    //--- PLANE PATH DATA
    vector<glm::vec2> paths;

    //--- COIN DATA
    float coinRotationY = 0.0f;
    float coinRotationSpeed = 20.0f;

    //--- CART DATA
    float cartX = 0.0f;
    float cartZ = 0.0f;
    GLfloat cartColor[] = { 0.65f, 0.16f, 0.16f };

    //--- AABB WIREFRAME COLOR
    GLfloat aabbColor[] = { 1.0f, 0.0f, 0.0f };

    //---  INIT CAMERA 
    glm::mat4 projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 10000.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    //--- LOAD CSV DATA FILE
    //--- I EXPECT A 32x32 CSV LIKE THE PLANE OF THE SIZE
    cout << "Loading map file" << endl;

    vector<vector<string>> content = CsvLoader().read("../data/map.csv");
	
    //--- KEEP TRACK OF THE CURRENT ROW TO LOAD ONE ROW PER RENDER LOOP
    int currentCell = 0;

    //--- INIT MATRIXES FOR INSTANCED DRAWING 
    vector<glm::mat4> treesMatrixes;
    vector<AABB> AABBs;

    //--- COMMON INDICES LIST FOR AABBs
    GLuint AABBsIndices[] = {
            0, 1, 3,
            1, 2, 3,
            2, 3, 6,
            3, 6, 7,
            5, 6, 7,
            4, 5, 7,
            0, 4, 5,
            0, 1, 5,
            0, 3, 4,
            0, 4, 7,
            1, 2, 6,
            1, 5, 6
        };

    cout << "Starting loading loop" << endl;

    //--- TIME MEASUREMENT
    long long maxMicroseconds = -1;

    //--- LOADING RENDER LOOP
    while(appState == AppStates::LoadingMap || appState == AppStates::LoadingAABBs || appState == AppStates::CreatingAABBsHierarchy)
    {
        if(glfwWindowShouldClose(window)) {
            shader.Delete();
            glfwTerminate();
            return 0;
        }

        if(appState == AppStates::CreatingAABBsHierarchy) {
            cout << "Creating AABBs' hierarchy" << endl;

            //--- AABB THAT TAKES THE WHOLE MAP
            AABBNode whole = AABBNode(-20.0f, 20.0f, 0.0f, 12.0f, -20.0f, 20.0f);
             
            //--- FIRST QUARTER OF THE WHOLE AABB
            AABBNode firstQuarter = AABBNode(-20.0f, 0.0f, 0.0f, 12.0f,  -20.0f, 0.0f);

            //--- QUARTERS OF THE FIRST QUARTER
            AABBNode firstQuarterFirstQuarter = AABBNode(-20, -10, 0, 12, -20, -10);
            AABBNode firstQuarterSecondQuarter = AABBNode(-10, 0, 0, 12, -20, -10);
            AABBNode firstQuarterThirdQuarter = AABBNode(-20, -10, 0, 12, -10, 0);
            AABBNode firstQuarterForthQuarter = AABBNode(-10, 0, 0, 12, -10, 0);

            firstQuarter.add_children(firstQuarterFirstQuarter);
            firstQuarter.add_children(firstQuarterSecondQuarter);
            firstQuarter.add_children(firstQuarterThirdQuarter);
            firstQuarter.add_children(firstQuarterForthQuarter);

            //--- SECOND QUARTER OF THE WHOLE AABB
            AABBNode secondQuarter = AABBNode(0.0f, 20.0f, 0.0f, 12.0f,  0.0f, 20.0f);

            //--- QUARTERS OF THE SECOND QUARTER
            AABBNode secondQuarterFirstQuarter = AABBNode(0, 10, 0, 12, 0, 10);
            AABBNode secondQuarterSecondQuarter = AABBNode(0, 10, 0, 12, 10, 20);
            AABBNode secondQuarterThirdQuarter = AABBNode(10, 20, 0, 12, 0, 10);
            AABBNode secondQuarterForthQuarter = AABBNode(10, 20, 0, 12, 10, 20);

            secondQuarter.add_children(secondQuarterFirstQuarter);
            secondQuarter.add_children(secondQuarterSecondQuarter);
            secondQuarter.add_children(secondQuarterThirdQuarter);
            secondQuarter.add_children(secondQuarterForthQuarter);

            //--- THIRD QUARTER OF THE WHOLE AABB
            AABBNode thirdQuarter = AABBNode(-20.0f, 0.0f, 0.0f, 12.0f,  0.0f, 20.0f);

            //--- QUARTERS OF THE SECOND QUARTER
            AABBNode thirdQuarterFirstQuarter = AABBNode(-20, -10, 0, 12, 0, 10);
            AABBNode thirdQuarterSecondQuarter = AABBNode(-20, -10, 0, 12, 10, 20);
            AABBNode thirdQuarterThirdQuarter = AABBNode(-10, 0, 0, 12, 0, 10);
            AABBNode thirdQuarterForthQuarter = AABBNode(-10, 0, 0, 12, 10, 20);

            thirdQuarter.add_children(thirdQuarterFirstQuarter);
            thirdQuarter.add_children(thirdQuarterSecondQuarter);
            thirdQuarter.add_children(thirdQuarterThirdQuarter);
            thirdQuarter.add_children(thirdQuarterForthQuarter);
            
            //--- FORTH QUARTER OF THE WHOLE AABB
            AABBNode forthQuarter = AABBNode(0.0f, 20.0f, 0.0f, 12.0f,  -20.0f, 0.0f);

            AABBNode forthQuarterFirstQuarter = AABBNode(0, 10, 0, 12, -20, -10);
            AABBNode forthQuarterSecondQuarter = AABBNode(0, 10, 0, 12, -10, 0);
            AABBNode forthQuarterThirdQuarter = AABBNode(10, 20, 0, 12, -20, -10);
            AABBNode forthQuarterForthQuarter = AABBNode(10, 20, 0, 12, -10, 0);

            forthQuarter.add_children(forthQuarterFirstQuarter);
            forthQuarter.add_children(forthQuarterSecondQuarter);
            forthQuarter.add_children(forthQuarterThirdQuarter);
            forthQuarter.add_children(forthQuarterForthQuarter);

            appState = AppStates::Loaded;
        }

        if(appState == AppStates::LoadingAABBs) {
            cout << "Calculating trees AABBs" << endl;
            for (auto i=treesMatrixes.begin(); i!=treesMatrixes.end(); ++i) {
                glm::mat4 matrix = *i;
                glm::vec3 treePos = glm::vec3(matrix[3].x, matrix[3].y, matrix[3].z);
                float treeSize = matrix[0].x / 1.5f;
                GLfloat dy = 5.0f * treeSize;
                vector<GLfloat> treeAABBsVertices;
                treeAABBsVertices.push_back(treePos.x + treeSize);
                treeAABBsVertices.push_back(dy);
                treeAABBsVertices.push_back(treePos.z - treeSize);

                treeAABBsVertices.push_back(treePos.x + treeSize);
                treeAABBsVertices.push_back(0);
                treeAABBsVertices.push_back(treePos.z - treeSize);

                treeAABBsVertices.push_back(treePos.x - treeSize);
                treeAABBsVertices.push_back(0);
                treeAABBsVertices.push_back(treePos.z - treeSize);

                treeAABBsVertices.push_back(treePos.x - treeSize);
                treeAABBsVertices.push_back(dy);
                treeAABBsVertices.push_back(treePos.z - treeSize);

                treeAABBsVertices.push_back(treePos.x + treeSize);
                treeAABBsVertices.push_back(dy);
                treeAABBsVertices.push_back(treePos.z + treeSize);

                treeAABBsVertices.push_back(treePos.x + treeSize);
                treeAABBsVertices.push_back(0);
                treeAABBsVertices.push_back(treePos.z + treeSize);

                treeAABBsVertices.push_back(treePos.x - treeSize);
                treeAABBsVertices.push_back(0);
                treeAABBsVertices.push_back(treePos.z + treeSize);

                treeAABBsVertices.push_back(treePos.x - treeSize);
                treeAABBsVertices.push_back(dy);
                treeAABBsVertices.push_back(treePos.z + treeSize);

                AABB aabb = AABB(treeAABBsVertices);
                AABBs.push_back(aabb);
            }

            glm::vec3 cartPos = glm::vec3(cartX, 0.0f, cartZ);
            vector<GLfloat> cartAABBsVertices;
            float dy = 2.0f;
            glm::vec3 cartSize = glm::vec3(2.0f, 0.0f, 1.5f);
            cartAABBsVertices.push_back(cartPos.x + cartSize.x);
            cartAABBsVertices.push_back(dy);
            cartAABBsVertices.push_back(cartPos.z - cartSize.z);

            cartAABBsVertices.push_back(cartPos.x + cartSize.x);
            cartAABBsVertices.push_back(0);
            cartAABBsVertices.push_back(cartPos.z - cartSize.z);

            cartAABBsVertices.push_back(cartPos.x - cartSize.x);
            cartAABBsVertices.push_back(0);
            cartAABBsVertices.push_back(cartPos.z - cartSize.z);

            cartAABBsVertices.push_back(cartPos.x - cartSize.x);
            cartAABBsVertices.push_back(dy);
            cartAABBsVertices.push_back(cartPos.z - cartSize.z);

            cartAABBsVertices.push_back(cartPos.x + cartSize.x);
            cartAABBsVertices.push_back(dy);
            cartAABBsVertices.push_back(cartPos.z + cartSize.z);

            cartAABBsVertices.push_back(cartPos.x + cartSize.x);
            cartAABBsVertices.push_back(0);
            cartAABBsVertices.push_back(cartPos.z + cartSize.z);

            cartAABBsVertices.push_back(cartPos.x - cartSize.x);
            cartAABBsVertices.push_back(0);
            cartAABBsVertices.push_back(cartPos.z + cartSize.z);

            cartAABBsVertices.push_back(cartPos.x - cartSize.x);
            cartAABBsVertices.push_back(dy);
            cartAABBsVertices.push_back(cartPos.z + cartSize.z);

            AABB aabb = AABB(cartAABBsVertices);
            AABBs.push_back(aabb);

            appState = AppStates::CreatingAABBsHierarchy;
        }

        //--- CONTINUE LOADING THE LEVEL
        if(appState == AppStates::LoadingMap) {
            if(currentCell < content.size()) {
                cout << "Loading map row #" << currentCell << endl;
                for (auto i=content[currentCell].begin(); i!=content[currentCell].end(); ++i) {
                    float position = i-content[currentCell].begin();
                    if(*i == "T") {
                        //--- TREES ARE RANDOMLY DISPLACED FROM THEIR 0.5x0.5 cell by a random value between -0.5f and 0.5f
                        float randX = (rand() % 10 - 5) / 10.f;
                        float randZ = (rand() % 10 - 5) / 10.f;
                        //--- TREES ARE RANDOMLY SCALED FROM 100% TO 150%
                        float randomScale = (100 + (rand() % 50)) / 100.f;
                        glm::mat4 treeMatrix = glm::mat4(1.0f);
                        treeMatrix = glm::translate(treeMatrix, glm::vec3(currentCell * 2 + 0.5 + randX, 0.0f, position * 2 + 0.5f + randZ));
                        treeMatrix = glm::scale(treeMatrix, glm::vec3(randomScale, randomScale, randomScale));
                        treesMatrixes.push_back(treeMatrix);
                    }
                    if(*i == "S") {
                        deltaX = currentCell * 2;
                        deltaZ = position * 2;
                    }
                    if(*i == "C") {
                        cartX = currentCell * 2;
                        cartZ = position * 2;
                    }
                    if(*i == "P") {
                        paths.push_back(glm::vec2(currentCell-16, position-16));
                    }
                }
            } else {
                appState = AppStates::LoadingAABBs;
            }
        }
        currentCell++;

        //---  UPDATE TIME 
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        coinRotationY += coinRotationSpeed * deltaTime;

        glfwPollEvents();

        //---  CLEAR 
        glStencilMask(0xFF);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glStencilMask(0x00);

        shader.Use();

        //--- SHADER LOCATIONS 
        GLint projectionMatrixLocation = glGetUniformLocation(shader.Program, "projectionMatrix");
        GLint viewMatrixLocation = glGetUniformLocation(shader.Program, "viewMatrix");
        GLint textureLocation = glGetUniformLocation(shader.Program, "tex");
        GLint repeatLocation = glGetUniformLocation(shader.Program, "repeat");
        GLint modelMatrixLocation = glGetUniformLocation(shader.Program, "modelMatrix");

        //--- SET COIN TEXTURE 
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureId[coinTextureIndex]);

        //--- PASS VALUES TO SHADER 
        glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(view));
        glUniform1i(textureLocation, 1);
        glUniform1f(repeatLocation, 1.0);
        
        //---  SET COIN MATRICES 
        coinModelMatrix = glm::mat4(1.0f);
        coinModelMatrix = glm::translate(coinModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
        coinModelMatrix = glm::rotate(coinModelMatrix, glm::radians(coinRotationY), glm::vec3(0.0f, 1.0f, 0.0f));
        coinModelMatrix = glm::scale(coinModelMatrix, glm::vec3(0.08f, 0.08f, 0.08f));
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(coinModelMatrix));

        //---  DRAW COIN 
        coinModel.Draw();

        glfwSwapBuffers(window);
    }

    cout << "Loading ended, binding new loop values" << endl;

    glClearColor(135.0f / 255.f, 206.0f / 255.f, 235.0f / 255.f, 1.0f);

    //delete &coinModel;

    //---  INIT UNIFORM BUFFER FOR TREES
    GLint uniformTreesMatrixBlockLocation = glGetUniformBlockIndex(shader.Program, "Matrices");
    glUniformBlockBinding(shader.Program, uniformTreesMatrixBlockLocation, 0);

    GLuint uboTreesMatrixBlock;
    glGenBuffers(1, &uboTreesMatrixBlock);
    glBindBuffer(GL_UNIFORM_BUFFER, uboTreesMatrixBlock);
    glBufferData(GL_UNIFORM_BUFFER, treesMatrixes.size() * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboTreesMatrixBlock, 0, treesMatrixes.size() * sizeof(glm::mat4));

    //---  FILL UNIFORM BUFFER
    glBindBuffer(GL_UNIFORM_BUFFER, uboTreesMatrixBlock);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, treesMatrixes.size() * sizeof(glm::mat4), glm::value_ptr(treesMatrixes[0]));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    cout << "Starting game loop" << endl;

    cout << "*********" << endl;

    cout << "Press WASD to move" << endl;

    cout << "Keep pressing mouseR and move mouse to rotate camera" << endl;

    //--- TO APPLY THE LENS EFFECT, WE RENDER TO A RENDER TARGET
    //--- LATER TO BE USED AS A TEXTURE

    //--- CREATING FRAME BUFFER
    GLuint frameBuffer = 0;
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    //--- CREATING THE TEXTURE
    GLuint quadTexture;
    glGenTextures(1, &quadTexture);
    glBindTexture(GL_TEXTURE_2D, quadTexture);

    //--- PASSING AN EMPTY IMAGE
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);

    //--- FILTERING
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    //--- DEPTH-STENCIL BUFFER   
    GLuint depthBuffer;
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

    //--- SET TEXTURE AS COLOR ATTACHMENT
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, quadTexture, 0);

    //--- SET THE LIST OF DRAW BUFFERS
    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);

    //--- CHECK FRAME BUFFER SANITY
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Frame buffer status " << glCheckFramebufferStatus(GL_FRAMEBUFFER) << ", aborting" << std::endl;
        return -1;
    }

    while(!glfwWindowShouldClose(window))
    {
        //--- RENDER TO FRAME BUFFER
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

        //---  UPDATE TIME 
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //---  CHECK INPUT EVENTS 
        glfwPollEvents();
        process_keys(window);

        //---  CLEAR 
        glStencilMask(0xFF);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glStencilMask(0x00);

        auto start = std::chrono::high_resolution_clock::now();

        //--- CALCULATE PLAYER AABB
        //--- THIS IS THE FIRST THING TO DO BECAUSE IT CAN MODIFY THE CAMERA VIEW

        //--- PLAYER POSITION VECTOR AND DELTAS
        glm::vec3 playerPos = glm::vec3(deltaX, 0, deltaZ);
        float playerSize = 0.4f;
        GLfloat dy = 1.25f;

        //--- VERTICES OF THE PLAYER'S AABB
        GLfloat playerVertices[] = {
                playerPos.x + playerSize, dy, playerPos.z - playerSize,
                playerPos.x + playerSize, 0.0f, playerPos.z - playerSize,
                playerPos.x - playerSize, 0.0f, playerPos.z - playerSize,
                playerPos.x - playerSize, dy, playerPos.z - playerSize,
                playerPos.x + playerSize, dy, playerPos.z + playerSize,
                playerPos.x + playerSize, 0.0f, playerPos.z + playerSize,
                playerPos.x - playerSize, 0.0f, playerPos.z + playerSize,
                playerPos.x - playerSize, dy, playerPos.z + playerSize
            };

        //--- CREATING AABB FROM VERTICES
        AABB playerAABB = AABB(playerVertices);

        //--- DEFAULT VALUE OF COLLISION DETECTED
        bool collision = false;

        //--- VERY LOW PERFORMANCE AABB CHECK
        //--- BEST CASE MEASURED ON MAC: 34 microseconds
        //--- WORST CASE MEASURED ON MAC: 372 microseconds
        //--- BEST CASE MEASURED ON PC: 196 microseconds
        //--- WORST CASE MEASURED ON PC: 45 microseconds
        for (auto i= AABBs.begin(); i!=AABBs.end(); ++i) {
            AABB tree = *i;
            bool collisionX = (tree.MinX <= playerAABB.MaxX && tree.MaxX >= playerAABB.MinX);
            bool collisionZ = (tree.MinZ <= playerAABB.MaxZ && tree.MaxZ >= playerAABB.MinZ);
            if(collisionX && collisionZ) {
                //--- THE PLAYER COLLIDES WITH SOMETHING
                collision = true;
                break;
            }
        }
        
        if(collision) {

            //--- CHECK COLLISION IF I MOVE ONLY IN THE Z DIRECTION
            playerPos = glm::vec3(oldDeltaX, 0, deltaZ);

            //--- VERTICES OF THE PLAYER'S AABB
            GLfloat zPlayerVertices[] = {
                playerPos.x + playerSize, dy * playerSize, playerPos.z - playerSize,
                playerPos.x + playerSize, 0.0f, playerPos.z - playerSize,
                playerPos.x - playerSize, 0.0f, playerPos.z - playerSize,
                playerPos.x - playerSize, dy * playerSize, playerPos.z - playerSize,
                playerPos.x + playerSize, dy * playerSize, playerPos.z + playerSize,
                playerPos.x + playerSize, 0.0f, playerPos.z + playerSize,
                playerPos.x - playerSize, 0.0f, playerPos.z + playerSize,
                playerPos.x - playerSize, dy * playerSize, playerPos.z + playerSize
            };

            //--- CREATING AABB FROM VERTICES
            playerAABB = AABB(zPlayerVertices);

            //--- DEFAULT VALUE OF COLLISION DETECTED
            bool ZmovementCollision = false;

            //--- VERY LOW PERFORMANCE AABB CHECK
            for (auto i= AABBs.begin(); i!=AABBs.end(); ++i) {
                AABB tree = *i;
                bool collisionX = (tree.MinX <= playerAABB.MaxX && tree.MaxX >= playerAABB.MinX);
                bool collisionZ = (tree.MinZ <= playerAABB.MaxZ && tree.MaxZ >= playerAABB.MinZ);
                if(collisionX && collisionZ) {
                    //--- THE PLAYER COLLIDES WITH SOMETHING
                    ZmovementCollision = true;
                    break;
                }
            }

            //--- CHECK COLLISION IF I MOVE ONLY IN THE X DIRECTION
            playerPos = glm::vec3(deltaX, 0, oldDeltaZ);

            //--- VERTICES OF THE PLAYER'S AABB
            GLfloat xPlayerVertices[] = {
                playerPos.x + playerSize, dy * playerSize, playerPos.z - playerSize,
                playerPos.x + playerSize, 0.0f, playerPos.z - playerSize,
                playerPos.x - playerSize, 0.0f, playerPos.z - playerSize,
                playerPos.x - playerSize, dy * playerSize, playerPos.z - playerSize,
                playerPos.x + playerSize, dy * playerSize, playerPos.z + playerSize,
                playerPos.x + playerSize, 0.0f, playerPos.z + playerSize,
                playerPos.x - playerSize, 0.0f, playerPos.z + playerSize,
                playerPos.x - playerSize, dy * playerSize, playerPos.z + playerSize
            };

            //--- CREATING AABB FROM VERTICES
            playerAABB = AABB(xPlayerVertices);

            //--- DEFAULT VALUE OF COLLISION DETECTED
            bool XmovementCollision = false;

            //--- VERY LOW PERFORMANCE AABB CHECK
            for (auto i= AABBs.begin(); i!=AABBs.end(); ++i) {
                AABB tree = *i;
                bool collisionX = (tree.MinX <= playerAABB.MaxX && tree.MaxX >= playerAABB.MinX);
                bool collisionZ = (tree.MinZ <= playerAABB.MaxZ && tree.MaxZ >= playerAABB.MinZ);
                if(collisionX && collisionZ) {
                    //--- THE PLAYER COLLIDES WITH SOMETHING
                    XmovementCollision = true;
                    break;
                }
            }

            if(XmovementCollision) {
                deltaX = oldDeltaX;
            }

            if(ZmovementCollision) {
                deltaZ = oldDeltaZ;
            }
            
        }
        
        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

        if(maxMicroseconds < microseconds) {
            cout << microseconds << endl;
            maxMicroseconds = microseconds;
        }

        //** UPDATE CAMERA POSITION TO FOLLOW PLAYER
        GLfloat distX = -sin(glm::radians(rotationY)) * cameraDistance;
        GLfloat distZ = cos(glm::radians(rotationY)) * cameraDistance;
        view = glm::lookAt(glm::vec3(deltaX + distX, 1.5f, deltaZ - distZ), glm::vec3(deltaX, 1.5f, deltaZ), glm::vec3(0.0f, 1.0f, 0.0f));

        //TODO:
        // 1) RAYCAST FROM CAMERA TO PLAYER
        // 2a) IF THERE'S AN OBJECT IN THE MIDDLE, MOVE CAMERA TOWARDS THE PLAYER INSTANTLY
        // 3a) IF THERE'S NOTHING IN THE MIDDLE, MOVE CAMERA BACK GRADUALLY UNTIL MAX_DISTANCE IS REACHED

        //--- USE SHADER 
        shader.Use();

        //--- SHADER LOCATIONS 
        GLint projectionMatrixLocation = glGetUniformLocation(shader.Program, "projectionMatrix");
        GLint viewMatrixLocation = glGetUniformLocation(shader.Program, "viewMatrix");
        GLint textureLocation = glGetUniformLocation(shader.Program, "tex");
        GLint repeatLocation = glGetUniformLocation(shader.Program, "repeat");
        GLint modelMatrixLocation = glGetUniformLocation(shader.Program, "modelMatrix");
        GLint modelMatrixesLocation = glGetUniformLocation(shader.Program, "modelMatrixes");
        GLint colorInLocation = glGetUniformLocation(shader.Program, "colorIn");
        GLint instancedLocation = glGetUniformLocation(shader.Program, "instanced");
        GLint distorsionLocation = glGetUniformLocation(shader.Program, "distorsion");
        GLint timeLocation = glGetUniformLocation(shader.Program, "time");

        //--- SET PLANE TEXTURE 
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureId[planeTextureIndex]);

        //--- PASS VALUES TO SHADER 
        glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(view));
        glUniform1i(textureLocation, 1);
        glUniform1f(repeatLocation, 80.0);
        glUniform1i(instancedLocation, false);
        glUniform1f(distorsionLocation, distorsion);
        glUniform1f(timeLocation, glfwGetTime());
        
        //---  SET PLANE MATRIX
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(planeModelMatrix));

        //---  DRAW PLANE 
        planeModel.Draw();

        GLuint subroutineIndex = glGetSubroutineIndex(shader.Program, GL_FRAGMENT_SHADER, "textured");
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subroutineIndex);

        //--- SET TREE TEXTURE 
        glBindTexture(GL_TEXTURE_2D, textureId[treeTextureIndex]);
        glUniform1f(repeatLocation, 1.0);
        glUniform1i(instancedLocation, true);

        //---  SET TREE MATRICES 
        glUniformMatrix4fv(modelMatrixesLocation, treesMatrixes.size(), GL_FALSE, glm::value_ptr(treesMatrixes[0]));

        //---  DRAW TREE
        treeModel.DrawInstanced(treesMatrixes.size());

        //--- TODO: OPTIMIZE BY REMOVING Y
        float distancePlayerCart = distance(playerPos, glm::vec3(cartX, 0.0f, cartZ));
        if(keys[GLFW_KEY_SPACE] && distancePlayerCart < 5) {
            //--- IN THE FIRST PASS, ALL FRAGMENTS PASS THE STENCIL TEST
            //--- ACTION WHEN STENCIL FAILS, DEPTH FAILS AND BOTH PASS
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilMask(0xFF);

            //--- SET CART TEXTURE
            glBindTexture(GL_TEXTURE_2D, textureId[cartTextureIndex]);
            glUniform1f(repeatLocation, 1.0);
            glUniform1i(instancedLocation, false);

            //---  SET CART MATRICES 
            cartModelMatrix = glm::mat4(1.0f);
            cartModelMatrix = glm::translate(cartModelMatrix, glm::vec3(cartX, 0.0f, cartZ));
            cartModelMatrix = glm::scale(cartModelMatrix, glm::vec3(1.25f, 1.25f, 1.25f));
            glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(cartModelMatrix));

            //---  DRAW CART 
            cartModel.Draw();

            // DON'T WRITE ON STENCIL BUFFER 
            glStencilMask(0x00);

            //--- DRAW BASE PLAYER
            subroutineIndex = glGetSubroutineIndex(shader.Program, GL_FRAGMENT_SHADER, "textured");
            glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subroutineIndex);

            //--- SET PLAYER TEXTURE 
            glBindTexture(GL_TEXTURE_2D, textureId[playerTextureIndex]);
            glUniform1f(repeatLocation, 1.0);

            //---  SET PLAYER MATRICES 
            playerModelMatrix = glm::mat4(1.0f);
            playerModelMatrix = glm::translate(playerModelMatrix, glm::vec3(deltaX, 0.0f, deltaZ));
            playerModelMatrix = glm::rotate(playerModelMatrix, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
            playerModelMatrix = glm::scale(playerModelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));
            glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(playerModelMatrix));

            //---  DRAW PLAYER 
            playerModel.Draw();

            
            // WRITE ON STENCIL BUFFER 
            glStencilMask(0xFF);

            //--- REMOVE PLAYER FROM STENCIL
            glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);

            glColorMask(false, false, false, false);
            glDepthMask(false);

            subroutineIndex = glGetSubroutineIndex(shader.Program, GL_FRAGMENT_SHADER, "textured");
            glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subroutineIndex);

            //--- SET PLAYER TEXTURE 
            glBindTexture(GL_TEXTURE_2D, textureId[playerTextureIndex]);
            glUniform1f(repeatLocation, 1.0);

            //---  SET PLAYER MATRICES 
            playerModelMatrix = glm::mat4(1.0f);
            playerModelMatrix = glm::translate(playerModelMatrix, glm::vec3(deltaX, 0.0f, deltaZ));
            playerModelMatrix = glm::rotate(playerModelMatrix, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
            playerModelMatrix = glm::scale(playerModelMatrix, glm::vec3(0.0305f, 0.0305f, 0.0305f));
            glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(playerModelMatrix));

            //---  DRAW PLAYER 
            playerModel.Draw();

            glColorMask(true, true, true, true);
            glDepthMask(true);

            // ONLY DRAW PARTS WHERE STENCIL BUFFER IS != 1
            glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

            // DON'T WRITE ON STENCIL BUFFER 
            glStencilMask(0x00);

            subroutineIndex = glGetSubroutineIndex(shader.Program, GL_FRAGMENT_SHADER, "redOutline");
            glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subroutineIndex);

            //---  SET UPSCALED CART MATRICES 
            cartModelMatrix = glm::mat4(1.0f);
            cartModelMatrix = glm::translate(cartModelMatrix, glm::vec3(cartX, 0.0f, cartZ));
            cartModelMatrix = glm::scale(cartModelMatrix, glm::vec3(1.27f, 1.27f, 1.27f));
            glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(cartModelMatrix));

            //---  DRAW CART 
            cartModel.Draw();

        } else {
            //--- SET CART TEXTURE
            glBindTexture(GL_TEXTURE_2D, textureId[cartTextureIndex]);
            glUniform1f(repeatLocation, 1.0);
            glUniform1i(instancedLocation, false);

            //---  SET CART MATRICES 
            cartModelMatrix = glm::mat4(1.0f);
            cartModelMatrix = glm::translate(cartModelMatrix, glm::vec3(cartX, 0.0f, cartZ));
            cartModelMatrix = glm::scale(cartModelMatrix, glm::vec3(1.25f, 1.25f, 1.25f));
            glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(cartModelMatrix));

            //---  DRAW CART 
            cartModel.Draw();

            //--- DRAW PLAYER
            subroutineIndex = glGetSubroutineIndex(shader.Program, GL_FRAGMENT_SHADER, "textured");
            glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subroutineIndex);

            //--- SET PLAYER TEXTURE 
            glBindTexture(GL_TEXTURE_2D, textureId[playerTextureIndex]);
            glUniform1f(repeatLocation, 1.0);

            //---  SET PLAYER MATRICES 
            playerModelMatrix = glm::mat4(1.0f);
            playerModelMatrix = glm::translate(playerModelMatrix, glm::vec3(deltaX, 0.0f, deltaZ));
            playerModelMatrix = glm::rotate(playerModelMatrix, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
            playerModelMatrix = glm::scale(playerModelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));
            glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(playerModelMatrix));

            //---  DRAW PLAYER 
            playerModel.Draw();
        }

        //--- RENDER TO QUAD
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        //---  CLEAR 
        glStencilMask(0xFF);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glStencilMask(0x00);

        view = glm::lookAt(glm::vec3(0.0f, 1.0f, 7.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        subroutineIndex = glGetSubroutineIndex(shader.Program, GL_FRAGMENT_SHADER, "pincushion");
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subroutineIndex);

        //--- SET PLANE TEXTURE 
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, quadTexture);

        //--- PASS VALUES TO SHADER 
        glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(view));
        glUniform1i(textureLocation, 1);
        glUniform1f(repeatLocation, 1.0);
        glUniform1i(instancedLocation, false);
        
        glm::mat4 planeModelMatrix2 = glm::mat4(1.0f);
        planeModelMatrix2 = glm::translate(planeModelMatrix2, glm::vec3(0.0f, 1.0f, -10.0f));
        planeModelMatrix2 = glm::rotate(planeModelMatrix2, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        planeModelMatrix2 = glm::scale(planeModelMatrix2, glm::vec3(1/16.0f * 17.0f, 1.0f, 1/16.0f * 10.0f));

        //---  SET PLANE MATRIX
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(planeModelMatrix2));

        //---  DRAW PLANE 
        planeModel.Draw();

        //--- SWAP BUFFERS
        glfwSwapBuffers(window);
    }

    //--- DELETE USED SHADERS
    shader.Delete();
    
    //--- CLOSE AND DELETE CONTEXT
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

    oldDeltaX = deltaX;
    oldDeltaZ = deltaZ;

    if(appState == AppStates::Loaded) {
        if(keys[GLFW_KEY_SPACE]) {
            distorsion -= distorsionSpeed * deltaTime;
            cameraDistance -= cameraZoomSpeed * deltaTime;
            cameraY -= cameraZoomSpeed * deltaTime;
            if(cameraDistance < MIN_CAMERA_DISTANCE) {
                cameraDistance = MIN_CAMERA_DISTANCE;
            }
            if(cameraY < MIN_CAMERA_Y_DELTA) {
                cameraY = MIN_CAMERA_Y_DELTA;
            }
            if(distorsion < MIN_DISTORSION) {
                distorsion = MIN_DISTORSION;
            }
        } else {
            distorsion += distorsionSpeed * deltaTime;
            cameraDistance += cameraZoomSpeed * deltaTime;
            cameraY += cameraZoomSpeed * deltaTime;
            if(cameraDistance > MAX_CAMERA_DISTANCE) {
                cameraDistance = MAX_CAMERA_DISTANCE;
            }
            if(cameraY > MAX_CAMERA_Y_DELTA) {
                cameraY = MAX_CAMERA_Y_DELTA;
            }
            if(distorsion > MAX_DISTORSION) {
                distorsion = MAX_DISTORSION;
            }
        }

        if(keys[GLFW_KEY_W]) {
            deltaX += sin(glm::radians(rotationY)) * speed * deltaTime;
            deltaZ += cos(glm::radians(rotationY)) * speed * deltaTime;
        }

        if(keys[GLFW_KEY_S]) {
            deltaX -= sin(glm::radians(rotationY)) * speed * deltaTime;
            deltaZ -= cos(glm::radians(rotationY)) * speed * deltaTime;
        }

        if(keys[GLFW_KEY_A] && !keys[GLFW_MOUSE_BUTTON_RIGHT]) {
            rotationY += deltaTime * 50.0f * rotationSpeed;
        }

        if(keys[GLFW_KEY_D] && !keys[GLFW_MOUSE_BUTTON_RIGHT]) {
            rotationY -= deltaTime * 50.0f * rotationSpeed;
        }

        if(keys[GLFW_MOUSE_BUTTON_RIGHT]) {
            double xPos, yPos;
            glfwGetCursorPos(window, &xPos, &yPos);
            double diffX = mouseXPos - xPos;
            mouseXPos = xPos;
            rotationY += diffX * rotationSpeed;
        }
    }
}

GLint LoadTexture(const char* path)
{
    GLuint textureImage;
    int w, h, channels;
    unsigned char* image;
    image = stbi_load(path, &w, &h, &channels, STBI_rgb);
    if (image == nullptr) {
        std::cout << "Failed to load texture!" << std::endl;
    }

    glGenTextures(1, &textureImage);
    glBindTexture(GL_TEXTURE_2D, textureImage);

    // 3 channels = RGB ; 4 channel = RGBA
    if (channels == 3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    }
    else if (channels == 4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    }
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
