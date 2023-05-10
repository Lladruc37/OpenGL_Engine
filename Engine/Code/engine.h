//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "platform.h"
#include "BufferObjects.h"
#include "assimp_model_loading.h"
#include "TexturedQuad.h"
#include "buffer_management.h"
#include "Debugging.h"
#include <glad/glad.h>
#include "Camera.h"

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
    i32   nchannels =0;
    i32   stride =0;
};

struct Texture
{
    GLuint      handle =0;
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
    GLuint             handle =0;
    std::string        filepath;
    std::string        programName;
    u64                lastWriteTimestamp =0; // What is this for?
    VertexBufferLayout vertexInputLayout;
};

struct Model
{
    u32 meshIdx = 0;
    std::vector<u32> materialIdx;
};

struct Submesh
{
    VertexBufferLayout vertexBufferLayout;
    std::vector<f32> vertices;
    std::vector<u32> indices;
    u32 vertexOffset = 0;
    u32 indexOffset = 0;
    std::vector<VAO> vaos;
};

struct Mesh
{
    std::vector<Submesh> submeshes;
    GLuint vertexBufferHandle = 0;
    GLuint indexBufferHandle = 0;
};

struct Material
{
    std::string name;
    vec3 albedo;
    vec3 emissive;
    vec3 specular;
    f32 smoothness =0.0f;
    u32 albedoTextureIdx =0;
    u32 emissiveTextureIdx=0;
    u32 specularTextureIdx = 0;
    u32 normalsTextureIdx =0;
    u32 bumpTextureIdx = 0;

    Material()
    {
        name = "";
        albedo = vec3(0.0f);
        emissive = vec3(0.0f);
        specular = vec3(0.0f);
    }

    Material(std::string _name,
        vec3 _albedo,
        vec3 _emissive,
        vec3 _specular,
        f32 _smoothness = 0.0f,
        u32 _albedoTextureIdx = 0,
        u32 _emissiveTextureIdx = 0,
        u32 _specularTextureIdx = 0,
        u32 _normalsTextureIdx = 0,
        u32 _bumpTextureIdx = 0)
    {
        name = _name;
        albedo = _albedo;
        emissive = _emissive;
        specular = _specular;
        smoothness = _smoothness;
        albedoTextureIdx = _albedoTextureIdx;
        emissiveTextureIdx = _emissiveTextureIdx;
        specularTextureIdx = _specularTextureIdx;
        normalsTextureIdx = _normalsTextureIdx;
        bumpTextureIdx = _bumpTextureIdx;
    }
};

struct Entity
{
    glm::mat4 worldMatrix;
    u32 modelIdx=0;
    u32 localParamsOffset=0;
    u32 localParamsSize=0;

    Entity(glm::mat4 worldMat, u32 modelIndex, u32 localOffset = 0, u32 localSize = 0)
    {
        worldMatrix = worldMat;
        modelIdx = modelIndex;
        localParamsOffset = localOffset;
        localParamsSize = localSize;
    }
};

enum LightType
{
    Directional_Light = 0,
    Point_Light,
    //Spotlight
};

struct Light
{
    LightType type;
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;

    Light(LightType t, vec3 pos, vec3 dir, vec3 amb, vec3 diff, vec3 spec, float cons = 1.0f)
    {
        type = t;
        position = pos;
        direction = dir;
        ambient = amb;
        diffuse = diff;
        specular = spec;
        constant = cons;
    }
};

struct Buffer
{
    GLuint handle;
    GLenum type;
    GLint size = 0;
    GLint head = 0;
    void* data; //mapped data
};

enum Mode
{
    Mode_TexturedQuad,
    Mode_Count
};

struct App
{
    // Loop
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    bool isRunning;

    // Input
    Input input;

    // Graphics
    char gpuName[64];
    char openGlVersion[64];

    ivec2 displaySize;

    u32 patrickModelId;
    u32 planeModelId;
    std::vector<Texture>  textures;
    std::vector<Material> materials;
    std::vector<Mesh> meshes;
    std::vector<Model> models;
    std::vector<Program>  programs;

    // program indices
    u32 texturedMeshProgramIdx;
    u32 texturedQuadProgramIdx;

    //Global Params
    u32 globalParamsOffset;
    u32 globalParamsSize;

    //Uniform
    Buffer cbuffer;
    int maxUniformBufferSize;
    int uniformBufferAlignment;

    //Lights
    std::vector<Light> lights;

    //entities
    std::vector<Entity> entities;

    //Camera
    Camera camera;

    // texture indices
    u32 diceTexIdx;
    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;

    //Info
    OpenGLInfo info;

    //Frame buffer object
    GLuint framebufferHandle;
    GLuint colorAttachmentHandle;

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

void CreateTextureQuad(App* app, Material myMaterial);

GLuint CreateTexture2DFromImage(Image image);

void FreeImage(Image image);

Image LoadImage(const char* filename);

u32 LoadProgram(App* app, const char* filepath, const char* programName);

GLuint CreateProgramFromSource(String programSource, const char* shaderName);
