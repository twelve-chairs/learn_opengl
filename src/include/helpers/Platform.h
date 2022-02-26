#ifndef PLATFORM_H
#define PLATFORM_H

class Platform{
public:
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    float x = position.x;
    float y = position.y;
    float z = position.z;

    float planeMaxWidth = 1.0f;
    float planeMaxHeight = 1.0f;
    float floorMin = 0.0f;

    explicit Platform(auto newPlaneMaxWidth, auto newPlaneMaxHeight, auto newFloorMin){
        planeMaxWidth = newPlaneMaxWidth;
        this->planeMaxHeight = newPlaneMaxHeight;
        this->floorMin = newFloorMin;
    }

private:
    void setupObjects(){

    }
};


#endif //PLATFORM_H