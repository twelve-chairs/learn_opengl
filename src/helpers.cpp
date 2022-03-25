#ifndef HELPERS_H
#define HELPERS_H

#include <random>
#include <cstdlib>
#include <filesystem>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <map>
#include <spdlog/spdlog.h>
#include "include/helpers/ModelAnimation.h"


[[maybe_unused]] static int randomInteger(int to, int from){
    std::random_device randomizerSeed;
    std::default_random_engine randomEngine(randomizerSeed());
    std::uniform_int_distribution<int> randomRange(from, to);
    return randomRange(randomEngine);
}

[[maybe_unused]] static float randomFloat(float to, float from){
    std::random_device randomizerSeed;
    std::default_random_engine randomEngine(randomizerSeed());
    std::uniform_real_distribution<float> distribution(from, to);
    return distribution(randomEngine);
}

static void createTexture(GLuint &texture, const std::string &path, bool alpha){
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    stbi_set_flip_vertically_on_load(false); // tell stb_image.h to flip loaded texture's on the y-axis.
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

static GLuint initFrameBuffer(auto &frameBufferObject, auto &frameBufferSize, auto &textureColorBuffer, auto &renderBufferObject){
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

[[maybe_unused]] static std::vector<unsigned int> generateDepthMap(){
    // configure depth map FBO
    const unsigned int SHADOW_WIDTH = 4096;
    const unsigned int SHADOW_HEIGHT = 4096;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::vector<unsigned int> result = {depthMapFBO, depthMap, SHADOW_WIDTH, SHADOW_HEIGHT};
    return result;
}

[[maybe_unused]] static auto loadModels(){
    std::vector<std::string> modelNames = {
            "unicorn/unicorn.glb",
            "unicorn/unicornMane.glb",
            "unicorn/unicornTail.glb",
            "wabbit/wabbit.glb",
            "frog/frog.glb",
            "grass/grass.glb",
            "primitives/plane.obj",
            "primitives/cube.obj",
            "primitives/sphere.obj"
    };

    std::map<std::string, ModelAnimation> models;
    for (const auto& model: modelNames){
        // Strip down to model name only, i.e. "primitives/sphere.obj" -> "sphere"
        unsigned int first = model.find('/') + 1;
        unsigned int last = model.find('.');
        std::string stripped = model.substr(first, last - first);
        std::string path = fmt::format("../src/include/assets/{}", model);
        models.insert({stripped, ModelAnimation(path)});
    }

    return models;
}

[[maybe_unused]] static auto initModels(auto &models, auto &planeMaxWidth, auto &planeMaxHeight){
    // Init models
    stbi_set_flip_vertically_on_load(false);
    models.at("unicorn").position = glm::vec3(0.0f, 0.0f, planeMaxHeight);
    models.at("unicorn").rotationDegrees = 180.0f;
    models.at("unicornMane").position = glm::vec3(0.0f, 0.0f, planeMaxHeight);
    models.at("unicornMane").rotationDegrees = 180.0f;
    models.at("unicornTail").position = glm::vec3(0.0f, 0.0f, planeMaxHeight);
    models.at("unicornTail").rotationDegrees = 180.0f;

    models.at("wabbit").position = glm::vec3(-planeMaxWidth, 0.0f, 0.0f);
    models.at("wabbit").movementOffset = 0.04f;

    models.at("frog").position = glm::vec3(planeMaxWidth, 0.0f, 0.0f);
    models.at("frog").movementOffset = 0.03f;
}

[[maybe_unused]] static auto loadTextures(){
    // Load and create a texture
    std::vector<std::string> textureNames = {
            "clouds.jpeg",
            "grass.png",
            "mario_mystery.png",
            "skydome.jpeg"
    };
    std::map<std::string, unsigned int> textures;
    for (const auto &textureName : textureNames){
        unsigned int textureId;
        std::string path = fmt::format("../src/include/assets/textures/{}", textureName);
        createTexture(textureId, path, path.find(".png") != string::npos);
        textures.insert({textureName, textureId});
    }

    return textures;
}

[[maybe_unused]] static auto loadShaders(){
    // Build and compile our shader programs
    std::vector<std::string> shaderNames = {
            "default",
            "frog",
            "unicornBody",
            "unicornMane",
            "unicornTail",
            "grass",
            "shadowMappingDepth"
    };
    std::map<std::string, Shader> shaders;
    for (const auto& shader: shaderNames){
        std::string vert = fmt::format("../src/include/shaders/{}.vert", shader);
        std::string frag = fmt::format("../src/include/shaders/{}.frag", shader);
        shaders.insert({shader, Shader(vert.c_str(), frag.c_str())});
    }

    return shaders;
}

#endif