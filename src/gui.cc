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

void GUIApplication::InitMainGUI(nanogui::Window* window) {
    window->setPosition(nanogui::Vector2i(15, 15));
    window->setLayout(new nanogui::GroupLayout());
    
    nanogui::Button* b = new nanogui::Button(window, "Button");
        b->setCallback([this](void) {
    });
}

void GUIApplication::Init2DTexture() {
    unsigned int indices[] = {0, 1, 2, 2, 3, 0};
    float vertices[] = {-1, -1, 1, 1, -1, 1, 1,  1, 1, -1,  1, 1};
    float uvs[] = {0, 1, 1, 1, 1, 0, 0, 0};
    float uvs_flipped[] = {1, 0, 0, 0, 0, 1, 1, 1};
    
    Eigen::Map<nanogui::MatrixXf> v_vertices(vertices,3,4);
    Eigen::Map<nanogui::MatrixXf> v_uvs(uvs,2,4);
    Eigen::Map<nanogui::MatrixXf> v_uvs_flipped(uvs_flipped,2,4);
    Eigen::Map<nanogui::MatrixXu> v_indices(indices,3,2);
    
    shader_texture_.Init("texture_shader");
    shader_texture_.shader_.bind();
    shader_texture_.shader_.uploadIndices(v_indices);
    shader_texture_.shader_.uploadAttrib("position", v_vertices);
    shader_texture_.shader_.uploadAttrib("vertexUV", v_uvs);
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

bool GUIApplication::BindCVMat2GLTexture(const cv::Mat& image, GLuint& texture, bool conv) const {
    if (!image.empty()) {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, image.ptr());
        return true;
    } else return false;
}

GUIApplication::GUIApplication(): nanogui::Screen(Eigen::Vector2i(100, 100), "Classy3DViewer") {
    this->setSize(nanogui::Vector2i(window_width_, window_height_));
    nanogui::Window *window = new nanogui::Window(this, "GUI");
    InitMainGUI(window);
    Init2DTexture();
    performLayout();
}

GUIApplication::~GUIApplication() {
    shader_texture_.shader_.free();
}

void GUIApplication::draw(NVGcontext *ctx) {
    /* Draw the user interface */
    Screen::draw(ctx);
}

void GUIApplication::drawContents() {
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    Render2DTexture();
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
