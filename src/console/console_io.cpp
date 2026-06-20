#include "console_io.hpp"

#include <iostream>

namespace app::console {

int readInt(const std::string &prompt) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) {
        return -1;
    }
    try {
        return std::stoi(line);
    } catch (...) {
        return -1;
    }
}

std::string readText(const std::string &prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

void printHelp() {
    std::cout << "Casual Dining Ordering MVP\n\n";
    std::cout << "Usage:\n";
    std::cout << "  restaurant_mvp manager\n";
    std::cout << "  restaurant_mvp cashier\n";
    std::cout << "  restaurant_mvp customer <table-code>\n";
    std::cout << "  restaurant_mvp kitchen [kitchen|bar]\n";
    std::cout << "  restaurant_mvp server [port]\n";
    std::cout << "  restaurant_mvp test\n";
    std::cout << "  restaurant_mvp reset\n\n";
    std::cout << "Data file: data/restaurant_db.txt\n";
}

}
