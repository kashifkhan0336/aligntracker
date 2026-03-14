
//namespaces
using namespace std::chrono;
using namespace std::chrono_literals;
using namespace sqlite_orm;

//Shell_notifyicon

constexpr UINT WM_MY_NOTIFYICON = WM_APP + 1;
const UINT MY_ICON_ID = 100;
NOTIFYICONDATA nid = {};
WNDPROC original_wndproc;

//state

struct Time {
    int id;
    time_point<local_t, system_clock::duration> time;
};
std::queue<Time> times;
struct Reminder {
private:
    zoned_time<system_clock::duration,const time_zone*> local_time = zoned_time{ current_zone(), system_clock::now() };
	time_point<local_t, system_clock::duration> current_time = local_time.get_local_time();
public:
    int id;
    std::string time;
    std::string status = "pending";
	std::string occasion;
	bool acknowledged = false;
    std::string created_on = std::vformat("{:%Y-%m-%d %H:%M:%S}", std::make_format_args(current_time));
};
Reminder currentNotification{};
auto storage = make_storage("data.db", make_table("reminders",
    make_column("id", &Reminder::id, primary_key().autoincrement()),
    make_column("time", &Reminder::time),
    make_column("status", &Reminder::status, default_value("pending")),
    make_column("occasion", &Reminder::occasion),
    make_column("acknowledged", &Reminder::acknowledged, default_value(false)),
    make_column("created_on", &Reminder::created_on)
));

//retrive and merge pending alarms to reminders queue
void getAlarms() {
    auto pending_alarms = storage.get_all<Reminder>(where(c(&Reminder::status) == "pending"));
    for (auto& alarm : pending_alarms) {
        std::cout << alarm.time << "," << alarm.status << std::endl;
        std::tm tm{};
        std::stringstream ss(alarm.time);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        auto tp = system_clock::from_time_t(std::mktime(&tm));
        auto local_time = zoned_time{ current_zone(), tp };
        times.push(Time{ alarm.id, local_time.get_local_time() });
        std::cout << local_time << std::endl;
    }
}
std::atomic<bool> isNotificationShown(false);

void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
	//std::cout << "TimerProc called at " << std::chrono::system_clock::to_time_t(system_clock::now()) << "\n";
    auto local_zone_time = zoned_time{ current_zone(), system_clock::now() };
    if (!times.empty()) {
        auto duration = duration_cast<seconds>(times.front().time - local_zone_time.get_local_time());
        std::cout << duration;
        if (duration < 5s && duration > 3s) {
            std::cout << "Time for alarm" << "\n";
			//currentNotificationText = "Time for " + storage.get<Reminder>(times.front().id).occasion;
			//currentNotificationId = times.front().id;
			currentNotification.id = times.front().id;
			currentNotification.time = storage.get<Reminder>(times.front().id).time;
			currentNotification.status = storage.get<Reminder>(times.front().id).status;
			currentNotification.occasion = storage.get<Reminder>(times.front().id).occasion;
			currentNotification.acknowledged = storage.get<Reminder>(times.front().id).acknowledged;
            isNotificationShown = true;
            storage.update_all(set(c(&Reminder::status) = "done"), where(c(&Reminder::id) == times.front().id));
            times.pop();
        }
    }
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

}
void timerFunc() {

    std::cout << "Timer Thread started\n";
    SetTimer(NULL, 0, 1000, (TIMERPROC)TimerProc);
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
const UINT IDI_EXIT = 421;
const UINT IDI_TEA = 422;
const UINT IDI_LUNCH = 423;
const UINT IDI_DINNER = 424;

//Custom wndproc of main_window for shell_notifyicon
LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CLOSE:
        ShowWindow(hwnd, SW_HIDE); // Hide the window
        return 0; // Handled the message, do not close the window
    case WM_SYSCOMMAND:
        if ((wParam & 0xFFF0) == SC_MINIMIZE) {
            
            ShowWindow(hwnd, SW_HIDE);
            return 0;
        }
        break;
    case WM_MY_NOTIFYICON:
        switch (LOWORD(lParam)) {
        case WM_RBUTTONUP: //right click
        case WM_CONTEXTMENU: //context menu
            POINT pt;
            GetCursorPos(&pt);
            {
                HMENU hMenu = CreatePopupMenu();
                AppendMenu(hMenu, MF_STRING, IDI_EXIT, "Exit");
                AppendMenu(hMenu, MF_STRING, IDI_TEA, "Tea");
                AppendMenu(hMenu, MF_STRING, IDI_LUNCH, "Lunch");
                AppendMenu(hMenu, MF_STRING, IDI_DINNER, "Dinner");
                UINT item = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY | TPM_LEFTBUTTON, pt.x, pt.y, 0, hwnd, NULL); //this will block
                std::cout << "item -> " << item << std::endl;
                switch (item)
                {
                case IDI_TEA:
                {
                    auto local_zone_time = zoned_time{ current_zone(), system_clock::now() };
                    auto t2 = local_zone_time.get_local_time() + 20s; //will be 2min for testing, change to 1h for actual use
                    Reminder reminder;
                    reminder.id = -1;
                    reminder.time = std::vformat("{:%Y-%m-%d %H:%M:%S}", std::make_format_args(t2));
                    reminder.status = "pending";
                    reminder.occasion = "Tea";
                    reminder.acknowledged = false;
                    auto reminder1Id = storage.insert(reminder);
                    //we need to merge pending alarms to queue after every write
                    getAlarms();
                    std::cout << reminder1Id << std::endl;
                }

                    break;
                case IDI_LUNCH:
                {
                    auto local_zone_time = zoned_time{ current_zone(), system_clock::now() };
                    auto t2 = local_zone_time.get_local_time() + 20s; //will be 2min for testing, change to 1h for actual use
                    Reminder reminder;
                    reminder.id = -1;
                    reminder.time = std::vformat("{:%Y-%m-%d %H:%M:%S}", std::make_format_args(t2));
                    reminder.status = "pending";
                    reminder.occasion = "Lunch";
                    reminder.acknowledged = false;
                    auto reminder1Id = storage.insert(reminder);
                    //we need to merge pending alarms to queue after every write
                    getAlarms();
                    std::cout << reminder1Id << std::endl;
                }
                    break;
                case IDI_DINNER:
                {
                    auto local_zone_time = zoned_time{ current_zone(), system_clock::now() };
					auto t2 = local_zone_time.get_local_time() + 20s; //will be 2min for testing, change to 1h for actual use
                    Reminder reminder; 
					reminder.id = -1;
					reminder.time = std::vformat("{:%Y-%m-%d %H:%M:%S}", std::make_format_args(t2));
					reminder.status = "pending";
					reminder.occasion = "Dinner";
					reminder.acknowledged = false;
                    
                    auto reminder1Id = storage.insert(reminder);
                    //we need to merge pending alarms to queue after every write
                    getAlarms();
                    std::cout << reminder1Id << std::endl;
                }
                    break;
                default:
                    break;
                }
                return 0;
                
            }
            
        case WM_LBUTTONDBLCLK: // Double-click event
            ShowWindow(hwnd, SW_SHOW); // Restore the window
            // Optional: remove the tray icon when window is visible
            // Shell_NotifyIcon(NIM_DELETE, &nid); 
            break;
        }
    }
    //pass control back to glfw wndProc
    return CallWindowProc(original_wndproc, hwnd, uMsg, wParam, lParam);
}



