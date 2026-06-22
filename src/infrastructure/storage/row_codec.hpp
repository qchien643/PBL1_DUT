#pragma once

#include <string>
#include <vector>

namespace app::storage {

using Row = std::vector<std::string>;

std::string encodeRow(const Row &fields);
Row decodeRow(const std::string &line);

}
