/*******************************************************
 * Copyright (c) 2018, Johanna Wald
 * All rights reserved.
 *
 * This file is distributed under the GNU Lesser General Public License v3.0.
 * The complete license agreement can be obtained at:
 * http://www.gnu.org/licenses/lgpl-3.0.html
 ********************************************************/

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/label.h>
#include <nanogui/layout.h>
#include <nanogui/toolbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/entypo.h>
#include <dirent.h>

#include "gui.h"
#include "util.h"

void GUIApplication::InitMainGUI(nanogui::Window* window) {
    window->setPosition(nanogui::Vector2i(15, 15));
    window->setLayout(new nanogui::GroupLayout());
    
    nanogui::Button* b = new nanogui::Button(window, "Button");
        b->setCallback([this](void) {
    });
}

void GUIApplication::InitShaders() {
    shader_texture_.Init("texture_shader");
    InitCoordinateSystem();
}

void GUIApplication::InitCoordinateSystem() {
    const int counter{10};
    const float distance{1.0f};
    const int indices_size{3 + 2*counter};
    const int line_size{6 + 4*counter};
    
    nanogui::MatrixXu indices_grid(2, indices_size);
    nanogui::MatrixXf positions_grid(3, line_size);
    nanogui::MatrixXf color_grid(3, line_size);
    
    index_indices_coordinate_system_ = 0;
    int index_positions = 0;
    int index_color = 0;
    
    const Eigen::Vector3f grid_color(0.72f, 0.72f, 0.72f);
    const Eigen::Vector3f camera_trajectory(0.0f, 0.0f, 0.0f);
    
    // Coordinte System
    positions_grid.col(index_positions++) << 0, 0, distance*counter;
    positions_grid.col(index_positions++) << 0, 0, 0;
    color_grid.col(index_color++) << 0, 0, 1;
    color_grid.col(index_color++) << 0, 0, 1;
    indices_grid.col(index_indices_coordinate_system_++) << index_positions-2, index_positions-1;
        
    positions_grid.col(index_positions++) << 0, distance*counter, 0;
    positions_grid.col(index_positions++) << 0, 0, 0;
    color_grid.col(index_color++) << 0, 1, 0;
    color_grid.col(index_color++) << 0, 1, 0;
    indices_grid.col(index_indices_coordinate_system_++) << index_positions-2, index_positions-1;
        
    positions_grid.col(index_positions++) << distance*counter, 0, 0;
    positions_grid.col(index_positions++) << 0, 0, 0;
    color_grid.col(index_color++) << 1, 0, 0;
    color_grid.col(index_color++) << 1, 0, 0;
    indices_grid.col(index_indices_coordinate_system_++) << index_positions-2, index_positions-1;
    
    for (int i = 1; i <= counter; i++) {
        color_grid.col(index_color++) << grid_color;
        color_grid.col(index_color++) << grid_color;
        color_grid.col(index_color++) << grid_color;
        color_grid.col(index_color++) << grid_color;
        
        positions_grid.col(index_positions++) << 0, i*distance, 0;
        positions_grid.col(index_positions++) << counter*distance, i*distance, 0;
        positions_grid.col(index_positions++) << i*distance, 0, 0;
        positions_grid.col(index_positions++) << i*distance, counter*distance, 0;
        
        indices_grid.col(index_indices_coordinate_system_++) << index_positions-4, index_positions-3;
        indices_grid.col(index_indices_coordinate_system_++) << index_positions-2, index_positions-1;
    }

    shader_coordinate_system_.Init("shader_coordinate_system");
    shader_coordinate_system_.shader_.bind();
    shader_coordinate_system_.shader_.uploadIndices(indices_grid);
    shader_coordinate_system_.shader_.uploadAttrib("position", positions_grid);
    shader_coordinate_system_.shader_.uploadAttrib("color", color_grid);
}

void GUIApplication::Render2DTexture() {
    /*
     GLuint textures_rgb;
     BindCVMat2GLTexture(, textures_rgb, true);
     glViewport(window_width, window_height, window_width, window_height);
     glBindTexture(GL_TEXTURE_2D, textures_rgb);
     shader_texture_.shader_.bind();
     shader_texture_.shader_.drawIndexed(GL_TRIANGLES, 0, 2);*/
}

