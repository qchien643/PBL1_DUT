#pragma once

#include <map>
#include <string>

namespace app::server {

struct HttpRequest {
    std::string method;
    std::string path;
    std::string rawTarget;
    std::map<std::string, std::string> query;
    std::map<std::string, std::string> headers;
    std::string body;
};

struct HttpResponse {
    int status{200};
    std::string contentType{"application/json; charset=utf-8"};
    std::map<std::string, std::string> headers;
    std::string body;
};

}
