#include "row_codec.hpp"

namespace app::storage {

std::string encodeRow(const Row &fields) {
    std::string line;
    for (size_t fieldIndex = 0; fieldIndex < fields.size(); ++fieldIndex) {
        if (fieldIndex > 0) {
            line.push_back('|');
        }
        for (char character : fields[fieldIndex]) {
            switch (character) {
                case '\\':
                    line += "\\\\";
                    break;
                case '|':
                    line += "\\|";
                    break;
                case '\n':
                    line += "\\n";
                    break;
                case '\r':
                    line += "\\r";
                    break;
                case '\t':
                    line += "\\t";
                    break;
                default:
                    line.push_back(character);
                    break;
            }
        }
    }
    return line;
}

Row decodeRow(const std::string &line) {
    Row fields;
    std::string field;
    bool escaped = false;
    for (char character : line) {
        if (escaped) {
            switch (character) {
                case 'n':
                    field.push_back('\n');
                    break;
                case 'r':
                    field.push_back('\r');
                    break;
                case 't':
                    field.push_back('\t');
                    break;
                default:
                    field.push_back(character);
                    break;
            }
            escaped = false;
            continue;
        }
        if (character == '\\') {
            escaped = true;
            continue;
        }
        if (character == '|') {
            fields.push_back(field);
            field.clear();
            continue;
        }
        field.push_back(character);
    }
    if (escaped) {
        field.push_back('\\');
    }
    fields.push_back(field);
    return fields;
}

}
