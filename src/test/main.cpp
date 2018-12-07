#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "gl_utils.h"
#include "app_test.h"

#include <chrono>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

static
void error_callback(int error, const char* description)
{
    printf("Error: %s\n", description);
}

static
void window_size_callback(GLFWwindow *window, int w, int h)
{
    App *app = (App*)glfwGetWindowUserPointer(window);
    if (app)
    {
        app->onWindowSize(window, w, h);
    }
}

static
void framebuffer_size_callback(GLFWwindow *window, int w, int h)
{
    App *app = (App*)glfwGetWindowUserPointer(window);
    if (app)
    {
        app->onFramebufferSize(window, w, h);
    }
}

static
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    App *app = (App*)glfwGetWindowUserPointer(window);
    if (app)
    {
        app->onKeyboard(window, key, scancode, action, mods);
    }
    else // TODO: not necessary
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

static
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    App *app = (App*)glfwGetWindowUserPointer(window);
    if (app)
    {
        app->onMouseClick(window, button, action, mods);
    }
}

static
void cursor_pos_callback(GLFWwindow* window, double mouse_x, double mouse_y)
{
    App *app = (App*)glfwGetWindowUserPointer(window);
    if (app)
    {
        app->onMouseMove(window, mouse_x, mouse_y);
    }
}

static
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    App *app = (App*)glfwGetWindowUserPointer(window);
    if (app)
    {
        app->onMouseScroll(window, xoffset, yoffset);
    }
}

static
bool init(GLFWwindow *window)
{
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);

    App *app = (App*)glfwGetWindowUserPointer(window);
    return (app && app->init(w, h));
}

static
void run(GLFWwindow* window)
{
    App *app = (App*)glfwGetWindowUserPointer(window);
    if (app)
    {
        app->run();
    }
}

void show_fps_window(bool should_refresh_fps, uint64_t fps)
{
    ImGuiWindowFlags window_flags = 0;
    //window_flags |= ImGuiWindowFlags_MenuBar;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoScrollbar;
    //window_flags |= ImGuiWindowFlags_NoMove;
    //window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoNav;
    bool *pOpen = nullptr;

    ImGui::Begin("FPS", pOpen, window_flags);

    static float values[30] = {0};
    static int values_offset = 0;
    static uint64_t max_fps = 1;
    if (should_refresh_fps)
    {
        values[values_offset] = (float)fps;
        values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
        if (fps > max_fps)
            max_fps = fps;
    }
    std::string text = std::string("FPS: ") + std::to_string(fps);
    ImGui::PushItemWidth(-1);
    ImGui::PlotLines("", values, IM_ARRAYSIZE(values), values_offset, text.c_str(), 0.0f, (float)max_fps, ImVec2(0, 50));
    //ImGui::PlotHistogram("", values, IM_ARRAYSIZE(values), 0, text.c_str(), 0.0f, (float)max_fps, ImVec2(0, 50));

    ImGui::End();
}

int main(int argc, char **argv)
{
    App *app = new AppTest();

    if (glfwInit() != GLFW_TRUE)
        return EXIT_FAILURE;
    
    glfwSetErrorCallback(error_callback);

    const char *glsl_version = "#version 460";
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
    glfwWindowHint(GLFW_DECORATED, GL_TRUE);
    GLFWwindow *window = glfwCreateWindow(1280, 720, "Test", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwSetWindowUserPointer(window, app);

    glfwSetWindowSizeCallback(window, window_size_callback);           // which of the two should be called??
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // ...
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        printf("Error initializing GLEW: %s\n", (const char*)glewGetErrorString(err));
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }
    
    glfwSwapInterval(1); // ou pas hein!

    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImGui::StyleColorsDark();

    // INIT GL RESOURCES
    if (!init(window))
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glutils::check_error();

    bool show_demo_window = true;
    
    // FPS
    auto timer = std::chrono::steady_clock();
    auto last_time = timer.now();
    auto last_time_for_dt = last_time;
    bool should_refresh_fps = false;
    uint64_t frame_counter = 0;
    uint64_t fps = 0;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // FPS
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(timer.now() - last_time_for_dt);
        last_time_for_dt = timer.now();
        float dt = duration.count() / 1000000.0f;
        should_refresh_fps = false;
        ++frame_counter;
        if (last_time + std::chrono::seconds(1) < timer.now())
        {
            last_time = timer.now();
            fps = frame_counter;
            frame_counter = 0;
            should_refresh_fps = true;
        }

        show_fps_window(should_refresh_fps, fps);

        if (show_demo_window)
        {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        ImGui::Render();

        run(window);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    delete app;
    return EXIT_SUCCESS;
}
