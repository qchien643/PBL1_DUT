#pragma once

#include "row_codec.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace app::storage {

class FileTableStore {
public:
    explicit FileTableStore(std::filesystem::path rootDirectory);

    bool tableExists(const std::string &tableName) const;
    std::vector<Row> loadRows(const std::string &tableName) const;
    bool saveRows(const std::string &tableName, const std::vector<Row> &rows, const std::vector<std::string> &columns = {}) const;
    const std::filesystem::path &root() const;

private:
    std::filesystem::path rootDirectory;

    std::filesystem::path tablePath(const std::string &tableName) const;
};

}
