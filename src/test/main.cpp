#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm_usage.h"

#include <stdlib.h>
#include <stdio.h>


static
void error_callback(int error, const char* description)
{
    printf("Error: %s\n", description);
}

static 
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static
void run(GLFWwindow* window)
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    
}

int main(int argc, char **argv)
{
    if (glfwInit() != GLFW_TRUE)
        return EXIT_FAILURE;
    
    glfwSetErrorCallback(error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwSetKeyCallback(window, key_callback);
    //glfwSetFramebufferSizeCallback
    //glfwSetWindowCloseCallback

    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK)
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwSwapInterval(1);

    // INIT GL RESOURCES

    while (!glfwWindowShouldClose(window))
    {
        // Keep running
        run(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
