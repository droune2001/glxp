#ifndef _APP_2018_12_03_H_
#define _APP_2018_12_03_H_

struct GLFWwindow;

class App
{
public:
    
    virtual bool init(int framebuffer_width, int framebuffer_height) = 0;
    virtual void shutdown() = 0;
    virtual void run(float dt) = 0;

    // callbacks
    virtual void onWindowSize(GLFWwindow* window, int w, int h) = 0;
    virtual void onFramebufferSize(GLFWwindow* window, int w, int h) = 0;
    virtual void onKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods) = 0;
    virtual void onMouseClick(GLFWwindow* window, int button, int action, int mods) = 0;
    virtual void onMouseMove(GLFWwindow* window, double mouse_x, double mouse_y) = 0;
    virtual void onMouseScroll(GLFWwindow* window, double xoffset, double yoffset) = 0;
};

#endif // _APP_2018_12_03_H_
