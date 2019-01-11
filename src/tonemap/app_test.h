#ifndef _APP_TEST_2018_12_03_H_
#define _APP_TEST_2018_12_03_H_

#include "app.h"
#include "arcball_camera.h"
#include "tiny_obj_loader.h"
#include "procgen.h"

#include <vector>
#include <map>
#include <memory>

class AppTest : public App
{
public:

    AppTest(void *options);

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


        glm::vec3 bbox_min;
        glm::vec3 bbox_max;

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
    bool load_textures();
    bool create_framebuffers();
    bool recreate_framebuffers();

    void add_to_scene(const std::string &name, const IndexedMesh &mesh);

    // Adds all the objects in an OBJ into the objects containers.
    void add_OBJ_to_scene(
        const tinyobj::attrib_t &obj_attribs,
        std::vector<tinyobj::shape_t> &obj_shapes,
        std::vector<tinyobj::material_t> &obj_material,
        bool normalize_size);

    void do_gui();
    void update_camera(float dt);

    void update_tonemap_curves();

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

    unsigned int _tex;
    unsigned int _3dlut_tex;
    unsigned int _sampler;
    unsigned int _nearest_sampler;
    unsigned int _linear_sampler;

    std::string _scene_path;

    program _simple_program;
    unsigned int _fullscreen_program;
    unsigned int _tonemap_program;
    unsigned int _3dlut_program;
    unsigned int _uni_width;
    unsigned int _uni_height;
    unsigned int _uni_splitview;
    unsigned int _dummy_vao;

    // framebuffers
    unsigned int _fb_hdr; // main rendering here
    unsigned int _fbtex_hdr_color;
    unsigned int _fbtex_hdr_depth;

    unsigned int _fb_ldr; // tone mapping here
    unsigned int _fbtex_ldr_color;
    unsigned int _fbtex_ldr_depth;


    DrawItemArray _v_objects;
    DrawItemMap _m_objects;
    unsigned int _current_picking_id = 1; // 0 and 0xffffffff are reserved.

    double _mouse_x = 0;
    double _mouse_y = 0;
    bool _mouse_pressed = false;
    
    Camera *current_camera() { return (_current_camera_idx != -1) ? _cameras[_current_camera_idx].get() : nullptr; };
    int _current_camera_idx = 0;
    std::vector<std::unique_ptr<Camera>> _cameras;

    glm::vec3 scene_bbox_min;
    glm::vec3 scene_bbox_max;

    int _window_width = 0;
    int _window_height = 0;
    int _fb_width = 0;
    int _fb_height = 0;

    // memory
    uint64_t gpu_memory = 0;

    // tonemap curves
    std::vector<float> _curve0;
    std::vector<float> _curve1;
    std::vector<float> _curve2;
    std::vector<float> _curve3;

    std::vector<glm::vec3> _3dlut;
    bool _draw3dlut = true;
    int _current_view = 0;
};

#endif // _APP_TEST_2018_12_03_H_
