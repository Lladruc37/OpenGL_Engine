//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "engine.h"
#include <imgui.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <iostream>

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
    app->programs.push_back(program);

    Program& texturedMeshProgram = app->programs[app->texturedMeshProgramIdx];
    //app->programUniformTexture = glGetUniformLocation(texturedGeometryProgram.handle, "uTexture");
    texturedMeshProgram.vertexInputLayout.attributes.push_back({ 0,3 }); //pos
    texturedMeshProgram.vertexInputLayout.attributes.push_back({ 2,2 }); //tex Coords

    //Vertex Shader Layout
    int attributeCount = 0;
    glGetProgramiv(program.handle, GL_ACTIVE_ATTRIBUTES, &attributeCount);
    for (int i = 0; i < attributeCount; ++i)
    {
        glGetActiveAttrib(program.handle, i, ARRAY_COUNT(attributeName), &attributeNameLength, &attributeSize, &attributeType, attributeName);
    }

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
    else
    {
        return UINT32_MAX;
    }
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

            assert(attributeWasLinked); //The submesh should provide an attribute for each vertex inputs

            //TODO: Shader reflection
            //glGetProgramiv(program.handle, GL_ACTIVE_ATTRIBUTES, &attributeCount);
            //glGetActiveAttrib(program.handle, i,
            //    ARRAY_COUNT(attributeName),
            //    &attributeNameLength,
            //    &attributeSize,
            //    &attributeType,
            //    attributeName);
            //attributeLocation = glGetAttribLocation(programHandle, attributeName);

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
    // TODO: Initialize your resources here!
    // - vertex buffers
    // - element/index buffers
    // - vaos
    // - programs (and retrieve uniform indices)
    // - textures

    app->mode = Mode_TexturedQuad;

    //INFO
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

    //Debugging
    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    app->modelIdx = LoadModel(app,"Patrick/Patrick.obj");

    //Program initialization
    app->texturedMeshProgramIdx = LoadProgram(app, "shaders.glsl", /*"SHOW_TEXTURED_MESH"*/"TEXTURED_GEOMETRY");


    //Texture initialization
    app->diceTexIdx = LoadTexture2D(app, "dice.png");
    app->whiteTexIdx = LoadTexture2D(app, "color_white.png");
    app->blackTexIdx = LoadTexture2D(app, "color_black.png");
    app->normalTexIdx = LoadTexture2D(app, "color_normal.png");
    app->magentaTexIdx = LoadTexture2D(app, "color_magenta.png");
}

void Gui(App* app)
{
    ImGui::Begin("Info");
    ImGui::Text("FPS: %f", 1.0f/app->deltaTime);
    ImGui::End();
    //TODO: INFO PANEL
    if (ImGui::BeginMenu("ImGui"))
    {
        ImGui::BulletText(app->info.version.c_str());
        ImGui::BulletText(app->info.renderer.c_str());
        ImGui::BulletText(app->info.vendor.c_str());
        ImGui::BulletText(app->info.GLSLVersion.c_str());
        ImGui::Text("Extensions:");
        ImGui::Text(app->info.extensions.c_str());
        ImGui::EndMenu();
    }
}

void Update(App* app)
{
    // You can handle app->input keyboard/mouse here
}

void Render(App* app)
{
    switch (app->mode)
    {
    case Mode_TexturedQuad:
    {
        // TODO: Draw your textured quad here!
        // - clear the framebuffer
        // - set the viewport
        // - set the blending state
        // - bind the texture into unit 0
        // - bind the program 
        //   (...and make its texture sample from unit 0)
        // - bind the vao
        // - glDrawElements() !!!


        Program& texturedMeshProgram = app->programs[app->texturedMeshProgramIdx];
        glUseProgram(texturedMeshProgram.handle);
        Model& model = app->models[app->modelIdx];
        Mesh& mesh = app->meshes[model.meshIdx];

        for (u32 i = 0; i < mesh.submeshes.size(); ++i)
        {
            GLuint VAO = FindVAO(mesh, i, texturedMeshProgram);
            glBindVertexArray(VAO);

            u32 submeshMaterialIdx = model.materialIdx[i];
            Material& submeshMaterial = app->materials[submeshMaterialIdx];

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTextureIdx].handle);
            glUniform1i(app->programUniformTexture, 0);

            Submesh& submesh = mesh.submeshes[i];
            glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
        }

        //OLD
        //glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //glViewport(0, 0, app->displaySize.x, app->displaySize.y);

        //Program& programTextureGeometry = app->programs[app->texturedMeshProgramIdx];
        //glUseProgram(programTextureGeometry.handle);
        //glBindVertexArray(app->vao);

        //glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        //glUniform1i(app->programUniformTexture, 0);
        //glActiveTexture(GL_TEXTURE0);
        //GLuint textureHandle = app->textures[app->diceTexIdx].handle;
        //glBindTexture(GL_TEXTURE_2D, textureHandle);

        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

        //glBindVertexArray(0);
        //glUseProgram(0);
    }
    break;

    default:;
    }
}

