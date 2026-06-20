#pragma once

#include "http_types.hpp"

#include <functional>
#include <string>

namespace app::server {

class SimpleHttpServer {
public:
    using Handler = std::function<HttpResponse(const HttpRequest &)>;

    explicit SimpleHttpServer(Handler handler);
    bool listen(int port);

private:
    Handler handler;

    static HttpRequest parseRequest(const std::string &rawRequest);
    static std::string serializeResponse(const HttpResponse &response);
};

}
