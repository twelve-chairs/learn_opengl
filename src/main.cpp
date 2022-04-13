#define STB_IMAGE_IMPLEMENTATION

#if defined(IMGUI_IMPL_OPENGL_ES2)
#endif

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#include <vector>
#include <map>

#include <GL/glew.h>
#include "imgui.h"
#include "include/imgui/imgui_impl_glfw.h"
#include "include/imgui/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <spdlog/spdlog.h>

#include "include/helpers/Camera.h"
#include "include/helpers/Light.h"
#include "include/helpers/Animator.h"
#include "include/helpers/Platform.h"

#include "renderers.cpp"
#include "helpers.cpp"


// Window settings
const unsigned int SCR_WIDTH = 1100;
const unsigned int SCR_HEIGHT = 800;

// Buffers
const unsigned int VAOs = 1;
const unsigned int VBOs = 5;

// Scene settings
const int cubeCount = 10;
const int barrierCount = 35;

std::vector<glm::vec3> cubePositions;
std::vector<glm::vec4> barrierPositions;

// Plane
const auto plane = Platform(randomFloat(30.0f, 50.0f), randomFloat(50.0f, 100.0f), 0.0f);

// Camera
auto camera = Camera();
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Lighting
auto light = Light();

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool jump = false;
float jumpStart = 0.0f;

// Testing
int testInt = 0;
float testFloat = 0.0f;
auto unicornColorTest = glm::vec3(0.163, 0.540, 0.571);
auto unicornManeColorTest = glm::vec3(0.765, 0.849, 0.086);
auto unicornTailColorTest = glm::vec3(0.765, 0.450, 0.450);

void setupHorizontalBarriers(){
    float corridorSize = 0.3f;
    float maxBarrierWidth = 1.0f;
    float minBarrierWidth = 0.2f;
    float maxBarrierLength = 0.4f;
    float maxBarrierHeight = 0.4f;

}

void playerJump(const float &currentFrame, auto &models){
    float floorOffset = 1.0f;
    float amplitude = 0.75f;
    float speed = 6.0f;
    float x = currentFrame - jumpStart;
    float playerView = plane.floorMin + floorOffset;

    if (x < 0.0) x = 0.0;

    if (jump){
        float y = amplitude * (glm::sin(speed * x)) + 1.0f;
        if (y >= playerView) {
            models.at("unicorn").position.y = y;
            models.at("unicornMane").position.y = y;
            models.at("unicornTail").position.y = y;
        }
        else {
            jump = false;
        }
    }
}

void processInput(GLFWwindow *window, auto &models){
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    float cameraSpeed = 8.0f * deltaTime;
    float offsetRate = 0.5f;

    if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) || glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
        cameraSpeed *= 3.0f;
        models.at("unicorn").position.y = plane.floorMin + 0.5f;
        models.at("unicornMane").position.y = plane.floorMin + 0.5f;
        models.at("unicornTail").position.y = plane.floorMin + 0.5f;
    }
    if ((glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) ||
        (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) ||
        (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) ||
        (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)) {
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            auto offset = camera.speed * camera.front;
            models.at("unicorn").position.z -= offsetRate;
            models.at("unicornMane").position.z -= offsetRate;
            models.at("unicornTail").position.z -= offsetRate;

            models.at("unicorn").rotationDegrees = 180.0f;
            models.at("unicornMane").rotationDegrees = 180.0f;
            models.at("unicornTail").rotationDegrees = 180.0f;
        }
        else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            auto offset = camera.speed * camera.front;
            models.at("unicorn").position.z += offsetRate;
            models.at("unicornMane").position.z += offsetRate;
            models.at("unicornTail").position.z += offsetRate;

            models.at("unicorn").rotationDegrees = 360.0f;
            models.at("unicornMane").rotationDegrees = 360.0f;
            models.at("unicornTail").rotationDegrees = 360.0f;
        }
        else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            auto offset = glm::normalize(glm::cross(camera.front, camera.up)) * camera.speed;

            models.at("unicorn").position.x -= offsetRate;
            models.at("unicornMane").position.x -= offsetRate;
            models.at("unicornTail").position.x -= offsetRate;

            models.at("unicorn").rotationDegrees = -90.0f;
            models.at("unicornMane").rotationDegrees = -90.0f;
            models.at("unicornTail").rotationDegrees = -90.0f;
