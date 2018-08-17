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

#include "mouse_controls.h"
#include "shader.h"

class GUIApplication: public nanogui::Screen {
private:
    // Window height and width.
    float window_width_{800};
    float window_height_{600};
    
    // camera intrinsics (used for projection matrix)
    float f_x_ = 0;
    float f_y_ = 0;
    const float image_width_{224};
    const float image_height_{172};
    
    const float near_{0.1f};
    const float far_{10000};

    std::string vertex_shader_2D_{""};
    std::string fragment_shader_2D_{""};

    // GUI mouse controls for user movement.
    GUIMouseControls mouse_controls_;

    // Model view projection matrix used in the renderer.
    Eigen::Matrix4f model_view_projection_;
    Eigen::Matrix4f projection_;
    Eigen::Matrix4f model_view_;

    // For rendering the indices of the coordinate system.
    int index_indices_coordinate_system_ = 0;

    // Shaders for rendering.
    Shader3Dcolored shader_coordinate_system_;
    Shader2D shader_texture_;

    // Initialize GUI.
    void InitMainGUI(nanogui::Window* window);
    // Prepare shaders and initialize buffers.
    void InitShaders();
    // Init Shader for drawing a coordiante system.
    void InitCoordinateSystem();
    // Renders 2D texture.
    void Render2DTexture();
    // Renders Coordinate System.
    void RenderCoordinateSystem();
    // Computes current poses for rendering.
    void UpdatePose();
public:
    GUIApplication();
    ~GUIApplication();
    
    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) override;
    virtual bool mouseMotionEvent(const nanogui::Vector2i &position, const nanogui::Vector2i &rel, int button, int modifiers) override;
    virtual bool mouseButtonEvent(const nanogui::Vector2i &position, int button, bool down, int modifiers) override;
    virtual bool mouseDragEvent(const nanogui::Vector2i &position, const nanogui::Vector2i &rel, int button, int modifiers) override;
    virtual bool scrollCallbackEvent(double x, double y) override;
    virtual bool resizeCallbackEvent(int width, int height) override;
    
    virtual bool cursorPosCallbackEvent(double x, double y);
    virtual bool mouseButtonCallbackEvent(int button, int action, int modifiers);
    virtual bool keyCallbackEvent(int key, int scancode, int action, int mods);
    virtual bool charCallbackEvent(unsigned int codepoint);
    virtual bool dropCallbackEvent(int count, const char **filenames) override;
    
    virtual void draw(NVGcontext *ctx) override;
    virtual void drawContents() override;
};

#endif
