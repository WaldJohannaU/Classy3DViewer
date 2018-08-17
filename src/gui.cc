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

#include <dirent.h>
#include <nanogui/combobox.h>
#include <nanogui/entypo.h>
#include <nanogui/glutil.h>
#include <nanogui/label.h>
#include <nanogui/layout.h>
#include <nanogui/messagedialog.h>
#include <nanogui/screen.h>
#include <nanogui/toolbutton.h>
#include <nanogui/window.h>

#include "gui.h"
#include "tinyply.h"
#include "util.h"

namespace C3DV_graphics {

bool loadAssImp(const char* path,
                std::vector<unsigned int>& indices,
                std::vector<float>& vertices,
                std::vector<float>& uvs,
                std::vector<float>& normals) {
    Assimp::Importer importer;
    
    std::ifstream f(path);
    if (!f.good())
        return false;
    
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);
    if (!scene) {
        std::cout << importer.GetErrorString() << std::endl;
        getchar();
        return false;
    }
    const aiMesh* mesh = scene->mMeshes[0];
    
    // Fill vertices positions
    vertices.reserve(mesh->mNumVertices*3);
    for (unsigned int i = 0; i < mesh->mNumVertices; i++){
        aiVector3D pos = mesh->mVertices[i];
        vertices.push_back(pos.x);
        vertices.push_back(pos.y);
        vertices.push_back(pos.z);
    }
    
    // Fill vertices texture coordinates
    uvs.reserve(mesh->mNumVertices);
    if (mesh->mTextureCoords[0] != nullptr) {
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            aiVector3D UVW = mesh->mTextureCoords[0][i]; // Assume only 1 set of UV coords; AssImp supports 8 UV sets.
            uvs.push_back(UVW.x);
            uvs.push_back(UVW.y);
        }
    }
    
    // Fill vertices normals
    normals.reserve(mesh->mNumVertices);
    for (unsigned int i=0; i < mesh->mNumVertices; i++){
        aiVector3D n = mesh->mNormals[i];
        normals.push_back(n.x);
        normals.push_back(n.y);
        normals.push_back(n.z);
    }
    
    // Fill face indices
    indices.reserve(3*mesh->mNumFaces);
    for (unsigned int i = 0; i < mesh->mNumFaces; i++){
        // Assume the model has only triangles.
        indices.push_back(mesh->mFaces[i].mIndices[0]);
        indices.push_back(mesh->mFaces[i].mIndices[1]);
        indices.push_back(mesh->mFaces[i].mIndices[2]);
    }
    return true;
}

bool BindCVMat2GLTexture(const cv::Mat& image, GLuint& imageTexture, bool conv) {
    if (!image.empty()) {
        glDeleteTextures(1, &imageTexture);
        glGenTextures(1, &imageTexture);
        glBindTexture(GL_TEXTURE_2D, imageTexture);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        glTexImage2D(GL_TEXTURE_2D,
                     0,                   // Pyramid level (for mip-mapping) - 0 is the top level
                     GL_RGB,              // Internal colour format to convert to
                     image.cols,
                     image.rows,
                     0,                   // Border width in pixels (can either be 1 or 0)
                     GL_BGR,              // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                     GL_UNSIGNED_BYTE,    // Image data type
                     image.ptr());        // The actual image data itself
        return true;
    } else return false;
}

};

void GUIApplication::InitMainGUI(nanogui::Window* window) {
    window->setPosition(nanogui::Vector2i(15, 15));
    window->setLayout(new nanogui::GroupLayout());
    
    nanogui::Button* b = new nanogui::Button(window, "Point Cloud");
    b->setCallback([this](void) {
        file_point_cloud_ = nanogui::file_dialog({ {"ply", "PLY File"} }, false);
        render_type_ = RenderType::PointCloud;
        Init3DCloud();
    });

    b = new nanogui::Button(window, "Surfel Map");
    b->setCallback([this](void) {
        render_type_ = RenderType::SurfelMap;
        file_surfel_map_ = nanogui::file_dialog({ {"ply", "PLY File"} }, false);
        Init3DSurfels();
    });

    b = new nanogui::Button(window, "3D Mesh");
    b->setCallback([this](void) {
        render_type_ = RenderType::Mesh3D;
        file_3D_mesh_ = nanogui::file_dialog({ {"obj", "OBJ File"} }, false);
        std::string texture_file = nanogui::file_dialog({ {"png", "PNG File"}, {"jpg", "JPG File"} }, false);
        cv::Mat texture_mat = cv::imread(texture_file);
        C3DV_graphics::BindCVMat2GLTexture(texture_mat, texture3D_mesh_, true);
        Init3DMesh();
    });
}

