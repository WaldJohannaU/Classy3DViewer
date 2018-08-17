/*******************************************************
 * Copyright (c) 2018, Johanna Wald
 * All rights reserved.
 *
 * This file is distributed under the GNU Lesser General Public License v3.0.
 * The complete license agreement can be obtained at:
 * http://www.gnu.org/licenses/lgpl-3.0.html
 ********************************************************/

#ifndef _H_GUI_
#define _H_GUI_

#include <array>
#include <set>

#include <nanogui/button.h>
#include <nanogui/checkbox.h>
#include <nanogui/glutil.h>
#include <nanogui/layout.h>
#include <nanogui/opengl.h>
#include <nanogui/screen.h>
#include <nanogui/textbox.h>
#include <nanogui/toolbutton.h>
#include <nanogui/window.h>
#include <opencv2/opencv.hpp>

#include "shader.h"

class GUIApplication: public nanogui::Screen {
private:
    // Window height and width.
    const float window_width_{800};
    const float window_height_{600};
    std::string vertex_shader_2D_{""};
    std::string fragment_shader_2D_{""};
    
    Shader2D shader_texture_;
    // Initialize GUI.
    void InitMainGUI(nanogui::Window* window);
    // Prepare shaders and initialize buffers.
    void InitShaders();
    // Renders 2D texture.
    void Render2DTexture();
    // Bind OpenCV texture to OpenGL buffer.
    bool BindCVMat2GLTexture(const cv::Mat& image, GLuint& imageTexture, bool conv) const;
public:
    GUIApplication();
    ~GUIApplication();
    
    virtual void draw(NVGcontext *ctx) override;
    virtual void drawContents() override;
    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) override;
};

#endif
