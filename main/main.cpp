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
#include <utils/vertices.h>

//---  we load the GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include<stb_image/stb_image.h>

#define COIN_INDEX 0
#define PLANE_INDEX 1
#define PLAYER_INDEX 2
#define CART_INDEX 3
#define TREE_INDEX 4

//---  APPLICATION WINDOW 
GLuint screenWidth = 1280, screenHeight = 720;

//---  INPUT KEY CALLBACK 
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
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

//---  TEXTURES AND MODELS
vector<GLint> textures;
vector<Model> models;
vector<glm::mat4> matrices;
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
#define MIN_DISTORSION -0.99f
#define MAX_DISTORSION 0.0f
GLfloat distorsion = MAX_DISTORSION;
GLfloat distorsionSpeed = 0.75f;

//---  APP_STATE 
enum class AppStates { LoadingMap, LoadingAABBs, CreatingAABBsHierarchy, Loaded };
AppStates appState = AppStates::LoadingMap;

//--- MATRIXES FOR INSTANCED DRAWING 
vector<glm::mat4> treesMatrixes;

//--- AABBs list
vector<AABB> AABBs;
AABB AABBhierarchy = AABB();
GLuint AABBsIndices[] = { 0, 1, 3,1, 2, 3, 2, 3, 6, 3, 6, 7, 5, 6, 7, 4, 5, 7, 0, 4, 5, 0, 1, 5, 0, 3, 4, 0, 4, 7, 1, 2, 6, 1, 5, 6 };

//--- CART DATA
float cartX = 0.0f;
float cartZ = 0.0f;
GLfloat cartColor[] = { 0.65f, 0.16f, 0.16f };

//--- INDEX TO KEEP TRACK OF THE CURRENT ROW TO LOAD ONE ROW PER RENDER LOOP
int currentCell = 0;

//--- PLANE PATH DATA
vector<glm::vec2> paths;

//--- LOAD CSV DATA FILE
//--- I EXPECT A 32x32 CSV LIKE THE PLANE OF THE SIZE
vector<vector<string>> content = CsvLoader().read("../data/map.csv");

//--- SHADER LOCATIONS
string locationNames[] { "projectionMatrix", "viewMatrix", "tex", "repeat", "modelMatrix", "modelMatrixes", "colorIn", "instanced", "distorsion", "time" }; 

#define LOCATION_PROJECTION_MATRIX 0
#define LOCATION_VIEW_MATRIX 1
#define LOCATION_TEXTURE 2
#define LOCATION_REPEAT 3
#define LOCATION_MODEL_MATRIX 4
#define LOCATION_MODEL_MATRIXES 5
#define LOCATION_COLOR 6
#define LOCATION_INSTANCED 7
#define LOCATION_DISTORSION 8
#define LOCATION_TIME 9

