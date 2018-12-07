#include "app_test.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm_usage.h"
#include "tiny_obj_loader.h"
#include "tiny_gltf.h"
#include "gl_utils.h"
#include "utils.h"

#include <vector>
#include <fstream>

bool AppTest::load_obj(const char *filename)
{
    tinyobj::attrib_t obj_attribs;
    std::vector<tinyobj::shape_t> obj_shapes;
    std::vector<tinyobj::material_t> obj_material;

    std::string warn;
    std::string err;

    std::ifstream ifs(filename);
    if (ifs.fail())
    {
        printf("FAILED to open file: %s\n", filename);
        return false;
    }
    tinyobj::MaterialFileReader mtlReader("./");
    printf("Loading \"%s\"...\n", filename);
    bool ret = tinyobj::LoadObj(&obj_attribs, &obj_shapes, &obj_material, &warn, &err, &ifs, &mtlReader, true, true);
    if (!warn.empty())
    {
        printf("WARN: %s\n", warn.c_str());
    }
    if (!err.empty())
    {
        printf("ERR: %s\n", err.c_str());
    }
    if (!ret)
    {
        printf("FAILED to parse: %s\n", filename);
        return false;
    }
    else
    {
        printf("SUCCESS!\n");
        // post-process tinyobj loaded model into our own format.

        printf("# of vertices         = %zd\n", obj_attribs.vertices.size() / 3);
        printf("# of normals          = %zd\n", obj_attribs.normals.size() / 3);
        printf("# of texcoords        = %zd\n", obj_attribs.texcoords.size() / 2);
        printf("# of shapes           = %zd\n", obj_shapes.size());
        for (size_t s = 0; s < obj_shapes.size(); ++s)
        {
            printf("   [%zd] # of indices  = %zd\n", s, obj_shapes[s].mesh.indices.size());
        }
        printf("# of materials        = %zd\n", obj_material.size());

        return true;
    }
}

bool AppTest::load_gltf(const char *filename)
{
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    printf("Loading \"%s\"...\n", filename);
    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
    if (!warn.empty())
    {
        printf("WARN: %s\n", warn.c_str());
    }
    if (!err.empty())
    {
        printf("ERR: %s\n", err.c_str());
    }
    if (!ret)
    {
        printf("FAILED to parse: %s\n", filename);
        return false;
    }
    else
    {
        printf("SUCCESS!\n");
        printf("Content dump:\n");
        printf("# accessors: %zd\n", model.accessors.size());
        printf("# animations: %zd\n", model.animations.size());
        printf("# buffers: %zd\n", model.buffers.size());
        printf("# bufferViews: %zd\n", model.bufferViews.size());
        printf("# materials: %zd\n", model.materials.size());
        printf("# meshes: %zd\n", model.meshes.size());
        printf("# nodes: %zd\n", model.nodes.size());
        printf("# textures: %zd\n", model.textures.size());
        printf("# images: %zd\n", model.images.size());
        printf("# skins: %zd\n", model.skins.size());
        printf("# samplers: %zd\n", model.samplers.size());
        printf("# cameras: %zd\n", model.cameras.size());
        printf("# scenes: %zd\n", model.scenes.size());
        printf("# lights: %zd\n", model.lights.size());
        printf("--------------------\n");

        auto PrintNode = [](const tinygltf::Model &model, const tinygltf::Node &node, int nodeIdx) 
        {
            printf("* [%d] Node Name: %s\n", nodeIdx, node.name.c_str());
            if (node.matrix.size() == 16)
            {
                printf("* [%d]   Node Matrix: %.2f %.2f %.2f %.2f ...\n", nodeIdx, node.matrix[0], node.matrix[1], node.matrix[2], node.matrix[3]);
            }
            else
            {
                if (node.translation.size() > 0)
                    printf("* [%d]   Node Translation: %.2f %.2f %.2f\n", nodeIdx, node.translation[0], node.translation[1], node.translation[2]);
                if (node.rotation.size() > 0)
                    printf("* [%d]   Node Rotate: %.2f %.2f %.2f\n", nodeIdx, node.rotation[0], node.rotation[1], node.rotation[2]);
                if (node.scale.size() > 0)
                    printf("* [%d]   Node Scale: %.2f %.2f %.2f\n", nodeIdx, node.scale[0], node.scale[1], node.scale[2]);
            }

            if (node.mesh >= 0)
            {
                const auto &mesh = model.meshes[node.mesh];
                printf("* [%d]   Mesh Name %s\n", nodeIdx, mesh.name.c_str());
                printf("* [%d]   Mesh Nb Primitives %zd\n", nodeIdx, mesh.primitives.size());
            }
        };




        if (!model.scenes.empty() && model.defaultScene >= 0)
        {
            printf("Default scene:\n");
            printf("* Name: %s\n", model.scenes[model.defaultScene].name.c_str());
            const auto &scene = model.scenes[model.defaultScene];
            printf("* Nb Nodes: %zd\n", scene.nodes.size());
            for (auto nodeIdx : scene.nodes)
            {
                const auto &node = model.nodes[nodeIdx];
                PrintNode(model, node, nodeIdx);
            }
        }

        if (!model.nodes.empty())
        {
            printf("ALL Nodes:\n");
            int i = 0;
            for (const auto &node : model.nodes)
            {
                PrintNode(model, node, i++);
            }
        }

        return true;
    }
}

