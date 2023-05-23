#include "engine.h"
#include <imgui.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <iostream>

#define BINDING(b) b

GLuint CreateProgramFromSource(String programSource, const char* shaderName)
{
    GLchar  infoLogBuffer[1024] = {};
    GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
    GLsizei infoLogSize;
    GLint   success;

    char versionString[] = "#version 430\n";
    char shaderNameDefine[128];
    sprintf(shaderNameDefine, "#define %s\n", shaderName);
    char vertexShaderDefine[] = "#define VERTEX\n";
    char fragmentShaderDefine[] = "#define FRAGMENT\n";

    const GLchar* vertexShaderSource[] = {
        versionString,
        shaderNameDefine,
        vertexShaderDefine,
        programSource.str
    };
    const GLint vertexShaderLengths[] = {
        (GLint) strlen(versionString),
        (GLint) strlen(shaderNameDefine),
        (GLint) strlen(vertexShaderDefine),
        (GLint) programSource.len
    };
    const GLchar* fragmentShaderSource[] = {
        versionString,
        shaderNameDefine,
        fragmentShaderDefine,
        programSource.str
    };
    const GLint fragmentShaderLengths[] = {
        (GLint) strlen(versionString),
        (GLint) strlen(shaderNameDefine),
        (GLint) strlen(fragmentShaderDefine),
        (GLint) programSource.len
    };

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, ARRAY_COUNT(vertexShaderSource), vertexShaderSource, vertexShaderLengths);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, ARRAY_COUNT(fragmentShaderSource), fragmentShaderSource, fragmentShaderLengths);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with fragment shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vshader);
    glAttachShader(programHandle, fshader);
    glLinkProgram(programHandle);
    glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programHandle, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    glUseProgram(0);

    glDetachShader(programHandle, vshader);
    glDetachShader(programHandle, fshader);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    return programHandle;
}

u32 LoadProgram(App* app, const char* filepath, const char* programName)
{
    String programSource = ReadTextFile(filepath);

    Program program = {};
    program.handle = CreateProgramFromSource(programSource, programName);
    program.filepath = filepath;
    program.programName = programName;
    program.lastWriteTimestamp = GetFileLastWriteTimestamp(filepath);

    //Vertex Shader Layout
    int attributeCount = 0;
    int attributeNameMaxLength = 0;
    char* attributeName;
    glGetProgramiv(program.handle, GL_ACTIVE_ATTRIBUTES, &attributeCount);
    glGetProgramiv(program.handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &attributeNameMaxLength);

    attributeName = new char[attributeNameMaxLength++];

    for (int i = 0; i < attributeCount; ++i)
    {
        int attributeNameLength = 0;
        int attributeSize = 0;
        GLenum attributeType;
        glGetActiveAttrib(program.handle, i, attributeNameMaxLength + 1, &attributeNameLength, &attributeSize, &attributeType, attributeName);
        u8 attributeLocation = glGetAttribLocation(program.handle, attributeName);

        u8 componentCount = 1;
        switch (attributeType)
        {
        case GL_FLOAT:
            componentCount = 1;
            break;
        case GL_FLOAT_VEC2:
            componentCount = 2;
            break;
        case GL_FLOAT_VEC3:
            componentCount = 3;
            break;
        case GL_FLOAT_VEC4:
            componentCount = 4;
            break;
        default:
            break;
        }
        program.vertexInputLayout.attributes.push_back({ attributeLocation,componentCount });
    }
    delete[] attributeName;
    app->programs.push_back(program);
    return app->programs.size() - 1;
}

Image LoadImage(const char* filename)
{
    Image img = {};
    stbi_set_flip_vertically_on_load(true);
    img.pixels = stbi_load(filename, &img.size.x, &img.size.y, &img.nchannels, 0);
    if (img.pixels)
    {
        img.stride = img.size.x * img.nchannels;
    }
    else
    {
        ELOG("Could not open file %s", filename);
    }
    return img;
}

void FreeImage(Image image)
{
    stbi_image_free(image.pixels);
}