//            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
//                models.at("unicorn").rotationDegrees -= 45.0f;
//                models.at("unicornMane").rotationDegrees -= 45.0f;
//                models.at("unicornTail").rotationDegrees -= 45.0f;
//            }
//            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
//                models.at("unicorn").rotationDegrees += 45.0f;
//                models.at("unicornMane").rotationDegrees += 45.0f;
//                models.at("unicornTail").rotationDegrees += 45.0f;
//            }
        }
        else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            auto offset = glm::normalize(glm::cross(camera.front, camera.up)) * camera.speed;

            models.at("unicorn").position.x += offsetRate;
            models.at("unicornMane").position.x += offsetRate;
            models.at("unicornTail").position.x += offsetRate;

            models.at("unicorn").rotationDegrees = 90.0f;
            models.at("unicornMane").rotationDegrees = 90.0f;
            models.at("unicornTail").rotationDegrees = 90.0f;
//            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
//                models.at("unicorn").rotationDegrees += 45.0f;
//                models.at("unicornMane").rotationDegrees += 45.0f;
//                models.at("unicornTail").rotationDegrees += 45.0f;
//            }
//            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
//                models.at("unicorn").rotationDegrees -= 45.0f;
//                models.at("unicornMane").rotationDegrees -= 45.0f;
//                models.at("unicornTail").rotationDegrees -= 45.0f;
//            }
        }
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !jump) {
        jumpStart = static_cast<float>(glfwGetTime());
        jump = true;
    }

    if (camera.position.x > plane.planeMaxWidth){
        camera.position.x = plane.planeMaxWidth;
    }
    if (camera.position.x < -plane.planeMaxWidth){
        camera.position.x = -plane.planeMaxWidth;
    }

    if (models.at("unicorn").position.x > plane.planeMaxWidth){
        models.at("unicorn").position.x = plane.planeMaxWidth;
        models.at("unicornMane").position.x = plane.planeMaxWidth;
        models.at("unicornTail").position.x = plane.planeMaxWidth;
    }
    if (models.at("unicorn").position.x < -plane.planeMaxWidth){
        models.at("unicorn").position.x = -plane.planeMaxWidth;
        models.at("unicornMane").position.x = -plane.planeMaxWidth;
        models.at("unicornTail").position.x = -plane.planeMaxWidth;
    }
    if (models.at("unicorn").position.z > plane.planeMaxHeight){
        models.at("unicorn").position.z = plane.planeMaxHeight;
        models.at("unicornMane").position.z = plane.planeMaxHeight;
        models.at("unicornTail").position.z = plane.planeMaxHeight;
    }
    if (models.at("unicorn").position.z < -plane.planeMaxHeight){
        models.at("unicorn").position.z = -plane.planeMaxHeight;
        models.at("unicornMane").position.z = -plane.planeMaxHeight;
        models.at("unicornTail").position.z = -plane.planeMaxHeight;
    }

    if (models.at("unicorn").position.y > plane.floorMin || models.at("unicorn").position.y < plane.floorMin){
        models.at("unicorn").position.y = plane.floorMin;
        models.at("unicornMane").position.y = plane.floorMin;
        models.at("unicornTail").position.y = plane.floorMin;
    }

    camera.position.x = models.at("unicorn").position.x + 7.5f;
    camera.position.z = models.at("unicorn").position.z + 10.0f;

    light.position.x = models.at("unicorn").position.x - 0.5f;
    light.position.z = models.at("unicorn").position.z - 2.5f;
}

void framebufferSizeCallback([[maybe_unused]] GLFWwindow *window, [[maybe_unused]] int width, [[maybe_unused]] int height){
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
}

