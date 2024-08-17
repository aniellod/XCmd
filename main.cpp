#include <iostream>
#include <cstdlib> // to read environment variables
#include <thread>

#include "MyTrayIcon.h"

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <syslog.h>

const char* g_PATH = ".xcmd-settings.csv";
inline bool static g_running = TRUE;

void send_to_socket(const std::string& message);


int socket_fd = -1;

struct AppConfig
{
    std::string executable;
    std::string flag;
    std::string arguments;
    std::string socketMessage;
    bool isSocketAction;
    pid_t pid;
    bool g_running;

    // Default constructor
    AppConfig() : executable(""), flag(""), arguments(""), socketMessage(""), isSocketAction(false), pid(0), g_running(false) {}

    AppConfig(const std::string& exe, const std::string& flg, const std::string& args, const std::string& socketMsg, bool socketAction)
        : executable(exe), flag(flg), arguments(args), socketMessage(socketMsg), isSocketAction(socketAction), pid(0), g_running(false) {}
};

std::unordered_map<KeySym, AppConfig> g_keyAppMap;

// Trim doesn't exist natively so I need this function to trim whitespace from a string
std::string trim(const std::string &str)
{
    size_t first = str.find_first_not_of(' ');
    if (first == std::string::npos)
        return str;
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, last - first + 1);
}

void init_logging()
{
    openlog("XCmd", LOG_PID | LOG_CONS, LOG_USER);
}

// Logging function
void logMessage(const std::string& message)
{
    std::ofstream logFile;
    logFile.open("/var/log/XCmd", std::ios_base::app);
    if (logFile.is_open())
        {
            logFile << message << std::endl;
            logFile.close();
        }
    else
        {
            std::cerr << "Failed to open log file: /var/log/XCmd" << std::endl;
        }
}

std::string envValue(const std::string& envVariable)
{
    const char* envValue = std::getenv(envVariable.c_str());
    return (envValue != nullptr) ? std::string(envValue) : "";
}

void run_gtk_main()
{
    Gtk::Main::run();
    exit(0);
}

void readConfig(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
        {
            std::cerr << "Failed to open config file: " << filename << std::endl;
            exit(1);
        }
    std::string line;
    g_keyAppMap.clear();  // Clear the existing configuration
    while (std::getline(file, line))
        {
            std::stringstream ss(line);
            std::string keySymStr, executable, flag, arguments, socketMessage;
            bool isSocketAction = false;
            std::getline(ss, keySymStr, ',');
            std::getline(ss, executable, ',');
            std::getline(ss, flag, ',');
            std::getline(ss, arguments, ',');
            std::getline(ss, socketMessage, ',');

            keySymStr = trim(keySymStr);
            if (!socketMessage.empty())
                {
                    isSocketAction = true;
                }

            // Directly use the character to get the KeySym
            KeySym keySym = XStringToKeysym(keySymStr.c_str());
            if (keySym == NoSymbol)
                {
                    std::cerr << "Invalid KeySym: " << keySymStr << std::endl;
                    continue;
                }

            g_keyAppMap[keySym] = AppConfig(executable, flag, arguments, socketMessage, isSocketAction);
            std::cout << "Configured: " << keySymStr << " -> " << executable << " " << flag << " " << arguments << " " << socketMessage << std::endl; // Debugging info



        }
    file.close();
}

void launchApp(const KeySym &keySym, MyTrayIcon &tray_icon)
{
    auto& config = g_keyAppMap[keySym];
    if (config.isSocketAction)
        {
            send_to_socket(config.socketMessage);
        }
    else
        {
            if (config.g_running)
                {
                    kill(config.pid, SIGTERM);
                    config.g_running = false;
                    tray_icon.set_status(FALSE);
                    logMessage("Application " + config.executable + " killed");
                    std::cout << "\nApplication " << config.executable << " killed" << std::endl;
                }
            else
                {
                    pid_t pid = fork();
                    if (pid == 0)
                        {
                            // Child process --
                            std::cout << "Launching: " << config.executable << " " << config.flag << " " << config.arguments << std::endl;
                            logMessage("Launching " + config.executable + " " + config.flag + " " + config.arguments);
                            execl(config.executable.c_str(), config.executable.c_str(), (config.flag.length()<1?NULL:config.flag.c_str()), config.arguments.c_str(), (char *)NULL);
                            // If execl returns, it must have failed
                            perror("execl failed, check your settings file.");
                            exit(1);
                        }
                    else if (pid > 0)
                        {
                            config.pid = pid;
                            config.g_running = true;
                            tray_icon.set_status(TRUE);
                            logMessage("Application " + config.executable + " launched");
                            std::cout << "Application " << config.executable << " launched" << std::endl;
                        }
                    else
                        {
                            std::string message = "Failed to launch application " + config.executable;
                            logMessage(message);
                            std::cerr << message << std::endl;
                        }
                }
        }
}

