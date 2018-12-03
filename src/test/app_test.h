#ifndef _APP_TEST_2018_12_03_H_
#define _APP_TEST_2018_12_03_H_

#include "app.h"

class AppTest : public App
{
    bool init() override;
    void shutdown() override;
    void draw(int width, int height) override;

    void onMousePress(int x, int y) override;
    void onKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods) override;
};

#endif // _APP_TEST_2018_12_03_H_
