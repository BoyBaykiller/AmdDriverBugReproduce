#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    std::cout << message << '\n';
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

    uint32_t width = 800;
    uint32_t height = 800;
    GLFWwindow* window = glfwCreateWindow(width, height, "Repro", nullptr, nullptr);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)(glfwGetProcAddress));

    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(MessageCallback, 0);

    bool shouldReproduce = true;
    {
        int32_t textureCount = 10000;
        for (int32_t i = 0; i < textureCount; i++)
        {
            uint32_t texture;
            glCreateTextures(GL_TEXTURE_2D, 1, &texture);
            glTextureStorage2D(texture, 1, GL_RGBA8, 4, 4);

            uint64_t handle = glGetTextureHandleARB(texture);
            if (shouldReproduce)
            {
                glMakeTextureHandleResidentARB(handle);
            }
        }
    }

    glfwSetTime(0.0);
    int32_t fpsCounter = 0;
    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetTime() > 1.0)
        {
            char title[16];
            sprintf(title, "FPS = %d", fpsCounter);
            glfwSetWindowTitle(window, title);

            fpsCounter = 0;
            glfwSetTime(0.0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        fpsCounter++;
    }

    return 0;
}