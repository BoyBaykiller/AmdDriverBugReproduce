#include <array>
#include "Common.h"

int CompileShader(GLenum shaderType, const char* srcCode)
{
    uint32_t shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &srcCode, 0);
    glCompileShader(shader);

    auto infoLogBuffer = std::array<char, 4096>({ '\0' });
    glGetShaderInfoLog(shader, infoLogBuffer.size(), nullptr, infoLogBuffer.data());
    printf(infoLogBuffer.data());

    return shader;
}

static void GLAPIENTRY GLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    printf("%s\n", message);
}

GLFWwindow* CreateOpenGLWindow(uint32_t width, uint32_t height, const char* name)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(width, height, name, nullptr, nullptr);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)(glfwGetProcAddress));

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(GLDebugCallback, 0);

    return window;
}