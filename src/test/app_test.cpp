#include "app_test.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm_usage.h"
#include "tiny_obj_loader.h"

#include <vector>
#include <fstream>




// TODO: move to common, tiny_obj_callbacks.

typedef struct
{
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texcoords;
    std::vector<int> v_indices;
    std::vector<int> vn_indices;
    std::vector<int> vt_indices;

    std::vector<tinyobj::material_t> materials;

} MyMesh;

MyMesh *g_Mesh = nullptr;

static
void vertex_cb(void *user_data, float x, float y, float z, float w)
{
    MyMesh *mesh = reinterpret_cast<MyMesh *>(user_data);
    printf("v[%zd] = %f, %f, %f (w %f)\n", mesh->vertices.size() / 3, x, y, z, w);

    mesh->vertices.push_back(x);
    mesh->vertices.push_back(y);
    mesh->vertices.push_back(z);
    // Discard w
}

static
void normal_cb(void *user_data, float x, float y, float z)
{
    MyMesh *mesh = reinterpret_cast<MyMesh *>(user_data);
    printf("vn[%zd] = %f, %f, %f\n", mesh->normals.size() / 3, x, y, z);

    mesh->normals.push_back(x);
    mesh->normals.push_back(y);
    mesh->normals.push_back(z);
}

static
void texcoord_cb(void *user_data, float x, float y, float z)
{
    MyMesh *mesh = reinterpret_cast<MyMesh *>(user_data);
    printf("vt[%zd] = %f, %f, %f\n", mesh->texcoords.size() / 3, x, y, z);

    mesh->texcoords.push_back(x);
    mesh->texcoords.push_back(y);
    mesh->texcoords.push_back(z);
}

static
void index_cb(void *user_data, tinyobj::index_t *indices, int num_indices)
{
    // NOTE: the value of each index is raw value.
    // For example, the application must manually adjust the index with offset
    // (e.g. v_indices.size()) when the value is negative(whic means relative
    // index).
    // Also, the first index starts with 1, not 0.
    // See fixIndex() function in tiny_obj_loader.h for details.
    // Also, 0 is set for the index value which
    // does not exist in .obj
    MyMesh *mesh = reinterpret_cast<MyMesh *>(user_data);

    for (int i = 0; i < num_indices; i++)
    {
        tinyobj::index_t idx = indices[i];
        printf("idx[%zd] = %d, %d, %d\n", mesh->v_indices.size(), idx.vertex_index, idx.normal_index, idx.texcoord_index);

        if (idx.vertex_index != 0)
        {
            mesh->v_indices.push_back(idx.vertex_index);
        }
        if (idx.normal_index != 0)
        {
            mesh->vn_indices.push_back(idx.normal_index);
        }
        if (idx.texcoord_index != 0)
        {
            mesh->vt_indices.push_back(idx.texcoord_index);
        }
    }
}

static
void usemtl_cb(void *user_data, const char *name, int material_idx)
{
    MyMesh *mesh = reinterpret_cast<MyMesh *>(user_data);
    if ((material_idx > -1) && (material_idx < mesh->materials.size()))
    {
        printf("usemtl. material id = %d(name = %s)\n", material_idx, mesh->materials[material_idx].name.c_str());
    }
    else
    {
        printf("usemtl. name = %s\n", name);
    }
}

static
void mtllib_cb(void *user_data, const tinyobj::material_t *materials, int num_materials)
{
    MyMesh *mesh = reinterpret_cast<MyMesh *>(user_data);
    printf("mtllib. # of materials = %d\n", num_materials);

    for (int i = 0; i < num_materials; i++)
    {
        mesh->materials.push_back(materials[i]);
    }
}

static
void group_cb(void *user_data, const char **names, int num_names)
{
    // MyMesh *mesh = reinterpret_cast<MyMesh*>(user_data);
    printf("group : name = \n");

    for (int i = 0; i < num_names; i++)
    {
        printf("  %s\n", names[i]);
    }
}

static
void object_cb(void *user_data, const char *name)
{
    // MyMesh *mesh = reinterpret_cast<MyMesh*>(user_data);
    printf("object : name = %s\n", name);
}









bool AppTest::init()
{
    tinyobj::callback_t cb;
    cb.vertex_cb = vertex_cb;
    cb.normal_cb = normal_cb;
    cb.texcoord_cb = texcoord_cb;
    cb.index_cb = index_cb;
    cb.usemtl_cb = usemtl_cb;
    cb.mtllib_cb = mtllib_cb;
    cb.group_cb = group_cb;
    cb.object_cb = object_cb;

    g_Mesh = new MyMesh();

    std::string warn;
    std::string err;
    //std::string filename = "sponza.obj";
    std::string filename = "CornellBox-Original.obj";
    std::ifstream ifs(filename.c_str());
    if (ifs.fail())
    {
        printf("FAILED to open file: %s\n", filename.c_str());
        return false;
    }
    tinyobj::MaterialFileReader mtlReader("./");
    bool ret = tinyobj::LoadObjWithCallback(ifs, cb, g_Mesh, &mtlReader, &warn, &err);
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
        printf("FAILED to parse: %s\n", filename.c_str());
    }

    printf("# of vertices         = %zd\n", g_Mesh->vertices.size()/3);
    printf("# of normals          = %zd\n", g_Mesh->normals.size()/3);
    printf("# of texcoords        = %zd\n", g_Mesh->texcoords.size()/2);
    printf("# of vertex indices   = %zd\n", g_Mesh->v_indices.size());
    printf("# of normal indices   = %zd\n", g_Mesh->vn_indices.size());
    printf("# of texcoord indices = %zd\n", g_Mesh->vt_indices.size());
    printf("# of materials        = %zd\n", g_Mesh->materials.size());

    return true;
}

void AppTest::shutdown()
{
    delete g_Mesh;
}

void AppTest::draw(int width, int height)
{
    glViewport(0, 0, width, height);
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void AppTest::onMousePress(int x, int y)
{

}

void AppTest::onKeyboard(GLFWwindow * window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}