void mouseCallback([[maybe_unused]] GLFWwindow *window, double xposIn, double yposIn){
    auto xpos = static_cast<float>(xposIn);
    auto ypos = static_cast<float>(yposIn);
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scrollCallback([[maybe_unused]] GLFWwindow *window, [[maybe_unused]] double xoffset, double yoffset){
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void renderDepth(auto &shaders, auto &models, auto &textures, auto &currentFrame, auto &depthMap, [[maybe_unused]] auto &grassCount, auto &animator){
    // Pass projection matrix to shader (FOV, aspect, near, far)
    glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
    // Camera/view transformation
    glm::mat4 view = glm::lookAt(camera.position, camera.position + camera.front, camera.up);

    // view/projection transformations
    shaders.at("shadowMappingDepth").use();
    shaders.at("shadowMappingDepth").setMat4("projection", projection);
    shaders.at("shadowMappingDepth").setMat4("view", view);

    // Render the loaded models
    renderPlane(shaders.at("shadowMappingDepth"), models.at("plane"), textures.at("grass.png"), depthMap, plane.planeMaxWidth, plane.planeMaxHeight);
    renderBarriers(shaders.at("shadowMappingDepth"), models.at("cube"), barrierCount, barrierPositions, textures.at("mario_mystery.png"), currentFrame);
    renderMysteryCubes(shaders.at("shadowMappingDepth"), models.at("cube"), cubeCount, cubePositions, textures.at("mario_mystery.png"), currentFrame);
    renderSkyDome(shaders.at("shadowMappingDepth"), models.at("sphere"), textures.at("skydome.jpeg"), currentFrame, plane.planeMaxHeight);
    renderWabbit(shaders.at("shadowMappingDepth"), models.at("wabbit"), textures, currentFrame, plane.planeMaxWidth);
    renderFrog(shaders.at("shadowMappingDepth"), models.at("frog"), textures, currentFrame, plane.planeMaxWidth);
    renderUnicorn(shaders.at("shadowMappingDepth"), models.at("unicorn"), textures, animator, light, unicornColorTest);
    renderUnicornMane(shaders.at("shadowMappingDepth"), models.at("unicornMane"), textures, light, unicornManeColorTest);
    renderUnicornTail(shaders.at("shadowMappingDepth"), models.at("unicornTail"), textures, light, unicornTailColorTest);
//    renderGrass(shaders.at("shadowMappingDepth"), models.at("grass"), grassCount, light);
    glActiveTexture(GL_TEXTURE0);
}

void render(auto &shaders, auto &models, auto &textures, auto &currentFrame, auto &depthMap, [[maybe_unused]] auto &grassCount, auto &animator){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    // Pass projection matrix to shader (FOV, aspect, near, far)
    glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
    // Camera/view transformation
    glm::mat4 view = glm::lookAt(camera.position, camera.position + camera.front, camera.up);

    // view/projection transformations
    for (const auto &each : shaders){
        shaders.at(each.first).use();
        shaders.at(each.first).setMat4("projection", projection);
        shaders.at(each.first).setMat4("view", view);
    }

    // Render the loaded models
    renderPlane(shaders.at("default"), models.at("plane"), textures.at("grass.png"), depthMap, plane.planeMaxWidth, plane.planeMaxHeight);
    renderBarriers(shaders.at("default"), models.at("cube"), barrierCount, barrierPositions, textures.at("wood.jpg"), currentFrame);
    renderMysteryCubes(shaders.at("default"), models.at("cube"), cubeCount, cubePositions, textures.at("mario_mystery.png"), currentFrame);
    renderSkyDome(shaders.at("default"), models.at("sphere"), textures.at("skydome.jpeg"), currentFrame, plane.planeMaxHeight);
    renderFrog(shaders.at("frog"), models.at("frog"), textures, currentFrame, plane.planeMaxWidth);
    renderWabbit(shaders.at("default"), models.at("wabbit"), textures, currentFrame, plane.planeMaxWidth);
    renderUnicorn(shaders.at("unicornBody"), models.at("unicorn"), textures, animator, light, unicornColorTest);
    renderUnicornMane(shaders.at("unicornMane"), models.at("unicornMane"), textures, light, unicornManeColorTest);
    renderUnicornTail(shaders.at("unicornTail"), models.at("unicornTail"), textures, light, unicornTailColorTest);
//    renderGrass(shaders.at("default"), models.at("grass"), grassCount, light);
    glActiveTexture(GL_TEXTURE0);
}

int main(){
    GLuint VAO[VAOs];
    GLuint VBO[VBOs];
    GLuint frameBufferObject;
    GLuint renderBufferObject;
    unsigned int textureColorBuffer;

    // OpenGL window default size and position
    auto glWindowSize = ImVec2(static_cast<float>(SCR_WIDTH), static_cast<float>(SCR_HEIGHT));
    ImVec2 frameBufferSize = glWindowSize;

    bool wireframe = false;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(static_cast<int>(SCR_WIDTH), static_cast<int>(SCR_HEIGHT), "twelvechairs", nullptr, nullptr);
    if (window == nullptr){
        spdlog::error("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // Anti-aliasing
//    glfwWindowHint(GLFW_SAMPLES, 4);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // Load GLEW so it configures OpenGL
    if (glewInit() != GLEW_OK){
        spdlog::error("glewInit");
        return 0;
    }

    auto shaders = loadShaders();
    auto textures = loadTextures();
    auto models = loadModels();

    initModels(models, plane.planeMaxWidth, plane.planeMaxHeight);
    initCamera(camera, models);

    std::string path = "../src/include/assets/unicorn/unicorn.glb";
    Animation danceAnimation(path, &models.at("unicorn"));
    Animator animator(&danceAnimation);

    // Static world space positions of our cubes
    for (auto n = 0; n < cubeCount; n++){
        cubePositions.emplace_back(
                randomFloat(-plane.planeMaxWidth, plane.planeMaxWidth),
                randomFloat(2.5f, 6.0f),
                randomFloat(-plane.planeMaxHeight, plane.planeMaxHeight)
        );
    }

    // Static world space positions of our barriers
    for (auto n = 0; n < barrierCount; n++){
        barrierPositions.emplace_back(
                randomFloat(-plane.planeMaxWidth, plane.planeMaxWidth),
                0.6f,
                randomFloat(-plane.planeMaxHeight, plane.planeMaxHeight),
                6.0f
        );
    }

    // Generate grass objects for GPU instancing
    const uint8_t grassCount = 250;
    auto grassPositions = std::vector(grassCount, glm::mat4(0.0f));
    for (unsigned int i = 0; i < grassCount; i++){
        auto model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(randomFloat(1.0f, 270.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, glm::vec3(randomFloat(-plane.planeMaxWidth, plane.planeMaxWidth),
                                                0.0f,
                                                randomFloat(-plane.planeMaxHeight, plane.planeMaxHeight))
        );
        grassPositions.at(i) = model;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");

    // Our state
    auto imguiMainBackgroundColor = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);

    initFrameBuffer(frameBufferObject, frameBufferSize, textureColorBuffer, renderBufferObject);

    auto depthResult = generateDepthMap();
    const unsigned int depthMapFBO = depthResult[0];
    const unsigned int depthMap = depthResult[1];
    const unsigned int shadowWidth = depthResult[2];
    const unsigned int shadowHeight = depthResult[3];

    // configure instanced array
    // -------------------------
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, static_cast<long>(sizeof(glm::mat4)) * grassCount, &grassPositions[0], GL_STATIC_DRAW);

    // set transformation matrices as an instance vertex attribute (with divisor 1)
    // -----------------------------------------------------------------------------------------------------------------------------------
    for (unsigned int i = 0; i < models.at("grass").meshes.size(); i++)
    {
        unsigned int tempVAO = models.at("grass").meshes[i].VAO;
        glBindVertexArray(tempVAO);
        // set attribute pointers for matrix (4 times vec4)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)nullptr);
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


    // Render loop
    while (!glfwWindowShouldClose(window)){
        auto currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        animator.UpdateAnimation(deltaTime);

        // Wireframe-only
        wireframe ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 1. Render depth of scene to texture (from light's perspective)
        glm::mat4 lightProjection;
        glm::mat4 lightView;
        glm::mat4 lightSpaceMatrix;
        float nearPlane = -1.5;
        float farPlane = plane.planeMaxHeight;
        lightProjection = glm::ortho(-plane.planeMaxWidth, plane.planeMaxWidth,
                                     -plane.planeMaxHeight, plane.planeMaxHeight,
                                     nearPlane, farPlane);
        // eye, center, up
        lightView = glm::lookAt(light.position, models.at("unicorn").position, glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;
        // render scene from light's point of view
        shaders.at("shadowMappingDepth").use();
        shaders.at("shadowMappingDepth").setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, static_cast<int>(shadowWidth), static_cast<int>(shadowHeight));
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glCullFace(GL_FRONT);
        renderDepth(shaders, models, textures, currentFrame, depthMap, grassCount, animator);
        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Reset viewport
        glViewport(0, 0, static_cast<int>(SCR_WIDTH), static_cast<int>(SCR_HEIGHT));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activate shader when setting uniforms/drawing objects
        for (const auto &[key, value]: shaders){
            shaders.at(key).use();
            shaders.at(key).setVec3("lightPos", light.position);
            shaders.at(key).setVec3("viewPos", camera.position);
            shaders.at(key).setMat4("lightSpaceMatrix", lightSpaceMatrix);
            shaders.at(key).setInt("diffuseTexture", 0);
            shaders.at(key).setInt("shadowMap", 1);
            shaders.at(key).setFloat("amb", 0.8f);
            shaders.at(key).setFloat("dif", 0.3f);
            shaders.at(key).setFloat("spc", 0.0f);
        }

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
                if (ImGui::MenuItem("Show / Hide Stats")){
                    //Do something
                }
                if (ImGui::MenuItem("Show / Hide Colors")){
                    //Do something
                }
                if (ImGui::MenuItem("Show / Hide Camera")){
                    //Do something
                }
                if (ImGui::MenuItem("Show / Hide Depth Map")){
                    //Do something
                }
                if (ImGui::MenuItem("Something")){
                    //Do something
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // OpenGL window
        static bool use_work_area = false;
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs;
        ImGuiViewport const* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
        ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

        ImGui::SetNextWindowPos(ImVec2(0, 40), ImGuiCond_Always);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("OpenGL", nullptr, flags);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::BeginChild("Render");
        ImGui::PopStyleVar();
        glWindowSize = ImGui::GetContentRegionAvail();
        frameBufferSize = glWindowSize;

        // Specify the viewport of OpenGL in the Window
        glViewport(0, 0, static_cast<int>(glWindowSize.x), static_cast<int>(glWindowSize.y));

        try {
            glEnable(GL_FRAMEBUFFER_SRGB);
            glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(frameBufferSize.x), static_cast<int>(frameBufferSize.y), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObject);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, static_cast<int>(frameBufferSize.x), static_cast<int>(frameBufferSize.y));
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBufferObject);
            glClear(GL_FRAMEBUFFER);
            glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
            // ============================================================================================================================

            render(shaders, models, textures, currentFrame, depthMap, grassCount, animator);

            // ============================================================================================================================
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            ImGui::Image(reinterpret_cast<ImTextureID>(textureColorBuffer), frameBufferSize, ImVec2(0, 1), ImVec2(1, 0));
            glDisable(GL_FRAMEBUFFER_SRGB);

        }
        catch (const std::exception &e){
            spdlog::error("ImGui::Image: {}", e.what());
        }
        ImGui::EndChild();
        ImGui::End();

        // Depth Map window
        ImGui::SetNextWindowPos(ImVec2(700, 20), ImGuiCond_Once);
        flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
        static bool showDepthMap = true;
        ImGui::Begin("Depth Map", &showDepthMap);
        ImGui::Image(reinterpret_cast<ImTextureID>(depthMap), frameBufferSize, ImVec2(0, 1), ImVec2(1, 0));
        ImGui::End();

        // Stats window
        ImGui::SetNextWindowPos(ImVec2(0, 20), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(220, 50), ImGuiCond_Once);
        flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
        ImGui::Begin("Stats", &showDepthMap, flags);
        ImGui::Text(" %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        // Stores the coordinates of the cursor
        double mouseX;
        double mouseY;
        // Fetches the coordinates of the cursor
        glfwGetCursorPos(window, &mouseX, &mouseY);
        ImGui::Text(" %.1f x %.1f", mouseX, mouseY);
        ImGui::End();

        // testLighting window
        ImGui::SetNextWindowPos( ImVec2(470, 20), ImGuiCond_Once);
        flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
        ImGui::Begin("Camera / Lights", nullptr);

        ImGui::SliderFloat("light.x", &light.position.x, -plane.planeMaxWidth - 10, plane.planeMaxWidth + 10);
        ImGui::SliderFloat("light.y", &light.position.y, -1, 300);
        ImGui::SliderFloat("light.z", &light.position.z, -plane.planeMaxHeight - 10, plane.planeMaxHeight + 10);

        ImGui::SliderFloat("zoom", &camera.zoom, -10, 200);

        ImGui::SliderFloat("camera.x", &camera.position.x, -plane.planeMaxWidth, plane.planeMaxWidth);
        ImGui::SliderFloat("camera.y", &camera.position.y, -1, 300);
        ImGui::SliderFloat("camera.z", &camera.position.z, -plane.planeMaxHeight, plane.planeMaxHeight);
        ImGui::SliderFloat("camera.p", &camera.pitch, -360.0f, 360.0f);
        ImGui::SliderFloat("camera.yaw", &camera.yaw, -360.0f, 360.0f);

        ImGui::Checkbox("Wireframe", &wireframe);
        ImGui::End();

        ImGui::SetNextWindowPos( ImVec2(230, 20), ImGuiCond_Once);
        ImGui::Begin("Unicorn", nullptr);
        ImGui::ColorPicker4("Body", (float*)&unicornColorTest, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_DisplayRGB, nullptr);
        ImGui::ColorPicker4("Mane", (float*)&unicornManeColorTest, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_DisplayRGB, nullptr);
        ImGui::ColorPicker4("Tail", (float*)&unicornTailColorTest, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_DisplayRGB, nullptr);

        ImGui::End();

        // ImGui::ShowDemoWindow(&show_demo_window);

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

