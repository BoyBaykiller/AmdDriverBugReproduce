#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

const char* vertexSrcCode = R"(
#version 460 core

const vec2 VertexPositions[] =
{
    vec2( -1.0, -1.0 ),
    vec2(  3.0, -1.0 ),
    vec2( -1.0,  3.0 )
};

out vec2 TexCoord;

void main()
{
    gl_Position = vec4(VertexPositions[gl_VertexID], 0.0, 1.0);
    TexCoord = gl_Position.xy * 0.5 + 0.5;
}
)";

const char* fragmentSrcCode = R"(
#version 460 core
#extension GL_ARB_bindless_texture : require

layout(location = 0) out vec4 FragColor;

layout(std430, binding = 0) restrict readonly buffer SSBO
{
    sampler2D Texture;
};

in vec2 TexCoord;

void main()
{
    // Note: Black screen only happens at Lod > 0
    int lod = 1;
    vec3 color = textureLod(Texture, TexCoord, lod).rgb;

    FragColor = vec4(color, 1.0);
}
)";

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    std::cout << message << '\n';
}

struct Vector4
{
    float r, g, b, a;
};

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

    uint32_t levels = 9;
    uint32_t size = 1 << levels;

    GLFWwindow* window = glfwCreateWindow(size, size, "Reproduce-Bug", nullptr, nullptr);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)(glfwGetProcAddress));

    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(MessageCallback, 0);

    uint32_t shaderProgram;
    {
        uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexSrcCode, 0);
        glCompileShader(vertexShader);

        uint32_t fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentSrcCode, 0);
        glCompileShader(fragmentShader);

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
    }

    {
        uint32_t dummyVAO;
        glCreateVertexArrays(1, &dummyVAO);
        glBindVertexArray(dummyVAO);
    }

    bool shouldReproduceBug = false;
    // Note: Also happens with other compressed formats like COMPRESSED_RGBA_S3TC_DXT3_EXT
    GLenum internalFormat = shouldReproduceBug ? GL_COMPRESSED_RGBA_BPTC_UNORM : GL_RGBA8;

    uint32_t texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    glTextureStorage2D(texture, levels, internalFormat, size, size);
    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    Vector4* pixels = new Vector4[size * size];
    for (int i = 0; i < size * size; i++)
    {
        int y = i / size;
        int x = i % size;
        Vector4 color = {
            .r = x / (float)size,
            .g = y / (float)size,
            .b = 0.0f,
            .a = 1.0,
        };
        pixels[i] = color;
    }
    glTextureSubImage2D(texture, 0, 0, 0, size, size, GL_RGBA, GL_FLOAT, pixels);

    uint64_t handle = glGetTextureHandleARB(texture);
    glMakeTextureHandleResidentARB(handle);

    // Note: When putting this before generating the handle it behaves correctly regardless of <shouldReproduceBug>
    glGenerateTextureMipmap(texture);

    uint32_t ssbo;
    glCreateBuffers(1, &ssbo);
    glNamedBufferStorage(ssbo, sizeof(handle), &handle, GL_NONE);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

    while (!glfwWindowShouldClose(window))
    {
        glUseProgram(shaderProgram);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}