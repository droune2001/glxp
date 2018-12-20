#include "app_test.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm_usage.h"
#include "tiny_obj_loader.h"
#include "tiny_gltf.h"
#include "gl_utils.h"
#include "utils.h"

#include <vector>
#include <fstream>

static std::string models_path = "../../../data/test/models/";
static std::string shaders_path = "../../../data/test/shaders/";

void AppTest::add_OBJ_to_scene(
    const tinyobj::attrib_t &obj_attribs, 
    std::vector<tinyobj::shape_t> &obj_shapes,
    std::vector<tinyobj::material_t> &obj_material,
    bool normalize_size)
{
    for (const auto &shape : obj_shapes)
    {
        const auto &object_name = shape.name;
        if (_m_objects.find(object_name) == _m_objects.end())
        {
            auto new_object = std::make_shared<DrawItem>();
            _v_objects.push_back(new_object);
            _m_objects[object_name] = new_object;
        }

        auto obj = _m_objects[object_name];

        //
        // compute bbox
        //
        glm::vec3 bbox_min(FLT_MAX, FLT_MAX, FLT_MAX);
        glm::vec3 bbox_max(FLT_MIN, FLT_MIN, FLT_MIN);
        for (size_t i = 0; i < obj_attribs.vertices.size() / 3; i += 3)
        {
            glm::vec3 position(obj_attribs.vertices[3 * i + 0], obj_attribs.vertices[3 * i + 1], obj_attribs.vertices[3 * i + 2]);
            bbox_min = glm::min(bbox_min, position);
            bbox_max = glm::max(bbox_max, position);
        }
        glm::vec3 middle = (bbox_max + bbox_min) / 2.0f;
        float scale_factor = 1.0f / glm::length(bbox_max - middle);

        obj->bbox_min = bbox_min;
        obj->bbox_max = bbox_max;

        struct vertex
        {
            glm::vec4 position; // 3 + ??
            glm::vec4 normal;  // 3 + ??
            glm::vec4 diffuse_color;
            glm::vec4 texcoords; // 2 + ??
        };

        // convert tinyobj_loader multi-index format to my own interleaved linear format
        std::vector<vertex> vertex_buffer;
        vertex_buffer.resize(obj_attribs.vertices.size() / 3); // model triangulated by tinyobj
        for (size_t i = 0; i < obj_attribs.vertices.size() / 3; ++i)
        {
            vertex &v = vertex_buffer[i];
            v.position = glm::vec4(obj_attribs.vertices[3 * i + 0], obj_attribs.vertices[3 * i + 1], obj_attribs.vertices[3 * i + 2], 1.0f);
            v.diffuse_color = glm::vec4(obj_attribs.colors[3 * i + 0], obj_attribs.colors[3 * i + 1], obj_attribs.colors[3 * i + 2], 1.0f);
            if (normalize_size)
            {
                v.position.xyz = scale_factor * (v.position.xyz - middle);
            }
        }

        std::vector<unsigned int> index_buffer;
        index_buffer.reserve(shape.mesh.indices.size());
        for (auto index : shape.mesh.indices)
        {
            if (index.vertex_index != -1)
            {
                int vi = index.vertex_index;
                int ni = index.normal_index;
                int ti = index.texcoord_index;

                index_buffer.push_back(index.vertex_index);

                // complete the vertex buffer with the other attributes, which may be indexed differently.
                // we may have to duplicate these in the process.
                vertex &v = vertex_buffer[vi];
                if (index.normal_index != -1)
                {
                    v.normal = glm::vec4(obj_attribs.normals[3 * ni + 0], obj_attribs.normals[3 * ni + 1], obj_attribs.normals[3 * ni + 2], 1.0f);
                }
                else
                {
                    v.normal = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); // default normal = UP
                }

                if (index.texcoord_index != -1)
                {
                    v.texcoords = glm::vec4(obj_attribs.texcoords[2 * ti + 0], obj_attribs.texcoords[2 * ni + 1], 1.0f, 1.0f);
                }
                else
                {
                    v.texcoords = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); // default TC = 0,0
                }
            }
        }


        size_t t = offsetof(vertex, normal);

        glCreateVertexArrays(1, &obj->vao);
        glBindVertexArray(obj->vao);

        //
        // vertex buffer
        //
#define USE_DSA
#ifdef USE_DSA

#define MAIN_VBO_BINDING_INDEX 0

