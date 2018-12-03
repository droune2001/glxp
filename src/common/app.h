#ifndef _APP_2018_12_03_H_
#define _APP_2018_12_03_H_

struct GLFWwindow;

class App
{
public:
    
    virtual bool init() = 0;
    virtual void draw(int width, int height) = 0;

    virtual void onMousePress(int x, int y) = 0;
    virtual void onKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods) = 0;
};

#endif // _APP_2018_12_03_H_
