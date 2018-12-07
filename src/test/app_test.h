#ifndef _APP_TEST_2018_12_03_H_
#define _APP_TEST_2018_12_03_H_

#include "app.h"

class AppTest : public App
{
    bool init(int framebuffer_width, int framebuffer_height) override;
    void shutdown() override;
    void run() override;

    void onWindowSize(GLFWwindow* window, int w, int h) override;
    void onFramebufferSize(GLFWwindow* window, int w, int h) override;
    void onKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods) override;
    void onMouseClick(GLFWwindow* window, int button, int action, int mods) override;
    void onMouseMove(GLFWwindow* window, double mouse_x, double mouse_y) override;
    void onMouseScroll(GLFWwindow* window, double xoffset, double yoffset) override;

private:

    bool load_obj(const char *filename);
    bool load_gltf(const char *filename);
    bool load_shaders();

private:

    struct program 
    {
        unsigned int program_id = 0;
        unsigned int vertex_shader_id = 0;
        unsigned int fragment_shader_id = 0;

        int attrib_in_position = -1;
        int attrib_in_color = -1;
        int attrib_in_texcoord = -1;

        int uni_model = -1;
        int uni_view = -1;
        int uni_proj = -1;
        int uni_tex = -1;
    };
    
    program _simple_program;

    int _window_width = 0;
    int _window_height = 0;
    int _fb_width = 0;
    int _fb_height = 0;
};

#endif // _APP_TEST_2018_12_03_H_
