#define GLFW_INCLUDE_NONE
#include <iostream>
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <Windows.h>
#include <thread>

std::atomic<bool> isNotificationShown(true);

void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime) {

    isNotificationShown = !isNotificationShown;

    std::cout << isNotificationShown << "Time up!\n";
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

}
void timerFunc() {

    std::cout << "Timer Thread started\n";
    SetTimer(NULL, 0, 3000, (TIMERPROC)TimerProc);
    MSG msg;
    // Main message loop
    while (GetMessage(&msg, NULL, 0, 0)) {
        // In this simple case, we are only interested in timer messages
        if (msg.message == WM_TIMER) {
            DispatchMessage(&msg); // This dispatches the message and calls TimerProc
        }
    }
    std::cout << "Thread over" << "\n";
}

int main()
{
    //vars
    GLFWwindow* main_window;
    GLFWwindow* notification_window;
    int notification_window_width = 600;
    int notification_window_height = 300;

    if (!glfwInit()) {
        std::cout << "GLFW failed!";
    }

    std::thread timerThread(timerFunc);
    timerThread.detach();


    //taskbar calc
    int monitor_width = 1920;
    int monitor_height = 1080;
    int taskbar_height = 60;
    int edge = 10;
    int work_area_width, work_area_height;

    glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), NULL, NULL, &work_area_width, &work_area_height);
    std::cout << work_area_width << "," << work_area_height;





    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    main_window = glfwCreateWindow(800, 600, "Dashboard", NULL, NULL);
    if (!main_window) {
        std::cout << "Dashboard window creation failed!";
    }

#pragma region Win32
    glfwWindowHint(GLFW_RESIZABLE, 0);
    glfwWindowHint(GLFW_DECORATED, 0);
    glfwWindowHint(GLFW_FLOATING, 1);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, 0);

    notification_window = glfwCreateWindow(notification_window_width, notification_window_height, "", NULL, main_window);
    if (!notification_window) {
        std::cout << "Notification window creation failed!";
    }
    glfwSetWindowPos(notification_window,
        (monitor_width - notification_window_width) - edge,
        (monitor_height - taskbar_height - notification_window_height) - edge
    );
    HWND hwnd = glfwGetWin32Window(notification_window);
    LONG ex = GetWindowLong(hwnd, GWL_EXSTYLE);
    ex |= WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE;
    ex &= ~WS_EX_APPWINDOW;
    SetWindowLong(hwnd, GWL_EXSTYLE, ex);
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
        SWP_FRAMECHANGED);
    ShowWindow(hwnd, SW_HIDE);
    ShowWindow(hwnd, SW_SHOWNOACTIVATE);





#pragma endregion

    glfwMakeContextCurrent(main_window);
    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cout << "Glad failed to load!";
    }
    //second window


    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());


    IMGUI_CHECKVERSION();
    ImGuiContext* main_window_ctx = ImGui::CreateContext();
    ImGui::SetCurrentContext(main_window_ctx);
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;
    ImGui_ImplGlfw_InitForOpenGL(main_window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    glfwMakeContextCurrent(notification_window);
    ImGuiContext* notification_window_ctx = ImGui::CreateContext();
    ImGui::SetCurrentContext(notification_window_ctx);
    ImGui_ImplGlfw_InitForOpenGL(notification_window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    while (!glfwWindowShouldClose(main_window) && !glfwWindowShouldClose(notification_window))
    {

        glfwMakeContextCurrent(main_window);

        ImGui::SetCurrentContext(main_window_ctx);
        if (glfwGetWindowAttrib(main_window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }


        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {

            bool show_another_window = true;
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            //ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(main_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(main_window);


        /* Make the second window's context current */
        glfwMakeContextCurrent(notification_window);
        //if (isNotificationShown) {

        //    glfwShowWindow(notification_window);
        //}
        //else {
        //    glfwHideWindow(notification_window);
        //}
        ImGui::SetCurrentContext(notification_window_ctx);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::SetNextWindowPos(ImVec2(0, 0));

            ImGui::Begin(
                "Hello, world!",
                NULL,
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration
            );

            ImGui::SetWindowFontScale(1.3f);
            // --- Title (optional with an icon) ---
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4); // extra top spacing
            // You could load a small icon texture and display it with ImGui::Image
            // For simplicity, we use a text emoji or a symbol
            ImGui::Text("Notification");  // Requires FontAwesome or similar
            // Without icon, just use:
            // ImGui::Text("Notification Title");
            ImGui::Separator();

            // --- Message area ---
            ImGui::TextWrapped("This is a Windows‑style notification with three buttons below. The message can be longer and will wrap.");

            ImGui::Dummy(ImVec2(0, 8)); // spacing


            float button_width = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2) / 3.0f;

            // Button 1
            if (ImGui::Button("Accept", ImVec2(button_width, 0)))
            {
                // action

            }
            ImGui::SameLine();

            // Button 2
            if (ImGui::Button("More info", ImVec2(button_width, 0)))
            {
                // action

            }
            ImGui::SameLine();

            // Button 3
            if (ImGui::Button("Dismiss", ImVec2(button_width, 0)))
            {
                // action

            }

            ImGui::End();
        }

        ImGui::Render();
        int ndisplay_w, ndisplay_h;
        glfwGetFramebufferSize(notification_window, &ndisplay_w, &ndisplay_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(notification_window);


        glfwPollEvents();
        // Keep running
    }
    glfwDestroyWindow(main_window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

