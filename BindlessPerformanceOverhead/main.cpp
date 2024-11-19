#include <iostream>
#include "../Common/Common.h"

int main()
{
    constexpr uint32_t width = 800;
    constexpr uint32_t height = 800;
    GLFWwindow* window = CreateOpenGLWindow(width, height, "Repro");

    bool shouldReproduce = false;
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