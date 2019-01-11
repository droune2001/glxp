#include "app_test.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm_usage.h"
#include "imgui.h"
#include "tiny_obj_loader.h"
#include "tiny_gltf.h"
#include "gl_utils.h"
#include "utils.h"
#include "procgen.h"

#include "FilmicCurve/FilmicToneCurve.h"
#include "FilmicCurve/FilmicColorGrading.h"

#include <vector>
#include <fstream>
#include <functional>

static std::string models_path = "../../../data/tonemap/models/";
static std::string texture_path = "../../../data/tonemap/models/";
static std::string shaders_path = "../../../data/tonemap/shaders/";

void AppTest::add_to_scene(const std::string &name, const IndexedMesh &mesh)
{
    unsigned int suffix = 0;
    std::string object_name = name;
    while (_m_objects.find(name) != _m_objects.end())
    {
        object_name = name + std::to_string(suffix++);
    }
    
    auto new_object = std::make_shared<DrawItem>();
    _v_objects.push_back(new_object);
    _m_objects[object_name] = new_object;

    auto obj = _m_objects[object_name];

    //
    // compute bbox
    //
    glm::vec3 bbox_min(FLT_MAX, FLT_MAX, FLT_MAX);
    glm::vec3 bbox_max(FLT_MIN, FLT_MIN, FLT_MIN);
    for (const auto &v : mesh.vertices)
    {
        bbox_min = glm::min(bbox_min, v.p);
        bbox_max = glm::max(bbox_max, v.p);
    }
    glm::vec3 middle = (bbox_max + bbox_min) / 2.0f;
    float scale_factor = 1.0f / glm::length(bbox_max - middle);

    obj->bbox_min = bbox_min;
    obj->bbox_max = bbox_max;

    struct vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 diffuse_color;
        glm::vec2 texcoords;
    };

    std::vector<vertex> vertex_buffer;
    vertex_buffer.resize(mesh.vertices.size()); // model triangulated by tinyobj
    for (size_t i = 0; i < mesh.vertices.size(); ++i)
    {
        const auto &v_src = mesh.vertices[i];
        auto &v_dst = vertex_buffer[i];
        v_dst.position = v_src.p;
        v_dst.normal = v_src.n;
        v_dst.diffuse_color = glm::vec3(1,1,1);
        v_dst.texcoords = v_src.uv;
    }

    const std::vector<unsigned int> &index_buffer = mesh.indices;


    //
    // separate this code?
    // function that takes vertices, indices, and obj&, and builds opengl buffers.
    //


    glCreateVertexArrays(1, &obj->vao);
    glBindVertexArray(obj->vao);

    //
    // vertex buffer
    //
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
    glVertexArrayAttribFormat(obj->vao, POSITION_SHADER_ATTRIB_INDEX, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribFormat(obj->vao, NORMAL_SHADER_ATTRIB_INDEX, 3, GL_FLOAT, GL_FALSE, offsetof(vertex, normal));
    glVertexArrayAttribFormat(obj->vao, COLOR_SHADER_ATTRIB_INDEX, 3, GL_FLOAT, GL_FALSE, offsetof(vertex, diffuse_color));
    glVertexArrayAttribFormat(obj->vao, TEXCOORD_SHADER_ATTRIB_INDEX, 2, GL_FLOAT, GL_FALSE, offsetof(vertex, texcoords));

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

    glBindVertexArray(0); // unbind current VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glutils::check_error();
}

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
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec3 diffuse_color;
            glm::vec2 texcoords;
        };

        // convert tinyobj_loader multi-index format to my own interleaved linear format
        std::vector<vertex> vertex_buffer;
        vertex_buffer.resize(obj_attribs.vertices.size() / 3); // model triangulated by tinyobj
        for (size_t i = 0; i < obj_attribs.vertices.size() / 3; ++i)
        {
            vertex &v = vertex_buffer[i];
            v.position = glm::vec3(obj_attribs.vertices[3 * i + 0], obj_attribs.vertices[3 * i + 1], obj_attribs.vertices[3 * i + 2]);
            v.diffuse_color = glm::vec3(obj_attribs.colors[3 * i + 0], obj_attribs.colors[3 * i + 1], obj_attribs.colors[3 * i + 2]);
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
                    v.normal = glm::vec3(obj_attribs.normals[3 * ni + 0], obj_attribs.normals[3 * ni + 1], obj_attribs.normals[3 * ni + 2]);
                }
                else
                {
                    v.normal = glm::vec3(0.0f, 1.0f, 0.0f); // default normal = UP
                }

                if (index.texcoord_index != -1)
                {
                    v.texcoords = glm::vec2(obj_attribs.texcoords[2 * ti + 0], obj_attribs.texcoords[2 * ti + 1]);
                }
                else
                {
                    v.texcoords = glm::vec2(0.0f, 0.0f); // default TC = 0,0
                }
            }
        }


        size_t t = offsetof(vertex, normal);

        glCreateVertexArrays(1, &obj->vao);
        glBindVertexArray(obj->vao);

        //
        // vertex buffer
        //
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
        glVertexArrayAttribFormat(obj->vao, POSITION_SHADER_ATTRIB_INDEX, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribFormat(obj->vao, NORMAL_SHADER_ATTRIB_INDEX, 3, GL_FLOAT, GL_FALSE, offsetof(vertex, normal));
        glVertexArrayAttribFormat(obj->vao, COLOR_SHADER_ATTRIB_INDEX, 3, GL_FLOAT, GL_FALSE, offsetof(vertex, diffuse_color));
        glVertexArrayAttribFormat(obj->vao, TEXCOORD_SHADER_ATTRIB_INDEX, 2, GL_FLOAT, GL_FALSE, offsetof(vertex, texcoords));

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

bool AppTest::load_textures()
{
    //
    // TEXTURES
    //
    //glutils::load_image_hdr(&_tex, models_path + "fish_hoek_beach_2k.hdr");
    glutils::load_image_hdr(&_tex, models_path + "venice_sunset_2k.hdr"); // HDR Max = 8384.

    //
    // 3D LUT
    //
    GLenum internalFormat = GL_RGB32F;
    GLenum format = GL_RGB;
    GLenum type = GL_FLOAT;

    glCreateTextures(GL_TEXTURE_3D, 1, &_3dlut_tex);
    glTextureStorage3D(_3dlut_tex, 1, internalFormat, 32, 32, 32);
    _3dlut.resize(32 * 32 * 32);
    for(int b=0; b<32; ++b)
    {
        for (int g = 0; g<32; ++g)
        {
            for (int r = 0; r<32; ++r)
            {
                int idx = b * 32 * 32 + g * 32 + r;
                _3dlut[idx] = glm::vec3(r/31.0f, g/31.0f, b/31.0f);
            }
        }
    }
    glTextureSubImage3D(_3dlut_tex, 0, 0, 0, 0, 32, 32, 32, format, type, _3dlut.data());

    // do it once
    update_tonemap_curves();

    //
    // SAMPLERS
    //
    glCreateSamplers(1, &_sampler);
    
    glSamplerParameteri(_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glSamplerParameteri(_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //glTextureParameteri(_tex, xxx, iii); // to parameter the sampler object embedded in the texture object.

    // NEAREST
    glCreateSamplers(1, &_nearest_sampler);

    glSamplerParameteri(_nearest_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(_nearest_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glSamplerParameteri(_nearest_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(_nearest_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // LINEAR
    glCreateSamplers(1, &_linear_sampler);

    glSamplerParameteri(_linear_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(_linear_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glSamplerParameteri(_linear_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(_linear_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(_linear_sampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return true;
}

bool AppTest::load_shaders()
{
    // simple
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
    }

    // fullscreen
    {
        auto vs = utils::read_file_content(shaders_path + "fullscreen.vert");
        auto fs = utils::read_file_content(shaders_path + "fullscreen.frag");

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

        _fullscreen_program = prog_id;
    }

    // 3dlut
    {
        auto vs = utils::read_file_content(shaders_path + "draw_lut.vert");
        auto fs = utils::read_file_content(shaders_path + "draw_lut.frag");

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

        _3dlut_program = prog_id;

        _uni_width = glGetUniformLocation(prog_id, "width");
        _uni_height = glGetUniformLocation(prog_id, "height");
    }

    // tonemap
    {
        auto vs = utils::read_file_content(shaders_path + "tonemap.vert");
        auto fs = utils::read_file_content(shaders_path + "tonemap.frag");

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

        _tonemap_program = prog_id;

        _uni_splitview = glGetUniformLocation(prog_id, "view");
    }

    return true;
}

bool AppTest::recreate_framebuffers()
{
    // release framebuffer resources.
    // ...

    return create_framebuffers();
}

bool AppTest::create_framebuffers()
{
    //
    // HDR Framebuffer
    //
    glCreateFramebuffers(1, &_fb_hdr);

    // COLOR - use a texture (to be able to use it later for reading, has mips)
    glCreateTextures(GL_TEXTURE_2D, 1, &_fbtex_hdr_color);
    glTextureStorage2D(_fbtex_hdr_color, 1, GL_RGBA32F, _fb_width, _fb_height);
    glNamedFramebufferTexture(_fb_hdr, GL_COLOR_ATTACHMENT0, _fbtex_hdr_color, 0);

    // DEPTH - use renderbuffer = 2D,no-mips,no-read (we could use a texture, though)
    glCreateRenderbuffers(1, &_fbtex_hdr_depth);
    glNamedRenderbufferStorage(_fbtex_hdr_depth, GL_DEPTH_COMPONENT32F, _fb_width, _fb_height);
    glNamedFramebufferRenderbuffer(_fb_hdr, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _fbtex_hdr_depth);

    // draw into attachment 0
    glNamedFramebufferDrawBuffer(_fb_hdr, GL_COLOR_ATTACHMENT0);

    glutils::check_error();

    //
    // LDR Framebuffer
    //
    glCreateFramebuffers(1, &_fb_ldr);

    // COLOR - use a texture (to be able to use it later for reading, has mips)
    glCreateTextures(GL_TEXTURE_2D, 1, &_fbtex_ldr_color);
    glTextureStorage2D(_fbtex_ldr_color, 1, GL_RGBA8, _fb_width, _fb_height);
    glNamedFramebufferTexture(_fb_ldr, GL_COLOR_ATTACHMENT0, _fbtex_ldr_color, 0);

    // DEPTH - use renderbuffer = 2D,no-mips,no-read (we could use a texture, though)
    glCreateRenderbuffers(1, &_fbtex_ldr_depth);
    glNamedRenderbufferStorage(_fbtex_ldr_depth, GL_DEPTH_COMPONENT32F, _fb_width, _fb_height);
    glNamedFramebufferRenderbuffer(_fb_ldr, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _fbtex_ldr_depth);

    // draw into attachment 0
    glNamedFramebufferDrawBuffer(_fb_ldr, GL_COLOR_ATTACHMENT0);

    glutils::check_error();

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
    bool ret = true;

    _fb_width = framebuffer_width;
    _fb_height = framebuffer_height;

    load_shaders();
    load_textures();
    create_framebuffers();

    // OBJ
    //if (_scene_path.empty())
    //{
    //    _scene_path = models_path + "bunny.obj";
    //    //_scene_path = models_path + "sponza.obj";
    //}
    //ret = load_obj(_scene_path.c_str());

    //add_to_scene("cube", make_flat_cube(1.0f, 1.0f, 1.0f));
    add_to_scene("sphere", make_icosphere(5, 1.0f));
    
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
    auto ac = std::make_unique<ArcballCamera>();
    ac->viewport = glm::ivec4(0, 0, _fb_width, _fb_height); // full framebuffer viewport
    ac->eye = glm::vec3(0.0f, 0.0f, 2.0f * scene_radius);
    ac->target = glm::vec3(0, 0, 0);
    ac->near_plane = 1.0f;
    ac->far_plane = 5.0f * scene_radius;// 100.0f;
    ac->fovy_degrees = 45.0f;
    ac->update(); // build initial matrices
    ac->setup(0.5f); // eye must be set.
    _cameras.emplace_back(std::move(ac));

    auto fc = std::make_unique<FpsCamera>();
    fc->viewport = glm::ivec4(0, 0, _fb_width, _fb_height); // full framebuffer viewport
    fc->eye = glm::vec3(0.0f, 0.0f, 2.0f * scene_radius);
    fc->dir = glm::vec3(0, 0, -1);
    fc->near_plane = 1.0f;
    fc->far_plane = 5.0f * scene_radius;// 100.0f;
    fc->fovy_degrees = 45.0f;
    fc->update(); // build initial matrices
    _cameras.emplace_back(std::move(fc));

    // VAO for fullscreen pass
    glCreateVertexArrays(1, &_dummy_vao);

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

void AppTest::update_camera(float dt)
{
    Camera *cm = current_camera();
    if (!cm)
    {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    bool left = io.KeysDown[GLFW_KEY_A];
    bool right = io.KeysDown[GLFW_KEY_D];
    bool forward = io.KeysDown[GLFW_KEY_W];
    bool backward = io.KeysDown[GLFW_KEY_S];
    bool up = io.KeysDown[GLFW_KEY_PAGE_UP];
    bool down = io.KeysDown[GLFW_KEY_PAGE_DOWN];

    glm::vec3 direction(0.0f);
    auto test = left ^ right;
    direction.x = !(left ^ right) ? 0.0f : ( left ? -1.0f : 1.0f);
    direction.y = !(down ^ up) ? 0.0f : (down ? -1.0f : 1.0f);
    direction.z = !(forward ^ backward) ? 0.0f : (forward ? -1.0f : 1.0f);

    direction *= dt;
    cm->translate(direction);

    cm->update(); // rebuild matrices

    cm->viewport = glm::ivec4(0, 0, _fb_width, _fb_height);
}

void AppTest::run(float dt)
{
    static float accum = 0.0f;
    accum += dt;

    Camera *cm = current_camera();
    if (!cm)
        return;
    
    //
    // update
    //
    update_camera(dt); // reads key states and translates/updates camera.

    //
    // DRAW scene in HDR framebuffer.
    //
    glBindFramebuffer(GL_FRAMEBUFFER, _fb_hdr);
    {
        glViewport(0, 0, _fb_width, _fb_height);

        // Clear
        const GLfloat clear_color[] = {
            (float)std::sin(accum) * 0.5f + 0.5f,
            (float)std::cos(accum) * 0.5f + 0.5f,
            0.0f, 1.0f };
        glClearBufferfv(GL_COLOR, 0, clear_color);
        glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

        // Background texture.
        glDisable(GL_DEPTH_TEST);
        glBindSampler(0, _sampler); // bind the sampler to the texture unit 0
        glBindTextureUnit(0, _tex); // bind the texture object to the texture unit 0
        glUseProgram(_fullscreen_program);
        glBindVertexArray(_dummy_vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        glUseProgram(0);

        // Scene content
        glEnable(GL_DEPTH_TEST);
        glUseProgram(_simple_program.program_id);

        // camera
        glProgramUniformMatrix4fv(_simple_program.program_id, _simple_program.uni_view, 1, GL_FALSE, glm::value_ptr(cm->view));
        glProgramUniformMatrix4fv(_simple_program.program_id, _simple_program.uni_proj, 1, GL_FALSE, glm::value_ptr(cm->proj));

        // global scene transform
        glm::vec3 scene_middle = (scene_bbox_max + scene_bbox_min) / 2.0f;
        glm::mat4 model(1);
        model = glm::translate(model, -scene_middle);
        glProgramUniformMatrix4fv(_simple_program.program_id, _simple_program.uni_model, 1, GL_FALSE, glm::value_ptr(model));

        // texture
        glBindSampler(0, _sampler); // bind the sampler to the texture unit 0
        glBindTextureUnit(0, _tex); // bind the texture object to the texture unit 0

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
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //
    // Tone-Mapping - READS hdr - WRITES ldr
    //
    glBindFramebuffer(GL_FRAMEBUFFER, _fb_ldr);
    {
        glBindSampler(0, _linear_sampler); // bind the sampler to the texture unit 0
        glBindTextureUnit(0, _fbtex_hdr_color); // bind the texture object to the texture unit 0

        glBindSampler(1, _linear_sampler);
        glBindTextureUnit(1, _3dlut_tex);

        glUseProgram(_tonemap_program);
        glUniform1i(_uni_splitview, _current_view);
        glBindVertexArray(_dummy_vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //
    // fullscreen pass - copy the LDR framebuffer to the screen (could use a blit)
    //
    {
        glBindSampler(0, _nearest_sampler); // bind the sampler to the texture unit 0
        glBindTextureUnit(0, _fbtex_ldr_color); // bind the texture object to the texture unit 0
        glUseProgram(_fullscreen_program);
        glBindVertexArray(_dummy_vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        glUseProgram(0);
    }

    // Draw 3d LUT
    if(_draw3dlut)
    {
        glBindSampler(1, _linear_sampler);
        glBindTextureUnit(1, _3dlut_tex);
        glUseProgram(_3dlut_program);
        glUniform1i(_uni_width, _fb_width);
        glUniform1i(_uni_height, _fb_height);
        glBindVertexArray(_dummy_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glUseProgram(0);
    }

    //
    // GUI - over the default framebuffer
    //
    do_gui();
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

    recreate_framebuffers();
}

void AppTest::onKeyboard(GLFWwindow * window, int key, int scancode, int action, int mods)
{
    (void)window;
    (void)scancode;
    (void)mods;
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        //if (key == GLFW_KEY_E)
        //    toto = !toto;
        
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
            auto *cm = current_camera();
            if (cm)
            {
                int invert_y = (_fb_height - _mouse_y) - 1;
                cm->mouse_click(_mouse_x, invert_y);
            }
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
        auto *cm = current_camera();
        if (cm)
        {
            int invert_y = (_fb_height - _mouse_y) - 1;
            cm->mouse_move(mouse_x, invert_y);
        }
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

static FilmicColorGrading::UserParams userParams; // User params are the input
static float g_ACESGamma = 1.0f;

void AppTest::update_tonemap_curves()
{
    int nb_steps = 256;
    float min_x = 0.0f;
    float max_x = 4.0f;

    auto build_curve = [nb_steps, min_x, max_x](std::vector<float> &data, std::function<float(float)> f)
    {
        data.resize(nb_steps); // fill with 0
        
        float delta = (max_x - min_x) / (nb_steps - 1);
        for (int i = 0; i < nb_steps; ++i)
        {
            float x = min_x + i * delta;
            data[i] = f(x);
        }
    };

    auto linear_curve = [nb_steps, min_x, max_x](float srcValue) 
    { 
        float range = max_x - min_x;
        return (srcValue-min_x)/range;
    };

    auto filmic_curve = [nb_steps, min_x, max_x](float srcValue)
    {
        float x = std::max(0.0f, srcValue - 0.004f);
        return (x*(6.2f*x + 0.5f)) / (x*(6.2f*x + 1.7f) + 0.06f);
    };

    build_curve(_curve0, linear_curve);
    build_curve(_curve1, filmic_curve);
    
    {
        //FilmicColorGrading::UserParams userParams; // User params are the input
        FilmicColorGrading::RawParams rawParams;
        FilmicColorGrading::EvalParams evalParams;
        FilmicColorGrading::BakedParams bakeParams;
        FilmicColorGrading::BakedParams bakeParamsForLut;

        FilmicColorGrading::RawFromUserParams(rawParams, userParams);
        FilmicColorGrading::EvalFromRawParams(evalParams, rawParams);
        FilmicColorGrading::BakeFromEvalParams(bakeParams, evalParams, 1024, FilmicColorGrading::kTableSpacing_Quadratic);

        _curve2.resize(nb_steps); // fill with 0
        float delta = (max_x - min_x) / (nb_steps - 1);
        for (int i = 0; i < nb_steps; ++i)
        {
            float x = min_x + i * delta;
            Vec3 srcColor = Vec3(x, x, x);
            Vec3 dstColor = bakeParams.EvalColor(srcColor);
            _curve2[i] = dstColor.x;
        }


        for (int b = 0; b<32; ++b)
        {
            for (int g = 0; g<32; ++g)
            {
                for (int r = 0; r<32; ++r)
                {
                    int idx = b * 32 * 32 + g * 32 + r;

                    //Vec3 srcColor = Vec3(r / 31.0f, g / 31.0f, b / 31.0f); // [0..1]
                    //Vec3 srcColor = Vec3(r / (31.0f / 2.0f), g / (31.0f / 2.0f), b / (31.0f / 2.0f)); // [0..2]
                    Vec3 srcColor = Vec3(r / (31.0f/4.0f), g / (31.0f / 4.0f), b / (31.0f / 4.0f)); // [0..4]
                    Vec3 dstColor = bakeParams.EvalColor(srcColor);
                    _3dlut[idx] = glm::vec3(dstColor.x, dstColor.y, dstColor.z);
                }
            }
        }

        // dont do that every frame
        GLenum format = GL_RGB;
        GLenum type = GL_FLOAT;
        glTextureSubImage3D(_3dlut_tex, 0, 0, 0, 0, 32, 32, 32, format, type, _3dlut.data());
    }

    // ACES
    {
        const glm::mat3 ACESInputMat =
        {
            { 0.59719, 0.35458, 0.04823 },
            { 0.07600, 0.90834, 0.01566 },
            { 0.02840, 0.13383, 0.83777 }
        };

        // ODT_SAT => XYZ => D60_2_D65 => sRGB
        const glm::mat3 ACESOutputMat =
        {
            { 1.60475, -0.53108, -0.07367 },
            { -0.10208,  1.10813, -0.00605 },
            { -0.00327, -0.07276,  1.07602 }
        };

        auto RRTAndODTFit = [](vec3 v) -> vec3
        {
            vec3 a = v * (v + 0.0245786f) - 0.000090537f;
            vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
            return a / b;
         };

        auto ACESFitted = [&](glm::vec3 color) -> glm::vec3
        {
            color = glm::transpose(ACESInputMat) * color; // transpose because HLSL matrix

                                                     // Apply RRT and ODT
            color = RRTAndODTFit(color);

            color = glm::transpose(ACESOutputMat) * color;

            // Clamp to [0, 1]
            color = glm::clamp(color, glm::vec3(0), glm::vec3(1));

            return color;
        };

        auto Linear_To_sRGB = [](glm::vec3 linear_color, float p) -> glm::vec3
        {
            return glm::pow(linear_color, glm::vec3(1.0f / p));
        };

        _curve3.resize(nb_steps); // fill with 0
        float delta = (max_x - min_x) / (nb_steps - 1);
        for (int i = 0; i < nb_steps; ++i)
        {
            float x = min_x + i * delta;
            glm::vec3 srcColor = glm::vec3(x, x, x);
            glm::vec3 dstColor = Linear_To_sRGB(ACESFitted(srcColor), g_ACESGamma);

            _curve3[i] = dstColor.x;
        }
    }
}

static void ImGuiLabelWindow(const char *text, float x, float y)
{
    ImVec2 window_pos = ImVec2(x, y);
    ImVec2 window_pos_pivot = ImVec2(0.5f, 0.5f);
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
    window_flags |= ImGuiWindowFlags_NoSavedSettings;
    window_flags |= ImGuiWindowFlags_NoFocusOnAppearing;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoScrollbar;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoNav;
    bool *pOpen = nullptr;

    ImGui::Begin(text, pOpen, window_flags);
    ImGui::Text(text);
    ImGui::End();
}

void AppTest::do_gui()
{
    static bool show_dialog = true;
    static bool reset_cam = false;

    // Early out if the window is collapsed, as an optimization.
    if (ImGui::Begin("Options", &show_dialog, ImGuiWindowFlags_None))
    {



        bool something_changed = false;

        ImGui::Combo("View", &_current_view, "Horizontal Split 4\0Quad Split 4\0Linear Only\0Filmic LUT Only\0ACES Only\0Filmic UC2 Only\0\0");

        //ImGui::PlotLines("L", _curve0.data(), _curve0.size(), 0, "Linear", 0.0f, 1.2f, ImVec2(0, 128)); 

        ImGui::PlotLines("F1", _curve1.data(), _curve1.size(), 0, "Filmic Uncharted 2 with Gamma", 0.0f, 1.0f, ImVec2(512, 128));

        ImGui::Checkbox("Draw 3D LUT", &_draw3dlut);
        something_changed = ImGui::SliderFloat("Toe Strength", &userParams.m_filmicToeStrength, 0.0f, 1.0f, "%.2f") || something_changed;
        something_changed = ImGui::SliderFloat("Toe Length", &userParams.m_filmicToeLength, 0.0f, 1.0f, "%.2f") || something_changed;
        something_changed = ImGui::SliderFloat("Shoulder Strength", &userParams.m_filmicShoulderStrength, 0.0f, 4.0f, "%.2f") || something_changed;
        something_changed = ImGui::SliderFloat("Shoulder Length", &userParams.m_filmicShoulderLength, 0.0f, 1.0f, "%.2f") || something_changed;
        something_changed = ImGui::SliderFloat("Shoulder Angle", &userParams.m_filmicShoulderAngle, 0.0f, 1.0f, "%.2f") || something_changed;
        something_changed = ImGui::SliderFloat("Filmic Gamma", &userParams.m_filmicGamma, 1.0f / 2.2f, 1.0f, "%.2f") || something_changed;
        ImGui::PlotLines("F2", _curve2.data(), _curve2.size(), 0, "Filmic J.Hable 2016", 0.0f, 1.0f, ImVec2(512, 128));

        something_changed = ImGui::SliderFloat("ACES Gamma", &g_ACESGamma, 1.0f, 2.2f, "%.2f") || something_changed;
        ImGui::PlotLines("A", _curve3.data(), _curve3.size(), 0, "ACES", 0.0f, 1.2f, ImVec2(512, 128));

        if (something_changed)
        {
            update_tonemap_curves();
        }

        // if combobox
        // choose which lut to fill
    }
    ImGui::End();


    //
    // LABELS
    //
    float stride = (float)_fb_width / 4.0f;
    float start = stride / 2.0f;
    ImGuiLabelWindow("Clamped Linear", start, 20.0f);
    start += stride;
    ImGuiLabelWindow("Filmic J.Hable 2016 (LUT)", start, 20.0f);
    start += stride;
    ImGuiLabelWindow("ACES + Gamma", start, 20.0f);
    start += stride;
    ImGuiLabelWindow("Filmic Uncharted 2 (with Gamma)", start, 20.0f);
}
