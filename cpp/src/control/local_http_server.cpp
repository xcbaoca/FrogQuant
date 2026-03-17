#include "control/local_http_server.hpp"

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <chrono>
#include <iostream>
#include <thread>

namespace fq::control {
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

LocalHttpServer::LocalHttpServer(unsigned short port, RequestHandler handler)
    : port_(port), handler_(std::move(handler)) {}

LocalHttpServer::~LocalHttpServer() {
    stop();
}

void LocalHttpServer::start() {
    if (running_.exchange(true)) return;
    th_ = std::thread([this] { run(); });
}

void LocalHttpServer::stop() {
    if (!running_.exchange(false)) return;
    if (th_.joinable()) th_.join();
}

void LocalHttpServer::run() {
    try {
        net::io_context ioc{1};
        tcp::acceptor acceptor(ioc, {net::ip::make_address("127.0.0.1"), port_});
        acceptor.non_blocking(true);

        while (running_.load()) {
            tcp::socket socket(ioc);
            beast::error_code ec;
            acceptor.accept(socket, ec);
            if (ec == net::error::would_block || ec == net::error::try_again) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }
            if (ec) continue;

            beast::flat_buffer buffer;
            http::request<http::string_body> req;
            http::read(socket, buffer, req, ec);
            if (ec) continue;

            unsigned status = 404;
            std::string resp = R"({"ok":false,"code":"E_NOT_FOUND","error":"route not found"})";
            const bool handled = handler_ ? handler_(std::string(req.method_string()),
                                                     std::string(req.target()),
                                                     req.body(),
                                                     status,
                                                     resp)
                                          : false;
            if (!handled) {
                status = 404;
                resp = R"({"ok":false,"code":"E_NOT_FOUND","error":"route not found"})";
            }

            http::response<http::string_body> res;
            res.version(req.version());
            res.result(static_cast<http::status>(status));
            res.set(http::field::content_type, "application/json");
            res.keep_alive(false);
            res.body() = resp;
            res.content_length(res.body().size());

            http::write(socket, res, ec);
            socket.shutdown(tcp::socket::shutdown_both, ec);
        }
    } catch (const std::exception& e) {
        std::cerr << "[control] server stopped by exception: " << e.what() << "\n";
    }
}

} // namespace fq::control
