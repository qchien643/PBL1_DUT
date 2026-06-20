#include "simple_http_server.hpp"

#include "../shared/utils.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <thread>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace app::server {

namespace {

std::string trim(std::string value) {
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
        value.erase(value.begin());
    }
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
        value.pop_back();
    }
    return value;
}

std::string urlDecode(const std::string &value) {
    std::string result;
    for (size_t index = 0; index < value.size(); ++index) {
        if (value[index] == '%' && index + 2 < value.size()) {
            const std::string hex = value.substr(index + 1, 2);
            result += static_cast<char>(std::strtol(hex.c_str(), nullptr, 16));
            index += 2;
        } else if (value[index] == '+') {
            result += ' ';
        } else {
            result += value[index];
        }
    }
    return result;
}

void closeSocket(int socketHandle) {
#ifdef _WIN32
    closesocket(socketHandle);
#else
    close(socketHandle);
#endif
}

}

SimpleHttpServer::SimpleHttpServer(Handler handler) : handler(std::move(handler)) {}

bool SimpleHttpServer::listen(int port) {
#ifdef _WIN32
    WSADATA wsaData{};
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return false;
    }
#endif

    const int serverSocket = static_cast<int>(socket(AF_INET, SOCK_STREAM, 0));
    if (serverSocket < 0) {
        std::cerr << "Could not create server socket.\n";
        return false;
    }

    int option = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&option), sizeof(option));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(static_cast<unsigned short>(port));

    if (bind(serverSocket, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0) {
        std::cerr << "Could not bind to port " << port << ".\n";
        closeSocket(serverSocket);
        return false;
    }

    if (::listen(serverSocket, 16) < 0) {
        std::cerr << "Could not listen on port " << port << ".\n";
        closeSocket(serverSocket);
        return false;
    }

    std::cout << "Server running at http://localhost:" << port << "\n";
    while (true) {
        sockaddr_in clientAddress{};
        socklen_t clientLength = sizeof(clientAddress);
        const int clientSocket = static_cast<int>(accept(serverSocket, reinterpret_cast<sockaddr *>(&clientAddress), &clientLength));
        if (clientSocket < 0) {
            continue;
        }

        std::thread([this, clientSocket]() {
            std::string rawRequest;
            char buffer[4096];
            int expectedLength = -1;

            while (true) {
                const int received = recv(clientSocket, buffer, sizeof(buffer), 0);
                if (received <= 0) {
                    break;
                }
                rawRequest.append(buffer, received);

                const size_t headerEnd = rawRequest.find("\r\n\r\n");
                if (headerEnd != std::string::npos && expectedLength < 0) {
                    const std::string headers = rawRequest.substr(0, headerEnd);
                    const std::string lowerHeaders = toLower(headers);
                    const size_t contentLengthPos = lowerHeaders.find("content-length:");
                    if (contentLengthPos != std::string::npos) {
                        const size_t lineEnd = lowerHeaders.find("\r\n", contentLengthPos);
                        const std::string value = headers.substr(contentLengthPos + 15, lineEnd - contentLengthPos - 15);
                        expectedLength = static_cast<int>(headerEnd + 4 + std::stoi(trim(value)));
                    } else {
                        expectedLength = static_cast<int>(headerEnd + 4);
                    }
                }

                if (expectedLength >= 0 && static_cast<int>(rawRequest.size()) >= expectedLength) {
                    break;
                }
            }

            HttpResponse response;
            try {
                response = handler(parseRequest(rawRequest));
            } catch (const std::exception &error) {
                response.status = 500;
                response.contentType = "application/json; charset=utf-8";
                response.body = "{\"ok\":false,\"error\":{\"code\":\"SERVER_ERROR\",\"message\":\"Internal server error.\"}}";
                std::cerr << "Request failed: " << error.what() << "\n";
            }

            const std::string serialized = serializeResponse(response);
            send(clientSocket, serialized.c_str(), static_cast<int>(serialized.size()), 0);
            closeSocket(clientSocket);
        }).detach();
    }
}

HttpRequest SimpleHttpServer::parseRequest(const std::string &rawRequest) {
    HttpRequest request;
    const size_t headerEnd = rawRequest.find("\r\n\r\n");
    const std::string headerBlock = headerEnd == std::string::npos ? rawRequest : rawRequest.substr(0, headerEnd);
    request.body = headerEnd == std::string::npos ? "" : rawRequest.substr(headerEnd + 4);

    std::stringstream stream(headerBlock);
    std::string requestLine;
    std::getline(stream, requestLine);
    if (!requestLine.empty() && requestLine.back() == '\r') {
        requestLine.pop_back();
    }

    std::stringstream requestLineStream(requestLine);
    requestLineStream >> request.method >> request.rawTarget;

    std::string targetPath = request.rawTarget;
    const size_t queryPos = targetPath.find('?');
    if (queryPos != std::string::npos) {
        const std::string queryString = targetPath.substr(queryPos + 1);
        targetPath = targetPath.substr(0, queryPos);
        std::stringstream queryStream(queryString);
        std::string pair;
        while (std::getline(queryStream, pair, '&')) {
            const size_t equalsPos = pair.find('=');
            if (equalsPos == std::string::npos) {
                request.query[urlDecode(pair)] = "";
            } else {
                request.query[urlDecode(pair.substr(0, equalsPos))] = urlDecode(pair.substr(equalsPos + 1));
            }
        }
    }
    request.path = targetPath.empty() ? "/" : targetPath;

    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        const size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            continue;
        }
        request.headers[toLower(trim(line.substr(0, colonPos)))] = trim(line.substr(colonPos + 1));
    }

    return request;
}

std::string SimpleHttpServer::serializeResponse(const HttpResponse &response) {
    std::stringstream stream;
    stream << "HTTP/1.1 " << response.status << ' ';
    if (response.status == 200) {
        stream << "OK";
    } else if (response.status == 404) {
        stream << "Not Found";
    } else {
        stream << "Error";
    }
    stream << "\r\n";
    stream << "Content-Type: " << response.contentType << "\r\n";
    stream << "Content-Length: " << response.body.size() << "\r\n";
    stream << "Connection: close\r\n";
    for (const auto &header : response.headers) {
        stream << header.first << ": " << header.second << "\r\n";
    }
    stream << "\r\n";
    stream << response.body;
    return stream.str();
}

}
