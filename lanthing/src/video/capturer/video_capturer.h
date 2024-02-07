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

#pragma once
#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <optional>

#include <ltlib/system.h>

namespace lt {

namespace video {

enum class CaptureFormat {
    D3D11_BGRA,
    MEM_I420,
};

class Capturer {
public:
    enum class Backend {
        Dxgi,
    };
    struct Frame {
        void* data;
        int64_t capture_timestamp_us;
    };

public:
    static std::unique_ptr<Capturer> create(Backend backend, ltlib::Monitor monitor);
    virtual ~Capturer();
    virtual std::optional<Frame> capture() = 0;
    virtual bool start() = 0;
    virtual void doneWithFrame() = 0;
    virtual Backend backend() const = 0;
    virtual int64_t luid() { return -1; }
    virtual void* device() = 0;
    virtual void* deviceContext() = 0;
    virtual void waitForVBlank() = 0;
    virtual uint32_t vendorID() = 0;
    virtual bool defaultOutput() = 0;
    virtual bool setCaptureFormat(CaptureFormat format) = 0;

protected:
    Capturer();
    virtual bool init() = 0;
};

} // namespace video

} // namespace lt