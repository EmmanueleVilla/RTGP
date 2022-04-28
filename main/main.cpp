//---  Std. Includes
#include <string>
#define GLM_ENABLE_EXPERIMENTAL
//---  Loader estensions OpenGL
#ifdef _WIN32
    #define APIENTRY __stdcall
#endif

#include <chrono>
#include <cmath>

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

#include <gl\GL.h>

#define STB_IMAGE_IMPLEMENTATION
#include<stb_image/stb_image.h>

#define COIN_INDEX 0
#define PLANE_INDEX 1
#define PLAYER_INDEX 2
#define CART_INDEX 3
#define TREE_INDEX 4
#define HOUSE_INDEX 5
#define ODOR_INDEX 6

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
#define CAMERA_DISTANCE_DELTA 0.1f
#define MAX_CAMERA_DISTANCE 5.0f
#define MIN_CAMERA_DISTANCE 0.1f
#define MIN_CAMERA_DISTANCE_SENSES 1.5f
#define MAX_CAMERA_Y_DELTA 1.0f
#define MIN_CAMERA_Y_DELTA 0.0f
GLfloat maxCameraDistance = MAX_CAMERA_DISTANCE;
GLfloat cameraDistance = MAX_CAMERA_DISTANCE;
GLfloat cameraY = MAX_CAMERA_Y_DELTA;
GLfloat cameraZoomSpeed = 3.0f;

//--- PINCUSHION DISTORSION
#define MIN_DISTORSION -0.99f
#define MAX_DISTORSION 0.0f
GLfloat distorsion = MAX_DISTORSION;
GLfloat distorsionSpeed = 0.75f;

//---  APP_STATE 
enum class AppStates { LoadingMap, LoadingAABBs, CreatingAABBsHierarchy, InterpolateOdorPath, Loaded };
AppStates appState = AppStates::LoadingMap;

enum class QuestStates { Cart, CartInspected, Odor };
QuestStates questState = QuestStates::Cart;

//--- MATRIXES FOR INSTANCED DRAWING 
vector<glm::mat4> treesMatrixes;

//--- AABBs list
vector<AABB> AABBs;
AABB AABBhierarchy = AABB();

//--- CART DATA
float cartX = 0.0f;
float cartZ = 0.0f;

//--- HOUSE DATA
float houseX = 0.0f;
float houseZ = 0.0f;

//--- INDEX TO KEEP TRACK OF THE CURRENT ROW TO LOAD ONE ROW PER RENDER LOOP
int currentCell = 0;

//--- PLANE PATH DATA
vector<glm::vec2> paths;

//--- ODOR PATH DATA
vector<glm::vec2> odor;
vector<Point> points;

//--- LOAD CSV DATA FILE
//--- I EXPECT A 32x32 CSV LIKE THE PLANE OF THE SIZE
vector<vector<string>> content = CsvLoader().read("../data/map.csv");

//--- SHADER LOCATIONS
string locationNames[] { "projectionMatrix", "viewMatrix", "tex", "repeat", "modelMatrix", "modelMatrixes", "colorIn", "distorsion", "time" }; 

#define LOCATION_PROJECTION_MATRIX 0
#define LOCATION_VIEW_MATRIX 1
#define LOCATION_TEXTURE 2
#define LOCATION_REPEAT 3
#define LOCATION_MODEL_MATRIX 4
#define LOCATION_MODEL_MATRIXES 5
#define LOCATION_COLOR 6
#define LOCATION_DISTORSION 7
#define LOCATION_TIME 8

//--- OUTLINE COLORS
GLfloat redColor[] = { 1.0f, 0.0f, 0.0f };
GLfloat yellowColor[] = { 1.0f, 1.0f, 0.0f };

