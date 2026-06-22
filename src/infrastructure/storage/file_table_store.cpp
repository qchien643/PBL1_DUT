#include "file_table_store.hpp"

#include <fstream>
#include <utility>

namespace app::storage {

FileTableStore::FileTableStore(std::filesystem::path rootDirectory) : rootDirectory(std::move(rootDirectory)) {}

bool FileTableStore::tableExists(const std::string &tableName) const {
    return std::filesystem::exists(tablePath(tableName));
}

std::vector<Row> FileTableStore::loadRows(const std::string &tableName) const {
    std::vector<Row> rows;
    std::ifstream input(tablePath(tableName));
    if (!input) {
        return rows;
    }

    std::string line;
    while (std::getline(input, line)) {
        if (line.empty() || line.front() == '#') {
            continue;
        }
        rows.push_back(decodeRow(line));
    }
    return rows;
}

bool FileTableStore::saveRows(const std::string &tableName, const std::vector<Row> &rows, const std::vector<std::string> &columns) const {
    const std::filesystem::path targetPath = tablePath(tableName);
    if (targetPath.has_parent_path()) {
        std::filesystem::create_directories(targetPath.parent_path());
    }

    const std::filesystem::path temporaryPath = targetPath.string() + ".tmp";
    std::ofstream output(temporaryPath);
    if (!output) {
        return false;
    }

    output << "# table=" << tableName << '\n';
    if (!columns.empty()) {
        output << "# columns=";
        for (size_t index = 0; index < columns.size(); ++index) {
            if (index > 0) {
                output << '|';
            }
            output << columns[index];
        }
        output << '\n';
    }

    for (const Row &row : rows) {
        output << encodeRow(row) << '\n';
    }
    output.close();
    if (!output) {
        return false;
    }

    std::error_code ignoredError;
    std::filesystem::remove(targetPath, ignoredError);
    std::filesystem::rename(temporaryPath, targetPath, ignoredError);
    return !ignoredError;
}

const std::filesystem::path &FileTableStore::root() const {
    return rootDirectory;
}

std::filesystem::path FileTableStore::tablePath(const std::string &tableName) const {
    return rootDirectory / (tableName + ".txt");
}

}
