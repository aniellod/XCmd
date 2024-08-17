#include "MyTrayIcon.h"
#include <iostream>
#include <glibmm/fileutils.h>

MyTrayIcon::MyTrayIcon(const std::string &icon_path, const std::string &icon_path_running)
    : icon_path_(icon_path), icon_path_running_(icon_path_running)
{
    std::cout << "MyTrayIcon constructor" << std::endl;

    if (!Glib::file_test(icon_path_, Glib::FILE_TEST_EXISTS))
        {
            std::cerr << "Icon file not found: " << icon_path_ << std::endl;
            return;
        }

    if (!Glib::file_test(icon_path_running_, Glib::FILE_TEST_EXISTS))
        {
            std::cerr << "Icon file not found: " << icon_path_running_ << std::endl;
            return;
        }

    std::cout << "Creating tray icon..." << std::endl;
    indicator_ = app_indicator_new("XCmd-Client", icon_path_.c_str(), APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
    app_indicator_set_icon_theme_path(indicator_, icon_path_.c_str());

    menu_ = new Gtk::Menu();

    menu_run_ = Gtk::manage(new Gtk::MenuItem("Run"));
    menu_run_->signal_activate().connect(sigc::mem_fun(*this, &MyTrayIcon::on_run));
    menu_->append(*menu_run_);

    menu_next_ = Gtk::manage(new Gtk::MenuItem("Next"));
    menu_next_->signal_activate().connect(sigc::mem_fun(*this, &MyTrayIcon::signal_next_emit));
    menu_->append(*menu_next_);

    menu_exit_ = Gtk::manage(new Gtk::MenuItem("Exit"));
    menu_exit_->signal_activate().connect(sigc::mem_fun(*this, &MyTrayIcon::on_exit));
    menu_->append(*menu_exit_);

    menu_->show_all();
    app_indicator_set_menu(indicator_, GTK_MENU(menu_->gobj()));
    app_indicator_set_status(indicator_, APP_INDICATOR_STATUS_ACTIVE);

    std::cout << "Tray icon created and visible." << std::endl;
}

void MyTrayIcon::set_menu(std::string menu_item)
{
    std::string MenuItem = menu_item;
}

void MyTrayIcon::set_status(bool is_running)
{
    if (is_running)
        {
            app_indicator_set_icon(indicator_, icon_path_running_.c_str());
            app_indicator_set_icon_theme_path(indicator_, icon_path_running_.c_str());
        }
    else
        {
            app_indicator_set_icon(indicator_, icon_path_.c_str());
            app_indicator_set_icon_theme_path(indicator_, icon_path_.c_str());
        }
}

void MyTrayIcon::on_run()
{
    std::cout << "Run action triggered." << std::endl;
    Gtk::MessageDialog dialog("Hello World", false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    set_status(TRUE);
    dialog.run();
}

sigc::signal<void> MyTrayIcon::signal_next()
{
    return next_signal_;
}

void MyTrayIcon::signal_next_emit()
{
    next_signal_.emit();
}


void MyTrayIcon::on_exit()
{
    std::cout << "Exit action triggered." << std::endl;
    Gtk::Main::quit();
}

