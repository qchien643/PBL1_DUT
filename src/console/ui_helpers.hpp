#pragma once

#include "../domain/models.hpp"

#include <string>

namespace app::console {

std::string formatMoney(int amount);
std::string explainTableState(const std::string &state);
std::string explainOrderItemStatus(const std::string &status);
void printLine(char character = '-', int width = 72);
void printTitle(const std::string &title, const std::string &subtitle = "");
void printEmpty(const std::string &message);
void printHint(const std::string &message);
void printResult(const OperationResult &result);

}