void GUIApplication::RenderCoordinateSystem() {
    shader_coordinate_system_.shader_.bind();
    shader_coordinate_system_.shader_.setUniform("model_view_projection", model_view_projection_);
    shader_coordinate_system_.shader_.drawIndexed(GL_LINES, 0, index_indices_coordinate_system_);
}

void GUIApplication::UpdatePose() {
    mouse_controls_.Update();
    
    // Update pose for rendering.
    const double gui_camera_fovy_x = 2 * atan((image_width_)/(2*f_x_));
    const double gui_camera_fovy_y = 2 * atan((image_height_)/(2*f_y_));
    projection_ = perspective<Eigen::Matrix4f::Scalar>(gui_camera_fovy_x, gui_camera_fovy_y, near_, far_);
    model_view_ = mouse_controls_.view_;
    model_view_projection_ = projection_ * model_view_;
}

GUIApplication::GUIApplication(): nanogui::Screen(Eigen::Vector2i(100, 100), "Classy3DViewer") {
    this->setSize(nanogui::Vector2i(window_width_, window_height_));
    nanogui::Window *window = new nanogui::Window(this, "GUI");
    InitMainGUI(window);
    InitShaders();
    performLayout();
}

GUIApplication::~GUIApplication() {
    shader_texture_.shader_.free();
    shader_coordinate_system_.shader_.free();
}

void GUIApplication::draw(NVGcontext *ctx) {
    /* Draw the user interface */
    Screen::draw(ctx);
}

void GUIApplication::drawContents() {
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    UpdatePose();
    
    Render2DTexture();
    RenderCoordinateSystem();
}

bool GUIApplication::keyboardEvent(int key, int scancode, int action, int modifiers) {
    if (!nanogui::Screen::keyboardEvent(key, scancode, action, modifiers)) {
        if (Screen::keyboardEvent(key, scancode, action, modifiers))
            return true;
        
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            setVisible(false);
            return true;
        }
    }
    return false;
}

bool GUIApplication::mouseButtonEvent(const nanogui::Vector2i &position, int button, bool down, int modifiers) {
    if (!nanogui::Screen::mouseButtonEvent(position, button, down, modifiers)) {
        return mouse_controls_.MouseButtonEvent(position, button, down, modifiers);;
    }
    return false;
}

bool GUIApplication::mouseMotionEvent(const nanogui::Vector2i &position, const nanogui::Vector2i &rel, int button, int modifiers) {
    if (!nanogui::Screen::mouseMotionEvent(position, rel, button, modifiers)) {
        // Get mouse position
        mouse_controls_.Pressed(position);
        return true;
    }
    return false;
}

bool GUIApplication::mouseDragEvent(const nanogui::Vector2i &position, const nanogui::Vector2i &rel, int button, int modifiers) {
    nanogui::Screen::mouseDragEvent(position, rel, button, modifiers);
    return true;
}

bool GUIApplication::scrollCallbackEvent(double x, double y) {
    if (!nanogui::Screen::scrollCallbackEvent(x, y)) {
        return mouse_controls_.ScrollCallbackEvent(x, y);
    }
    return false;
}

bool GUIApplication::resizeCallbackEvent(int width, int height) {
    nanogui::Screen::resizeCallbackEvent(width, height);
    // is called if function is declared virtual in nanogui
    window_width_ = width;
    window_height_ = height;
    return true;
}

bool GUIApplication::cursorPosCallbackEvent(double x, double y) {
    // not called when virtual is not in the nanogui
    nanogui::Screen::cursorPosCallbackEvent(x, y);
    return true;
}

bool GUIApplication::mouseButtonCallbackEvent(int button, int action, int modifiers) {
    // not called when virtual is not in the nanogui
    nanogui::Screen::mouseButtonCallbackEvent(button, action, modifiers);
    return true;
}

bool GUIApplication::keyCallbackEvent(int key, int scancode, int action, int mods) {
    // not called when virtual is not in the nanogui
    nanogui::Screen::keyCallbackEvent(key, scancode, action, mods);
    return true;
}

bool GUIApplication::charCallbackEvent(unsigned int codepoint) {
    // not called when virtual is not in the nanogui
    nanogui::Screen::charCallbackEvent(codepoint);
    return true;
}

bool GUIApplication::dropCallbackEvent(int count, const char **filenames) {
    // is called if function is declared virtual in nanogui
    nanogui::Screen::dropCallbackEvent(count, filenames);
    return true;
}
