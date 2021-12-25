#define STB_IMAGE_IMPLEMENTATION

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#include <spdlog/spdlog.h>
#include <filesystem>
#include <chrono>
#include <thread>
#include "shader_s.h"
#include "primitives.h"
#include "Model.h"
#include <iostream>

void framebufferSizeCallback(GLFWwindow* window, int width, int height = false);
void processInput(GLFWwindow *window);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// settings
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 1000;

const int VAOs = 1;
const int VBOs = 5;

GLuint VAO[VAOs];
GLuint VBO[VBOs];

// camera
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

// camera
float lastX = SCR_WIDTH / 2.0f  ;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float yaw   = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch =  0.0f;
float fov   =  45.0f;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
bool jump = false;
float jumpStart = 0.0f;

float testAngle = 0.0f;

void createTexture(GLuint& texture, const std::string& path, bool alpha){
    // texture 1
    // ---------
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
    float floorOffset = 1.5f;
    float amplitude = 3.0f;
    float speed = 5.0f;
    float x = currentFrame - jumpStart;
    float playerView = floorMin + floorOffset;

    if (x < 0.0) x = 0.0;

    if (jump){
        float y = amplitude * (glm::sin(speed * x)) + floorOffset;
        (y >= playerView) ? cameraPos.y = y : jump = false;
    }
}