GLuint CreateTexture2DFromImage(Image image)
{
    GLenum internalFormat = GL_RGB8;
    GLenum dataFormat     = GL_RGB;
    GLenum dataType       = GL_UNSIGNED_BYTE;

    switch (image.nchannels)
    {
        case 3: dataFormat = GL_RGB; internalFormat = GL_RGB8; break;
        case 4: dataFormat = GL_RGBA; internalFormat = GL_RGBA8; break;
        default: ELOG("LoadTexture2D() - Unsupported number of channels");
    }

    GLuint texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.size.x, image.size.y, 0, dataFormat, dataType, image.pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texHandle;
}

u32 LoadTexture2D(App* app, const char* filepath)
{
    for (u32 texIdx = 0; texIdx < app->textures.size(); ++texIdx)
        if (app->textures[texIdx].filepath == filepath)
            return texIdx;

    Image image = LoadImage(filepath);
    if (image.pixels)
    {
        Texture tex = {};
        tex.handle = CreateTexture2DFromImage(image);
        tex.filepath = filepath;

        u32 texIdx = app->textures.size();
        app->textures.push_back(tex);

        FreeImage(image);
        return texIdx;
    }
    return UINT32_MAX;
}

u32 CreateTextureQuad(App* app)
{
    const u32 indices[] = {
    0,1,2,
    0,2,3
    };

    //pos, uv
    const f32 vertices[] = {
    -1.0f, -1.0f, 0.0f,0.0f, //bottom right
    1.0f, -1.0f,1.0f,0.0f, //bottom left
    1.0f, 1.0f, 1.0f,1.0f, //top right
    -1.0f, 1.0f,0.0f,1.0f, //top left
    };

    u32 vao, vbo,ebo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return vao;
}


void CreateTextureQuadGeometry(App* app, Material myMaterial)
{
    const u32 indices[] = {
    0,1,2,
    0,2,3
    };

    //pos, normal, uv
    const f32 vertices[] = {
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, //bottom right
    0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, //bottom left
    0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, //top right
    -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, //top left
    };

    Submesh submesh;
    for (int i = 0; i < 6; ++i)
        submesh.indices.push_back(indices[i]);

    for (int i = 0; i < 20; ++i)
        submesh.vertices.push_back(vertices[i]);

    Mesh mesh;
    glGenBuffers(1, &mesh.vertexBufferHandle);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &mesh.indexBufferHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(submesh.vertices), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(submesh.vertices), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(submesh.vertices), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);
    glBindVertexArray(0);

    submesh.vaos.push_back(VAO{ vao,app->programs[app->texturedMeshProgramIdx].handle });

    //Create the vertex format
    VertexBufferLayout vertexBufferLayout = {};
    vertexBufferLayout.attributes.push_back(VertexBufferAttribute{ 0, 3, 0 });
    vertexBufferLayout.stride = 3 * sizeof(float);
    vertexBufferLayout.attributes.push_back(VertexBufferAttribute{ 1, 3, vertexBufferLayout.stride });
    vertexBufferLayout.stride += 3 * sizeof(float);
    vertexBufferLayout.attributes.push_back(VertexBufferAttribute{ 2, 2, vertexBufferLayout.stride });
    vertexBufferLayout.stride += 2 * sizeof(float);
    submesh.vertexBufferLayout = vertexBufferLayout;
    submesh.vertexOffset = 0;
    submesh.indexOffset = 0;

    app->materials.push_back(myMaterial);
    mesh.submeshes.push_back(submesh);
    app->meshes.push_back(mesh);
    Model model;
    model.materialIdx.push_back(app->materials.size() - 1u);
    model.meshIdx = (u32)app->meshes.size() - 1u;
    app->models.push_back(model);
    app->planeModelIdx = (u32)app->models.size() - 1u;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program)
{
    Submesh& submesh = mesh.submeshes[submeshIndex];

    //Try finding a VAO for this submesh/program
    for (u32 i = 0; i < (u32)submesh.vaos.size(); ++i)
    {
        if (submesh.vaos[i].programHandle == program.handle)
            return submesh.vaos[i].handle;
    }

    GLuint vaoHandle = 0;
    //Create a new VAO for this submesh/program
    {
        glGenVertexArrays(1, &vaoHandle);
        glBindVertexArray(vaoHandle);

        glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);

        //We have to link all vertex inputs attributes to attributes in the vertex buffer
        for (u32 i = 0; i < program.vertexInputLayout.attributes.size(); ++i)
        {
            bool attributeWasLinked = false;

            for (u32 j = 0; j < submesh.vertexBufferLayout.attributes.size(); ++j)
            {
                //The submesh should provide an attribute for each vertex inputs
                if (program.vertexInputLayout.attributes[i].location == submesh.vertexBufferLayout.attributes[j].location)
                {
                    const u32 index = submesh.vertexBufferLayout.attributes[j].location;
                    const u32 ncomp = submesh.vertexBufferLayout.attributes[j].componentCount;
                    const u32 offset = submesh.vertexBufferLayout.attributes[j].offset + submesh.vertexOffset;
                    const u32 stride = submesh.vertexBufferLayout.stride;
                    glVertexAttribPointer(index, ncomp, GL_FLOAT, GL_FALSE, stride, (void*)(u64)offset);
                    glEnableVertexAttribArray(index);

                    attributeWasLinked = true;
                    break;
                }
            }
            assert(attributeWasLinked);
        }
        glBindVertexArray(0);
    }

    //Store it in the list of vaos for this submesh
    VAO vao = { vaoHandle,program.handle };
    submesh.vaos.push_back(vao);

    return vaoHandle;
}

