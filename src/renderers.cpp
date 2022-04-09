#include "include/helpers/ModelAnimation.h"
#include "include/helpers/Shader.h"
#include <cstdlib>
#include <random>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <map>
#include <spdlog/spdlog.h>

void renderGrass(Shader &shader, auto &grassModel, auto &grassCount, auto &light){
    shader.use();
    shader.setVec3("lightPos", light.position);
    shader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    for (unsigned int i = 0; i < grassModel.meshes.size(); i++){
        glBindVertexArray(grassModel.meshes[i].VAO);
        glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(grassModel.meshes[i].indices.size()), GL_UNSIGNED_INT, nullptr, grassCount);
        glBindVertexArray(0);
    }
    glActiveTexture(GL_TEXTURE0);
}

void renderUnicorn(Shader &shader, auto &unicornModel, auto &textures, auto &animator, auto &light, auto &unicornColorTest){
    auto transforms = animator.GetFinalBoneMatrices();
    shader.use();
    for (int i = 0; i < transforms.size(); ++i) {
        shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
    }
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, unicornModel.position);
    model = glm::rotate(model, glm::radians(unicornModel.rotationDegrees), unicornModel.rotationAxis);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::scale(model, unicornModel.scale);
    shader.setMat4("model", model);
    shader.setVec3("lightPos", light.position);
    shader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setVec3("objectColor", unicornColorTest);
    unicornModel.Draw(shader);
    glActiveTexture(GL_TEXTURE0);
}

void renderUnicornMane(Shader &shader, auto &unicornModel, auto &textures, auto &light, auto &unicornManeColorTest){
    shader.use();
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, unicornModel.position);
    model = glm::rotate(model, glm::radians(unicornModel.rotationDegrees), unicornModel.rotationAxis);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::scale(model, unicornModel.scale);
    shader.setMat4("model", model);
    shader.setVec3("lightPos", light.position);
    shader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setVec3("objectColor", unicornManeColorTest);
    unicornModel.Draw(shader);
    glActiveTexture(GL_TEXTURE0);
}

void renderUnicornTail(Shader &shader, auto &unicornModel, auto &textures, auto &light, auto &unicornTailColorTest){
    shader.use();
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, unicornModel.position);
    model = glm::rotate(model, glm::radians(unicornModel.rotationDegrees), unicornModel.rotationAxis);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::scale(model, unicornModel.scale);
    shader.setMat4("model", model);
    shader.setVec3("lightPos", light.position);
    shader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setVec3("objectColor", unicornTailColorTest);
    unicornModel.Draw(shader);
    glActiveTexture(GL_TEXTURE0);
}

void renderWabbit(Shader &shader, auto &wabbitModel, auto &textures, auto &currentFrame, auto &planeMaxWidth){
    shader.use();
    glm::mat4 model = glm::mat4(1.0f);

    // Wacky Wabbit Antics
    float amplitude = 0.3f;
    float speed = 3.0f;
    float degrees = wabbitModel.rotationDegrees;
    float offset = wabbitModel.movementOffset;

    float x = wabbitModel.position.x;
    if (x >= planeMaxWidth){
        offset = -abs(offset);
        x = planeMaxWidth + offset;
    }
    else if (x <= -planeMaxWidth){
        offset = abs(offset);
        x = -planeMaxWidth + offset;
    }
    else {
        x = x + offset;
    }

    float y = amplitude * (glm::sin(speed * x)) + 0.2f;
    float z = 10.0f * sin(0.25f * x);

    model = glm::translate(model, glm::vec3(x, y, z));
    if (offset > 0){
        degrees = 90.0f;
        if (wabbitModel.position.z > z){
            degrees += 45.0f;
        }
        else {
            degrees -= 45.0f;
        }
    }
    else{
        degrees = -90.0f;
        if (wabbitModel.position.z > z){
            degrees -= 45.0f;
        }
        else {
            degrees += 45.0f;
        }
    }
    wabbitModel.position = glm::vec3(x, y, z);
    wabbitModel.rotationDegrees = degrees;
    wabbitModel.movementOffset = offset;

    model = glm::rotate(model, glm::radians(degrees), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
    shader.setMat4("model", model);
    wabbitModel.Draw(shader);
    glActiveTexture(GL_TEXTURE0);
}

void renderFrog(Shader &shader, auto &frogModel, auto &textures, auto &currentFrame, auto &planeMaxWidth){
    shader.use();
    glm::mat4 model = glm::mat4(1.0f);

    // Funky Frog Antics
    float amplitude = 0.3f;
    float speed = 9.0f;
    float offset = frogModel.movementOffset;
    float degrees = frogModel.rotationDegrees;
    float x = frogModel.position.x;

    if (x >= planeMaxWidth){
        offset = -abs(offset);
        x = planeMaxWidth + offset;
    }
    else if (x <= -planeMaxWidth){
        offset = abs(offset);
        x = -planeMaxWidth + offset;
    }
    else {
        x = x + offset;
    }

    float y = amplitude * (glm::sin(speed * x)) + 0.2f;
    float z = 10.0f * sin(0.25f * x);

    model = glm::translate(model, glm::vec3(x, y, z));

    if (offset > 0.0f){
        degrees = 90.0f;
        if (frogModel.position.z >= z){
            degrees += 45.0f;
        }
        else {
            degrees -= 45.0f;
        }
    }
    else{
        degrees = -90.0f;
        if (frogModel.position.z >= z){
            degrees -= 45.0f;
        }
        else {
            degrees += 45.0f;
        }
    }
    frogModel.position = glm::vec3(x, y, z);
    frogModel.rotationDegrees = degrees;
    frogModel.movementOffset = offset;


    model = glm::rotate(model, glm::radians(degrees), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
    shader.setMat4("model", model);
    frogModel.Draw(shader);
    glActiveTexture(GL_TEXTURE0);
}

void renderPlane(Shader &shader, auto &planeModel, auto &texture, auto &depthMap, auto planeMaxWidth, auto planeMaxHeight){
    shader.use();
    shader.setFloat("dif", 0.5);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    auto model = glm::mat4(1.0f);
    model = glm::translate(model, planeModel.position);
    model = glm::scale(model, glm::vec3(planeMaxWidth, 10.0f, planeMaxHeight));
    shader.setMat4("model", model);
    planeModel.Draw(shader);
    glActiveTexture(GL_TEXTURE0);
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
    glActiveTexture(GL_TEXTURE0);
}

void renderSkyDome(Shader &shader, auto &mesh, auto &texture, auto &currentFrame, auto planeMaxHeight) {
    shader.use();
    shader.setInt("shadowMap", 0);
    shader.setFloat("amb", 0.8);
    shader.setFloat("dif", 0.0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, currentFrame * 0.01f, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(planeMaxHeight * 10.0f,
                                        planeMaxHeight * 4.0f,
                                        planeMaxHeight * 10.0f));
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    shader.setMat4("model", model);
    mesh.Draw(shader);
    glActiveTexture(GL_TEXTURE0);
}