int main(){
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Anti-aliasing
    glfwWindowHint(GLFW_SAMPLES, 4);
    glEnable(GL_MULTISAMPLE);

    // Load GLEW so it configures OpenGL
    if (glewInit() != GLEW_OK){
        spdlog::error("glewInit");
        return 0;
    }

    // build and compile our shader zprogram
    // ------------------------------------
    Shader defaultShader("../src/include/default.vert", "../src/include/default.frag");

    // static world space positions of our cubes and pyramids
    int cubeCount = 10;
    glm::vec3 cubePositions[cubeCount];
    for (int n = 0; n < cubeCount; n++) {
        cubePositions[n] = glm::vec3(
                glm::linearRand(-planeMax, planeMax),
                glm::linearRand(floorMin + 0.75f, 5.0f),
                glm::linearRand(-planeMax, planeMax)
        );
    }

    int grassCount = 200;
    glm::vec3 grassPositions[grassCount];
    for (int n = 0; n < grassCount; n++) {
        grassPositions[n] = glm::vec3(
                glm::linearRand(-planeMax, planeMax),
                glm::linearRand(floorMin, floorMin + 0.25f),
                glm::linearRand(-planeMax, planeMax)
        );
    }

    int pyramidCount = 8;
    glm::vec3 pyramidPositions[] = {
            glm::vec3(
                -planeMax,
                0.5f,
                -planeMax),
            glm::vec3(
                    planeMax,
                    0.5f,
                    -planeMax),
            glm::vec3(
                    0.0f,
                    0.5f,
                    -planeMax),
            glm::vec3(
                    -planeMax,
                    0.5f,
                    0.0f),
            glm::vec3(
                    0.0f,
                    0.5f,
                    planeMax),
            glm::vec3(
                    planeMax,
                    0.5f,
                    0.0f),
            glm::vec3(
                    -planeMax,
                    0.5f,
                    planeMax),
            glm::vec3(
                    planeMax,
                    0.5f,
                    planeMax)
    };

//    glGenVertexArrays(VAOs, VAO);
//    glGenBuffers(VBOs, VBO);
//
//    glBindVertexArray(VAO[0]);
//
//    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
//
//    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
//
//    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);
//
//    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(woodVertices), woodVertices, GL_STATIC_DRAW);


    // load models
    // -----------
    stbi_set_flip_vertically_on_load(true);
    Model guitar("../src/include/backpack/backpack.obj");
    Model plane("../src/include/plane.obj");
    Model cube("../src/include/cube.obj");
    Model grass("../src/include/91-trava-kolosok/trava.obj");

    // Build collection
    std::vector<Model> grassObjects(200, guitar);

    // load and create a texture
    // -------------------------
    unsigned int texture_container, texture_face, texture_grass, texture_mystery, texture_bricks, texture_rock, texture_wood, texture_sky;
    createTexture(texture_container, "../src/include/container.jpeg", false);
    createTexture(texture_face, "../src/include/awesomeface.png", true);
    createTexture(texture_grass, "../src/include/grass.png", true);
    createTexture(texture_mystery, "../src/include/mario_mystery.png", true);
    createTexture(texture_bricks, "../src/include/mario_bricks.png", false);
    createTexture(texture_rock, "../src/include/rock.jpg", false);
    createTexture(texture_wood, "../src/include/wood.jpg", false);
    createTexture(texture_sky, "../src/include/space.png", true);

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------
    defaultShader.use();
    defaultShader.setInt("texture_container", 0);
    defaultShader.setInt("texture_face", 1);
    defaultShader.setInt("texture_grass", 2);
    defaultShader.setInt("texture_mystery", 3);
    defaultShader.setInt("texture_bricks", 4);
    defaultShader.setInt("texture_mario_sand", 5);
    defaultShader.setInt("texture_rock", 6);
    defaultShader.setInt("texture_wood", 7);
    defaultShader.setInt("texture_sky", 8);

    glEnable(GL_DEPTH_TEST);

//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)){
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // range 0 to PI, normalize, iterate
        processInput(window);

        // Handle jumps
        playerJump(currentFrame);

        // render
        // ------
        glClearColor(0.2f, 0.4f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // activate shader
        defaultShader.use();

        // pass projection matrix to shader (note that in this case it could change every frame)
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        defaultShader.setMat4("projection", projection);

        // camera/view transformation
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        defaultShader.setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first

        // render boxes
        glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // texture coord attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_mystery);
//
        for (unsigned int n = 0; n < cubeCount; n++){
            // calculate the model matrix for each object and pass it to shader before drawing
            model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first

            // Rotate each model at a slight offset
            float angle = 20.0f * currentFrame * ((float)n + 1);
            model = glm::rotate(model, glm::radians(angle), cubePositions[n]);
            model = glm::translate(model, cubePositions[n]);

            defaultShader.setMat4("model", model);
            glDepthFunc(GL_LEQUAL);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // render outside world
//        renderObject(0, texture_sky, model, defaultShader, 36, glm::vec3(0, 0, 0), 90.0);
//
//        // render crosshair
//        float offset = 0.1f;
//        glm::vec3 temp = cameraPos + (cameraFront * offset);
//        // TODO: add 'static' property to orient to camera's view making it look like it is in front of you
//        glm::vec3 position = temp;
//        renderObject(0, texture_container, model, defaultShader, 36, position, 0.0005);
//
//        // render player
//        offset = 0.1f;
//        temp = cameraPos + (cameraFront * offset);
//        model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
//        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
//        model = glm::rotate(model, glm::radians(testAngle), cameraFront);
//        model = glm::translate(model, cameraFront);
//        defaultShader.setMat4("model", model);
//        glDepthFunc(GL_LEQUAL);
//        glDrawArrays(GL_TRIANGLES, 0, 36);
//
        // render plane
//        renderObject(1, texture_grass, model, defaultShader, 6, glm::vec3(0, 0, 0), 1.0f);
//
//        // render pyramidCount
//        for (int n = 0; n < pyramidCount; n++) {
//            renderObject(2, texture_rock, model, defaultShader, 24, pyramidPositions[n]);
//        }
//
        // render planks
//        renderObject(3, texture_wood, model, defaultShader, 36, glm::vec3(20.0f, 0.5f, 0.0f), 1.0f, 90.0f);
//        renderObject(3, texture_wood, model, defaultShader, 36, glm::vec3(-20.0f, 0.5f, 0.0f), 1.0f, 90.0f);
//        renderObject(3, texture_wood, model, defaultShader, 36, glm::vec3(20.0f, 0.5f, 0.0f), 1.0f);
//        renderObject(3, texture_wood, model, defaultShader, 36, glm::vec3(-20.0f, 0.5f, 0.0f), 1.0f);
//
//        renderObject(3, texture_wood, model, defaultShader, 36, glm::vec3(20.0f, 1.0f, 0.0f), 1.0f, 90.0f);
//        renderObject(3, texture_wood, model, defaultShader, 36, glm::vec3(-20.0f, 1.0f, 0.0f), 1.0f, 90.0f);
//        renderObject(3, texture_wood, model, defaultShader, 36, glm::vec3(20.0f, 1.0f, 0.0f), 1.0f);
//        renderObject(3, texture_wood, model, defaultShader, 36, glm::vec3(-20.0f, 1.0f, 0.0f), 1.0f);


        // render the loaded model
        glActiveTexture(GL_TEXTURE0);

        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
        model = glm::translate(model, glm::vec3(0.5f, 2.0f, 0.8f)); // translate it down so it's at the center of the scene
        defaultShader.setMat4("model", model);
        guitar.Draw(defaultShader);

        glBindTexture(GL_TEXTURE_2D, texture_sky);
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(50.0f, 50.0f, 50.0f));
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        defaultShader.setMat4("model", model);
        cube.Draw(defaultShader);

        glBindTexture(GL_TEXTURE_2D, texture_grass);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        defaultShader.setMat4("model", model);
        plane.Draw(defaultShader);

        for (int n=0; n < grassObjects.size() - 1; n++) {
            model = glm::mat4(1.0f);
            model = glm::scale(model, glm::vec3(0.001f, 0.001f, 0.001f));
            model = glm::translate(model, grassPositions[n]);
            defaultShader.setMat4("model", model);
            grassObjects[n].Draw(defaultShader);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteBuffers(VBOs, VBO);
    glDeleteVertexArrays(VAOs, VAO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window){
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 3.0f * deltaTime;
    if (glfwGetKey(window, (GLFW_KEY_RIGHT_SHIFT)) || glfwGetKey(window, (GLFW_KEY_LEFT_SHIFT)) == GLFW_PRESS){
        cameraSpeed *= 3.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (!jump) {
            jumpStart = glfwGetTime();
            jump = true;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        testAngle += 0.1f;
        spdlog::info("testAngle: {}", testAngle);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        testAngle -= 0.1f;
        spdlog::info("testAngle: {}", testAngle);
    }

    if (cameraPos.x > planeMax * 1){
        cameraPos.x = planeMax;
    }
    if (cameraPos.x < -planeMax * 1){
        cameraPos.x = -planeMax;
    }
    if (cameraPos.z > planeMax * 1){
        cameraPos.z = planeMax;
    }
    if (cameraPos.z < -planeMax * 1){
        cameraPos.z = -planeMax;
    }

    if (cameraPos.y > floorMin + 1.5f || cameraPos.y < floorMin + 1.5f){
        cameraPos.y = floorMin + 1.5f;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebufferSizeCallback(GLFWwindow* window, int width, int height){
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
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

    yaw += xoffset;
    pitch += yoffset;

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

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset){
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 90.0f)
        fov = 90.0f;
}