void GUIApplication::InitShaders() {
    shader_texture_.Init("texture_shader");
    InitCoordinateSystem();
    Init3DCloud();
    Init3DSurfels();
    Init3DMesh();
}

void GUIApplication::InitCoordinateSystem() {
    const int counter{10};
    const float distance{1.0f};
    const int indices_size{3 + 2*counter};
    const int line_size{6 + 4*counter};
    
    nanogui::MatrixXu indices_grid(2, indices_size);
    nanogui::MatrixXf positions_grid(3, line_size);
    nanogui::MatrixXf color_grid(3, line_size);
    
    indices_coordinate_system_ = 0;
    int index_positions = 0;
    int index_color = 0;
    
    const Eigen::Vector3f grid_color(0.72f, 0.72f, 0.72f);
    const Eigen::Vector3f camera_trajectory(0.0f, 0.0f, 0.0f);
    
    // Coordinte System
    positions_grid.col(index_positions++) << 0, 0, distance*counter;
    positions_grid.col(index_positions++) << 0, 0, 0;
    color_grid.col(index_color++) << 0, 0, 1;
    color_grid.col(index_color++) << 0, 0, 1;
    indices_grid.col(indices_coordinate_system_++) << index_positions-2, index_positions-1;
        
    positions_grid.col(index_positions++) << 0, distance*counter, 0;
    positions_grid.col(index_positions++) << 0, 0, 0;
    color_grid.col(index_color++) << 0, 1, 0;
    color_grid.col(index_color++) << 0, 1, 0;
    indices_grid.col(indices_coordinate_system_++) << index_positions-2, index_positions-1;
        
    positions_grid.col(index_positions++) << distance*counter, 0, 0;
    positions_grid.col(index_positions++) << 0, 0, 0;
    color_grid.col(index_color++) << 1, 0, 0;
    color_grid.col(index_color++) << 1, 0, 0;
    indices_grid.col(indices_coordinate_system_++) << index_positions-2, index_positions-1;
    
    for (int i = 1; i <= counter; i++) {
        color_grid.col(index_color++) << grid_color;
        color_grid.col(index_color++) << grid_color;
        color_grid.col(index_color++) << grid_color;
        color_grid.col(index_color++) << grid_color;
        
        positions_grid.col(index_positions++) << 0, i*distance, 0;
        positions_grid.col(index_positions++) << counter*distance, i*distance, 0;
        positions_grid.col(index_positions++) << i*distance, 0, 0;
        positions_grid.col(index_positions++) << i*distance, counter*distance, 0;
        
        indices_grid.col(indices_coordinate_system_++) << index_positions-4, index_positions-3;
        indices_grid.col(indices_coordinate_system_++) << index_positions-2, index_positions-1;
    }

    shader_coordinate_system_.Init("shader_coordinate_system");
    shader_coordinate_system_.shader_.bind();
    shader_coordinate_system_.shader_.uploadIndices(indices_grid);
    shader_coordinate_system_.shader_.uploadAttrib("position", positions_grid);
    shader_coordinate_system_.shader_.uploadAttrib("color", color_grid);
}

void GUIApplication::Init3DCloud() {
    Eigen::Vector3f color;
    std::vector<Eigen::Vector3f> reference_points_color_;
    std::ifstream ss(file_point_cloud_, std::ios::binary);
    tinyply::PlyFile input_file(ss);
    std::vector<float> vertices;
    std::vector<float> raw_normals;
    std::vector<uint8_t> raw_colors;
    uint32_t vertex_count = input_file.request_properties_from_element("vertex", { "x", "y", "z" }, vertices);
    input_file.request_properties_from_element("vertex", { "nx", "ny", "nz" }, raw_normals);
    input_file.request_properties_from_element("vertex", { "red", "green", "blue", "alpha" }, raw_colors);
    input_file.read(ss);
    for (int i = 0; i < vertex_count; i++) {
        color[0] = raw_colors[4*i] / 255.0f;
        color[1] = raw_colors[4*i+1] / 255.0f;
        color[2] = raw_colors[4*i+2] / 255.0f;
        // Instead of pushing it to a vector we should use the vector directly (different data formats).
        reference_points_color_.push_back(color);
    }
    indices_3D_cloud_ = vertex_count;
    Eigen::Map<nanogui::MatrixXf> positions_cs(&vertices[0], 3, indices_3D_cloud_);
    Eigen::Map<nanogui::MatrixXf> color_surfel(&reference_points_color_[0][0], 3, indices_3D_cloud_);
    shader_3D_cloud_.Init("shader_cloud3D");
    shader_3D_cloud_.shader_.bind();
    shader_3D_cloud_.shader_.uploadAttrib("position", positions_cs);
    shader_3D_cloud_.shader_.uploadAttrib("color", color_surfel);
}