//notification window
void showNotificationWindow() {
    // Show the notification window
    // You can use ImGui to render the content of the notification window here
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
        ImGui::Text(currentNotification.occasion.c_str());  // Requires FontAwesome or similar
        // Without icon, just use:
        // ImGui::Text("Notification Title");
        ImGui::Separator();

        // --- Message area ---
        ImGui::TextWrapped("This is a Windows‑style notification with three buttons below. The message can be longer and will wrap.");

        ImGui::Dummy(ImVec2(0, 8)); // spacing


        float button_width = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2) / 3.0f;

        // Button 1
        if (ImGui::Button("Snooze", ImVec2(button_width, 0)))
        {
            auto local_zone_time = zoned_time{ current_zone(), system_clock::now() };
            auto t2 = local_zone_time.get_local_time() + 20s;
            storage.update_all(set(c(&Reminder::acknowledged) = true), where(c(&Reminder::id) == currentNotification.id));
			storage.update_all(
                set(
                    c(&Reminder::time) = std::vformat("{:%Y-%m-%d %H:%M:%S}", std::make_format_args(t2))
                , c(&Reminder::status) = "pending"), where(c(&Reminder::id) == currentNotification.id));
			getAlarms();
            isNotificationShown = false;

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
			storage.update_all(set(c(&Reminder::acknowledged) = true), where(c(&Reminder::id) == currentNotification.id));
			isNotificationShown = false;
        }

        ImGui::End();
    }

}


int main()
{
    //vars
    GLFWwindow* main_window;
    GLFWwindow* notification_window;
    int notification_window_width = 600;
    int notification_window_height = 300;
    storage.sync_schema();
    getAlarms();
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

    //get main_window hwnd for shell_notifyicon
    HWND main_window_hwnd = glfwGetWin32Window(main_window);
    original_wndproc = (WNDPROC)GetWindowLongPtr(main_window_hwnd, GWLP_WNDPROC);
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(main_window_hwnd, GWLP_HINSTANCE);
    SetWindowLongPtr(main_window_hwnd, GWLP_WNDPROC, (LONG_PTR)MainWindowProc);

    //NOTIFYICONDATA
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = main_window_hwnd;
    nid.uID = MY_ICON_ID;
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP;
    nid.uCallbackMessage = WM_MY_NOTIFYICON;
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    //lstrcpyn(nid.szTip, TEXT("MY C++ Tray"), ARRAYSIZE(nid.szTip));
    strcpy(nid.szTip, "Dashboard");
    
    Shell_NotifyIcon(NIM_ADD, &nid);
    //(NIM_DELETE, &nid);
    nid.uVersion = NOTIFYICON_VERSION_4;
    Shell_NotifyIcon(NIM_SETVERSION, &nid);
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
\
while (!glfwWindowShouldClose(main_window) && !glfwWindowShouldClose(notification_window))
    {
		//glfwWaitEvents(); // Wait for events to avoid busy looping when windows are not active
        glfwMakeContextCurrent(main_window);
		glfwSwapInterval(1); // Enable vsync
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

        if (isNotificationShown) {

            glfwShowWindow(notification_window);
        }
        else {
            glfwHideWindow(notification_window);
        }
        ImGui::SetCurrentContext(notification_window_ctx);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        showNotificationWindow();


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
    Shell_NotifyIcon(NIM_DELETE, &nid);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

