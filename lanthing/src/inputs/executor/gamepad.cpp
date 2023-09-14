/*
 * BSD 3-Clause License
 * 
 * Copyright (c) 2023 Zhennan Tu <zhennan.tu@gmail.com>
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <Windows.h>
#include <g3log/g3log.hpp>
#include <inputs/executor/gamepad.h>

namespace lt {

std::map<PVIGEM_CLIENT, Gamepad*> Gamepad::map_s;

std::unique_ptr<Gamepad>
Gamepad::create(std::function<void(uint32_t, uint16_t, uint16_t)> gamepad_response) {
    auto gp = std::make_unique<Gamepad>(gamepad_response);
    if (gp->connect()) {
        return gp;
    }
    else {
        return nullptr;
    }
}
Gamepad::Gamepad(std::function<void(uint32_t, uint16_t, uint16_t)> gamepad_response)
    : gamepad_response_{gamepad_response} {}

bool Gamepad::plugin(uint32_t index) {
    if (index >= XUSER_MAX_COUNT) {
        return false;
    }

    if (gamepad_target_[index]) {
        return true;
    }

    PVIGEM_TARGET gamepad = vigem_target_x360_alloc();
    VIGEM_ERROR ret = vigem_target_add(gamepad_driver_, gamepad);
    if (!VIGEM_SUCCESS(ret)) {
        LOG(WARNING) << "Add x360 failed";
        vigem_target_free(gamepad);
        return false;
    }

    ret = vigem_target_x360_register_notification(gamepad_driver_, gamepad,
                                                  &Gamepad::on_gamepad_response);
    if (!VIGEM_SUCCESS(ret)) {
        vigem_target_x360_unregister_notification(gamepad);
        LOG(WARNING) << "Register x360 failed";
        vigem_target_free(gamepad);
        return false;
    }
    gamepad_target_[index] = gamepad;
    LOG(INFO) << "Plug in gamepad " << index << " success";
    return true;
}

void Gamepad::plugout(uint32_t index) {
    if (index >= XUSER_MAX_COUNT) {
        return;
    }

    if (!gamepad_target_[index]) {
        return;
    }

    vigem_target_x360_unregister_notification(gamepad_target_[index]);
    vigem_target_remove(gamepad_driver_, gamepad_target_[index]);
    vigem_target_free(gamepad_target_[index]);
    gamepad_target_[index] = nullptr;
    LOG(INFO) << "Plug out gamepad " << index;
}

bool Gamepad::submit(uint32_t index, const XUSB_REPORT& report) {
    if (!plugin(index)) {
        return false;
    }

    auto ret = vigem_target_x360_update(gamepad_driver_, gamepad_target_[index], report);
    if (!VIGEM_SUCCESS(ret)) {
        LOG(WARNING) << "Submit x360 input failed";
        return false;
    }
    return true;
}

bool Gamepad::connect() {
    gamepad_driver_ = vigem_alloc();
    auto ret = vigem_connect(gamepad_driver_);
    if (!VIGEM_SUCCESS(ret)) {
        vigem_free(gamepad_driver_);
        gamepad_driver_ = nullptr;
        LOG(WARNING) << "Connect to vigem failed";
        return false;
    }
    else {
        Gamepad::map_s[gamepad_driver_] = this;
        return true;
    }
}

void Gamepad::on_gamepad_response(PVIGEM_CLIENT client, PVIGEM_TARGET target, UCHAR large_motor,
                                  UCHAR small_motor, UCHAR led_number) {
    auto iter = Gamepad::map_s.find(client);
    if (iter == Gamepad::map_s.end()) {
        LOG(WARNING) << "Can not find vigem client " << client;
        return;
    }
    auto that = iter->second;
    for (uint32_t i = 0; i < XUSER_MAX_COUNT; i++) {
        if (that->gamepad_target_[i] != nullptr && that->gamepad_target_[i] == target) {
            that->gamepad_response_(i, large_motor, small_motor);
        }
    }
}

} // namespace lt