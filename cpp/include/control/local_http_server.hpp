#pragma once

#include <atomic>
#include <functional>
#include <string>
#include <thread>

namespace fq::control {

class LocalHttpServer {
public:
    // 返回 true 代表已处理；status/body 由 handler 填充。
    using RequestHandler = std::function<bool(const std::string& method,
                                              const std::string& target,
                                              const std::string& body,
                                              unsigned& status,
                                              std::string& resp_body)>;

    LocalHttpServer(unsigned short port, RequestHandler handler);
    ~LocalHttpServer();

    void start();
    void stop();

private:
    void run();

    unsigned short port_;
    RequestHandler handler_;

    std::atomic<bool> running_{false};
    std::thread th_;
};

} // namespace fq::control
