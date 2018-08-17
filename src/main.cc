/*******************************************************
 * Copyright (c) 2018, Johanna Wald
 * All rights reserved.
 *
 * This file is distributed under the GNU Lesser General Public License v3.0.
 * The complete license agreement can be obtained at:
 * http://www.gnu.org/licenses/lgpl-3.0.html
 ********************************************************/

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
