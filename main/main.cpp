//---  Std. Includes
#include <string>
#define GLM_ENABLE_EXPERIMENTAL
//---  Loader estensions OpenGL
#ifdef _WIN32
    #define APIENTRY __stdcall
#endif

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
GLfloat deltaZ = 0.0f;
GLfloat deltaX = 0.0f;
GLfloat speed = 25.0f;
GLfloat rotationY = 90.0f;
GLfloat rotationSpeed = 2.0f;

//---  APP_STATE 
enum class AppStates { Loading, Loaded };
AppStates appState = AppStates::Loading;

//--- SHOW WIREFRAME BOOLEAN
bool showWireframe = false;
bool showAABB = false;

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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //---  INIT SHADERS 
    Shader shader = Shader("base.vert", "base.frag");
    SetupShader(shader.Program);
    
    //---  LOAD MODELS 
    Model coinModel("../models/coin.obj");
    Model planeModel("../models/plane.obj");
    Model playerModel("../models/player.obj");
    Model treeModel("../models/tree.obj");
    Model cartModel("../models/tree.obj");

    //--- LOAD TEXTURES 
    int coinTextureIndex = 0;
    int planeTextureIndex = 1;
    int playerTextureIndex = 2;
    int treeTextureIndex = 3;
    textureId.push_back(LoadTexture("../textures/coin.jpeg"));
    textureId.push_back(LoadTexture("../textures/plane.jpg"));
    textureId.push_back(LoadTexture("../textures/player.png"));
    textureId.push_back(LoadTexture("../textures/tree.jpg"));

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

    //--- AABB COLOR
    GLfloat aabbColor[] = { 1.0f, 0.0f, 0.0f };

    //---  INIT CAMERA 
    glm::mat4 projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 10000.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    //--- LOAD DATA FILE
    //--- I EXPECT A 32x32 CSV AS THE PLANE OF THE SIZE
    cout << "Loading map file" << endl;
	vector<vector<string>> content;
	vector<string> row;
	string line, word;
	fstream file ("../data/map.csv", ios::in);
	if(file.is_open()) {
		while(getline(file, line))
		{
			row.clear();
 
			stringstream str(line);
 
			while(getline(str, word, ',')) {
				row.push_back(word);
            }
			content.push_back(row);
		}
	} else {
		std::cout << "Failed to load level data" << std::endl;
        return -1;
    }

    row.clear();
    row.shrink_to_fit();

    //--- KEEP TRACK OF THE CURRENT ROW TO LOAD ONE ROW PER RENDER LOOP
    int current = 0;

    //--- INIT MATRIXES FOR INSTANCED DRAWING 
    vector<glm::mat4> treesMatrixes;
    vector<AABB> treesAABBs;

    cout << "Starting loading loop" << endl;

    //--- LOADING RENDER LOOP
    while(appState == AppStates::Loading)
    {
        if(glfwWindowShouldClose(window)) {
            shader.Delete();
            glfwTerminate();
            return 0;
        }

        //--- CONTINUE LOADING THE LEVEL
        if(current < content.size()) {
            for (auto i=content[current].begin(); i!=content[current].end(); ++i) {
                float position = i-content[current].begin();
                if(*i == "T") {
                    //--- TREES ARE RANDOMLY DISPLACED FROM THEIR 0.5x0.5 cell by a random value between -0.5f and 0.5f
                    float randX = (rand() % 10 - 5) / 10.f;
                    float randZ = (rand() % 10 - 5) / 10.f;
                    //--- TREES ARE RANDOMLY SCALED FROM 100% TO 150%
                    float randomScale = (100 + (rand() % 50)) / 100.f;
                    glm::mat4 treeMatrix = glm::mat4(1.0f);
                    treeMatrix = glm::translate(treeMatrix, glm::vec3(current * 2 + 0.5 + randX, 0.0f, position * 2 + 0.5f + randZ));
                    treeMatrix = glm::scale(treeMatrix, glm::vec3(randomScale, randomScale, randomScale));
                    treesMatrixes.push_back(treeMatrix);
                }
                if(*i == "S") {
                    deltaX = current * 2;
                    deltaZ = position * 2;
                }
                if(*i == "C") {
                    cartX = current * 2;
                    cartZ = position * 2;
                }
                if(*i == "P") {
                    paths.push_back(glm::vec2(current-16, position-16));
                }
            }
        } else {
            appState = AppStates::Loaded;
        }
        current++;

        if(showWireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        //---  UPDATE TIME 
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        coinRotationY += coinRotationSpeed * deltaTime;

        glfwPollEvents();

        //---  CLEAR 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

    cout << "Press P to show wireframes" << endl;

    cout << "Press B to show AABBs" << endl;

    while(!glfwWindowShouldClose(window))
    {
        //---  UPDATE TIME 
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //---  CHECK INPUT EVENTS 
        glfwPollEvents();
        process_keys(window);

        if(showWireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        //---  CLEAR 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //** UPDATE CAMERA POSITION TO FOLLOW PLAYER
        GLfloat distX = sin(glm::radians(rotationY)) * 2.5f;
        GLfloat distZ = cos(glm::radians(rotationY)) * 2.5f;
        view = glm::lookAt(glm::vec3(deltaX - distX, 2.0f, 2.5f + deltaZ + distZ * -1.0f), glm::vec3(deltaX + distX, 0.0f, 2.5f + deltaZ + distZ), glm::vec3(0.0f, 1.0f, 0.0f));
        
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

        //--- SET PLANE TEXTURE 
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureId[planeTextureIndex]);

        //--- PASS VALUES TO SHADER 
        glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(view));
        glUniform1i(textureLocation, 1);
        glUniform1f(repeatLocation, 80.0);
        glUniform1i(instancedLocation, false);
        
        //---  SET PLANE MATRIX
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(planeModelMatrix));

        //---  DRAW PLANE 
        planeModel.Draw();

        GLuint subroutineIndex = glGetSubroutineIndex(shader.Program, GL_FRAGMENT_SHADER, "textured");
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subroutineIndex);

        //--- SET PLAYER TEXTURE 
        glBindTexture(GL_TEXTURE_2D, textureId[playerTextureIndex]);
        glUniform1f(repeatLocation, 1.0);

        //---  SET PLAYER MATRICES 
        playerModelMatrix = glm::mat4(1.0f);
        playerModelMatrix = glm::translate(playerModelMatrix, glm::vec3(deltaX, 0.0f, 2.5f + deltaZ));
        playerModelMatrix = glm::rotate(playerModelMatrix, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
        playerModelMatrix = glm::scale(playerModelMatrix, glm::vec3(0.11f, 0.11f, 0.11f));
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(playerModelMatrix));

        //---  DRAW PLAYER 
        playerModel.Draw();

        //--- SET TREE TEXTURE 
        glBindTexture(GL_TEXTURE_2D, textureId[treeTextureIndex]);
        glUniform1f(repeatLocation, 1.0);
        glUniform1i(instancedLocation, true);

        //---  SET TREE MATRICES 
        glUniformMatrix4fv(modelMatrixesLocation, treesMatrixes.size(), GL_FALSE, glm::value_ptr(treesMatrixes[0]));

        //---  DRAW TREE
        treeModel.DrawInstanced(treesMatrixes.size());

        GLuint index = glGetSubroutineIndex(shader.Program, GL_FRAGMENT_SHADER, "fixedColor");
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &index);

        //--- SET CART COLOR
        glUniform3fv(colorInLocation, 1, cartColor);
        glUniform1i(instancedLocation, false);

        //---  SET CART MATRICES 
        cartModelMatrix = glm::mat4(1.0f);
        cartModelMatrix = glm::translate(cartModelMatrix, glm::vec3(cartX, 0.0f, cartZ));
        cartModelMatrix = glm::scale(cartModelMatrix, glm::vec3(2.0f, 1.0f, 2.0f));
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(cartModelMatrix));

        //---  DRAW CART 
        cartModel.Draw();

        //--- SET AABB COLOR
        glUniform3fv(colorInLocation, 1, aabbColor);

        if(showAABB) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            for (auto i=treesMatrixes.begin(); i!=treesMatrixes.end(); ++i) {
                glm::mat4 matrix = *i;
                glm::vec3 treePos = glm::vec3(matrix[3].x / 2, matrix[3].y / 2, matrix[3].z / 2);
                float treeScale = matrix[0].x / 3.5f;
                GLfloat dx = 8.0f;
                GLfloat dz = 15.0f;
                GLfloat dy = 9.0f;
                GLfloat vertices[] = {
                    treePos.x + treeScale - dx,     dy * treeScale,  treePos.z - treeScale - dz,  // Top Right
                    treePos.x + treeScale - dx,     0,               treePos.z - treeScale - dz,  // Bottom Right
                    treePos.x - treeScale - dx,     0,               treePos.z - treeScale - dz,  // Bottom Left
                    treePos.x - treeScale - dx,     dy * treeScale,  treePos.z - treeScale - dz,  // Top Left
                    treePos.x + treeScale - dx,     dy * treeScale,  treePos.z + treeScale - dz,  // Top Right
                    treePos.x + treeScale - dx,     0,               treePos.z + treeScale - dz,  // Bottom Right
                    treePos.x - treeScale - dx,     0,               treePos.z + treeScale - dz,  // Bottom Left
                    treePos.x - treeScale - dx,     dy * treeScale,  treePos.z + treeScale - dz   // Top Left
                };

                GLuint indices[] = {
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

                GLuint VBO, VAO, EBO;
                glGenVertexArrays(1, &VAO);
                glGenBuffers(1, &VBO);
                glGenBuffers(1, &EBO);
                // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
                glBindVertexArray(VAO);

                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

                glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind

                glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO

                // Draw our first triangle
                glUseProgram(shader.Program);
                glBindVertexArray(VAO);
                //glDrawArrays(GL_TRIANGLES, 0, 6);
                glDrawElements(GL_TRIANGLES, 32, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }
        }

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

    if(key == GLFW_KEY_P && action == GLFW_PRESS) {
        showWireframe = !showWireframe;
    }

    if(key == GLFW_KEY_B && action == GLFW_PRESS) {
        showAABB = !showAABB;
    }

    if(action == GLFW_PRESS) {
        keys[key] = true;
    }

    if(action == GLFW_RELEASE) {
        keys[key] = false;
    }
}

void process_keys(GLFWwindow* window) {

    if(appState != AppStates::Loading) {
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
