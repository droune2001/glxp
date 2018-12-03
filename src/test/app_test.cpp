#include "app_test.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm_usage.h"

bool AppTest::init()
{
    return true;
}

void AppTest::draw(int width, int height)
{
    glViewport(0, 0, width, height);
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void AppTest::onMousePress(int x, int y)
{

}

void AppTest::onKeyboard(GLFWwindow * window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}