void send_to_socket(const std::string& message)
{
    if (socket_fd == -1)
        {
            socket_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (socket_fd < 0)
                {
                    logMessage("Failed to create socket");
                    std::cerr << "Failed to create socket" << std::endl;
                    return;
                }
            struct sockaddr_in server_addr;
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(21002);
            if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
                {
                    logMessage("Invalid address");
                    std::cerr << "Invalid address" << std::endl;
                    close(socket_fd);
                    socket_fd = -1;
                    return;
                }
            if (connect(socket_fd, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0)
                {
                    logMessage("Failed to connect to server");
                    std::cerr << "Failed to connect to server" << std::endl;
                    close(socket_fd);
                    socket_fd = -1;
                    return;
                }
        }
    if (write(socket_fd, message.c_str(), message.length()) < 0)
        {
            logMessage("Failed to send message");
            std::cerr << "Failed to send message" << std::endl;
            close(socket_fd);
            socket_fd = -1;
            return;
        }
    std::string wmessage = "Message sent to socket: " + message;
    logMessage(wmessage);
}

void on_next()
{
    std::cout << "Next action triggered from main" << std::endl;
    send_to_socket("next");
}

int main(int argc, char* argv[])
{
    bool super_r_pressed = false;
    bool combination_detected = false;

    // Initialize logging
    init_logging();

    //read settings
    std::string home = envValue("HOME");
    std::string path = home + "/" + g_PATH;
    readConfig(path);

    Gtk::Main kit(argc, argv);  // Initialize GTK
    std::string icon_path = "/home/dimeglio@dm.ici/Documents/dev/code blocks projects/XS/red-icon.png";
    std::string icon_path_g_running = "/home/dimeglio@dm.ici/Documents/dev/code blocks projects/XS/green-icon.png";
    MyTrayIcon tray_icon(icon_path, icon_path_g_running);

    std::thread gtk_thread(run_gtk_main);

    // Connect the next signal to the on_next function
    tray_icon.signal_next().connect(sigc::ptr_fun(&on_next));

    /* Get the DISPLAY environment variable */
    char *xDisp = getenv ("DISPLAY");
    if (xDisp == NULL)
        {
            std::cerr << "DISPLAY environment variable is not set." << std::endl;
            return 1;
        }

    Display* display = XOpenDisplay(xDisp);
    if (display == NULL)
        {
            logMessage("Cannot open display");
            std::cerr << "Cannot open display" << std::endl;
            return 1;
        }

    Window root = DefaultRootWindow(display);
    XEvent event;
    KeyCode superRKeyCode = XKeysymToKeycode(display, XK_Super_R);
    XGrabKey(display, superRKeyCode, AnyModifier, root, True, GrabModeAsync, GrabModeAsync);
    XSelectInput(display, root, KeyPressMask | KeyReleaseMask);

    logMessage("XCmd started");

    while (g_running)
        {
            XNextEvent(display, &event);
            if (event.type == KeyPress || event.type == KeyRelease)
                {
                    XKeyEvent* keyEvent = (XKeyEvent*)&event;
                    if (keyEvent->keycode == superRKeyCode)
                        {
                            if (event.type == KeyPress)
                                {
                                    super_r_pressed = true;
                                    combination_detected = false;
                                }
                            else if (event.type == KeyRelease)
                                {
                                    super_r_pressed = false;
                                    if (!combination_detected)
                                        {
                                            logMessage("No action configured for this key combination");
                                            std::cerr << "No action configured for this key combination" << std::endl;
                                        }
                                }
                        }

                    if (event.type == KeyPress && super_r_pressed)
                        {
                            KeySym keySym = XkbKeycodeToKeysym(display, keyEvent->keycode, 0, 0);
                            if (keySym == XK_q)
                                {
                                    Gtk::Main::quit();
                                    break; //to exit.
                                }
                            if (keySym == XK_r)
                                {
                                    logMessage("Reloading configuration...");
                                    std::cout << "Reloading configuration..." << std::endl;
                                    readConfig(path);
                                    logMessage("Configuration reloaded.");
                                    std::cout << "Configuration reloaded." << std::endl;
                                }
                            else if (g_keyAppMap.find(keySym) != g_keyAppMap.end())
                                {
                                    combination_detected = true;
                                    launchApp(keySym, tray_icon);
                                }
                            else
                                {
                                    combination_detected = true;  // Consider it detected even if no action is configured
                                }
                        }
                }

        }

    gtk_thread.join();
    Gtk::Main::quit();
    XCloseDisplay(display);
    logMessage("XCmd stopped");
    std::cout << "XCmd stopped\n";
    closelog();

    return 0;
}
