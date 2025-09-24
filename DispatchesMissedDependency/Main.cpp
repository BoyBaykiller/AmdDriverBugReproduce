#include <cstdio>
#include <cstring>
#include "../Common/Common.h"

struct DispatchIndirectCommand
{
    unsigned NumGroupsX;
    unsigned NumGroupsY;
    unsigned NumGroupsZ;
};

struct GpuWavefrontPTHeader
{
    DispatchIndirectCommand DispatchCommand;
    unsigned Counts[2];
    unsigned PingPongIndex;    
};


struct GpuWavefrontRay
{
    float PreviousIOROrTraverseCost;
};

const char* computeSrcCode = R"(
#version 460 core

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

struct GpuDispatchCommand
{
    int NumGroupsX;
    int NumGroupsY;
    int NumGroupsZ;
};

struct GpuWavefrontRay
{
    float PreviousIOROrTraverseCost;
};

layout(std430, binding = 0) buffer HeaderSSBO
{
    GpuDispatchCommand DispatchCommand;
    uint Counts[2];
    uint PingPongIndex;
} headerSSBO;

layout(std430, binding = 1) buffer WavefrontRaySSBO
{
    GpuWavefrontRay Rays[];
} wavefrontRaySSBO;

void main()
{
    uint pingPongIndex = headerSSBO.PingPongIndex;
    if (gl_GlobalInvocationID.x >= headerSSBO.Counts[pingPongIndex])
    {
        return;
    }

    if (gl_GlobalInvocationID.x == 0)
    {
        // reset the arguments that were used to launch this compute shader
        atomicAdd(headerSSBO.DispatchCommand.NumGroupsX, -int(gl_NumWorkGroups.x));
    }

    wavefrontRaySSBO.Rays[gl_GlobalInvocationID.x].PreviousIOROrTraverseCost++;

    uint index = atomicAdd(headerSSBO.Counts[1 - pingPongIndex], 1u);

    // Add workgroup for the next dispatch
    if (index % gl_WorkGroupSize.x == 0)
    {
        atomicAdd(headerSSBO.DispatchCommand.NumGroupsX, 1);
    }
})";

int main()
{
    GLFWwindow* window = CreateOpenGLWindow(800, 800, "Repro");
    
    uint32_t shaderProgram;
    {
        uint32_t computeShader = CompileShader(GL_COMPUTE_SHADER, computeSrcCode);

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, computeShader);
        glLinkProgram(shaderProgram);
    }

    unsigned rayCount = 1024 * 1024;

    unsigned headerBuffer;
    {
        glCreateBuffers(1, &headerBuffer);

        GpuWavefrontPTHeader header = {
            .DispatchCommand = {
                .NumGroupsX = rayCount / 32,
                .NumGroupsY = 1,
                .NumGroupsZ = 1,
            },
            .Counts = {0, rayCount},
            .PingPongIndex = 0,
        };
        glNamedBufferStorage(headerBuffer, sizeof(GpuWavefrontPTHeader), &header, GL_DYNAMIC_STORAGE_BIT);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, headerBuffer);
    }

    unsigned wavefrontRayBuffer;
    {
        glCreateBuffers(1, &wavefrontRayBuffer);
        glNamedBufferStorage(wavefrontRayBuffer, sizeof(GpuWavefrontRay) * rayCount, nullptr, GL_DYNAMIC_STORAGE_BIT);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, wavefrontRayBuffer);
    }

    GpuWavefrontRay* downloadedRays = new GpuWavefrontRay[rayCount];
    while (!glfwWindowShouldClose(window))
    {
        float rayClear = 0.0f;
        glClearNamedBufferData(wavefrontRayBuffer, GL_R32F, GL_RED, GL_FLOAT, &rayClear);

        for (int i = 1; i < 7; i++)
        {
            unsigned pingPongIndex = i % 2;
            
            glNamedBufferSubData(headerBuffer, offsetof(GpuWavefrontPTHeader, PingPongIndex), sizeof(unsigned), &pingPongIndex); // upload ping pong index
            
            unsigned counterClear = 0u;
            glNamedBufferSubData(headerBuffer, offsetof(GpuWavefrontPTHeader, Counts[0]) + (1 - pingPongIndex) * sizeof(unsigned), sizeof(unsigned), &counterClear); // reset counter
            
            glUseProgram(shaderProgram);
            glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, headerBuffer);
            glDispatchComputeIndirect(offsetof(GpuWavefrontPTHeader, DispatchCommand));
            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            bool reproduce = true; // this will make the test fail
            if (!reproduce)
            {
                // If we dont explicitly sync with glFinish results are wrong
                glFinish();
            }

            if (i == 6)
            {
                glGetNamedBufferSubData(wavefrontRayBuffer, 0, sizeof(GpuWavefrontRay) * rayCount, downloadedRays);
                for (int i = 0; i < rayCount; i++)
                {
                    float v = downloadedRays[i].PreviousIOROrTraverseCost;
                    if (v != 6)
                    {
                        printf("Test failed, all values should be equal to 6.0f, [%d] = %f\n. ", i, v);
                        return 0;
                    }
                }
            }
            printf("No errors detected...\r");
            fflush(stdout);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}