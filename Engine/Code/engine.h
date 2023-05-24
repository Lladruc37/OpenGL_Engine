#pragma once

#include "platform.h"
#include "BufferObjects.h"
#include "assimp_model_loading.h"
#include "TexturedQuad.h"
#include "buffer_management.h"
#include "Debugging.h"
#include <glad/glad.h>
#include "Camera.h"

typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;

struct Image
{
    void* pixels;
    ivec2 size;
    i32 nchannels = 0;
    i32 stride = 0;
};

struct Texture
{
    GLuint handle = 0;
    std::string filepath;
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
    GLuint handle = 0;
    std::string filepath;
    std::string programName;
    u64 lastWriteTimestamp = 0;
    VertexBufferLayout vertexInputLayout;

    Program(GLuint _handle = 0,u64 _lastWriteTimestamp = 0)
        : handle(_handle),lastWriteTimestamp(_lastWriteTimestamp)
    {
        filepath.clear();
        programName.clear();
        vertexInputLayout = VertexBufferLayout();
    }
};

struct Model
{
    u32 meshIdx = 0;
    std::vector<u32> materialIdx;

    Model(u32 _meshIdx = 0) : meshIdx(_meshIdx)
    {
        materialIdx.clear();
    }
};

struct Submesh
{
    VertexBufferLayout vertexBufferLayout;
    std::vector<f32> vertices;
    std::vector<u32> indices;
    u32 vertexOffset = 0;
    u32 indexOffset = 0;
    std::vector<VAO> vaos;

    Submesh(u32 _vertexOffset = 0, u32 _indexOffset = 0)
        : vertexOffset(_vertexOffset), indexOffset(_indexOffset)
    {
        vertexBufferLayout = VertexBufferLayout();
        vertices.clear();
        indices.clear();
        vaos.clear();
    }
};

struct Mesh
{
    std::vector<Submesh> submeshes;
    GLuint vertexBufferHandle = 0;
    GLuint indexBufferHandle = 0;

    Mesh(GLuint _vertexBufferHandle = 0,GLuint _indexBufferHandle = 0)
        : vertexBufferHandle(_vertexBufferHandle),indexBufferHandle(_indexBufferHandle)
    {
        submeshes.clear();
    }
};

struct Material
{
    std::string name;
    vec3 albedo = vec3(0.0f);
    vec3 emissive = vec3(0.0f);
    vec3 specular = vec3(0.0f);
    f32 smoothness = 0.0f;
    u32 albedoTextureIdx = 0;
    u32 emissiveTextureIdx = 0;
    u32 specularTextureIdx = 0;
    u32 normalsTextureIdx = 0;
    u32 bumpTextureIdx = 0;

    Material()
    {
        name = "";
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
        : name(_name),
        albedo(_albedo),
        specular(_specular),
        smoothness(_smoothness),
        albedoTextureIdx(_albedoTextureIdx),
        emissiveTextureIdx(_emissiveTextureIdx),
        specularTextureIdx(_specularTextureIdx),
        normalsTextureIdx(_normalsTextureIdx),
        bumpTextureIdx(_bumpTextureIdx)
    {}
};

struct Entity
{
    glm::mat4 worldMatrix;
    u32 modelIdx = 0;
    u32 localParamsOffset = 0;
    u32 localParamsSize = 0;

    Entity(glm::mat4 worldMat, u32 modelIndex, u32 localOffset = 0, u32 localSize = 0)
        : worldMatrix(worldMat),modelIdx(modelIndex),localParamsOffset(localOffset),localParamsSize(localSize)
    {}
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
    vec3 position = vec3(0.0f);
    vec3 direction = vec3(0.0f);
    vec3 ambient = vec3(0.0f);
    vec3 diffuse = vec3(0.0f);
    vec3 specular = vec3(0.0f);
    float constant = 0.0f;

    Light(LightType t, vec3 pos, vec3 dir, vec3 amb, vec3 diff, vec3 spec, float cons = 1.0f)
        : type(t),position(pos),direction(dir),ambient(amb),diffuse(diff),specular(spec),constant(cons)
    {}
};

struct Buffer
{
    GLuint handle;
    GLenum type;
    GLint size = 0;
    GLint head = 0;
    void* data; //Mapped data
};

struct App
{
    //--Loop--
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    bool isRunning;

    //--Graphics--
    char gpuName[64];
    char openGlVersion[64];
    ivec2 displaySize;

    //--Input--
    Input input;

    //--Camera--
    Camera camera;

    //--Lists of loaded info--
    std::vector<Texture>  textures;
    std::vector<Material> materials;
    std::vector<Mesh> meshes;
    std::vector<Model> models;
    std::vector<Program>  programs;
    std::vector<Light> lights;
    std::vector<Entity*> entities;

    //--VAO index--
    GLuint vaoIdx;

    //--Models indices--
    u32 patrickModelIdx;
    u32 planeModelIdx;

    //--Program indices--
    u32 texturedMeshProgramIdx;
    u32 texturedQuadProgramIdx;
    u32 postProcessingProgramIdx;

    //--Global Params--
    u32 globalParamsOffset;
    u32 globalParamsSize;

    //--Uniform buffer--
    Buffer cbuffer;
    int maxUniformBufferSize;
    int uniformBufferAlignment;

    //--Texture indices--
    u32 diceTexIdx;
    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;

    //--ImGui & related--
    OpenGLInfo info;
    bool isInfo = false;
    bool isEngine = false;

    //--Frame buffer object--
    GLuint framebufferHandle;

    //Attachments
    GLuint positionAttachmentHandle;
    GLuint normalAttachmentHandle;
    GLuint albedoAttachmentHandle;
    GLuint specularAttachmentHandle;
    GLuint depthAttachmentHandle;

    //--Post processing frame buffer--
    GLuint framebufferPostProcessingHandle;

    //Attachments
    GLuint finalColorAttachmentHandle;

    //--Program uniforms--
    //Mesh
    GLuint programUniformDiffuse;

    //Textured quad
    GLuint programUniformRenderTarget;
    GLuint programUniformLightingPosition;
    GLuint programUniformLightingNormal;
    GLuint programUniformLightingAlbedo;
    GLuint programUniformLightingSpec;
    GLuint programUniformLightingDepth;

    //Post processing
    GLuint programUniformPostProcessing;
    std::vector<std::string> renderTargets;
    int currentRenderTarget;
};

GLuint CreateProgramFromSource(String programSource, const char* shaderName);
u32 LoadProgram(App* app, const char* filepath, const char* programName);
Image LoadImage(const char* filename);
void FreeImage(Image image);
GLuint CreateTexture2DFromImage(Image image);
u32 LoadTexture2D(App* app, const char* filepath);

u32 CreateTextureQuad(App* app);
void CreateTextureQuadGeometry(App* app, Material myMaterial);
GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program);

void Init(App* app);
void InfoInit(App* app);
void DebugInit();
void FrameBufferInit(App* app);
void FrameBufferCheck();

void Gui(App* app);
void Update(App* app);

void Render(App* app);
void GeometryPass(App* app);
void LightingPass(App* app);
void PostProcessingPass(App* app);