#define POSITION_SHADER_ATTRIB_INDEX 0 // THIS one is the binding location in the shader.
#define NORMAL_SHADER_ATTRIB_INDEX 1
#define COLOR_SHADER_ATTRIB_INDEX 2
#define TEXCOORD_SHADER_ATTRIB_INDEX 3

        glCreateBuffers(1, &obj->vertex_buffer_id);

        // init buffer with initial data (flags == 0 -> STATIC_DRAW, no map permitted.)
        glNamedBufferStorage(obj->vertex_buffer_id, vertex_buffer.size() * sizeof(vertex), vertex_buffer.data(), 0);
        gpu_memory += vertex_buffer.size() * sizeof(vertex);

        // Add a VBO to the VAO.The offset is the global offset of the beginning of the first struct, not individual components.
        glVertexArrayVertexBuffer(obj->vao, MAIN_VBO_BINDING_INDEX, obj->vertex_buffer_id, 0, sizeof(vertex));

        // Specify format. The offsets are for individual components, relative to the beginning of the struct.
        glVertexArrayAttribFormat(obj->vao, POSITION_SHADER_ATTRIB_INDEX, 4, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribFormat(obj->vao, NORMAL_SHADER_ATTRIB_INDEX, 4, GL_FLOAT, GL_FALSE, offsetof(vertex, normal));
        glVertexArrayAttribFormat(obj->vao, COLOR_SHADER_ATTRIB_INDEX, 4, GL_FLOAT, GL_FALSE, offsetof(vertex, diffuse_color));
        glVertexArrayAttribFormat(obj->vao, TEXCOORD_SHADER_ATTRIB_INDEX, 4, GL_FLOAT, GL_FALSE, offsetof(vertex, texcoords));

        // map a vao attrib index to a shader attrib binding locations.
        glVertexArrayAttribBinding(obj->vao, POSITION_SHADER_ATTRIB_INDEX, MAIN_VBO_BINDING_INDEX);
        glVertexArrayAttribBinding(obj->vao, NORMAL_SHADER_ATTRIB_INDEX, MAIN_VBO_BINDING_INDEX);
        glVertexArrayAttribBinding(obj->vao, COLOR_SHADER_ATTRIB_INDEX, MAIN_VBO_BINDING_INDEX);
        glVertexArrayAttribBinding(obj->vao, TEXCOORD_SHADER_ATTRIB_INDEX, MAIN_VBO_BINDING_INDEX);

        // enable the attribute
        glEnableVertexArrayAttrib(obj->vao, POSITION_SHADER_ATTRIB_INDEX);
        glEnableVertexArrayAttrib(obj->vao, NORMAL_SHADER_ATTRIB_INDEX);
        glEnableVertexArrayAttrib(obj->vao, COLOR_SHADER_ATTRIB_INDEX);
        glEnableVertexArrayAttrib(obj->vao, TEXCOORD_SHADER_ATTRIB_INDEX);

        //
        // index buffer
        //
        glCreateBuffers(1, &obj->index_buffer_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj->index_buffer_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer.size() * sizeof(unsigned int), index_buffer.data(), GL_STATIC_DRAW);
        gpu_memory += index_buffer.size() * sizeof(unsigned int);

        obj->nb_elements = index_buffer.size();

#else
        glBindBuffer(GL_ARRAY_BUFFER, obj->position_buffer_id);
        glBufferData(GL_ARRAY_BUFFER, position_buffer.size() * sizeof(float), position_buffer.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
#endif

        glBindVertexArray(0); // unbind current VAO
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glutils::check_error();
    }
}

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
    tinyobj::MaterialFileReader mtlReader(models_path);
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

        // create hardware buffers for all objects in the obj, and add it to the scene container
        add_OBJ_to_scene(obj_attribs, obj_shapes, obj_material, false);
        printf("=> total gpu memory = %zd\n", gpu_memory);

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
    auto vs = utils::read_file_content(shaders_path + "simple.vert");
    auto fs = utils::read_file_content(shaders_path + "simple.frag");

    GLuint vs_id = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs_id = glCreateShader(GL_FRAGMENT_SHADER);

    if (!glutils::compile_shader(vs_id, vs.data(), vs.size()))
        return false;
    if (!glutils::compile_shader(fs_id, fs.data(), fs.size()))
        return false;

    GLuint prog_id = glCreateProgram();

    if (!glutils::link_program(prog_id, vs_id, fs_id))
        return false;

    glDeleteShader(vs_id);
    glDeleteShader(fs_id);
    
    _simple_program.program_id = prog_id;

    _simple_program.attrib_in_position = glGetAttribLocation(prog_id, "inPosition");
    _simple_program.attrib_in_normal = glGetAttribLocation(prog_id, "inNormal"); 
    _simple_program.attrib_in_color = glGetAttribLocation(prog_id, "inColor");
    _simple_program.attrib_in_texcoord = glGetAttribLocation(prog_id, "inTexCoord");

    _simple_program.uni_model = glGetUniformLocation(prog_id, "model");
    _simple_program.uni_view = glGetUniformLocation(prog_id, "view");
    _simple_program.uni_proj = glGetUniformLocation(prog_id, "proj");
    _simple_program.uni_tex = glGetUniformLocation(prog_id, "tex");

    return true;
}

AppTest::AppTest(void * options)
{
    struct options_t
    {
        int width;
        int height;
        std::string in_filename;
        int dontrender;
        int verbose;
        int extraverbose;
    };
    
    options_t o = *(options_t*)options;

    if (!o.in_filename.empty())
    {
        _scene_path = o.in_filename;
    }
}

