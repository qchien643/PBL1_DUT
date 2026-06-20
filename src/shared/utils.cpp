#include "utils.hpp"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace app {

std::string sanitize(std::string value) {
    for (char &character : value) {
        if (character == '|') {
            character = '/';
        }
        if (character == '\n' || character == '\r') {
            character = ' ';
        }
    }
    return value.empty() ? "-" : value;
}

std::string restore(const std::string &value) {
    return value == "-" ? "" : value;
}

std::vector<std::string> split(const std::string &line, char delimiter) {
    std::vector<std::string> fields;
    std::string field;
    std::stringstream stream(line);
    while (std::getline(stream, field, delimiter)) {
        fields.push_back(field);
    }
    return fields;
}

std::string joinIds(const std::vector<int> &ids) {
    std::string result;
    for (size_t index = 0; index < ids.size(); ++index) {
        if (index > 0) {
            result += ",";
        }
        result += std::to_string(ids[index]);
    }
    return result.empty() ? "-" : result;
}

std::vector<int> parseIds(const std::string &value) {
    std::vector<int> ids;
    if (value == "-" || value.empty()) {
        return ids;
    }

    std::string token;
    std::stringstream stream(value);
    while (std::getline(stream, token, ',')) {
        if (!token.empty()) {
            ids.push_back(std::stoi(token));
        }
    }
    return ids;
}

std::string nowStamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &currentTime);
#else
    localtime_r(&currentTime, &localTime);
#endif

    std::stringstream stream;
    stream << std::put_time(&localTime, "%Y-%m-%d_%H:%M:%S");
    return stream.str();
}

std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

}