void GUIApplication::Init3DSurfels() {
    Eigen::Vector3f color;
    std::ifstream ss(file_surfel_map_, std::ios::binary);
    tinyply::PlyFile input_file(ss);
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> radius;
    std::vector<float> curvatures;
    std::vector<float> qualities;
    std::vector<uint8_t> colors;
    uint32_t vertex_count = input_file.request_properties_from_element("vertex", { "x", "y", "z" }, vertices);
    input_file.request_properties_from_element("vertex", { "nx", "ny", "nz" }, normals);
    input_file.request_properties_from_element("vertex", { "curvature" }, curvatures);
    input_file.request_properties_from_element("vertex", { "red", "green", "blue", "alpha" }, colors);
    input_file.request_properties_from_element("vertex", { "quality" }, qualities);
    input_file.request_properties_from_element("vertex", { "radius" }, radius);
    input_file.read(ss);
    
    nanogui::MatrixXf positions_discs(3, vertex_count*6);
    nanogui::MatrixXf normal_discs(3, vertex_count*6);
    nanogui::MatrixXf color_surfel_discs(3, vertex_count*6);
    nanogui::MatrixXf texture_discs(2, vertex_count*6);
    
    for (int i = 0; i < vertex_count; i++) {
        const Eigen::Vector3f point(vertices[3*i], vertices[3*i+1], vertices[3*i+2]);
        const Eigen::Vector3f normal(normals[3*i], normals[3*i+1], normals[3*i+2]);
        const Eigen::Vector3f color(colors[4*i] / 255.0f, colors[4*i+1] / 255.0f, colors[4*i+2] / 255.0f);

        for (int s = 0; s <= 5; s++) {
            color_surfel_discs.col(6*i+s) << color;
            normal_discs.col(6*i+s) << normal;
        }
        const float surfel_radius = kSqrt2 * radius[i]/1000.0f;
        
        Eigen::Vector3f u;
        if (std::abs(normal.dot(Eigen::Vector3f::UnitX())) >
            std::abs(normal.dot(Eigen::Vector3f::UnitY()))) {
            u = surfel_radius * normal.cross(Eigen::Vector3f::UnitX()).normalized();
        } else {
            u = surfel_radius * normal.cross(Eigen::Vector3f::UnitY()).normalized();
        }
        const Eigen::Vector3f v = normal.cross(u);
        
        const Eigen::Vector3f p0 = point - u;
        const Eigen::Vector3f p1 = point - v;
        const Eigen::Vector3f p2 = point + u;
        const Eigen::Vector3f p3 = point + v;
        
        positions_discs.col(6*i)   << p0;
        positions_discs.col(6*i+1) << p1;
        positions_discs.col(6*i+2) << p2;
        positions_discs.col(6*i+3) << p0;
        positions_discs.col(6*i+4) << p2;
        positions_discs.col(6*i+5) << p3;
        
        texture_discs.col(6*i) << 0, 0;
        texture_discs.col(6*i+1) << 1, 0;
        texture_discs.col(6*i+2) << 1, 1;
        texture_discs.col(6*i+3) << 0, 0;
        texture_discs.col(6*i+4) << 1, 1;
        texture_discs.col(6*i+5) << 0, 1;
    }
    indices_3D_surfels_ = vertex_count*6;

    const std::string vertex_shader_surfels{"#version 330\n"
        "uniform mat4 modelView;\n"
        "uniform mat4 u_projection;\n"
        "layout(location = 0) in vec3 position;\n"
        "layout(location = 1) in vec3 color;\n"
        "layout(location = 2) in vec3 texture;\n"
        "layout(location = 3) in vec3 normal;\n"
        "out vec2 v_surfel_coord;\n"
        "out vec3 colorV;\n"
        "out vec4 normalV;\n"
        "out vec4 positionV;\n"
        "void main() {\n"
        "    v_surfel_coord = vec2(2.0 * texture - 1.0)\n;"
        "    vec4 normalV = modelView * vec4(normal, 0);"
        "    vec3 test = vec3(0.5, 0.5, 1);"
        "    float diffuse = abs(dot(normalV.xyz, normalize(test)));"
        "    vec3 color2 = color;"
        "    color2 *= diffuse;"
        "    vec4 positionV = modelView * vec4(position, 1);"
        "    gl_Position = u_projection * positionV;"
        "    colorV = color2;\n"
        "}"};
    
    const std::string fragment_shader_surfels{"#version 330\n"
        "in vec3 colorV;\n"
        "in vec2 v_surfel_coord;"
        "out vec4 color;\n"
        "out float sq_norm;\n"
        "void main() {\n"
        "    float sq_norm = dot(v_surfel_coord, v_surfel_coord);\n"
        "    if (sq_norm > 1.f) discard;\n"
        "    color = vec4(colorV, 1.0);\n"
        "}"};
    
    shader_3D_surfels_.Init("shader_surfels3D", vertex_shader_surfels, fragment_shader_surfels);
    shader_3D_surfels_.shader_.bind();
    shader_3D_surfels_.shader_.bind();
    shader_3D_surfels_.shader_.uploadAttrib("normal", normal_discs);
    shader_3D_surfels_.shader_.uploadAttrib("position", positions_discs);
    shader_3D_surfels_.shader_.uploadAttrib("color", color_surfel_discs);
    shader_3D_surfels_.shader_.uploadAttrib("texture", texture_discs);
}