bool AppTest::init(int framebuffer_width, int framebuffer_height)
{
    _fb_width = framebuffer_width;
    _fb_height = framebuffer_height;

    load_shaders();

    // OBJ
    if (_scene_path.empty())
    {
        _scene_path = models_path + "bunny.obj";
        //_scene_path = models_path + "sponza.obj";
    }
    bool ret = load_obj(_scene_path.c_str());

    //
    // compute the whole scene bbox
    //
    scene_bbox_min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    scene_bbox_max = glm::vec3(FLT_MIN, FLT_MIN, FLT_MIN);
    for (const auto &obj : _v_objects)
    {
        scene_bbox_min = glm::min(scene_bbox_min, obj->bbox_min);
        scene_bbox_max = glm::max(scene_bbox_max, obj->bbox_max);
    }
    glm::vec3 scene_middle = (scene_bbox_max + scene_bbox_min) / 2.0f;
    float scene_radius = glm::length(scene_bbox_max - scene_middle);
    
    // GLTF
    //std::string filename = "scene.gltf";
    //bool ret = load_gltf(filename.c_str());

    //
    // Camera
    //
    _camera.viewport = glm::ivec4(0, 0, _fb_width, _fb_height); // full framebuffer viewport
    _camera.eye = glm::vec3(0.0f, 0.0f, 2.0f * scene_radius);
    _camera.target = glm::vec3(0, 0, 0);
    //_camera.eye = glm::vec3(0.0f, 0.0f, 0.0f);
    //_camera.target = glm::vec3(0, 0, -1);
    _camera.near_plane = 1.0f;
    _camera.far_plane = 5.0f * scene_radius;// 100.0f;
    _camera.fovy_degrees = 45.0f;
    _camera.update(); // build initial matrices
    _camera.setup(0.5f); // eye must be set.

    return ret;
}

void AppTest::shutdown()
{
    // release shaders
    glDeleteProgram(_simple_program.program_id);

    // release buffers

    // release vao
    for (const auto &obj : _v_objects)
    {
        glDeleteVertexArrays(1, &obj->vao);
    }
}

void AppTest::run(float dt)
{
    static float accum = 0.0f;
    accum += dt;

    //
    // update
    //
    _camera.update(); // rebuild matrices

    //
    // draw
    //

    glViewport(_camera.viewport[0], _camera.viewport[1], _camera.viewport[2], _camera.viewport[3]);

    const GLfloat clear_color[] = { 
        (float)std::sin(accum) * 0.5f + 0.5f,
        (float)std::cos(accum) * 0.5f + 0.5f,
        0.0f, 1.0f };
    glClearBufferfv(GL_COLOR, 0, clear_color);
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

    glm::mat4 model(1); // fake.
    
    glEnable(GL_DEPTH_TEST);

    glUseProgram(_simple_program.program_id);

    // camera
    glProgramUniformMatrix4fv(_simple_program.program_id, _simple_program.uni_view, 1, GL_FALSE, glm::value_ptr(_camera.view));
    glProgramUniformMatrix4fv(_simple_program.program_id, _simple_program.uni_proj, 1, GL_FALSE, glm::value_ptr(_camera.proj));

    // global scene transform
    glm::vec3 scene_middle = (scene_bbox_max + scene_bbox_min) / 2.0f;
    model = glm::translate(model, -scene_middle);
    glProgramUniformMatrix4fv(_simple_program.program_id, _simple_program.uni_model, 1, GL_FALSE, glm::value_ptr(model));

    for (const auto &obj : _v_objects)
    {
        //glProgramUniformMatrix4fv(_simple_program.program_id, _simple_program.uni_model, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(obj->vao);
        glDrawElements(GL_TRIANGLES, obj->nb_elements, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
    glUseProgram(0);

    glDisable(GL_DEPTH_TEST);
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

        // pourrave, juste pour test.
        // TODO: record un vecteur unitaire d'impulse, et appliquer ca avec la speed
        // de la camera au moment du update.
        _camera.translate(glm::vec3(mv_x * 0.05f, mv_y * 0.05f, mv_z * 0.05f));
        
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
    if (button == GLFW_MOUSE_BUTTON_LEFT) 
    {
        if (action == GLFW_PRESS) 
        {
            _mouse_pressed = true;
            // TODO: why doubles??? convert all to int.
            double invert_y = ((double)_fb_height - _mouse_y) - 1.0;
            _camera.start(_mouse_x, invert_y);
        }
        else if (action == GLFW_RELEASE) 
        {
            _mouse_pressed = false;
        }
    }

    //if (button == GLFW_MOUSE_BUTTON_RIGHT)
    //if (button == GLFW_MOUSE_BUTTON_MIDDLE)
}

void AppTest::onMouseMove(GLFWwindow* window, double mouse_x, double mouse_y)
{
    (void)window;

    if (_mouse_pressed) 
    {
        double invert_y = ((double)_fb_height - _mouse_y) - 1.0;
        _camera.move(mouse_x, invert_y);
    }

    // Update mouse point
    _mouse_x = mouse_x;
    _mouse_y = mouse_y;
}

void AppTest::onMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
    //auto& dist = app->camera.dist;

    //dist -= (float)yoffset * 0.3f;
    //dist -= (float)xoffset * 0.3f;
    //dist = glm::clamp(dist, 0.5f, 10.f);
}
