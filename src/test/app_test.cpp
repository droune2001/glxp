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

static 
void add_obj_to_scene(
    const tinyobj::attrib_t &obj_attribs, 
    std::vector<tinyobj::shape_t> &obj_shapes,
    std::vector<tinyobj::material_t> &obj_material,
    AppTest::object *obj,
    bool normalize_size)
{
    // First shape only.
    const auto &shape = obj_shapes[0];

    // compute bbox
    glm::vec3 bbox_min(FLT_MAX, FLT_MAX, FLT_MAX);
    glm::vec3 bbox_max(FLT_MIN, FLT_MIN, FLT_MIN);
    for (size_t i = 0; i < obj_attribs.vertices.size() / 3; ++i)
    {
        glm::vec3 position(obj_attribs.vertices[i + 0], obj_attribs.vertices[i + 1], obj_attribs.vertices[i + 2]);
        bbox_min = glm::min(bbox_min, position);
        bbox_max = glm::max(bbox_max, position);
    }

    glm::vec3 middle = (bbox_max + bbox_min) / 2.0f;
    float scale_factor = 1.0f / glm::length(bbox_max - middle);

    glCreateVertexArrays(1, &obj->vao);
    glBindVertexArray(obj->vao);





    // index buffer
    glCreateBuffers(1, &obj->index_buffer_id);
    std::vector<unsigned int> index_buffer;
    index_buffer.resize(shape.mesh.indices.size());
    for (auto index : shape.mesh.indices)
    {
        index_buffer.push_back(index.vertex_index);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj->index_buffer_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer.size() * sizeof(unsigned int), index_buffer.data(), GL_STATIC_DRAW);

    obj->nb_elements = index_buffer.size();







    // vertex(positions) buffer
    glCreateBuffers(1, &obj->position_buffer_id);
    std::vector<float> position_buffer;
    position_buffer.resize(obj_attribs.vertices.size());
    for(size_t i = 0; i < obj_attribs.vertices.size()/3; ++i)
    {
        glm::vec3 position(obj_attribs.vertices[i + 0], obj_attribs.vertices[i + 1], obj_attribs.vertices[i + 2]);
        if (normalize_size)
        {
            position = scale_factor * (position - middle);
        }

        position_buffer.push_back(position.x);
        position_buffer.push_back(position.y);
        position_buffer.push_back(position.z);
    }
    glBindBuffer(GL_ARRAY_BUFFER, obj->position_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, position_buffer.size() * sizeof(float), position_buffer.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // vertex(color) buffer
    glCreateBuffers(1, &obj->color_buffer_id);
    std::vector<float> color_buffer;
    color_buffer.resize(obj_attribs.colors.size());
    for (auto color : obj_attribs.colors)
    {
        color_buffer.push_back(color);
    }
    glBindBuffer(GL_ARRAY_BUFFER, obj->color_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, color_buffer.size() * sizeof(float), color_buffer.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    // vertex(texcoords) buffer
    glCreateBuffers(1, &obj->texcoord_buffer_id);
    std::vector<float> texcoord_buffer;
    texcoord_buffer.resize(obj_attribs.texcoords.size());
    for (auto texcoord : obj_attribs.texcoords)
    {
        texcoord_buffer.push_back(texcoord);
    }
    glBindBuffer(GL_ARRAY_BUFFER, obj->texcoord_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, texcoord_buffer.size() * sizeof(float), texcoord_buffer.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(3);

    glBindVertexArray(0); // unbind current VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glutils::check_error();
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
        add_obj_to_scene(obj_attribs, obj_shapes, obj_material, &_single_object, true);

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
    _simple_program.attrib_in_color = glGetAttribLocation(prog_id, "inColor");
    _simple_program.attrib_in_normal = glGetAttribLocation(prog_id, "inNormal");
    _simple_program.attrib_in_texcoord = glGetAttribLocation(prog_id, "inTexCoord");

    _simple_program.uni_model = glGetUniformLocation(prog_id, "model");
    _simple_program.uni_view = glGetUniformLocation(prog_id, "view");
    _simple_program.uni_proj = glGetUniformLocation(prog_id, "proj");
    _simple_program.uni_tex = glGetUniformLocation(prog_id, "tex");

    return true;
}
GLuint vao;
bool AppTest::init(int framebuffer_width, int framebuffer_height)
{
    _fb_width = framebuffer_width;
    _fb_height = framebuffer_height;

    load_shaders();

    // OBJ
    std::string filename = models_path + "bunny.obj";
    //std::string filename = "sponza.obj";
    bool ret = load_obj(filename.c_str());

    // GLTF
    //std::string filename = "scene.gltf";
    //bool ret = load_gltf(filename.c_str());





    const glm::vec4 positions[3] = {
        glm::vec4(0.25, -0.25, 0.5, 1.0),
        glm::vec4(-0.25, -0.25, 0.5, 1.0),
        glm::vec4(0.25, 0.25, 0.5, 1.0)
    };

    const glm::vec4 colors[] = {
        glm::vec4(1.0, 0.0, 0.0, 1.0),
        glm::vec4(0.0, 1.0, 0.0, 1.0),
        glm::vec4(0.0, 0.0, 1.0, 1.0)
    };

    //GLuint vao;
    glCreateVertexArrays(1, &vao);
    GLuint vbo[2];
    glCreateBuffers(2, vbo);

    // POSITION
    {
        // init buffer with initial data (and no flags...)
        glNamedBufferStorage(vbo[0], sizeof(positions) * sizeof(glm::vec4), glm::value_ptr(positions[0]), 0);

        // bind to vao (bindless version)
        #define POSITION_BINDING_INDEX 0
        glVertexArrayVertexBuffer(vao, POSITION_BINDING_INDEX, vbo[0], 0, sizeof(glm::vec4));

        // specify format
        #define POSITION_SHADER_ATTRIB_INDEX 0 // THIS one is the binding location in the shader.
        // when using ann interleaved buffer, there will be many binding points in the vao
        // where we define the strides and offsets, but in the shader all will be bound to
        // a single attrib, that will be a struct.
        glVertexArrayAttribFormat(vao, POSITION_SHADER_ATTRIB_INDEX, 4, GL_FLOAT, GL_FALSE, 0);

        // map a vao attrib index to a shader attrib binding location (?)
        glVertexArrayAttribBinding(vao, POSITION_BINDING_INDEX, POSITION_SHADER_ATTRIB_INDEX);

        // enable the attribute
        glEnableVertexArrayAttrib(vao, POSITION_BINDING_INDEX);
    }

    // COLOR
    {
        // init buffer with initial data (and no flags...)
        glNamedBufferStorage(vbo[1], sizeof(colors) * sizeof(glm::vec4), glm::value_ptr(colors[0]), 0);

        // bind to vao (bindless version)
        #define COLOR_BINDING_INDEX 1
        glVertexArrayVertexBuffer(vao, COLOR_BINDING_INDEX, vbo[1], 0, sizeof(glm::vec4));

        // specify format
        #define COLOR_SHADER_ATTRIB_INDEX 1
        glVertexArrayAttribFormat(vao, COLOR_SHADER_ATTRIB_INDEX, 4, GL_FLOAT, GL_FALSE, 0);

        // map a vao attrib index to a shader attrib binding location (?)
        glVertexArrayAttribBinding(vao, COLOR_BINDING_INDEX, COLOR_SHADER_ATTRIB_INDEX);

        // enable the attribute
        glEnableVertexArrayAttrib(vao, COLOR_BINDING_INDEX);
    }






    return ret;
}

void AppTest::shutdown()
{
    // release shaders
    glDeleteProgram(_simple_program.program_id);

    // release buffers

    // release vao
    glDeleteVertexArrays(1, &_single_object.vao);
}

void AppTest::run(float dt)
{
    static float accum = 0.0f;
    accum += dt;

    glViewport(0, 0, _fb_width, _fb_height);
    const GLfloat clear_color[] = { 
        (float)std::sin(accum) * 0.5f + 0.5f,
        (float)std::cos(accum) * 0.5f + 0.5f,
        0.0f, 1.0f };
    glClearBufferfv(GL_COLOR, 0, clear_color);
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

    // camera
    glm::mat4 view(1);
    view = glm::translate(view, glm::vec3(0,0,-5));

    glm::mat4 model(1);

    glm::mat4 proj = glm::perspective(45.0f, 1.33f, 1.0f, 10.0f);


    glUseProgram(_simple_program.program_id);

    //glBindVertexArray(vao);
    glBindVertexArray(_single_object.vao);

    GLint offset_location = glGetAttribLocation(_simple_program.program_id, "offset");
    GLfloat offset_attrib[] = {
        (float)sin(accum) * 0.5f,
        (float)cos(accum) * 0.6f,
        0.0f, 0.0f };
    glVertexAttrib4fv(offset_location, offset_attrib);

    glProgramUniformMatrix4fv(_simple_program.program_id, _simple_program.uni_model, 1, GL_FALSE, glm::value_ptr(model));
    glProgramUniformMatrix4fv(_simple_program.program_id, _simple_program.uni_view, 1, GL_FALSE, glm::value_ptr(view));
    glProgramUniformMatrix4fv(_simple_program.program_id, _simple_program.uni_proj, 1, GL_FALSE, glm::value_ptr(proj));

    //glDrawArrays(GL_TRIANGLES, 0, 3);








    //glBindBuffer(GL_ARRAY_BUFFER, _single_object.position_buffer_id);
    //glVertexAttribPointer(_simple_program.attrib_in_position, 3, GL_FLOAT, GL_FALSE, 0, 0);
    //glEnableVertexAttribArray(_simple_program.attrib_in_position);

    //glBindBuffer(GL_ARRAY_BUFFER, _single_object.color_buffer_id);
    //glVertexAttribPointer(_simple_program.attrib_in_color, 3, GL_FLOAT, GL_FALSE, 0, 0);
    //glEnableVertexAttribArray(_simple_program.attrib_in_color);

    //glBindBuffer(GL_ARRAY_BUFFER, _single_object.texcoord_buffer_id);
    //glVertexAttribPointer(_simple_program.attrib_in_texcoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
    //glEnableVertexAttribArray(_simple_program.attrib_in_texcoord);

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _single_object.index_buffer_id);
    //glDrawElements(GL_TRIANGLES, _single_object.nb_elements, GL_UNSIGNED_INT, 0);
    glDrawArrays(GL_TRIANGLES, 0, 99);
    //glUseProgram(0);
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
