#include "camera.hpp"
#include "glm/geometric.hpp"
#include "window.hpp"
#include "math/vector.hpp"
#include <cstring>

using namespace GameZero;

// camera constructor
GameZero::Camera::Camera(const char* name, Window& window) : name(name), window(window){
    // mouse motion event callback
    window.RegisterMouseMotionEventCallback([this](MouseMotionEventInfo& info){
        // calculate yaw and pitch
        yaw     += float(info.relativePosition.x) * angularSpeed;
        pitch   += float(-info.relativePosition.y) * angularSpeed;

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
    });

    // keyboard event callback
    window.RegisterKeyboardEventCallback([this](KeyboardEventInfo& info){
        if(info.key == Keyboard::KeyUp){
            position += speed * front;
        }

        if(info.key == Keyboard::KeyDown){
            position -= speed * front;
        }

        if(info.key == Keyboard::KeyRight){
            position += glm::normalize(glm::cross(front, up)) * speed;
        }

        if(info.key == Keyboard::KeyLeft){
            position -= glm::normalize(glm::cross(front, up)) * speed;
        }

        if(info.key == Keyboard::KeyW){
            position += speed * up;  
        }
                        
        if(info.key == Keyboard::KeyS){
            position -= speed * up;
        }

        return true;
    });
}