//--- UTILS METHODS
void clear();
void setTexture(int index, GLint repeatLocation, float repeatValue);
void loadAABBs();
void addToAABBsHierarchy(vector<AABB> aabb);
void loadNextRow();
void drawPlayer(Shader shader, vector<GLint> locations, float scaleModifier);
void drawCart(Shader shader, vector<GLint> locations, float scaleModifier, string subroutine);

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
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Witcher senses demo", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

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

    //--- ENABLE DEPTH TEST, STENCIL TEST AND ALPHA BLENDING
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    //--- SET CLEAR COLOR
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //---  INIT SHADERS 
    Shader shader = Shader("base.vert", "base.frag");
    SetupShader(shader.Program);

    //--- LOAD TEXTURES MODELS, MATRICES
    string names[] { "coin", "plane", "dog", "cart", "tree" }; 
    for (string name : names) {
        textures.push_back(LoadTexture(("../textures/" + name + ".jpg").c_str()));
        models.push_back(Model("../models/" + name + ".obj"));
        matrices.push_back(glm::mat4(1.0f));
    }

    cout << "Loaded textures and models" << endl;

    //--- INIT FIXED PLANE MATRIX
    matrices[PLANE_INDEX] = glm::translate(matrices[PLANE_INDEX], glm::vec3(32.0f, 0.0f, 32.0f));
    matrices[PLANE_INDEX] = glm::rotate(matrices[PLANE_INDEX], glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    matrices[PLANE_INDEX] = glm::scale(matrices[PLANE_INDEX], glm::vec3(2.0f, 2.0f, 2.0f));

    //--- COIN DATA
    float coinRotationY = 0.0f;
    float coinRotationSpeed = 20.0f;

    //---  INIT CAMERA 
    glm::mat4 projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 10000.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

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
            addToAABBsHierarchy( { AABBs.begin(), AABBs.begin() + 1 } );
            cout << "******************" << endl;
            cout << "******************" << endl;
            cout << "                  " << endl;
            cout << AABBhierarchy.fullPrint(0) << endl;
            cout << "                  " << endl;
            cout << "******************" << endl;
            cout << "******************" << endl;
            appState = AppStates::Loaded;
        }

        if(appState == AppStates::LoadingAABBs) {
            loadAABBs();
        }

        //--- CONTINUE LOADING THE LEVEL
        if(appState == AppStates::LoadingMap) {
            if(currentCell < content.size()) {
                loadNextRow();
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

        clear();

        shader.Use();

        //--- SHADER LOCATIONS
        vector<GLint> locations;
        for (string name : locationNames) {
            locations.push_back(glGetUniformLocation(shader.Program, name.c_str()));
        }

        setTexture(COIN_INDEX, locations[LOCATION_REPEAT], 1.0f);

        //--- PASS VALUES TO SHADER 
        glUniformMatrix4fv(locations[LOCATION_PROJECTION_MATRIX], 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(locations[LOCATION_VIEW_MATRIX], 1, GL_FALSE, glm::value_ptr(view));
        
        //---  SET COIN MATRICES 
        matrices[COIN_INDEX] = glm::mat4(1.0f);
        matrices[COIN_INDEX] = glm::translate(matrices[COIN_INDEX], glm::vec3(0.0f, 0.0f, 0.0f));
        matrices[COIN_INDEX] = glm::rotate(matrices[COIN_INDEX], glm::radians(coinRotationY), glm::vec3(0.0f, 1.0f, 0.0f));
        matrices[COIN_INDEX] = glm::scale(matrices[COIN_INDEX], glm::vec3(0.08f, 0.08f, 0.08f));
        glUniformMatrix4fv(locations[LOCATION_MODEL_MATRIX], 1, GL_FALSE, glm::value_ptr(matrices[COIN_INDEX]));

        //---  DRAW COIN 
        models[COIN_INDEX].Draw(locations[LOCATION_INSTANCED]);

        glfwSwapBuffers(window);
    }

    cout << "Loading ended, binding new loop values" << endl;

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

    cout << "Move mouse while pressing mouseR to rotate camera" << endl;

    //--- TO APPLY THE LENS EFFECT, WE RENDER TO A RENDER TARGET
    //--- LATER TO BE USED AS A TEXTURE

    //--- CREATING FRAME BUFFER
    GLuint frameBuffer = 0;
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    //--- CREATING THE TEXTURES
    GLuint firstTexture;
    glGenTextures(1, &firstTexture);
    glBindTexture(GL_TEXTURE_2D, firstTexture);

    //--- PASSING AN EMPTY IMAGE
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);

    //--- FILTERING
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    GLuint secondTexture;
    glGenTextures(1, &secondTexture);
    glBindTexture(GL_TEXTURE_2D, secondTexture);

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
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, firstTexture, 0);

    //--- SET THE LIST OF DRAW BUFFERS
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
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

        glClearColor(135.0f / 255.f, 206.0f / 255.f, 235.0f / 255.f, 1.0f);

        //---  UPDATE TIME 
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //---  CHECK INPUT EVENTS 
        glfwPollEvents();
        process_keys(window);

        clear();

        auto start = std::chrono::high_resolution_clock::now();

        //--- CALCULATE PLAYER AABB
        //--- THIS IS THE FIRST THING TO DO BECAUSE IT CAN MODIFY THE CAMERA VIEW

        //--- PLAYER POSITION VECTOR AND DELTAS
        glm::vec3 playerPos = glm::vec3(deltaX, 0, deltaZ);
        float playerSize = 0.4f;
        GLfloat dy = 1.25f;

        //--- CREATING AABB FROM VERTICES
        AABB playerAABB = AABB(VerticesBuilder().build(playerPos, dy, glm::vec3(playerSize)));

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

            //--- CREATING AABB FROM VERTICES
            playerAABB = AABB(VerticesBuilder().build(playerPos, dy, glm::vec3(playerSize)));

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

            //--- CREATING AABB FROM VERTICES
            playerAABB = AABB(VerticesBuilder().build(playerPos, dy, glm::vec3(playerSize)));

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
        vector<GLint> locations;
        for (string name : locationNames) {
            locations.push_back(glGetUniformLocation(shader.Program, name.c_str()));
        }

        glUniform1i(locations[LOCATION_TEXTURE], 1);

        setTexture(PLANE_INDEX, locations[LOCATION_REPEAT], 80.0f);

        //--- PASS VALUES TO SHADER 
        glUniformMatrix4fv(locations[LOCATION_PROJECTION_MATRIX], 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(locations[LOCATION_VIEW_MATRIX], 1, GL_FALSE, glm::value_ptr(view));
        glUniform1f(locations[LOCATION_DISTORSION], distorsion);
        glUniform1f(locations[LOCATION_TIME], glfwGetTime());
        
        //---  SET PLANE MATRIX
        glUniformMatrix4fv(locations[LOCATION_MODEL_MATRIX], 1, GL_FALSE, glm::value_ptr(matrices[PLANE_INDEX]));

        //---  DRAW PLANE 
        models[PLANE_INDEX].Draw(locations[LOCATION_INSTANCED]);

        GLuint subroutineIndex = glGetSubroutineIndex(shader.Program, GL_FRAGMENT_SHADER, "textured");
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subroutineIndex);

        //--- SET TREE TEXTURE 
        setTexture(TREE_INDEX, locations[LOCATION_REPEAT], 1.0f);

        //---  SET TREE MATRICES 
        glUniformMatrix4fv(locations[LOCATION_MODEL_MATRIXES], treesMatrixes.size(), GL_FALSE, glm::value_ptr(treesMatrixes[0]));

        //---  DRAW TREE
        models[TREE_INDEX].DrawInstanced(treesMatrixes.size(), locations[LOCATION_INSTANCED]);

        drawCart(shader, locations, 1.0f, "textured");

        drawPlayer(shader, locations, 1.0f);

        //--- TODO: OPTIMIZE BY REMOVING Y
        float distancePlayerCart = distance(playerPos, glm::vec3(cartX, 0.0f, cartZ));
        if(keys[GLFW_KEY_SPACE] && distancePlayerCart < 7.5) {

            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, secondTexture, 0);

            glClearColor(0.0f, 1.0f, 0.0f, 1.0f);

            clear();

            glColorMask(false, false, false, false);
            glDepthMask(false);

            //--- IN THE FIRST PASS, ALL FRAGMENTS PASS THE STENCIL TEST
            //--- ACTION WHEN STENCIL FAILS, DEPTH FAILS AND BOTH PASS
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilMask(0xFF);

            drawCart(shader, locations, 1.0f, "textured");

            //--- REMOVE PLAYER FROM STENCIL
            glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);

            drawPlayer(shader, locations, 1.0f);
            
            glColorMask(true, true, true, true);
            glDepthMask(true);

            glStencilMask(0x00);

            //--- DRAW PLANE ONLY WHERE STENCIL IS !=1
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            glStencilFunc(GL_EQUAL, 1, 0xFF);

            view = glm::lookAt(glm::vec3(0.0f, 1.0f, 7.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

             //--- PASS VALUES TO SHADER 
            glUniformMatrix4fv(locations[LOCATION_PROJECTION_MATRIX], 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(locations[LOCATION_VIEW_MATRIX], 1, GL_FALSE, glm::value_ptr(view));

            subroutineIndex = glGetSubroutineIndex(shader.Program, GL_FRAGMENT_SHADER, "redOutline");
            glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subroutineIndex);

            glm::mat4 planeModelMatrix2 = glm::mat4(1.0f);
            planeModelMatrix2 = glm::translate(planeModelMatrix2, glm::vec3(0.0f, 1.0f, -10.0f));
            planeModelMatrix2 = glm::rotate(planeModelMatrix2, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            planeModelMatrix2 = glm::scale(planeModelMatrix2, glm::vec3(1/16.0f * 17.0f, 1.0f, 1/16.0f * 10.0f));

            //---  SET PLANE MATRIX
            glUniformMatrix4fv(locations[LOCATION_MODEL_MATRIX], 1, GL_FALSE, glm::value_ptr(planeModelMatrix2));

            //---  DRAW PLANE 
            models[PLANE_INDEX].Draw(locations[LOCATION_INSTANCED]);

            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            
        }

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, firstTexture, 0);

        //--- RENDER TO QUAD
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        glClearColor(135.0f / 255.f, 206.0f / 255.f, 235.0f / 255.f, 1.0f);

        clear();

        view = glm::lookAt(glm::vec3(0.0f, 1.0f, 7.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        subroutineIndex = glGetSubroutineIndex(shader.Program, GL_FRAGMENT_SHADER, "pincushion");
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subroutineIndex);

        //--- PASS VALUES TO SHADER 
        glUniformMatrix4fv(locations[LOCATION_PROJECTION_MATRIX], 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(locations[LOCATION_VIEW_MATRIX], 1, GL_FALSE, glm::value_ptr(view));

        //--- SET PLANE TEXTURE 
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, firstTexture);
        
        glm::mat4 planeModelMatrix2 = glm::mat4(1.0f);
        planeModelMatrix2 = glm::translate(planeModelMatrix2, glm::vec3(0.0f, 1.0f, -10.0f));
        planeModelMatrix2 = glm::rotate(planeModelMatrix2, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        planeModelMatrix2 = glm::scale(planeModelMatrix2, glm::vec3(1/16.0f * 17.0f, 1.0f, 1/16.0f * 10.0f));

        //---  SET PLANE MATRIX
        glUniformMatrix4fv(locations[LOCATION_MODEL_MATRIX], 1, GL_FALSE, glm::value_ptr(planeModelMatrix2));

        //---  DRAW PLANE 
        models[PLANE_INDEX].Draw(locations[LOCATION_INSTANCED]);

        //--- SET PLANE TEXTURE 
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, secondTexture);

        subroutineIndex = glGetSubroutineIndex(shader.Program, GL_FRAGMENT_SHADER, "tracePlane");
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subroutineIndex);
        
        glm::mat4 planeModelMatrix3 = glm::mat4(1.0f);
        planeModelMatrix3 = glm::translate(planeModelMatrix3, glm::vec3(0.0f, 1.0f, -9.95f));
        planeModelMatrix3 = glm::rotate(planeModelMatrix3, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        planeModelMatrix3 = glm::scale(planeModelMatrix3, glm::vec3(1/16.0f * 17.0f, 1.0f, 1/16.0f * 10.0f));

        //---  SET PLANE MATRIX
        glUniformMatrix4fv(locations[LOCATION_MODEL_MATRIX], 1, GL_FALSE, glm::value_ptr(planeModelMatrix3));

        //---  DRAW PLANE 
        models[PLANE_INDEX].Draw(locations[LOCATION_INSTANCED]);

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

void clear() {
    glStencilMask(0xFF);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glStencilMask(0x00);
}

void setTexture(int index, GLint repeatLocation, float repeatValue) {
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[index]);
    glUniform1f(repeatLocation, repeatValue);
}

void addToAABBsHierarchy(vector<AABB> AABBlist) {
    cout << "***********************" << endl;
    cout << "Adding AABB to AABBs' hierarchy" << endl;
    for (auto i=AABBlist.begin(); i!=AABBlist.end(); ++i) {
        AABB current = *i;
        cout << "Check collision of " << current.toString() << endl;
        AABBhierarchy.addAABBToHierarchy(current);
    }
}

void loadAABBs() {
    cout << "Calculating trees AABBs" << endl;
    for (auto i=treesMatrixes.begin(); i!=treesMatrixes.end(); ++i) {
        glm::mat4 matrix = *i;
        glm::vec3 treePos = glm::vec3(matrix[3].x, matrix[3].y, matrix[3].z);
        float treeSize = matrix[0].x / 1.5f;
        GLfloat dy = 5.0f * treeSize;
        AABB aabb = AABB(VerticesBuilder().build(treePos, dy, glm::vec3(treeSize)));
        AABBs.push_back(aabb);
    }

    glm::vec3 cartPos = glm::vec3(cartX, 0.0f, cartZ);
    float dy = 2.0f;
    glm::vec3 cartSize = glm::vec3(1.75f, 0.0f, 1.25f);
    AABB aabb = AABB(VerticesBuilder().build(cartPos, dy, glm::vec3(cartSize)));
    AABBs.push_back(aabb);

    appState = AppStates::CreatingAABBsHierarchy;
}

void loadNextRow() {
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
}

void drawPlayer(Shader shader, vector<GLint> locations, float scaleModifier) {
    //--- DRAW PLAYER
    GLuint subroutineIndex = glGetSubroutineIndex(shader.Program, GL_FRAGMENT_SHADER, "textured");
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subroutineIndex);

    //--- SET PLAYER TEXTURE 
    setTexture(PLAYER_INDEX, locations[LOCATION_REPEAT], 1.0f);

    //---  SET PLAYER MATRICES 
    matrices[PLAYER_INDEX] = glm::mat4(1.0f);
    matrices[PLAYER_INDEX] = glm::translate(matrices[PLAYER_INDEX], glm::vec3(deltaX, 0.0f, deltaZ));
    matrices[PLAYER_INDEX] = glm::rotate(matrices[PLAYER_INDEX], glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    matrices[PLAYER_INDEX] = glm::scale(matrices[PLAYER_INDEX], glm::vec3(0.03f * scaleModifier, 0.03f * scaleModifier, 0.03f * scaleModifier));
    glUniformMatrix4fv(locations[LOCATION_MODEL_MATRIX], 1, GL_FALSE, glm::value_ptr(matrices[PLAYER_INDEX]));

    //---  DRAW PLAYER 
    models[PLAYER_INDEX].Draw(locations[LOCATION_INSTANCED]);
}

void drawCart(Shader shader, vector<GLint> locations, float scaleModifier, string subroutine) {
    //--- CHECK SUBROUTINES
    GLuint subroutineIndex = glGetSubroutineIndex(shader.Program, GL_FRAGMENT_SHADER, subroutine.c_str());
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subroutineIndex);

    //--- SET CART TEXTURE
    setTexture(CART_INDEX, locations[LOCATION_REPEAT], 1.0f);

    //---  SET CART MATRICES 
    matrices[CART_INDEX] = glm::mat4(1.0f);
    matrices[CART_INDEX] = glm::translate(matrices[CART_INDEX], glm::vec3(cartX, 0.0f, cartZ));
    matrices[CART_INDEX] = glm::scale(matrices[CART_INDEX], glm::vec3(1.25f * scaleModifier, 1.25f * scaleModifier, 1.25f * scaleModifier));
    glUniformMatrix4fv(locations[LOCATION_MODEL_MATRIX], 1, GL_FALSE, glm::value_ptr(matrices[CART_INDEX]));

    //---  DRAW CART 
    models[CART_INDEX].Draw(locations[LOCATION_INSTANCED]);
}