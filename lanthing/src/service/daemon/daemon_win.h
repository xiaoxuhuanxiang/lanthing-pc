#pragma once
#include <ltlib/win_service.h>

namespace lt {

namespace svc {

class Service;

class LanthingWinService : public ltlib::WinApp {
public:
    LanthingWinService();
    ~LanthingWinService() override;
    void onStart() override;
    void onStop() override;
    void run() override;

private:
    std::unique_ptr<Service> impl_;
    bool is_stop_ = false;
};

} // namespace svc

} // namespace lt