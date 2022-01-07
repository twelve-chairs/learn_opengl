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
#include "Shader.h"
#include "Model.h"
#include <iostream>

void framebufferSizeCallback(GLFWwindow* window, int width, int height = false);
void processInput(GLFWwindow *window);
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
glm::vec3 lightPos(-1.0f, 3.0f, 2.5f);

// Timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
bool jump = false;
float jumpStart = 0.0f;

const float planeMax = 20.0f;
const float floorMin = 0.0f;

bool wireframe = false;

Model unicorn;
Model wabbit;

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

void playerJump(float& currentFrame){
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
            unicorn.position.y = y;
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




int main(){
    auto temp = lineArray(3.0, 3.0, -11.0, 5.0, 0.25);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "TwelveChairs", NULL, NULL);
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

    // Build and compile our shader program
    Shader defaultShader("../src/default.vert", "../src/default.frag");
    Shader lightingShader("../src/light.vert", "../src/light.frag");
    Shader grassShaderInstanced("../src/grass.vert", "../src/grass.frag");

    unicorn.getModel("../src/include/assets/unicorn/unicorn.obj");
    wabbit.getModel("../src/include/assets/wabbit/wabbit.obj");
    Model grass("../src/include/assets/grass/trava.obj");
    Model plane("../src/include/assets/primitives/plane.obj");
    Model cube("../src/include/assets/primitives/cube.obj");
    Model sphere("../src/include/assets/primitives/sphere.obj");


    // Static world space positions of our cubes and pyramids
    int cubeCount = 10;
    std::vector<glm::vec3> cubePositions;
    for (unsigned int n = 0; n < cubeCount; n++){
        cubePositions.emplace_back(
                glm::linearRand(-planeMax, planeMax),
                glm::linearRand(1.0f, 3.0f),
                glm::linearRand(-planeMax, planeMax)
        );
    }

    int pyramidCount = 8;
    glm::vec3 pyramidPositions[] = {
            glm::vec3(-planeMax, 0.5f, -planeMax),
            glm::vec3(planeMax, 0.5f, -planeMax),
            glm::vec3(0.0f, 0.5f, -planeMax),
            glm::vec3(-planeMax, 0.5f, 0.0f),
            glm::vec3(0.0f, 0.5f, planeMax),
            glm::vec3(planeMax, 0.5f, 0.0f),
            glm::vec3(-planeMax, 0.5f, planeMax),
            glm::vec3(planeMax, 0.5f, planeMax)
    };

    unsigned int grassCount = 1500;
    glm::mat4 *modelMatrices;
    modelMatrices = new glm::mat4[grassCount];
    for (unsigned int i = 0; i < grassCount; i++){
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(
                glm::linearRand(-planeMax, planeMax),
                glm::linearRand(-0.3f, -0.1f),
                glm::linearRand(-planeMax, planeMax)
        ));
        model = glm::rotate(model, glm::radians(unicorn.rotationDegrees), unicorn.rotationAxis);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));

        modelMatrices[i] = model;
    }

    // load models
    stbi_set_flip_vertically_on_load(false);
    unicorn.scale = glm::vec3(0.35f, 0.35f, 0.35f);
    unicorn.rotationDegrees = 180.0f;

    // Load and create a texture
    unsigned int texture_container, texture_face, texture_grass, texture_mystery, texture_bricks, texture_rock, texture_wood, texture_sky, texture_skybox, texture_clouds, texture_skydome, texture_fur;
    createTexture(texture_face, "../src/include/assets/textures/awesomeface.png", true);
    createTexture(texture_clouds, "../src/include/assets/textures/clouds.jpeg", false);
    createTexture(texture_container, "../src/include/assets/textures/container.jpeg", false);
    createTexture(texture_fur, "../src/include/assets/textures/fur.jpeg", false);
    createTexture(texture_grass, "../src/include/assets/textures/grass.png", true);
    createTexture(texture_mystery, "../src/include/assets/textures/mario_mystery.png", true);
    createTexture(texture_bricks, "../src/include/assets/textures/mario_bricks.png", false);
    createTexture(texture_rock, "../src/include/assets/textures/rock.jpeg", false);
    createTexture(texture_skydome, "../src/include/assets/textures/skydome.jpeg", false);
    createTexture(texture_wood, "../src/include/assets/textures/wood.jpg", false);
    createTexture(texture_sky, "../src/include/assets/textures/space.png", true);


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

    // shader configuration
    // --------------------
    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);


    static float lightAmbient = 1.0f;
    static float lightDiffuse = 0.15f;
    static float lightSpecular = 0.5f;

    static float materialSpecular = 0.5f;
    static float materialShine = 0.7f;


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
    for (unsigned int i = 0; i < grass.meshes.size(); i++){
        unsigned int vao = grass.meshes[i].VAO;
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


    // Render loop
    while (!glfwWindowShouldClose(window)){
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Wireframe-only
        wireframe ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activate shader when setting uniforms/drawing objects
        defaultShader.use();
        defaultShader.setVec3("light.position", lightPos);
        defaultShader.setVec3("viewPos", cameraPos);

        // Light properties
        defaultShader.setVec3("light.ambient", glm::vec3(lightAmbient));
        defaultShader.setVec3("light.diffuse", glm::vec3(lightDiffuse));
        defaultShader.setVec3("light.specular", glm::vec3(lightSpecular));

        // Material properties
        defaultShader.setVec3("material.specular", glm::vec3(materialSpecular));
        defaultShader.setFloat("material.shininess", materialShine);

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
            grassShaderInstanced.use();
            grassShaderInstanced.setMat4("projection", projection);
            grassShaderInstanced.setMat4("view", view);

            lightingShader.use();
            lightingShader.setMat4("projection", projection);
            lightingShader.setMat4("view", view);

            defaultShader.use();
            defaultShader.setMat4("projection", projection);
            defaultShader.setMat4("view", view);

            // Render the loaded models
            glBindTexture(GL_TEXTURE_2D, texture_grass);
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(planeMax, 10.0f, planeMax));
            defaultShader.setMat4("model", model);
            //defaultShader.setInt("material.diffuse", texture_grass);
            plane.Draw(defaultShader);

            glBindTexture(GL_TEXTURE_2D, texture_mystery);
            for (unsigned int n = 0; n < cubeCount; n++) {
                model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
                float angle = 20.0f * currentFrame * ((float)n + 1);
                model = glm::translate(model, cubePositions[n]);
                model = glm::rotate(model, glm::radians(angle), cubePositions[n]);
                model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
                defaultShader.setMat4("model", model);
//                defaultShader.setInt("material.diffuse", texture_mystery);
                cube.Draw(defaultShader);
            }

            glBindTexture(GL_TEXTURE_2D, texture_skydome);
            model = glm::mat4(1.0f);
            model = glm::rotate(model, currentFrame * 0.01f, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(220.0f, 220.0f, 220.0f));
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
            defaultShader.setMat4("model", model);
            sphere.Draw(defaultShader);



            glBindTexture(GL_TEXTURE_2D, texture_fur);
            defaultShader.use();
            model = glm::mat4(1.0f);
//            model = glm::translate(model, glm::vec3(wabbit.line[(int)currentFrame][0], 0.0f, wabbit.line[(int)currentFrame][1]));
            model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
            defaultShader.setMat4("model", model);
            wabbit.Draw(defaultShader);


            defaultShader.use();
            model = glm::mat4(1.0f);
            model = glm::translate(model, unicorn.position);
            model = glm::rotate(model, glm::radians(unicorn.rotationDegrees), unicorn.rotationAxis);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
            model = glm::scale(model, unicorn.scale);
            defaultShader.setMat4("model", model);
//            defaultShader.setInt("material.diffuse", texture_mystery);
            unicorn.Draw(defaultShader);


            grassShaderInstanced.use();
            grassShaderInstanced.setInt("texture_diffuse1", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, grass.textures_loaded[0].id); // note: we also made the textures_loaded vector public (instead of private) from the model class.
            for (unsigned int i = 0; i < grass.meshes.size(); i++)
            {
                glBindVertexArray(grass.meshes[i].VAO);
                glDrawElementsInstanced(GL_TRIANGLES, grass.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, grassCount);
                glBindVertexArray(0);
            }


//             Draw the lamp object
            glActiveTexture(GL_TEXTURE0);
            lightingShader.use();
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(lightPos.x, lightPos.y, lightPos.z));
            lightingShader.setMat4("model", model);


            // ============================================================================================================================
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            ImGui::Image(reinterpret_cast<ImTextureID>(textureColorBuffer), frameBufferSize, ImVec2(0, 1), ImVec2(1, 0));
        }
        catch (std::exception &e){
            spdlog::error("ImGui::Image: {}", e.what());
        }
        ImGui::EndChild();
        ImGui::End();

        // Stats window
        ImGui::SetNextWindowPos(ImVec2(0, 20), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(250, 70), ImGuiCond_Always);
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
        ImGui::SetNextWindowSize(ImVec2(250, 250), ImGuiCond_Always);
        ImGui::Begin("testLighting", NULL, flags);
        ImGui::SliderFloat("lightAmbient", &lightAmbient, 0.0, 1.0);
        ImGui::SliderFloat("lightDiffuse", &lightDiffuse, 0.0, 1.0);
        ImGui::SliderFloat("lightSpecular", &lightSpecular, 0.0, 1.0);
        ImGui::SliderFloat("materialSpecular", &materialSpecular, 0.0, 1.0);
        ImGui::SliderFloat("materialShine", &materialShine, 0.0, 100.0);
        ImGui::SliderFloat("light.x", &lightPos.x, -planeMax - 10, planeMax + 10);
        ImGui::SliderFloat("light.y", &lightPos.y, -1, 300);
        ImGui::SliderFloat("light.z", &lightPos.z, -planeMax - 10, planeMax + 10);
        ImGui::Checkbox("Wireframe", &wireframe);
        ImGui::End();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        processInput(window);

        // Handle jumps
        playerJump(currentFrame);

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


