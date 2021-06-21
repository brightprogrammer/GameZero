/**
 * @file camera.hpp
 * @author Siddharth Mishra (bshock665@gmail.com)
 * @brief Camera class
 * @version 0.1
 * @date 2021-06-21
 * 
 * Copyright (c) 2021 Siddharth Mishra. All Rights Reserved.
 * 
 */

#ifndef GAMEZERO_CAMERA_HPP
#define GAMEZERO_CAMERA_HPP

#include "common.hpp"
#include "window.hpp"
#include "vulkan/types.hpp"

namespace GameZero{

    class Camera{
        /// mouse motion callback from camera
        bool MouseMotionCallback(const Vector2f& pos, const Vector2f& relPos, Window* window);
        /// keyboard callback from camera
        bool KeyboardCallback(Keyboard key, bool keyDown, Window* window);

        /// camera data is in one struct so that it can be
        /// easily copied in just one call
        struct GPUCameraData{
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 proj;
        };

        /// camera data
        GPUCameraData cameraData;

    public:
        /**
         * @brief Construct a new Camera object.
         *        Camera needs window to calculate matrices.
         * 
         * @param camera name
         * @param window that will use this camera
         */
        Camera(const char* name, Window& window);

        /// camera name
        const char* name;
        /// window that this camera is attached to
        Window& window;

        /// camera position
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 20.0f);
        /// camera up direction
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        /// camera front face face direction
        glm::vec3 front = glm::vec3(0.0f, 0.0f, 1.0f);

        /// movement speed
        float speed = 0.5f;
        /// angular speed
        float angularSpeed = 0.5f;

        /// initial field of view
        float fieldOfView = 25.f;
        /// min field of view
        float minFieldOfView = 1.f;
        /// max field of view 
        float maxFieldOfView = 45.f;

        /// initial yaw
        float yaw = -90.f;
        /// min yaw : horizontal angle
        float minYaw = -90.f;
        /// max yaw : horizontal angle
        float maxYaw = 90.f;

        /// initial pitch
        float pitch = 89.f;
        /// min pitch : vertical angle
        float minPitch = -89.f;
        /// max pitch : vertical angle
        float maxPitch = 89.f;
    };

}

#endif//GAMEZERO_CAMERA_HPP
