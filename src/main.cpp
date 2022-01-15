#define STB_IMAGE_IMPLEMENTATION

#if defined(IMGUI_IMPL_OPENGL_ES2)
#endif

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#include <GL/glew.h>
#include "imgui.h"
#include "include/imgui/imgui_impl_glfw.h"
#include "include/imgui/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <stb_image.h>
#include <spdlog/spdlog.h>
#include <filesystem>
#include <chrono>
#include <thread>
#include "include/helpers/Shader.h"
#include "include/helpers/Model.h"
#include <iostream>
#include <string>
#include <regex>

void framebufferSizeCallback(GLFWwindow* window, int width, int height = false);
void processInput(GLFWwindow *window, auto &models);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// settings
unsigned int SCR_WIDTH = 1100;
unsigned int SCR_HEIGHT = 800;

const int VAOs = 1;
const int VBOs = 5;

GLuint VAO[VAOs];
GLuint VBO[VBOs];

GLuint frameBufferObject;
GLuint renderBufferObject;
unsigned int textureColorBuffer;

// OpenGL window default size and position
ImVec2 glWindowSize = ImVec2(SCR_WIDTH, SCR_HEIGHT);
ImVec2 frameBufferSize = glWindowSize;
ImVec2 glWindowPosition;

// Camera
glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 8.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 2.0f,  0.0f);

float lastX = SCR_WIDTH / 2.0f  ;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right, so we initially rotate a bit to the left.
float pitch = 0.0f;
float fieldOfView = 45.0f;

// Lighting
glm::vec3 lightPos(0.0f, 100.0f, 0.0f);

// Timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
bool jump = false;
float jumpStart = 0.0f;

const float planeMax = 20.0f;
const float floorMin = 0.0f;

int cubeCount = 10;
std::vector<glm::vec3> cubePositions;

float offset = 0.05f;

bool wireframe = false;

// Models
//Model unicorn;
//Model wabbit;
glm::vec3 unicornColorTest;


