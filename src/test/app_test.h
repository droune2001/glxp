#ifndef _APP_TEST_2018_12_03_H_
#define _APP_TEST_2018_12_03_H_

#include "app.h"
#include "arcball_camera.h"
#include "tiny_obj_loader.h"

#include <vector>
#include <map>
#include <memory>

class AppTest : public App
{
public:

    bool init(int framebuffer_width, int framebuffer_height) override;
    void shutdown() override;
    void run(float dt) override;

    void onWindowSize(GLFWwindow* window, int w, int h) override;
    void onFramebufferSize(GLFWwindow* window, int w, int h) override;
    void onKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods) override;
    void onMouseClick(GLFWwindow* window, int button, int action, int mods) override;
    void onMouseMove(GLFWwindow* window, double mouse_x, double mouse_y) override;
    void onMouseScroll(GLFWwindow* window, double xoffset, double yoffset) override;

    struct DrawItem
    {
        unsigned int vao = 0;

        unsigned int index_buffer_id = 0;
        unsigned int vertex_buffer_id = 0;

        unsigned int nb_elements = 0;

        unsigned int picking_id = 0xffffffff;
    };

    using DrawItemSharedPtr = std::shared_ptr<DrawItem>;
    using DrawItemArray = std::vector<DrawItemSharedPtr>;
    using DrawItemKey = std::string;
    using DrawItemMap = std::map<DrawItemKey, DrawItemSharedPtr>;

private:

    bool load_obj(const char *filename);
    bool load_gltf(const char *filename);
    bool load_shaders();
    
    // Adds all the objects in an OBJ into the objects containers.
    void add_OBJ_to_scene(
        const tinyobj::attrib_t &obj_attribs,
        std::vector<tinyobj::shape_t> &obj_shapes,
        std::vector<tinyobj::material_t> &obj_material,
        bool normalize_size);
private:

    struct program 
    {
        unsigned int program_id = 0;

        int attrib_in_position = -1;
        int attrib_in_color = -1;
        int attrib_in_normal = -1;
        int attrib_in_texcoord = -1;

        int uni_model = -1;
        int uni_view = -1;
        int uni_proj = -1;
        int uni_tex = -1;
    };
    
    program _simple_program;
    
    DrawItemArray _v_objects;
    DrawItemMap _m_objects;
    unsigned int _current_picking_id = 1; // 0 and 0xffffffff are reserved.

    double _mouse_x = 0;
    double _mouse_y = 0;
    bool _mouse_pressed = false;

    ArcballCamera _camera;

    int _window_width = 0;
    int _window_height = 0;
    int _fb_width = 0;
    int _fb_height = 0;
};

#endif // _APP_TEST_2018_12_03_H_
