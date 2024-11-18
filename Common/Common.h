#include <glad/glad.h>
#include <GLFW/glfw3.h>

int CompileShader(GLenum shaderType, const char* srcCode);
GLFWwindow* CreateOpenGLWindow(uint32_t width, uint32_t height, const char* name);