void createTexture(GLuint& texture, const std::string& path, bool alpha){
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    int width, height, nrChannels;
    unsigned char *data = stbi_load(std::filesystem::path(path).c_str(), &width, &height, &nrChannels, 0);
    if (data){
        if (alpha){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        else{
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else{
        spdlog::error("Failed to load texture");
    }
    stbi_image_free(data);
}

void renderObject(int vboID, int textureID, glm::mat4 model, Shader& shader, int points, glm::vec3 position, float newScale = 1.0f, float rotate = 0.0f){
    glBindBuffer(GL_ARRAY_BUFFER, VBO[vboID]);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
    model = glm::scale(model, glm::vec3(newScale, newScale, newScale));
    model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, position);

    shader.setMat4("model", model);
    glDepthFunc(GL_LEQUAL);
    glDrawArrays(GL_TRIANGLES, 0, points);
}

void playerJump(float& currentFrame, auto &models){
    float floorOffset = 1.0f;
    float amplitude = 0.75f;
    float speed = 6.0f;
    float x = currentFrame - jumpStart;
    float playerView = floorMin + floorOffset;

    if (x < 0.0) x = 0.0;

    if (jump){
        float y = amplitude * (glm::sin(speed * x)) + 1.0;
        if (y >= playerView) {
            cameraPos.y = y + floorOffset;
            models.at("unicorn/unicorn.obj").position.y = y;
        }
        else {
            jump = false;
        }
    }
}

GLuint initFrameBuffer(){
    glGenFramebuffers(1, &frameBufferObject);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
    // generate texture
    glGenTextures(1, &textureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frameBufferSize.x, frameBufferSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // attach it to currently bound framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);

    glGenRenderbuffers(1, &renderBufferObject);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObject);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, frameBufferSize.x, frameBufferSize.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBufferObject);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        spdlog::error("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return frameBufferObject;
}

void renderGrass(){
//            grassShaderInstanced.use();
//            grassShaderInstanced.setInt("texture_diffuse1", 0);
//            glActiveTexture(GL_TEXTURE0);
//            glBindTexture(GL_TEXTURE_2D, grass.textures_loaded[0].id); // note: we also made the textures_loaded vector public (instead of private) from the model class.
//            for (unsigned int i = 0; i < grass.meshes.size(); i++)
//            {
//                glBindVertexArray(grass.meshes[i].VAO);
//                glDrawElementsInstanced(GL_TRIANGLES, grass.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, grassCount);
//                glBindVertexArray(0);
//            }
}

void renderUnicorn(Shader &shader, auto &models, auto &textures){
    shader.use();
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, models.at("unicorn/unicorn.obj").position);
    model = glm::rotate(model, glm::radians(models.at("unicorn/unicorn.obj").rotationDegrees), models.at("unicorn/unicorn.obj").rotationAxis);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::scale(model, models.at("unicorn/unicorn.obj").scale);
    shader.setMat4("model", model);
    shader.setVec3("lightPos", lightPos);
    shader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setVec3("objectColor", unicornColorTest);
    models.at("unicorn/unicorn.obj").Draw(shader);
}

void renderWabbit(Shader &shader, auto &models, auto &textures){
    shader.use();
//    glBindTexture(GL_TEXTURE_2D, texture_fur);

    shader.use();
    glm::mat4 model = glm::mat4(1.0f);
    if (models.at("wabbit/wabbit.obj").position.x >= planeMax || models.at("wabbit/wabbit.obj").position.z >= planeMax){
        offset = -offset;
    }
    if (models.at("wabbit/wabbit.obj").position.x <= -planeMax || models.at("wabbit/wabbit.obj").position.z <= -planeMax){
        offset = abs(offset);
    }

    // Wacky Wabbit Antics
    float amplitude = 0.3f;
    float speed = 7.0f;
    float degrees = 0.0f;

    float x = models.at("wabbit/wabbit.obj").position.x + (offset);
    float y = amplitude * (glm::sin(speed * x)) + 0.2f;
    float z = 10.0f * sin(0.25f * x);

    model = glm::translate(model, glm::vec3(x, y, z));
    if (offset > 0){
        degrees = 90.0f;
        if (models.at("wabbit/wabbit.obj").position.z > z){
            degrees += 45.0f;
        }
        else {
            degrees -= 45.0f;
        }
    }
    else{
        degrees = -90.0f;
        if (models.at("wabbit/wabbit.obj").position.z > z){
            degrees -= 45.0f;
        }
        else {
            degrees += 45.0f;
        }
    }
    models.at("wabbit/wabbit.obj").position = glm::vec3(x, y, z);

    model = glm::rotate(model, glm::radians(degrees), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
    shader.setMat4("model", model);
    models.at("wabbit/wabbit.obj").Draw(shader);
}

void renderPlane(Shader &shader, auto &mesh, auto &texture){
    shader.use();
    glBindTexture(GL_TEXTURE_2D, texture);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(planeMax, 10.0f, planeMax));
    shader.setMat4("model", model);
    mesh.Draw(shader);
}

void renderMysteryCubes(Shader &shader, auto &mesh, auto &count, auto &positions, auto &texture, auto &currentFrame){
    shader.use();
    glBindTexture(GL_TEXTURE_2D, texture);
    for (unsigned int n = 0; n < count; n++) {
        glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        float angle = 20.0f * currentFrame * ((float)n + 1);
        model = glm::translate(model, positions[n]);
        model = glm::rotate(model, glm::radians(angle), positions[n]);
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        shader.setMat4("model", model);
        mesh.Draw(shader);
    }
}

void renderSkyDome(Shader &shader, auto &mesh, auto &texture, auto &currentFrame) {
    shader.use();
    glBindTexture(GL_TEXTURE_2D, texture);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, currentFrame * 0.01f, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(200.0f, 200.0f, 200.0f));
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    shader.setMat4("model", model);
    mesh.Draw(shader);
}

void renderLamp(Shader &shader){
    //             Draw the lamp object
    glActiveTexture(GL_TEXTURE0);
    shader.use();
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(lightPos.x, lightPos.y, lightPos.z));
    shader.setMat4("model", model);
}