//--- UTILS METHODS
void clear();
void setTexture(int index, GLint repeatLocation, float repeatValue);
void loadAABBs();
void addToAABBsHierarchy(vector<AABB> aabb);
void loadNextRow();
void interpolateOdorPath();
void drawPlayer(Shader shader, vector<GLint> locations, float scaleModifier);
void drawCart(Shader shader, vector<GLint> locations, float scaleModifier, string subroutine);
glm::vec2 buildCameraPosition(GLfloat distance);
string vecToString(glm::vec2 vector);
string vecToString(glm::vec3 vector);

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
    glEnable(GL_PROGRAM_POINT_SIZE);

    //--- SET CLEAR COLOR
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //---  INIT SHADERS 
    Shader baseShader = Shader("base.vert", "base.frag");
    Shader pointsShader = Shader("points.vert", "points.frag", "points.geom");

    //--- LOAD TEXTURES MODELS, MATRICES
    string names[] { "coin", "plane", "dog", "cart", "tree", "house", "odor" }; 
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
    long long playerCollisionµs = -1;
    long long cameraCollisionµs = -1;

    //--- LOADING RENDER LOOP
    while(appState != AppStates::Loaded)
    {
        if(glfwWindowShouldClose(window)) {
            baseShader.Delete();
            glfwTerminate();
            return 0;
        }

        if(appState == AppStates::InterpolateOdorPath) {
            interpolateOdorPath();
            appState = AppStates::Loaded;
        }


        if(appState == AppStates::CreatingAABBsHierarchy) {
            addToAABBsHierarchy( { AABBs.begin(), AABBs.end() } );
            appState = AppStates::InterpolateOdorPath;
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

        baseShader.Use();

        //--- SHADER LOCATIONS
        vector<GLint> locations;
        for (string name : locationNames) {
            locations.push_back(glGetUniformLocation(baseShader.Program, name.c_str()));
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
        GLuint vertSubIndex = glGetSubroutineIndex(baseShader.Program, GL_VERTEX_SHADER, "standard");
        glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &vertSubIndex);
        models[COIN_INDEX].Draw();

        glfwSwapBuffers(window);
    }

    cout << "Loading ended, binding new loop values" << endl;

    //delete &coinModel;

    //---  INIT UNIFORM BUFFER FOR TREES
    GLint uniformTreesMatrixBlockLocation = glGetUniformBlockIndex(baseShader.Program, "Matrices");
    glUniformBlockBinding(baseShader.Program, uniformTreesMatrixBlockLocation, 0);

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
        bool collision = AABBhierarchy.checkXZCollision(playerAABB);
        
        if(collision) {

            //--- CHECK COLLISION IF I MOVE ONLY IN THE Z DIRECTION
            playerPos = glm::vec3(oldDeltaX, 0, deltaZ);

            //--- CREATING AABB FROM VERTICES
            playerAABB = AABB(VerticesBuilder().build(playerPos, dy, glm::vec3(playerSize)));

            //--- DEFAULT VALUE OF COLLISION DETECTED
            bool ZmovementCollision = AABBhierarchy.checkXZCollision(playerAABB);

            //--- CHECK COLLISION IF I MOVE ONLY IN THE X DIRECTION
            playerPos = glm::vec3(deltaX, 0, oldDeltaZ);

            //--- CREATING AABB FROM VERTICES
            playerAABB = AABB(VerticesBuilder().build(playerPos, dy, glm::vec3(playerSize)));

            //--- DEFAULT VALUE OF COLLISION DETECTED
            bool XmovementCollision = AABBhierarchy.checkXZCollision(playerAABB);

            if(XmovementCollision) {
                deltaX = oldDeltaX;
            }

            if(ZmovementCollision) {
                deltaZ = oldDeltaZ;
            }
            
        }
        
        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

        if(playerCollisionµs < microseconds) {
            playerCollisionµs = microseconds;
        }

        start = std::chrono::high_resolution_clock::now();
        
        //--- IF CAMERADISTANCE IS LESS THAN MAX CAMERA DISTANCE, I REDUCE IT
        if(cameraDistance > maxCameraDistance) {
            cameraDistance = maxCameraDistance;
        }

        //--- GET POSITION OF CAMERA BASED ON DISTANCE FROM PLAYER
        glm::vec2 currentCamera = buildCameraPosition(cameraDistance);
        glm::vec2 farCamera = buildCameraPosition(min(maxCameraDistance, cameraDistance + CAMERA_DISTANCE_DELTA));

        //--- CHECK IF CURRENT AND FAR CAMERA COLLIDES
        bool currentCollision = AABBhierarchy.checkSegmentXZCollision(currentCamera, glm::vec2(playerPos.x, playerPos.z));
        bool farCameraCollision = AABBhierarchy.checkSegmentXZCollision(farCamera, glm::vec2(playerPos.x, playerPos.z));

        //--- IF I DON'T COLLIDE, I'M NOT AT MAX CAMERA DISTANCE AND FAR CAMERA DOESN'T COLLIDE, I MOVE TO FAR CAMERA
        if(!currentCollision && !farCameraCollision && cameraDistance < maxCameraDistance - EPSILON) {
            cameraDistance += CAMERA_DISTANCE_DELTA;
            view = glm::lookAt(glm::vec3(farCamera.x, 1.5f, farCamera.y), glm::vec3(deltaX, 1.5f, deltaZ), glm::vec3(0.0f, 1.0f, 0.0f));
        } else if(currentCollision) {
            //--- IF I COLLIDE, I GO NEAR PLAYER BY STEPS UNTIL I COLLIDE NO MORE
            glm::vec2 nearCamera = buildCameraPosition(max(MIN_CAMERA_DISTANCE, cameraDistance - CAMERA_DISTANCE_DELTA));
            while(AABBhierarchy.checkSegmentXZCollision(nearCamera, glm::vec2(playerPos.x, playerPos.z)) && cameraDistance > MIN_CAMERA_DISTANCE + EPSILON) {
                cameraDistance -= CAMERA_DISTANCE_DELTA;
                nearCamera = buildCameraPosition(cameraDistance);
            }
            view = glm::lookAt(glm::vec3(nearCamera.x, 1.5f, nearCamera.y), glm::vec3(deltaX, 1.5f, deltaZ), glm::vec3(0.0f, 1.0f, 0.0f));
        } else {
            //--- IF I DON'T COLLIDE AND I CAN'T MOVE THE CAMERA AWAY, I KEEP THE CURRENT CAMERA
            view = glm::lookAt(glm::vec3(currentCamera.x, 1.5f, currentCamera.y), glm::vec3(deltaX, 1.5f, deltaZ), glm::vec3(0.0f, 1.0f, 0.0f));
        }

        elapsed = std::chrono::high_resolution_clock::now() - start;
        microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

        if(cameraCollisionµs < microseconds) {
            cameraCollisionµs = microseconds;
        }

        //cout << "Camera collision: " << cameraCollisionµs << "micros\tPlayer collision: " << playerCollisionµs << "micros" << endl;

        //--- USE SHADER 
        baseShader.Use();

        //--- SHADER LOCATIONS
        vector<GLint> locations;
        for (string name : locationNames) {
            locations.push_back(glGetUniformLocation(baseShader.Program, name.c_str()));
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

        GLuint vertSubIndex = glGetSubroutineIndex(baseShader.Program, GL_VERTEX_SHADER, "standard");
        glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &vertSubIndex);

        //---  DRAW PLANE 
        models[PLANE_INDEX].Draw();

        GLuint fragmSubIndex = glGetSubroutineIndex(baseShader.Program, GL_FRAGMENT_SHADER, "textured");
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &fragmSubIndex);

        //--- SET HOUSE TEXTURE
        setTexture(HOUSE_INDEX, locations[LOCATION_REPEAT], 1.0f);

        //---  SET HOUSE MATRICES 
        matrices[HOUSE_INDEX] = glm::mat4(1.0f);
        matrices[HOUSE_INDEX] = glm::translate(matrices[HOUSE_INDEX], glm::vec3(houseX, 2.8f, houseZ));
        matrices[HOUSE_INDEX] = glm::rotate(matrices[HOUSE_INDEX], glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        matrices[HOUSE_INDEX] = glm::rotate(matrices[HOUSE_INDEX], glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        matrices[HOUSE_INDEX] = glm::scale(matrices[HOUSE_INDEX], glm::vec3(5.0f, 5.0f, 5.0f));
        glUniformMatrix4fv(locations[LOCATION_MODEL_MATRIX], 1, GL_FALSE, glm::value_ptr(matrices[HOUSE_INDEX]));

        //---  DRAW HOUSE 
        models[HOUSE_INDEX].Draw();

        //--- SET TREE TEXTURE 
        setTexture(TREE_INDEX, locations[LOCATION_REPEAT], 1.0f);

        //---  SET TREE MATRICES 
        glUniformMatrix4fv(locations[LOCATION_MODEL_MATRIXES], treesMatrixes.size(), GL_FALSE, glm::value_ptr(treesMatrixes[0]));

        //---  DRAW TREE
        vertSubIndex = glGetSubroutineIndex(baseShader.Program, GL_VERTEX_SHADER, "instanced");
        glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &vertSubIndex);
        models[TREE_INDEX].DrawInstanced(treesMatrixes.size());

        drawCart(baseShader, locations, 1.0f, "textured");

        drawPlayer(baseShader, locations, 1.0f);

        pointsShader.Use();

        glm::mat4 pointMatrix = glm::mat4(1.0f);
        pointMatrix = glm::translate(pointMatrix, glm::vec3(0.0f, 0.0f, 0.0f));

        glUniformMatrix4fv(glGetUniformLocation(pointsShader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(pointsShader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));

        glUniform1i(glGetUniformLocation(pointsShader.Program, "tex"), 1);
        glUniform1f(glGetUniformLocation(pointsShader.Program, "time"), glfwGetTime());

        //--- CREATE BUFFERS
        GLuint VAO, VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        //--- BIND VAO
        glBindVertexArray(VAO);

        //--- PUT VERTICES IN VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Point), &points[0], GL_STATIC_DRAW);

        //--- ACTIVATE FIRST ATTRIBUTE
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (GLvoid*)0);

        //--- SET TEXTURE
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textures[ODOR_INDEX]);

        //--- DRAW
        glDrawArrays(GL_POINTS, 0, points.size());

        baseShader.Use();

        //--- CLEAR SECOND TEXTURE OF FRAME BUFFER
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, secondTexture, 0);
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        clear();

        float distancePlayerCart = distance(glm::vec2(playerPos.x, playerPos.z), glm::vec2(cartX, cartZ));

        if((distorsion > -0.99f || keys[GLFW_KEY_SPACE]) && distancePlayerCart < 7.5) {
            if(questState == QuestStates::Cart && keys[GLFW_KEY_ENTER]) {
                questState = QuestStates::CartInspected;
            }
            glColorMask(false, false, false, false);
            glDepthMask(false);

            //--- IN THE FIRST PASS, ALL FRAGMENTS PASS THE STENCIL TEST
            //--- ACTION WHEN STENCIL FAILS, DEPTH FAILS AND BOTH PASS
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilMask(0xFF);

            drawCart(baseShader, locations, 1.0f, "textured");

            //--- REMOVE PLAYER FROM STENCIL
            glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);

            drawPlayer(baseShader, locations, 1.0f);
            
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

            fragmSubIndex = glGetSubroutineIndex(baseShader.Program, GL_FRAGMENT_SHADER, "fixedColor");
            glUniform3fv(locations[LOCATION_COLOR], 1, questState == QuestStates::Cart ? redColor : yellowColor);
            glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &fragmSubIndex);

            glm::mat4 planeModelMatrix2 = glm::mat4(1.0f);
            planeModelMatrix2 = glm::translate(planeModelMatrix2, glm::vec3(0.0f, 1.0f, -10.0f));
            planeModelMatrix2 = glm::rotate(planeModelMatrix2, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            planeModelMatrix2 = glm::scale(planeModelMatrix2, glm::vec3(1/16.0f * 17.0f, 1.0f, 1/16.0f * 10.0f));

            //---  SET PLANE MATRIX
            glUniformMatrix4fv(locations[LOCATION_MODEL_MATRIX], 1, GL_FALSE, glm::value_ptr(planeModelMatrix2));

            //---  DRAW PLANE
            vertSubIndex = glGetSubroutineIndex(baseShader.Program, GL_VERTEX_SHADER, "standard");
            glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &vertSubIndex);
            models[PLANE_INDEX].Draw();

            glStencilFunc(GL_ALWAYS, 1, 0xFF);
        }

        //--- BIND BACK TO FIRST TEXTURE
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, firstTexture, 0);

        //--- RENDER TO QUAD
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        glClearColor(135.0f / 255.f, 206.0f / 255.f, 235.0f / 255.f, 1.0f);

        clear();

        view = glm::lookAt(glm::vec3(0.0f, 1.0f, 7.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        fragmSubIndex = glGetSubroutineIndex(baseShader.Program, GL_FRAGMENT_SHADER, "pincushion");
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &fragmSubIndex);

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
        models[PLANE_INDEX].Draw();

        //--- SET PLANE TEXTURE 
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, secondTexture);

        fragmSubIndex = glGetSubroutineIndex(baseShader.Program, GL_FRAGMENT_SHADER, "tracePlane");
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &fragmSubIndex);
        
        glm::mat4 planeModelMatrix3 = glm::mat4(1.0f);
        planeModelMatrix3 = glm::translate(planeModelMatrix3, glm::vec3(0.0f, 1.0f, -9.95f));
        planeModelMatrix3 = glm::rotate(planeModelMatrix3, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        planeModelMatrix3 = glm::scale(planeModelMatrix3, glm::vec3(1/16.0f * 17.0f, 1.0f, 1/16.0f * 10.0f));

        //---  SET PLANE MATRIX
        glUniformMatrix4fv(locations[LOCATION_MODEL_MATRIX], 1, GL_FALSE, glm::value_ptr(planeModelMatrix3));

        //---  DRAW PLANE 
        models[PLANE_INDEX].Draw();

        //--- SWAP BUFFERS
        glfwSwapBuffers(window);
    }

    //--- DELETE USED SHADERS
    baseShader.Delete();
    
    //--- CLOSE AND DELETE CONTEXT
    glfwTerminate();

    return 0;
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
            maxCameraDistance -= cameraZoomSpeed * deltaTime;
            cameraY -= cameraZoomSpeed * deltaTime;
            if(maxCameraDistance < MIN_CAMERA_DISTANCE_SENSES) {
                maxCameraDistance = MIN_CAMERA_DISTANCE_SENSES;
            }
            if(cameraY < MIN_CAMERA_Y_DELTA) {
                cameraY = MIN_CAMERA_Y_DELTA;
            }
            if(distorsion < MIN_DISTORSION) {
                distorsion = MIN_DISTORSION;
            }
        } else {
            distorsion += distorsionSpeed * deltaTime;
            maxCameraDistance += cameraZoomSpeed * deltaTime;
            cameraY += cameraZoomSpeed * deltaTime;
            if(maxCameraDistance > MAX_CAMERA_DISTANCE) {
                maxCameraDistance = MAX_CAMERA_DISTANCE;
            }
            if(cameraY > MAX_CAMERA_Y_DELTA) {
                cameraY = MAX_CAMERA_Y_DELTA;
            }
            if(distorsion > MAX_DISTORSION) {
                distorsion = MAX_DISTORSION;
                if(questState == QuestStates::CartInspected) {
                    questState = QuestStates::Odor;
                }
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

void interpolateOdorPath() {
    int numPoints = odor.size();
    vector<glm::vec2> tangents;
    vector<glm::vec2> slopes;
    tangents.push_back(glm::vec2((rand() % 30 - 15), (rand() % 30 - 15)));
    slopes.push_back(glm::vec2((rand() % 50) / 10.0f, (rand() % 20) / 10.0f));
    for (std::size_t i = 1; i != odor.size() - 1; ++i) {
        tangents.push_back(glm::vec2(odor[i].x - odor[i-1].x, odor[i].y - odor[i-1].y) + glm::vec2(odor[i+1].x - odor[i].x, odor[i+1].y - odor[i].y));
        slopes.push_back(glm::vec2((rand() % 50) / 10.0f, (rand() % 20) / 10.0f));
    }
    tangents.push_back(glm::vec2((rand() % 30 - 15), (rand() % 30 - 15)));
    slopes.push_back(glm::vec2((rand() % 50) / 10.0f, (rand() % 20) / 10.0f));

    for (std::size_t i = 0; i != odor.size() - 1; ++i) {
        Point first = Point();
        first.Position = glm::vec3(odor[i].x, 2.0f, odor[i].y);
        points.push_back(first);
        for(int index = 1; index < 10; ++index) {
            float value = index / 10.0f;
            glm::vec2 p0 = ((float)((2 * pow(value, 3) - 3 * pow(value, 2) + 1))) * odor[i];
            glm::vec2 m0 = ((float)((pow(value, 3) - 2 * pow(value, 2) + value))) * tangents[i];
            glm::vec2 m1 = ((float)((pow(value, 3) - pow(value, 2)))) * tangents[i + 1];
            glm::vec2 p1 = ((float)((-2 * pow(value, 3) + 3 * pow(value, 2)))) * odor[i + 1];
            float y = 2.0f;
            glm::vec3 result = glm::vec3(p0 + m0 + m1 + p1, y);
            Point point = Point();
            point.Position = glm::vec3(result.x, result.z, result.y);
            points.push_back(point);
        }
    }
    Point last = Point();
    last.Position = glm::vec3(odor[numPoints - 1].x, 2.0f, odor[numPoints - 1].y);
    points.push_back(last);

    for (auto i=points.begin(); i!=points.end(); ++i) {
        Point current = *i;
        cout << current.Position.x << "\t" << current.Position.y << "\t" << current.Position.z << endl;
    }
}

void addToAABBsHierarchy(vector<AABB> AABBlist) {
    cout << "Adding AABB to AABBs' hierarchy" << endl;
    for (auto i=AABBlist.begin(); i!=AABBlist.end(); ++i) {
        AABB current = *i;
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
        if(*i == "H") {
            houseX = currentCell * 2;
            houseZ = position * 2;
        }
        if(*i == "P") {
            paths.push_back(glm::vec2(currentCell, position));
        }
        if(*i == "O") {
            odor.push_back(glm::vec2(currentCell * 2, position * 2));
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
    GLuint vertSubIndex = glGetSubroutineIndex(shader.Program, GL_VERTEX_SHADER, "standard");
    glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &vertSubIndex);
    models[PLAYER_INDEX].Draw();
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
    GLuint vertSubIndex = glGetSubroutineIndex(shader.Program, GL_VERTEX_SHADER, "standard");
    glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &vertSubIndex);
    models[CART_INDEX].Draw();
}

string vecToString(glm::vec2 vector) {
    return "[ " + std::to_string(vector.x) + " :: " + std::to_string(vector.y) + " ]";
}

string vecToString(glm::vec3 vector) {
    return "[ " + std::to_string(vector.x) + " :: " + std::to_string(vector.y) + " :: " + std::to_string(vector.z) + " ]";
}

glm::vec2 buildCameraPosition(GLfloat distance) {
    GLfloat distX = -sin(glm::radians(rotationY)) * distance;
    GLfloat distZ = cos(glm::radians(rotationY)) * distance;
    GLfloat cameraX = deltaX + distX;
    GLfloat cameraZ = deltaZ - distZ;
    return glm::vec2(cameraX, cameraZ);
}