#pragma once

#include <GLFW/glfw3.h>

enum CameraInput
{
    Forward = 0,
    Right,
    Back,
    Left
};

struct Camera
{
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection;
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 cameraDirection;

    float pitch = 0.0f;
    float yaw = -90.0f;
    //float roll;

    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraRight;
    glm::vec3 cameraUp;
    float cameraSpeed = 5.0f;

    void ProcessInput(CameraInput cameraInput)
    {
        switch (cameraInput)
        {
        case Forward:
            cameraPos += cameraSpeed * cameraFront;
            break;
        case Right:
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
            break;
        case Back:
            cameraPos -= cameraSpeed * cameraFront;
            break;
        case Left:
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
            break;
        }
    }

    void UpdateCamera()
    {
        cameraDirection = glm::normalize(cameraPos - cameraTarget);

        glm::vec3 rotatedDirection;
        rotatedDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        rotatedDirection.y = sin(glm::radians(pitch));
        rotatedDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(rotatedDirection);

        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        cameraRight = glm::normalize(glm::cross(up, cameraFront));
        cameraUp = glm::cross(cameraFront, cameraRight);
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    }

    void ProcessMouseInput(glm::vec2 delta)
    {
        yaw += delta.x*0.5;
        pitch -= delta.y*0.5;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }
};