void GUIApplication::Init3DMesh() {
    std::vector<unsigned int> indices;
    std::vector<float> vertices;
    std::vector<float> uvs;
    std::vector<float> normals;
    
    C3DV_graphics::loadAssImp(file_3D_mesh_.c_str(), indices, vertices, uvs, normals);
    
    indices_3D_mesh_ = indices.size() / 3;
    
    Eigen::Map<nanogui::MatrixXf> eigen_vertices(vertices.data(), 3, (vertices.size() / 3));
    Eigen::Map<nanogui::MatrixXf> eigen_normals(normals.data(), 3, (normals.size() / 3));
    Eigen::Map<nanogui::MatrixXf> eigen_uv(uvs.data(), 2, (uvs.size() / 2));
    Eigen::Map<nanogui::MatrixXu> eigen_indices(indices.data(), 3, (indices.size() / 3));
    
    shader_3D_mesh_.Init("shader_mesh3D");
    shader_3D_mesh_.shader_.bind();
    shader_3D_mesh_.shader_.uploadIndices(eigen_indices);
    shader_3D_mesh_.shader_.uploadAttrib("position", eigen_vertices);
    shader_3D_mesh_.shader_.uploadAttrib("vertexUV", eigen_uv);
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
    shader_coordinate_system_.shader_.drawIndexed(GL_LINES, 0, indices_coordinate_system_);
}

void GUIApplication::Render3DCloud() {
    shader_3D_cloud_.shader_.bind();
    shader_3D_cloud_.shader_.setUniform("model_view_projection", model_view_projection_);
    shader_3D_cloud_.shader_.drawArray(GL_POINTS, 0, indices_3D_cloud_);
}

void GUIApplication::Render3DSurfels() {
    shader_3D_surfels_.shader_.bind();
    shader_3D_surfels_.shader_.setUniform("modelView", model_view_);
    shader_3D_surfels_.shader_.setUniform("u_projection", projection_);
    shader_3D_surfels_.shader_.drawArray(GL_TRIANGLES, 0, indices_3D_surfels_);
}

void GUIApplication::Render3DMesh() {
    glBindTexture(GL_TEXTURE_2D, texture3D_mesh_);
    shader_3D_mesh_.shader_.bind();
    shader_3D_mesh_.shader_.setUniform("model_view_projection", model_view_projection_);
    shader_3D_mesh_.shader_.drawIndexed(GL_TRIANGLES, 0, indices_3D_mesh_);
}

void GUIApplication::UpdatePose() {
    mouse_controls_.Update();
    
    // Update pose for rendering.
    const double gui_camera_fovy_x = 2 * atan((window_width_)/(2*f_x_));
    const double gui_camera_fovy_y = 2 * atan((window_height_)/(2*f_y_));
    projection_ = C3DV_camera::perspective<Eigen::Matrix4f::Scalar>(gui_camera_fovy_x, gui_camera_fovy_y, near_, far_);
    model_view_ = mouse_controls_.view_;
    model_view_projection_ = projection_ * model_view_;
}

GUIApplication::GUIApplication(): nanogui::Screen(Eigen::Vector2i(100, 100), "Classy3DViewer") {
    this->setSize(nanogui::Vector2i(window_width_, window_height_));
    nanogui::Window *window = new nanogui::Window(this, "Load Data");
    InitMainGUI(window);
    InitShaders();
    performLayout();
    mouse_controls_.Reset();
}

GUIApplication::~GUIApplication() {
    shader_texture_.shader_.free();
    shader_coordinate_system_.shader_.free();
    shader_3D_cloud_.shader_.free();
    shader_3D_surfels_.shader_.free();
    shader_3D_mesh_.shader_.free();
}

void GUIApplication::draw(NVGcontext *ctx) {
    /* Draw the user interface */
    Screen::draw(ctx);
}

void GUIApplication::drawContents() {
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    RenderCoordinateSystem();
    glPointSize(5);
    if (render_type_ == RenderType::PointCloud)
        Render3DCloud();
    else if (render_type_ == RenderType::SurfelMap)
        Render3DSurfels();
    else if (render_type_ == RenderType::Mesh3D)
        Render3DMesh();
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    
    UpdatePose();
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
