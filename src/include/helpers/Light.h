#ifndef LIGHT_H
#define LIGHT_H

class Light{
public:
    glm::vec3 position = glm::vec3(0.0f, 10.0f, 0.0f);
    float x = position.x;
    float y = position.y;
    float z = position.z;

    explicit Light(glm::vec3 position = glm::vec3(0.0f, 10.0f, 0.0f)){
        this->position = position;
    }
};

#endif //LIGHT_H
