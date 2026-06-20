#pragma once

#include "../../domain/models.hpp"
#include "../../infrastructure/file_database.hpp"

#include <vector>

namespace app::recommendation_ai_ml {

struct Recommendation {
    int itemId{};
    double score{};
};

std::vector<Recommendation> recommendForSession(FileDatabase &database, const SessionRecord &session, int limit);

}
