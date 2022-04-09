#ifndef CAMERA_H
#define CAMERA_H

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  60.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // camera Attributes
    glm::vec3 position{};
    glm::vec3 front;
    glm::vec3 up{};
    glm::vec3 right{};
    glm::vec3 worldUp{};
    // euler Angles
    float yaw{};
    float pitch{};
    // camera options
    float speed;
    float sensitivity;
    float zoom;

    // constructor with vectors
    explicit Camera(glm::vec3 newPosition = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 newUp = glm::vec3(0.0f, 1.0f, 0.0f), float newYaw = YAW, float newPitch = PITCH) : front(glm::vec3(0.0f, 0.0f, -1.0f)), speed(SPEED), sensitivity(SENSITIVITY), zoom(ZOOM){
        this->position = newPosition;
        this->worldUp = newUp;
        this->yaw = newYaw;
        this->pitch = newPitch;
        this->updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    [[nodiscard]] glm::mat4 GetViewMatrix() const{
        return glm::lookAt(position, position + front, up);
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime){
        float velocity = speed * deltaTime;
        if (direction == FORWARD)
            position += front * velocity;
        if (direction == BACKWARD)
            position -= front * velocity;
        if (direction == LEFT)
            position -= right * velocity;
        if (direction == RIGHT)
            position += right * velocity;
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset = 0.0f, float yoffset = 0.0f, GLboolean constrainPitch = true){
//        xoffset *= this->sensitivity;
//        yoffset *= this->sensitivity;
//
//        this->yaw += xoffset;
//        this->pitch += yoffset;
//
//        // make sure that when pitch is out of bounds, screen doesn't get flipped
//        if (constrainPitch)
//        {
//            if (this->pitch > 89.0f)
//                this->pitch = 89.0f;
//            if (this->pitch < -89.0f)
//                this->pitch = -89.0f;
//        }

        // update front, right and up Vectors using the updated Euler angles
        updateCameraVectors();
        //    if (firstMouse){
//        lastX = xpos;
//        lastY = ypos;
//        firstMouse = false;
//    }
//
//    float xoffset = xpos - lastX;
//    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
//    lastX = xpos;
//    lastY = ypos;
//
//    xoffset *= camera.sensitivity;
//    yoffset *= camera.sensitivity;
//
//    camera.yaw += xoffset;
//    camera.pitch += yoffset;
//
//    // make sure that when pitch is out of bounds, screen doesn't get flipped
//    if (camera.pitch > 89.0f)
//        camera.pitch = 89.0f;
//    if (camera.pitch < -89.0f)
//        camera.pitch = -89.0f;
//
//    glm::vec3 front;
//    front.x = cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
//    front.y = sin(glm::radians(camera.pitch));
//    front.z = sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
//    camera.front = glm::normalize(front);
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset){
        zoom -= (float)yoffset;
        if (zoom < 1.0f)
            zoom = 1.0f;
        if (zoom > 90.0f)
            zoom = 90.0f;
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors(){
        // calculate the new front vector
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        this->front = glm::normalize(front);
        // also re-calculate the right and up vector
        this->right = glm::normalize(glm::cross(front, worldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        this->up = glm::normalize(glm::cross(right, front));
    }
};

#endif