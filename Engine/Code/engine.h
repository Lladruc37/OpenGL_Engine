//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "platform.h"
#include "BufferObjects.h"
#include "assimp_model_loading.h"
#include "TexturedQuad.h"
#include "Debugging.h"
#include <glad/glad.h>

typedef glm::vec2  vec2;
typedef glm::vec3  vec3;
typedef glm::vec4  vec4;
typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;

struct Image
{
    void* pixels;
    ivec2 size;
    i32   nchannels;
    i32   stride;
};

struct Texture
{
    GLuint      handle;
    std::string filepath;
};

struct VertexV3V2
{
    glm::vec3 pos;
    glm::vec2 uv;
};

struct OpenGLInfo
{
    std::string version;
    std::string renderer;
    std::string vendor;
    std::string GLSLVersion;
    std::string extensions;
};

struct Program
{
    GLuint             handle;
    std::string        filepath;
    std::string        programName;
    u64                lastWriteTimestamp; // What is this for?
    VertexBufferLayout vertexInputLayout;
};

struct Model
{
    u32 meshIdx;
    std::vector<u32> materialIdx;
};

struct Submesh
{
    VertexBufferLayout vertexBufferLayout;
    std::vector<f32> vertices;
    std::vector<u32> indices;
    u32 vertexOffset;
    u32 indexOffset;
    std::vector<VAO> vaos;
};

struct Mesh
{
    std::vector<Submesh> submeshes;
    GLuint vertexBufferHandle;
    GLuint indexBufferHandle;
};

struct Material
{
    std::string name;
    vec3 albedo;
    vec3 emissive;
    f32 smoothness;
    u32 albedoTextureIdx;
    u32 emissiveTextureIdx;
    u32 specularTextureIdx;
    u32 normalsTextureIdx;
    u32 bumpTextureIdx;
};

enum Mode
{
    Mode_TexturedQuad,
    Mode_Count
};

struct App
{
    // Loop
    f32  deltaTime;
    bool isRunning;

    // Input
    Input input;

    // Graphics
    char gpuName[64];
    char openGlVersion[64];

    ivec2 displaySize;

    u32 textMeshIdx;
    std::vector<Texture>  textures;
    std::vector<Material> materials;
    std::vector<Mesh> meshes;
    std::vector<Model> models;
    std::vector<Program>  programs;

    // program indices
    u32 texturedMeshProgramIdx;
    u32 texturedQuadProgramIdx;
    
    //per model
    glm::mat4 model = glm::mat4(1.0f);

    //per shader
    int modelLoc;
    int viewLoc;
    int projectionLoc;
    
    //TODO: Add Camera
    //per camera
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection;

    //void processInput(GLFWwindow* window)
    //{
    //    ...
    //        const float cameraSpeed = 0.05f; // adjust accordingly
    //    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    //        cameraPos += cameraSpeed * cameraFront;
    //    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    //        cameraPos -= cameraSpeed * cameraFront;
    //    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    //        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    //    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    //        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    //}


    // texture indices
    u32 diceTexIdx;
    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;

    //Info
    OpenGLInfo info;

    // Mode
    Mode mode;

    // Embedded geometry (in-editor simple meshes such as
    // a screen filling quad, a cube, a sphere...)
    GLuint embeddedVertices;
    GLuint embeddedElements;

    // Location of the texture uniform in the textured quad shader
    GLuint programUniformTexture;

    // VAO object to link our screen filling quad with our textured quad shader
    GLuint vao;
};

void Init(App* app);

void Gui(App* app);

void Update(App* app);

void Render(App* app);

GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program);

u32 LoadTexture2D(App* app, const char* filepath);

GLuint CreateTexture2DFromImage(Image image);

void FreeImage(Image image);

Image LoadImage(const char* filename);

u32 LoadProgram(App* app, const char* filepath, const char* programName);

GLuint CreateProgramFromSource(String programSource, const char* shaderName);
