#include "ui_helpers.hpp"

#include <algorithm>
#include <iostream>

namespace app::console {

std::string formatMoney(int amount) {
    const bool negative = amount < 0;
    std::string digits = std::to_string(negative ? -amount : amount);
    std::string formatted;
    int groupCount = 0;

    for (auto iterator = digits.rbegin(); iterator != digits.rend(); ++iterator) {
        if (groupCount == 3) {
            formatted.push_back(',');
            groupCount = 0;
        }
        formatted.push_back(*iterator);
        ++groupCount;
    }

    if (negative) {
        formatted.push_back('-');
    }
    std::reverse(formatted.begin(), formatted.end());
    return formatted + " VND";
}

std::string explainTableState(const std::string &state) {
    if (state == "AVAILABLE") {
        return "Available";
    }
    if (state == "OCCUPIED") {
        return "Serving";
    }
    if (state == "CLEANING") {
        return "Waiting clean";
    }
    return state;
}

std::string explainOrderItemStatus(const std::string &status) {
    if (status == "SUBMITTED") {
        return "Waiting cashier approval";
    }
    if (status == "ACCEPTED") {
        return "Waiting kitchen/bar";
    }
    if (status == "PREPARING") {
        return "Preparing";
    }
    if (status == "READY") {
        return "Ready to serve";
    }
    if (status == "SERVED") {
        return "Served";
    }
    if (status == "CANCEL_REQUESTED") {
        return "Cancel requested";
    }
    if (status == "NEEDS_CUSTOMER_CONFIRMATION") {
        return "Needs customer confirmation";
    }
    if (status == "ISSUE_PENDING_DECISION") {
        return "Kitchen issue pending decision";
    }
    if (status == "CANCELLED") {
        return "Cancelled - not charged";
    }
    if (status == "REJECTED") {
        return "Rejected";
    }
    return status;
}

void printLine(char character, int width) {
    for (int index = 0; index < width; ++index) {
        std::cout << character;
    }
    std::cout << '\n';
}

void printTitle(const std::string &title, const std::string &subtitle) {
    std::cout << '\n';
    printLine('=');
    std::cout << title << '\n';
    if (!subtitle.empty()) {
        std::cout << subtitle << '\n';
    }
    printLine('=');
}

void printEmpty(const std::string &message) {
    std::cout << "[EMPTY] " << message << '\n';
}

void printHint(const std::string &message) {
    std::cout << "Hint: " << message << '\n';
}

void printResult(const OperationResult &result) {
    std::cout << (result.ok ? "[OK] " : "[WARN] ") << result.message << '\n';
    if (!result.code.empty() && result.code != "OK") {
        std::cout << "Code: " << result.code << '\n';
    }
    if (!result.requiredAction.empty()) {
        std::cout << "Required action: " << result.requiredAction << '\n';
    }
}

}
