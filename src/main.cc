#include "gui.h"

int main(int argc, char** argv) {
    nanogui::init();
    {
        nanogui::ref<GUIApplication> app{new GUIApplication()};
        app->drawAll();
        app->setVisible(true);
        nanogui::mainloop();
    }
    nanogui::shutdown();
    return 0;
}
