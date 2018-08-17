/*******************************************************
 * Copyright (c) 2018, Johanna Wald
 * All rights reserved.
 *
 * This file is distributed under the GNU Lesser General Public License v3.0.
 * The complete license agreement can be obtained at:
 * http://www.gnu.org/licenses/lgpl-3.0.html
 ********************************************************/

#include "mouse_controls.h"

void GUIMouseControls::Reset() {
    horizontal_angle_temp_ = horizontal_angle_;
    vertical_angle_temp_ = vertical_angle_;
    
    delta_pos_.setZero();
    pos_offset_control_ = Eigen::Vector3f::Zero();
    pos_offset_control_temp_ = Eigen::Vector3f::Zero();
}

void GUIMouseControls::ButtonReleased() {
    button_pressed_ = false;
    end_pos_ = current_pos_;
    delta_pos_.setZero();
    horizontal_angle_ = horizontal_angle_temp_;
    vertical_angle_ = vertical_angle_temp_;
    pos_offset_control_ = pos_offset_control_temp_;
}

void GUIMouseControls::UpdateLeft() {
    horizontal_angle_temp_ = horizontal_angle_ + mouse_speed_ * delta_pos_.x();
    vertical_angle_temp_ = vertical_angle_ + mouse_speed_ * delta_pos_.y();
}

void GUIMouseControls::UpdateRight(const Eigen::Vector3f& eyeup_controls, const Eigen::Vector3f& right) {
    pos_offset_control_temp_ = pos_offset_control_ - delta_pos_.x() * 0.01f * right + delta_pos_.y() * 0.01f * eyeup_controls;
}

void GUIMouseControls::Pressed(const nanogui::Vector2i &position) {
    if (button_pressed_) {
        if (!initialized_)
            previous_pos_ = current_pos_;
        current_pos_[0] = static_cast<float>(position.x());
        current_pos_[1] = static_cast<float>(position.y());
        delta_pos_ = current_pos_ - previous_pos_;
        initialized_ = true;
    }
}
