#ifndef MYTRAYICON_H
#define MYTRAYICON_H

#include <gtkmm.h>
#include <libayatana-appindicator3-0.1/libayatana-appindicator/app-indicator.h>
#include <string>
#include <sigc++/sigc++.h>

class MyTrayIcon
{
public:
    MyTrayIcon(const std::string &icon_path, const std::string &icon_path_running);
    void set_status(bool is_running);
    void on_run();
    void on_next();
    void on_exit();
    void set_menu(std::string menu_item);

    // Signal for the Next action
    sigc::signal<void> signal_next();

private:
    AppIndicator *indicator_;
    Gtk::Menu *menu_;
    Gtk::MenuItem *menu_run_;
    Gtk::MenuItem *menu_next_;
    Gtk::MenuItem *menu_exit_;
    Gtk::StatusIcon *status_icon;

    std::string icon_path_;
    std::string icon_path_running_;

    sigc::signal<void> next_signal_;

    void signal_next_emit(); // Method to emit the next signal
};

#endif // MYTRAYICON_H