void render(auto &shaders, auto &models, auto &textures, auto &currentFrame){
    // Render
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);

    // Pass projection matrix to shader
    glm::mat4 projection = glm::perspective(glm::radians(fieldOfView), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    // Camera/view transformation
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    // Model. Make sure to initialize matrix to identity matrix first
    glm::mat4 model = glm::mat4(1.0f);

    // view/projection transformations
    shaders.at("grass").use();
    shaders.at("grass").setMat4("projection", projection);
    shaders.at("grass").setMat4("view", view);

    shaders.at("light").use();
    shaders.at("light").setMat4("projection", projection);
    shaders.at("light").setMat4("view", view);

    shaders.at("unicornBody").use();
    shaders.at("unicornBody").setMat4("projection", projection);
    shaders.at("unicornBody").setMat4("view", view);

    shaders.at("default").use();
    shaders.at("default").setMat4("projection", projection);
    shaders.at("default").setMat4("view", view);


    // Render the loaded models
    renderPlane(shaders.at("default"), models.at("primitives/plane.obj"), textures.at("grass.png"));
    renderMysteryCubes(shaders.at("default"), models.at("primitives/cube.obj"), cubeCount, cubePositions, textures.at("mario_mystery.png"), currentFrame);
    renderSkyDome(shaders.at("default"), models.at("primitives/sphere.obj"), textures.at("skydome.jpeg"), currentFrame);
    renderWabbit(shaders.at("default"), models, textures);
    renderUnicorn(shaders.at("unicornBody"), models, textures);
    renderLamp(shaders.at("light"));
}

int main(){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "twelvechairs", NULL, NULL);
    if (window == NULL){
        spdlog::error("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
//    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Anti-aliasing
    glfwWindowHint(GLFW_SAMPLES, 4);
    glEnable(GL_MULTISAMPLE);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // Load GLEW so it configures OpenGL
    if (glewInit() != GLEW_OK){
        spdlog::error("glewInit");
        return 0;
    }

    // Build and compile our shader programs
    std::vector<std::string> shaderNames = {
            "default",
            "unicornBody",
            "unicornMane",
            "unicornTail",
            "light",
            "grass",
            "shadowMappingDepth"
    };
    std::map<std::string, Shader> shaders;
    for (const auto& shader: shaderNames){
        std::string vert = fmt::format("../src/include/shaders/{}.vert", shader);
        std::string frag = fmt::format("../src/include/shaders/{}.frag", shader);
        shaders.insert({shader, Shader(vert.c_str(), frag.c_str())});
    }

    // Load and create a texture
    std::vector<std::string> textureNames = {
            "awesomeface.png",
            "clouds.jpeg",
            "container.jpeg",
            "fur.jpeg",
            "grass.png",
            "mario_mystery.png",
            "rock.jpeg",
            "skydome.jpeg",
            "wood.jpg",
            "space.png"
    };
    std::map<std::string, unsigned int> textures;
    for (const auto &textureName : textureNames){
        unsigned int textureId;
        std::string path = fmt::format("../src/include/assets/textures/{}", textureName);
        createTexture(textureId, path, path.find(".png") != string::npos ? true : false);
        textures.insert({textureName, textureId});
    }

    std::vector<std::string> modelNames = {
            "unicorn/unicorn.obj",
            "wabbit/wabbit.obj",
            "grass/trava.obj",
            "primitives/plane.obj",
            "primitives/cube.obj",
            "primitives/sphere.obj"
    };
    std::map<std::string, Model> models;
    for (const auto& model: modelNames){
        std::string path = fmt::format("../src/include/assets/{}", model);
        models.insert({model, Model(path.c_str())});
    }


    // Static world space positions of our cubes and pyramids
    for (unsigned int n = 0; n < cubeCount; n++){
        cubePositions.emplace_back(
                glm::linearRand(-planeMax, planeMax),
                glm::linearRand(2.5f, 6.0f),
                glm::linearRand(-planeMax, planeMax)
        );
    }

    // Generate grass objects for GPU instancing
    unsigned int grassCount = 1200;
    glm::mat4 *modelMatrices;
    modelMatrices = new glm::mat4[grassCount];
    for (unsigned int i = 0; i < grassCount; i++){
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(
                glm::linearRand(-planeMax, planeMax),
                glm::linearRand(-0.3f, -0.1f),
                glm::linearRand(-planeMax, planeMax)
        ));
        model = glm::rotate(model, glm::radians(models.at("unicorn/unicorn.obj").rotationDegrees), models.at("unicorn/unicorn.obj").rotationAxis);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));

        modelMatrices[i] = model;
    }

    // Init models
    stbi_set_flip_vertically_on_load(false);
    models.at("unicorn/unicorn.obj").scale = glm::vec3(0.35f, 0.35f, 0.35f);
    models.at("unicorn/unicorn.obj").rotationDegrees = 180.0f;
    models.at("wabbit/wabbit.obj").position = glm::vec3(-20.0f, 0.0f, 20.0f);


    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