bool AppTest::load_shaders()
{
    auto vs = utils::read_file_content("./simple.vert");
    auto fs = utils::read_file_content("./simple.frag");

    GLuint vs_id = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs_id = glCreateShader(GL_FRAGMENT_SHADER);

    if (!glutils::compile_shader(vs_id, vs.data(), vs.size()))
        return false;
    if (!glutils::compile_shader(fs_id, fs.data(), fs.size()))
        return false;

    GLuint prog_id = glCreateProgram();

    if (!glutils::link_program(prog_id, vs_id, fs_id))
        return false;

    _simple_program.vertex_shader_id = vs_id;
    _simple_program.fragment_shader_id = fs_id;
    _simple_program.program_id = prog_id;

    _simple_program.attrib_in_position = glGetAttribLocation(prog_id, "inPosition");
    _simple_program.attrib_in_color = glGetAttribLocation(prog_id, "inColor");
    _simple_program.attrib_in_texcoord = glGetAttribLocation(prog_id, "inTexCoord");

    _simple_program.uni_model = glGetUniformLocation(prog_id, "model");
    _simple_program.uni_view = glGetUniformLocation(prog_id, "view");
    _simple_program.uni_proj = glGetUniformLocation(prog_id, "proj");
    _simple_program.uni_tex = glGetUniformLocation(prog_id, "tex");

    return true;
}

bool AppTest::init(int framebuffer_width, int framebuffer_height)
{
    _fb_width = framebuffer_width;
    _fb_height = framebuffer_height;

    load_shaders();

    // OBJ
    std::string filename = "bunny.obj";
    //std::string filename = "sponza.obj";
    bool ret = load_obj(filename.c_str());

    // GLTF
    //std::string filename = "scene.gltf";
    //bool ret = load_gltf(filename.c_str());

    return ret;
}

void AppTest::shutdown()
{
    // release shaders
    // release buffers
}

void AppTest::run()
{
    glViewport(0, 0, _fb_width, _fb_height);
    glClearColor(0.2f, 0.2f, 0.2f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // camera

}

void AppTest::onWindowSize(GLFWwindow * window, int w, int h)
{
    _window_width = w;
    _window_height = h;
}

void AppTest::onFramebufferSize(GLFWwindow* window, int w, int h)
{
    _fb_width = w;
    _fb_height = h;
}

void AppTest::onKeyboard(GLFWwindow * window, int key, int scancode, int action, int mods)
{
    (void)window;
    (void)scancode;
    (void)mods;
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        // Move camera
        float mv_x = 0, mv_y = 0, mv_z = 0;
        if (key == GLFW_KEY_K)
            mv_x += 1;
        else if (key == GLFW_KEY_J)
            mv_x += -1;
        else if (key == GLFW_KEY_L)
            mv_y += 1;
        else if (key == GLFW_KEY_H)
            mv_y += -1;
        else if (key == GLFW_KEY_P)
            mv_z += 1;
        else if (key == GLFW_KEY_N)
            mv_z += -1;
        // camera.move(mv_x * 0.05, mv_y * 0.05, mv_z * 0.05);
        
        // Close window
        if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
    }
}

void AppTest::onMouseClick(GLFWwindow* window, int button, int action, int mods)
{
    (void)window;
    (void)mods;
    //if (button == GLFW_MOUSE_BUTTON_LEFT) {
    //    if (action == GLFW_PRESS) {
    //        mouseLeftPressed = true;
    //        trackball(prev_quat, 0.0, 0.0, 0.0, 0.0);
    //    }
    //    else if (action == GLFW_RELEASE) {
    //        mouseLeftPressed = false;
    //    }
    //}
    //if (button == GLFW_MOUSE_BUTTON_RIGHT) {
    //    if (action == GLFW_PRESS) {
    //        mouseRightPressed = true;
    //    }
    //    else if (action == GLFW_RELEASE) {
    //        mouseRightPressed = false;
    //    }
    //}
    //if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
    //    if (action == GLFW_PRESS) {
    //        mouseMiddlePressed = true;
    //    }
    //    else if (action == GLFW_RELEASE) {
    //        mouseMiddlePressed = false;
    //    }
    //}
}

void AppTest::onMouseMove(GLFWwindow* window, double mouse_x, double mouse_y)
{
    //(void)window;
    //float rotScale = 1.0f;
    //float transScale = 2.0f;

    //if (mouseLeftPressed) {
    //    trackball(prev_quat, rotScale * (2.0f * prevMouseX - width) / (float)width,
    //              rotScale * (height - 2.0f * prevMouseY) / (float)height,
    //              rotScale * (2.0f * mouse_x - width) / (float)width,
    //              rotScale * (height - 2.0f * mouse_y) / (float)height);

    //    add_quats(prev_quat, curr_quat, curr_quat);
    //}
    //else if (mouseMiddlePressed) {
    //    eye[0] -= transScale * (mouse_x - prevMouseX) / (float)width;
    //    lookat[0] -= transScale * (mouse_x - prevMouseX) / (float)width;
    //    eye[1] += transScale * (mouse_y - prevMouseY) / (float)height;
    //    lookat[1] += transScale * (mouse_y - prevMouseY) / (float)height;
    //}
    //else if (mouseRightPressed) {
    //    eye[2] += transScale * (mouse_y - prevMouseY) / (float)height;
    //    lookat[2] += transScale * (mouse_y - prevMouseY) / (float)height;
    //}

    //// Update mouse point
    //prevMouseX = mouse_x;
    //prevMouseY = mouse_y;
}

void AppTest::onMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
    //auto& dist = app->camera.dist;

    //dist -= (float)yoffset * 0.3f;
    //dist -= (float)xoffset * 0.3f;
    //dist = glm::clamp(dist, 0.5f, 10.f);
}
