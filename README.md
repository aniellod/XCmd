# XCmd
A Customizable, GTK-Based Hotkey Manager with Socket Integration

Overview

XCmd is a powerful, highly configurable hotkey manager that integrates with GTK and facilitates communication via sockets. It reads settings from a CSV file (~/.xcmd-settings.csv by default), allowing users to define custom key combinations, associated executables, flags, arguments, and socket messages. The application features a system tray icon, logging, and a reload mechanism for effortless configuration updates.

Key Features:

    Customizable Hotkeys:
    * Define key combinations (e.g., Super+R + [key]) and associate them with:
    + Executables with flags and arguments
    + Socket messages (sent to localhost:21002 by default)
    
    GTK System Tray Integration:
    * Displays a tray icon with customizable icons for running/non-running states
    * Supports a "Next" action, triggering a socket message ("next" by default)
    
    Configuration and Reloading:
    * Reads settings from a CSV file (~/.xcmd-settings.csv by default)
    * Reloads configuration on Super+R + R hotkey press
    Logging and Error Handling:
    * Logs events to /var/log/XCmd (configurable) and stderr
    * Reports errors and invalid configurations

Building:
    
    You'll need a C++ compiler (like g++) and the development headers for X11 and GTK+.
    
    Compile the code with necessary flags:

CXXFLAGS=$(pkg-config --cflags gtkmm-3.0 libayatana-appindicator3-0.1)
LIBS=$(pkg-config --libs gtkmm-3.0 libayatana-appindicator3-0.1) -lX11 -lXtst

g++ -Wall -fexceptions -std=c++17 -Weffc++ -pg -Og -g $CXXFLAGS -I./ -c main.cpp -o main.o
g++ -Wall -fexceptions -std=c++17 -Weffc++ -pg -Og -g $CXXFLAGS -I./ -c MyTrayIcon.cpp -o MyTrayIcon.o
g++ -o XCmd main.o MyTrayIcon.o $CXXFLAGS $LIBS
    
Usage:

    Configuration:
    * Create/Edit ~/.xcmd-settings.csv with the format: KeySym,Executable,Flag,Arguments,SocketMessage (e.g., F1,myapp,-v,arg1,arg2,socket-msg)
    * Set HOME environment variable to locate the config file
    
    Run XCmd:
    * Execute the application, which will:
    + Initialize GTK and the system tray icon
    + Establish a socket connection (if needed)
    + Begin listening for hotkey presses
    
    Hotkey Usage:
    * Press Super+R + [configured key] to trigger associated actions
    * Press Super+R + R to reload the configuration
    * Press Super+R + Q to quit XCmd

    Ensure the configuration file exists and is properly formatted.
    Execute the compiled binary: ./XCmd
    A tray icon will appear. Use the configured hotkeys to trigger actions.

Configuration file format:
    ( Keyboard key,path/to/executable,command line parameter1,clp2,  )
    ( Keyboard key,,,,command  [sends command to port 21002]         )
    Example:
    F7,/usr/bin/xed,settings.csv,--new-window,

Dependencies and Build Requirements:

    GTK+ (for system tray integration)
    X11 and XKB libraries (for hotkey detection and window management)
    POSIX-compliant environment (for sockets, logging, and process management)
    XCmd grabs the Super_R key globally.
    The socket functionality is currently hardcoded to connect to 127.0.0.1:21002.
    Ensure you have the necessary permissions to run applications and write to the log file.

Notes:

    Ensure the DISPLAY environment variable is set for proper X11 interaction.
    Customize the socket connection and log file paths as needed.
    This implementation provides a solid foundation for further enhancements, such as:
    + Additional socket message formatting options
    + Support for multiple socket connections or destinations
    + Improved error handling and diagnostics
    + Integration with other GUI frameworks or notification systems

By leveraging XCmd, you can create a tailored, efficient hotkey experience that seamlessly bridges your keyboard, executables, and networked applications.


    
