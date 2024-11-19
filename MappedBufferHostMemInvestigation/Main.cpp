#include <iostream>
#include "../Common/Common.h"

const char* vertexSrcCode = R"(
#version 460 core

void main()
{
    gl_Position = vec4(0.0);
})";

const char* fragmentSrcCode = R"(
#version 460 core

void main()
{
})";

int main()
{
    constexpr uint32_t width = 800;
    constexpr uint32_t height = 800;
    GLFWwindow* window = CreateOpenGLWindow(width, height, "Repro");

    uint32_t shaderProgram;
    {
        uint32_t vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSrcCode);
        uint32_t fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSrcCode);

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
    }

    uint32_t bufSize = 1 << 30; // 1GiB
    uint32_t buf;
    glCreateBuffers(1, &buf);

    // OpenGL has no API to know which backing memory heap is used for a buffer.
    // We can still tell if it's DEVICE or HOST memory from the FPS shown in the window title (HOST mem => lower FPS).
    // An other way is to look at "Dedicated GPU memory" and "Shared GPU memory" in Windows Task Manager.

    int testCase = 0;
    GLenum bufferAccessFlags = 0;
    switch (testCase)
    {
    case 0:
        // Without ReBAR/SAM => HOST memory (even when buffer is well below 256Mib)
        // With    ReBAR/SAM => DEVICE memory
        bufferAccessFlags = GL_MAP_WRITE_BIT;
        break;

    case 1:
        // Without ReBAR/SAM => HOST memory
        // With    ReBAR/SAM => HOST memory (Why does GL_MAP_COHERENT_BIT force HOST mem?)
        bufferAccessFlags = GL_MAP_WRITE_BIT | GL_MAP_COHERENT_BIT;
        break;

    case 2:
        // Without ReBAR/SAM => HOST memory (even when buffer is well below 256Mib)
        // With    ReBAR/SAM => HOST memory (was explained in https://vanguard.amd.com/project/feedback/view.html?cap=00de81e7400a4f968b2b7c7ed88e35b8&uf=45bf9ddf-f35f-4d7a-b97d-0c2cdbef4825&commentId=52c46a3b43d94550818a70f2ff16769c)
        bufferAccessFlags = GL_MAP_READ_BIT;
        break;

    case 3:
        // not mapped => DEVICE memory
        bufferAccessFlags = GL_NONE/* | GL_DYNAMIC_STORAGE_BIT*/;
        break;
    }

    if ((bufferAccessFlags & GL_MAP_WRITE_BIT) == GL_MAP_WRITE_BIT ||
        (bufferAccessFlags & GL_MAP_READ_BIT) == GL_MAP_READ_BIT)
    {
        bufferAccessFlags |= GL_MAP_PERSISTENT_BIT;
    }
    glNamedBufferStorage(buf, bufSize, nullptr, bufferAccessFlags);

    uint32_t vao;
    glCreateVertexArrays(1, &vao);
    glVertexArrayElementBuffer(vao, buf);

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

        glBindVertexArray(vao);
        glUseProgram(shaderProgram);
        glDrawElements(GL_TRIANGLES, bufSize / sizeof(uint32_t), GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(window);
        glfwPollEvents();

        fpsCounter++;
    }

    return 0;
}