void processInput(GLFWwindow *window){
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
        unicorn.position += offset;
        unicorn.rotationDegrees = 180.0f;
        fieldOfView = 45;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        auto offset = cameraSpeed * cameraFront;
        cameraPos -= offset;
        unicorn.position -= offset;
        unicorn.rotationDegrees = 360.0f;
        fieldOfView = 55;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        auto offset = glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        cameraPos -= offset;
        unicorn.position -= offset;
        unicorn.rotationDegrees = -90.0f;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
            unicorn.rotationDegrees -= 45.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
            unicorn.rotationDegrees += 45.0f;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        auto offset = glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        cameraPos += offset;
        unicorn.position += offset;
        unicorn.rotationDegrees = 90.0f;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
            unicorn.rotationDegrees += 45.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
            unicorn.rotationDegrees -= 45.0f;
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
    if (cameraPos.z > planeMax + (1 / unicorn.scale.x) * 2.0f){
        cameraPos.z = planeMax + (1 / unicorn.scale.x) * 2.0f;
    }
    if (cameraPos.z < -planeMax + (1 / unicorn.scale.x) * 2.0f){
        cameraPos.z = -planeMax + (1 / unicorn.scale.x) * 2.0f;
    }

    if (unicorn.position.x > planeMax){
        unicorn.position.x = planeMax;
    }
    if (unicorn.position.x < -planeMax){
        unicorn.position.x = -planeMax;
    }
    if (unicorn.position.z > planeMax){
        unicorn.position.z = planeMax;
    }
    if (unicorn.position.z < -planeMax){
        unicorn.position.z = -planeMax;
    }

    if (cameraPos.y > floorMin + 2.0f || cameraPos.y < floorMin + 2.0f){
        cameraPos.y = floorMin + 2.0f;
    }
    if (unicorn.position.y > floorMin || unicorn.position.y < floorMin){
        unicorn.position.y = floorMin;
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