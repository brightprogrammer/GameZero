#include "camera.hpp"
#include "glm/geometric.hpp"
#include "window.hpp"
#include "math/vector.hpp"
#include <cstring>

using namespace GameZero;

bool GameZero::Camera::MouseMotionCallback(const Vector2f& pos, const Vector2f relPos, Window* window){
    // calculate yaw and pitch
    yaw     += float(relPos.x) * angularSpeed;
    pitch   += float(-relPos.y) * angularSpeed;

    // clamp pitch : vertical angular motion
    if(pitch > 89.f) pitch = 89.f;
    if(pitch < -89.f) pitch = -89.f;

    // calculate camera front direction
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    
    // normalize value
    front = glm::normalize(front);

    return true;
}

bool GameZero::Camera::KeyboardCallback(Keyboard key, bool keyDown, Window* window){
    if(key == Keyboard::KeyUp){
        position += speed * front;
    }

    if(key == Keyboard::KeyDown){
        position -= speed * front;
    }

    if(key == Keyboard::KeyRight){
        position += glm::normalize(glm::cross(front, up)) * speed;
    }

    if(key == Keyboard::KeyLeft){
        position -= glm::normalize(glm::cross(front, up)) * speed;
    }

    if(key == Keyboard::KeyW){
        position += speed * up;  
    }
                    
    if(key == Keyboard::KeyS){
        position -= speed * up;
    }

    return true;
}

// camera constructor
GameZero::Camera::Camera(const char* name, Window& window) : name(name), window(window){
    window.RegisterCallback(name, &GameZero::Camera::MouseMotionCallback);
    window.RegisterCallback(name, &GameZero::Camera::KeyboardCallback);
}