void Init(App* app)
{
    InfoInit(app);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    DebugInit();

    app->renderTargets.push_back("final color");
    app->renderTargets.push_back("position color");
    app->renderTargets.push_back("normal color");
    app->renderTargets.push_back("albedo color");
    app->renderTargets.push_back("spec color");
    app->currentRenderTarget = 0;

    FrameBufferInit(app);

    //Texture initialization
    app->diceTexIdx = LoadTexture2D(app, "dice.png");
    app->whiteTexIdx = LoadTexture2D(app, "color_white.png");
    app->blackTexIdx = LoadTexture2D(app, "color_black.png");
    app->normalTexIdx = LoadTexture2D(app, "color_normal.png");
    app->magentaTexIdx = LoadTexture2D(app, "color_magenta.png");

    //Materials
    Material planeMat = Material("plane_mat", vec3(1.0f), vec3(0.0f), vec3(0.5f), 64.0f, app->magentaTexIdx);

    //Patrick
    app->patrickModelIdx = LoadModel(app, "Patrick/Patrick.obj");
    app->texturedMeshProgramIdx = LoadProgram(app, "shaders.glsl", "GEOMETRY_PASS");
    app->programUniformDiffuse = glGetUniformLocation(app->programs[app->texturedMeshProgramIdx].handle, "material.diffuse");

    //TextQuadGeometry
    CreateTextureQuadGeometry(app, planeMat);

    //TextQuad
    app->vaoIdx = CreateTextureQuad(app);
    app->texturedQuadProgramIdx = LoadProgram(app, "shaders.glsl", "LIGHTING_PASS");
    app->programUniformRenderTarget = glGetUniformLocation(app->programs[app->texturedQuadProgramIdx].handle, "renderTarget");
    app->programUniformLightingPosition = glGetUniformLocation(app->programs[app->texturedQuadProgramIdx].handle, "gPosition");
    app->programUniformLightingNormal = glGetUniformLocation(app->programs[app->texturedQuadProgramIdx].handle, "gNormal");
    app->programUniformLightingAlbedo = glGetUniformLocation(app->programs[app->texturedQuadProgramIdx].handle, "gAlbedo");
    app->programUniformLightingSpec = glGetUniformLocation(app->programs[app->texturedQuadProgramIdx].handle, "gSpec");
    app->postProcessingProgramIdx = LoadProgram(app, "shaders.glsl", "POST_PROCESSING_PASS");
    app->programUniformPostProcessing = glGetUniformLocation(app->programs[app->postProcessingProgramIdx].handle, "finalImage");

    //Entities
    Entity p1(glm::mat4(1.0f), app->patrickModelIdx);
    p1.worldMatrix = glm::translate(p1.worldMatrix, vec3(-1.0f, 0.0f, -3.0f));
    app->entities.push_back(p1);
    Entity p2(glm::mat4(1.0f), app->patrickModelIdx);
    p2.worldMatrix = glm::translate(p2.worldMatrix, vec3(10.0f, 5.0f, 0.0f));
    p2.worldMatrix = glm::rotate(p2.worldMatrix, glm::radians(-60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    app->entities.push_back(p2);

    Entity plane(glm::mat4(1.0f), app->planeModelIdx);
    plane.worldMatrix = glm::translate(plane.worldMatrix, vec3(-0.5f, -3.5f, -0.5f));
    plane.worldMatrix = glm::rotate(plane.worldMatrix, glm::radians(-90.0f), glm::vec3(1.0f, .0f, 0.0f));
    plane.worldMatrix = glm::scale(plane.worldMatrix, vec3(40.0f));
    app->entities.push_back(plane);

    //Lights
    Light l1(LightType::Directional_Light, vec3(0.0f), vec3(-0.2f, -1.0f, -0.3f), vec3(0.0f,0.0f,0.4f), vec3(0.25f), vec3(0.5f));
    app->lights.push_back(l1);
    Light l2(LightType::Point_Light, vec3(-4.0f, 1.5f, -5.0f), vec3(0.0f), vec3(1.0f,0.0f,0.0f), vec3(0.5f), vec3(1.0f),0.005f);
    app->lights.push_back(l2);
    Light l3(LightType::Point_Light, vec3(5.0f, 0.0f, -6.0f), vec3(0.0f), vec3(0.0f,1.0f,0.0f), vec3(0.5f), vec3(1.0f), 0.005f);
    app->lights.push_back(l3);
    Light l4(LightType::Point_Light, vec3(-0.5f, 0.5f, 4.0f), vec3(0.0f), vec3(0.05f, 0.05f, 0.0f), vec3(0.5f), vec3(1.0f));
    app->lights.push_back(l4);
    Light l5(LightType::Point_Light, vec3(6.5f, 6.5f, 4.5f), vec3(0.0f), vec3(1.0f), vec3(0.5f), vec3(1.0f), 0.01f);
    app->lights.push_back(l5);

    //Coordinate System / MVP Matrices
    app->camera.projection = glm::perspective(glm::radians(45.0f), (float)app->displaySize.x / app->displaySize.y, 0.1f, 100.0f);

    //Creating buffer
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &app->cbuffer.size);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &app->uniformBufferAlignment);

    glGenBuffers(1, &app->cbuffer.handle);
    glBindBuffer(GL_UNIFORM_BUFFER, app->cbuffer.handle);
    glBufferData(GL_UNIFORM_BUFFER, app->cbuffer.size, NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glViewport(0, 0, app->displaySize.x, app->displaySize.y);
}

void InfoInit(App* app)
{
    app->info.version = (char*)glGetString(GL_VERSION);
    app->info.renderer = (char*)glGetString(GL_RENDERER);
    app->info.vendor = (char*)glGetString(GL_VENDOR);
    app->info.GLSLVersion = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
    app->info.extensions = (char*)"";
    GLint num_extensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    for (int i = 0; i < num_extensions; ++i)
    {
        app->info.extensions += (const char*)glGetStringi(GL_EXTENSIONS, GLuint(i));
        app->info.extensions += "\n";
    }
}

void DebugInit()
{
    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
}

void FrameBufferInit(App* app)
{
    glGenFramebuffers(1, &app->framebufferHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, app->framebufferHandle);

    // position color buffer
    glGenTextures(1, &app->positionAttachmentHandle);
    glBindTexture(GL_TEXTURE_2D, app->positionAttachmentHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, app->displaySize.x, app->displaySize.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, app->positionAttachmentHandle, 0);

    // normal color buffer
    glGenTextures(1, &app->normalAttachmentHandle);
    glBindTexture(GL_TEXTURE_2D, app->normalAttachmentHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, app->displaySize.x, app->displaySize.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, app->normalAttachmentHandle, 0);

    // albedo color buffer
    glGenTextures(1, &app->albedoAttachmentHandle);
    glBindTexture(GL_TEXTURE_2D, app->albedoAttachmentHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, app->displaySize.x, app->displaySize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, app->albedoAttachmentHandle, 0);

    // specular color buffer
    glGenTextures(1, &app->specularAttachmentHandle);
    glBindTexture(GL_TEXTURE_2D, app->specularAttachmentHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, app->displaySize.x, app->displaySize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, app->specularAttachmentHandle, 0);

    // depth buffer
    glGenTextures(1, &app->depthAttachmentHandle);
    glBindTexture(GL_TEXTURE_2D, app->depthAttachmentHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, app->displaySize.x, app->displaySize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, app->depthAttachmentHandle, 0);

    GLuint drawBuffers[] = { GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2,GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(ARRAY_COUNT(drawBuffers), drawBuffers);

    FrameBufferCheck();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //--Post Processing--
    glGenFramebuffers(1, &app->framebufferPostProcessingHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, app->framebufferPostProcessingHandle);

    // position color buffer
    glGenTextures(1, &app->finalColorAttachmentHandle);
    glBindTexture(GL_TEXTURE_2D, app->finalColorAttachmentHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, app->displaySize.x, app->displaySize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, app->finalColorAttachmentHandle, 0);

    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    FrameBufferCheck();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBufferCheck()
{
    GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        //Something went wrong
        switch (framebufferStatus)
        {
        case GL_FRAMEBUFFER_UNDEFINED:
            ELOG("GL_FRAMEBUFFER_UNDEFINED");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            ELOG("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            ELOG("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            ELOG("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            ELOG("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER");
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            ELOG("GL_FRAMEBUFFER_UNSUPPORTED");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            ELOG("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            ELOG("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS");
            break;
        default:
            ELOG("Unkown framebuffer status error");
            break;
        }
    }
}

void Gui(App* app)
{
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    windowFlags |= ImGuiWindowFlags_NoBackground;
    windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    //--DockSpace--
    if (ImGui::Begin("Dockspace", 0, windowFlags))
    {
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspaceId = ImGui::GetID("DefaultDockspace");
            ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
        }
    }
    ImGui::End();
    ImGui::PopStyleVar(3);

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Windows"))
        {
            ImGui::Checkbox("Info", &app->isInfo);
            ImGui::Checkbox("Engine", &app->isEngine);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    //--Info--
    if (app->isInfo)
    {
        ImGui::Begin("Info");
        ImGui::Text("FPS: %f", 1.0f / app->deltaTime);
        ImGui::Text("\n\n");
        ImGui::Text("Versions:");
        ImGui::BulletText(app->info.version.c_str());
        ImGui::BulletText(app->info.renderer.c_str());
        ImGui::BulletText(app->info.vendor.c_str());
        ImGui::BulletText(app->info.GLSLVersion.c_str());
        ImGui::Text("\n\n");
        ImGui::Text("Extensions:");
        ImGui::Text(app->info.extensions.c_str());
        ImGui::End();
    }

    //--Engine--
    if (app->isEngine)
    {
        ImGui::Begin("Engine");
        if (ImGui::BeginCombo("Render Target", app->renderTargets[app->currentRenderTarget].c_str()))
        {
            for (int i = 0; i < app->renderTargets.size(); ++i)
            {
                bool is_selected = (app->renderTargets[app->currentRenderTarget] == app->renderTargets[i]);
                if (ImGui::Selectable(app->renderTargets[i].c_str(), is_selected))
                    app->currentRenderTarget = i;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::End();
    }
}

void Update(App* app)
{
    float currentFrame = glfwGetTime();
    app->deltaTime = currentFrame - app->lastFrame;
    app->lastFrame = currentFrame;

    //--Sprint--
    if (app->input.keys[K_SHIFT] == BUTTON_PRESSED)
        app->camera.cameraSpeed = 5.0f * app->deltaTime;
    else
        app->camera.cameraSpeed = 2.5f * app->deltaTime;

    //--Camera controls--
    if (app->input.keys[K_W] == BUTTON_PRESSED)
        app->camera.ProcessInput(CameraInput::Forward);
    if (app->input.keys[K_S] == BUTTON_PRESSED)
        app->camera.ProcessInput(CameraInput::Back);
    if (app->input.keys[K_A] == BUTTON_PRESSED)
        app->camera.ProcessInput(CameraInput::Left);
    if (app->input.keys[K_D] == BUTTON_PRESSED)
        app->camera.ProcessInput(CameraInput::Right);
    app->camera.UpdateCamera();

    glBindBuffer(GL_UNIFORM_BUFFER, app->cbuffer.handle);
    app->cbuffer.data = (u8*)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
    app->cbuffer.head = 0;

    //--Global Params--
    app->globalParamsOffset = app->cbuffer.head;
    PushVec3(app->cbuffer, app->camera.cameraPos);
    PushUInt(app->cbuffer, app->lights.size());
    for (u32 i = 0; i < app->lights.size(); ++i)
    {
        AlignHead(app->cbuffer, sizeof(vec4));
        Light& light = app->lights[i];
        PushUInt(app->cbuffer, light.type);
        PushVec3(app->cbuffer, light.position);
        PushVec3(app->cbuffer, light.direction);
        PushVec3(app->cbuffer, light.ambient);
        PushVec3(app->cbuffer, light.diffuse);
        PushVec3(app->cbuffer, light.specular);
        PushFloat(app->cbuffer, light.constant);
    }
    app->globalParamsSize = app->cbuffer.head - app->globalParamsOffset;

    //--Local Params--
    for (int i = 0; i < app->entities.size(); ++i)
    {
        AlignHead(app->cbuffer, app->uniformBufferAlignment);

        Entity& entity = app->entities[i];
        glm::mat4 world = entity.worldMatrix;
        glm::mat4 MVP = app->camera.projection * app->camera.view * world;

        entity.localParamsOffset = app->cbuffer.head;
        PushMat4(app->cbuffer, world);
        PushMat4(app->cbuffer, MVP);
        entity.localParamsSize = app->cbuffer.head - entity.localParamsOffset;
    }

    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Render(App* app)
{
    //--------------Render Loop Logic--------------
    //-Bind framebuffer
    //-Clear & enable tests
    //-Geometry pass & fill uniforms
    //-Bind framebuffer
    //-Clear & disable tests
    //-Lighting pass & bind textures
    //-Clear
    //-Post processing pass

    GeometryPass(app);
    LightingPass(app);
    PostProcessingPass(app);
}

void GeometryPass(App* app)
{
    glBindFramebuffer(GL_FRAMEBUFFER, app->framebufferHandle);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, app->displaySize.x, app->displaySize.y);

    //Global parameters binding buffer
    glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(0), app->cbuffer.handle, app->globalParamsOffset, app->globalParamsSize);

    //Per entity
    for (int i = 0; i < app->entities.size(); ++i)
    {
        //Local parameters binding buffer
        glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(1), app->cbuffer.handle, app->entities[i].localParamsOffset, app->entities[i].localParamsSize);

        Program& texturedMeshProgram = app->programs[app->texturedMeshProgramIdx];
        glUseProgram(texturedMeshProgram.handle);
        Model& model = app->models[app->entities[i].modelIdx];
        Mesh& mesh = app->meshes[model.meshIdx];

        //Per submesh
        for (u32 i = 0; i < mesh.submeshes.size(); ++i)
        {
            GLuint VAO = FindVAO(mesh, i, texturedMeshProgram);
            glBindVertexArray(VAO);

            u32 submeshMaterialIdx = model.materialIdx[i];
            Material& submeshMaterial = app->materials[submeshMaterialIdx];

            glActiveTexture(GL_TEXTURE0);
            //diffuse
            glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTextureIdx].handle);
            glUniform1i(app->programUniformDiffuse, 0);
            //specular
            glUniform3fv(glGetUniformLocation(texturedMeshProgram.handle, "material.specular"), 1, glm::value_ptr(submeshMaterial.specular));
            //shininess
            glUniform1f(glGetUniformLocation(texturedMeshProgram.handle, "material.shininess"), submeshMaterial.smoothness);

            Submesh& submesh = mesh.submeshes[i];
            glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
        }
    }
}

void LightingPass(App* app)
{
    glBindFramebuffer(GL_FRAMEBUFFER, app->framebufferPostProcessingHandle);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(app->programs[app->texturedQuadProgramIdx].handle);
    glBindVertexArray(app->vaoIdx);

    //Render target
    glUniform1i(app->programUniformRenderTarget, app->currentRenderTarget);

    //Position attachment
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, app->positionAttachmentHandle);
    glUniform1i(app->programUniformLightingPosition, 0);

    //Normal attachment
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, app->normalAttachmentHandle);
    glUniform1i(app->programUniformLightingNormal, 1);

    //Albedo attachment
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, app->albedoAttachmentHandle);
    glUniform1i(app->programUniformLightingAlbedo, 2);

    //Specular attachment
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, app->specularAttachmentHandle);
    glUniform1i(app->programUniformLightingSpec, 3);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
}

void PostProcessingPass(App* app)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(app->programs[app->texturedQuadProgramIdx].handle);
    glBindVertexArray(app->vaoIdx);

    //Final color attachment
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, app->finalColorAttachmentHandle);
    glUniform1i(app->programUniformPostProcessing, 0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
}