//    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");

    // Our state
    bool show_demo_window = false;
    auto imguiMainBackgroundColor = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);

    initFrameBuffer();

    // configure depth map FBO
    // -----------------------
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // shader configuration
    // --------------------
    shaders.at("light").use();
    shaders.at("light").setInt("material.diffuse", 0);

    static float lightAmbient = 1.0f;
    static float lightDiffuse = 0.0f;
    static float lightSpecular = 0.0f;

    static float materialSpecular = 0.5f;
    static float materialShine = 0.1f;


    // configure instanced array
    // -------------------------
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, grassCount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

    // set transformation matrices as an instance vertex attribute (with divisor 1)
    // note: we're cheating a little by taking the, now publicly declared, VAO of the model's mesh(es) and adding new vertexAttribPointers
    // normally you'd want to do this in a more organized fashion, but for learning purposes this will do.
    // -----------------------------------------------------------------------------------------------------------------------------------
    for (unsigned int i = 0; i < models.at("grass/trava.obj").meshes.size(); i++){
        unsigned int vao = models.at("grass/trava.obj").meshes[i].VAO;
        glBindVertexArray(vao);
        // set attribute pointers for matrix (4 times vec4)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);

        glBindVertexArray(0);
    }
    float offset = 0.05f;

    // Render loop
    while (!glfwWindowShouldClose(window)){
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Wireframe-only
        wireframe ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 1. render depth of scene to texture (from light's perspective)
        // --------------------------------------------------------------
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 1.0f, far_plane = 7.5f;
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;
        // render scene from light's point of view
        shaders.at("shadowMappingDepth").use();
        shaders.at("shadowMappingDepth").setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures.at("grass.png"));
        render(shaders, models, textures, currentFrame);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // reset viewport
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activate shader when setting uniforms/drawing objects
        shaders.at("default").use();
        shaders.at("default").setVec3("light.position", lightPos);
        shaders.at("default").setVec3("viewPos", cameraPos);
        // Light properties
        shaders.at("default").setVec3("light.ambient", glm::vec3(lightAmbient));
        shaders.at("default").setVec3("light.diffuse", glm::vec3(lightDiffuse));
        shaders.at("default").setVec3("light.specular", glm::vec3(lightSpecular));
        // Material properties
        shaders.at("default").setVec3("material.specular", glm::vec3(materialSpecular));
        shaders.at("default").setFloat("material.shininess", materialShine);

        // Activate shader when setting uniforms/drawing objects
        shaders.at("unicornBody").use();
        shaders.at("unicornBody").setVec3("lightPos", lightPos);
        shaders.at("unicornBody").setVec3("viewPos", cameraPos);
        // Light properties
        shaders.at("unicornBody").setVec3("light.ambient", glm::vec3(lightAmbient));
        shaders.at("unicornBody").setVec3("light.diffuse", glm::vec3(lightDiffuse));
        shaders.at("unicornBody").setVec3("light.specular", glm::vec3(lightSpecular));

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar()){
            if (ImGui::BeginMenu("File")){
                if (ImGui::MenuItem("Open")){
                    //Do something
                }
                if (ImGui::MenuItem("Exit")){
                    // Delete all the objects we've created
                    ImGui_ImplOpenGL3_Shutdown();
                    ImGui_ImplGlfw_Shutdown();
                    ImGui::DestroyContext();

                    // Delete window before ending the program
                    glfwDestroyWindow(window);
                    // Terminate GLFW before ending the program
                    glfwTerminate();
                    return 0;

                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Options")){
                if (ImGui::MenuItem("Something")){
                    //Do something
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // OpenGL window
        static bool use_work_area = false;
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
        ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

        ImGui::Begin("OpenGL", NULL, flags);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::BeginChild("Render");
        auto glWindowSize = ImGui::GetContentRegionAvail();
        auto glWindowPosition = ImGui::GetWindowPos();
        frameBufferSize.x = glWindowSize.x;
        frameBufferSize.y = glWindowSize.y;

        // Specify the viewport of OpenGL in the Window
        glViewport(0, 0, glWindowSize.x, glWindowSize.y);

        SCR_WIDTH = frameBufferSize.x / 2;
        SCR_HEIGHT = frameBufferSize.y / 2;

        try {
            glEnable(GL_FRAMEBUFFER_SRGB);
            glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frameBufferSize.x, frameBufferSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObject);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, frameBufferSize.x, frameBufferSize.y);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBufferObject);
            glClear(GL_FRAMEBUFFER);
            glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
            // ============================================================================================================================

            render(shaders, models, textures, currentFrame);

            // ============================================================================================================================
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            ImGui::Image(reinterpret_cast<ImTextureID>(textureColorBuffer), frameBufferSize, ImVec2(0, 1), ImVec2(1, 0));
            glDisable(GL_FRAMEBUFFER_SRGB);

        }
        catch (std::exception &e){
            spdlog::error("ImGui::Image: {}", e.what());
        }
        ImGui::EndChild();
        ImGui::End();

        // Stats window
        ImGui::SetNextWindowPos(ImVec2(0, 20), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(220, 50), ImGuiCond_Always);
        flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
        ImGui::Begin("Stats", NULL, flags);
        ImGui::Text(" %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        // Stores the coordinates of the cursor
        double mouseX;
        double mouseY;
        // Fetches the coordinates of the cursor
        glfwGetCursorPos(window, &mouseX, &mouseY);
        ImGui::Text(" %.1f x %.1f", mouseX, mouseY);
        ImGui::End();

        // testLighting window
        ImGui::SetNextWindowPos( ImVec2(0, 80), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(220, 300), ImGuiCond_Always);
        ImGui::Begin("testLighting", NULL, flags);
        ImGui::SliderFloat("lAmbient", &lightAmbient, 0.0, 1.0);
        ImGui::SliderFloat("lDiffuse", &lightDiffuse, 0.0, 1.0);
        ImGui::SliderFloat("lSpecular", &lightSpecular, 0.0, 1.0);
        ImGui::SliderFloat("mSpecular", &materialSpecular, 0.0, 1.0);
        ImGui::SliderFloat("mShine", &materialShine, 0.0, 100.0);
        ImGui::SliderFloat("light.x", &lightPos.x, -planeMax - 10, planeMax + 10);
        ImGui::SliderFloat("light.y", &lightPos.y, -1, 300);
        ImGui::SliderFloat("light.z", &lightPos.z, -planeMax - 10, planeMax + 10);
        ImGui::SliderFloat("camera.x", &cameraPos.x, -planeMax - 10, planeMax + 10);
        ImGui::SliderFloat("camera.y", &cameraPos.y, -1, 300);
        ImGui::SliderFloat("camera.z", &cameraPos.z, -planeMax - 10, planeMax + 10);
        ImGui::Checkbox("Wireframe", &wireframe);
        ImGui::End();

        ImGui::SetNextWindowPos( ImVec2(0, 390), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(220, 160), ImGuiCond_Always);
        ImGui::Begin("Unicorn Color", NULL, flags);
        ImGui::ColorPicker4("Color", (float*)&unicornColorTest, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_DisplayRGB, NULL);
        ImGui::End();


        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        processInput(window, models);
        playerJump(currentFrame, models);

        // Rendering
        ImGui::Render();
        glClearColor(imguiMainBackgroundColor.x, imguiMainBackgroundColor.y, imguiMainBackgroundColor.z, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteBuffers(VBOs, VBO);
    glDeleteVertexArrays(VAOs, VAO);

    glfwTerminate();
    return 0;
}


void processInput(GLFWwindow *window, auto &models){
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    float cameraSpeed = 3.0f * deltaTime;
    if (glfwGetKey(window, (GLFW_KEY_RIGHT_SHIFT)) || glfwGetKey(window, (GLFW_KEY_LEFT_SHIFT)) == GLFW_PRESS){
        cameraSpeed *= 3.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        auto offset = cameraSpeed * cameraFront;
        cameraPos += offset;
        models.at("unicorn/unicorn.obj").position += offset;
        models.at("unicorn/unicorn.obj").rotationDegrees = 180.0f;
        fieldOfView = 45;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        auto offset = cameraSpeed * cameraFront;
        cameraPos -= offset;
        models.at("unicorn/unicorn.obj").position -= offset;
        models.at("unicorn/unicorn.obj").rotationDegrees = 360.0f;
        fieldOfView = 60;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        auto offset = glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        cameraPos -= offset;
        models.at("unicorn/unicorn.obj").position -= offset;
        models.at("unicorn/unicorn.obj").rotationDegrees = -90.0f;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
            models.at("unicorn/unicorn.obj").rotationDegrees -= 45.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
            models.at("unicorn/unicorn.obj").rotationDegrees += 45.0f;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        auto offset = glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        cameraPos += offset;
        models.at("unicorn/unicorn.obj").position += offset;
        models.at("unicorn/unicorn.obj").rotationDegrees = 90.0f;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
            models.at("unicorn/unicorn.obj").rotationDegrees += 45.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
            models.at("unicorn/unicorn.obj").rotationDegrees -= 45.0f;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (!jump) {
            jumpStart = glfwGetTime();
            jump = true;
        }
    }
//    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
//        testAngle += 0.1f;
//        spdlog::info("testAngle: {}", testAngle);
//    }
//    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
//        testAngle -= 0.1f;
//        spdlog::info("testAngle: {}", testAngle);
//    }

    if (cameraPos.x > planeMax){
        cameraPos.x = planeMax;
    }
    if (cameraPos.x < -planeMax){
        cameraPos.x = -planeMax;
    }
    if (cameraPos.z > planeMax + 8.0f){
        cameraPos.z = planeMax + 8.0f;
    }
    if (cameraPos.z < -planeMax + 8.0f){
        cameraPos.z = -planeMax + 8.0f;
    }

    if (models.at("unicorn/unicorn.obj").position.x > planeMax){
        models.at("unicorn/unicorn.obj").position.x = planeMax;
    }
    if (models.at("unicorn/unicorn.obj").position.x < -planeMax){
        models.at("unicorn/unicorn.obj").position.x = -planeMax;
    }
    if (models.at("unicorn/unicorn.obj").position.z > planeMax){
        models.at("unicorn/unicorn.obj").position.z = planeMax;
    }
    if (models.at("unicorn/unicorn.obj").position.z < -planeMax){
        models.at("unicorn/unicorn.obj").position.z = -planeMax;
    }

    if (cameraPos.y > floorMin + 2.0f || cameraPos.y < floorMin + 2.0f){
        cameraPos.y = floorMin + 2.0f;
    }
    if (models.at("unicorn/unicorn.obj").position.y > floorMin || models.at("unicorn/unicorn.obj").position.y < floorMin){
        models.at("unicorn/unicorn.obj").position.y = floorMin;
    }

}

void framebufferSizeCallback(GLFWwindow* window, int width, int height){
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos){
    if (firstMouse){
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

//    yaw += xoffset;
//    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset){
    fieldOfView -= (float)yoffset;
    if (fieldOfView < 1.0f)
        fieldOfView = 1.0f;
    if (fieldOfView > 90.0f)
        fieldOfView = 90.0f;
}