#include "recommendation_service.hpp"

#include "../../policies/business_policies.hpp"
#include "../../shared/utils.hpp"

#include <algorithm>

namespace app::recommendation_ai_ml {

namespace {

std::vector<double> itemFactors(const MenuItemRecord &item) {
    const std::string name = toLower(item.name);
    double protein = 0.0;
    double fresh = 0.0;
    double drink = item.category == "drink" ? 1.0 : 0.0;
    double dessert = item.category == "dessert" ? 1.0 : 0.0;
    double premium = std::min(1.0, item.price / 100000.0);

    if (name.find("beef") != std::string::npos || name.find("chicken") != std::string::npos || name.find("seafood") != std::string::npos) {
        protein = 1.0;
    }
    if (name.find("vegetable") != std::string::npos || name.find("fruit") != std::string::npos || name.find("orange") != std::string::npos) {
        fresh = 1.0;
    }
    return {protein, fresh, drink, dessert, premium};
}

double dotProduct(const std::vector<double> &left, const std::vector<double> &right) {
    double result = 0.0;
    for (size_t index = 0; index < std::min(left.size(), right.size()); ++index) {
        result += left[index] * right[index];
    }
    return result;
}

int soldQuantity(const FileDatabase &database, int menuItemId) {
    int quantity = 0;
    for (const OrderItemRecord &item : database.orderItems) {
        if (item.menuItemId == menuItemId && item.status != "CANCELLED" && item.status != "REJECTED") {
            quantity += item.quantity;
        }
    }
    return quantity;
}

}

std::vector<Recommendation> recommendForSession(FileDatabase &database, const SessionRecord &session, int limit) {
    std::vector<double> sessionVector(5, 0.0);
    int signalCount = 0;
    std::vector<int> orderedItemIds;

    for (const OrderRecord &order : database.orders) {
        if (order.sessionId != session.id) {
            continue;
        }
        for (const OrderItemRecord &orderItem : database.orderItems) {
            if (orderItem.orderId != order.id || orderItem.status == "CANCELLED" || orderItem.status == "REJECTED") {
                continue;
            }
            MenuItemRecord *menuItem = database.findMenuItemById(orderItem.menuItemId);
            if (menuItem == nullptr) {
                continue;
            }

            orderedItemIds.push_back(menuItem->id);
            const std::vector<double> factors = itemFactors(*menuItem);
            for (size_t index = 0; index < sessionVector.size(); ++index) {
                sessionVector[index] += factors[index] * orderItem.quantity;
            }
            signalCount += orderItem.quantity;
        }
    }

    if (signalCount > 0) {
        for (double &value : sessionVector) {
            value /= signalCount;
        }
    }

    std::vector<Recommendation> recommendations;
    for (const MenuItemRecord &item : database.menuItems) {
        if (!policy::canOrderItem(item)) {
            continue;
        }
        if (std::find(orderedItemIds.begin(), orderedItemIds.end(), item.id) != orderedItemIds.end()) {
            continue;
        }

        double score = 0.0;
        if (signalCount > 0) {
            score = dotProduct(sessionVector, itemFactors(item));
        } else {
            score = soldQuantity(database, item.id) * 0.25;
            if (item.category == "food") {
                score += 0.5;
            }
        }
        if (item.category == "drink" || item.category == "dessert") {
            score += 0.2;
        }
        recommendations.push_back({item.id, score});
    }

    std::sort(recommendations.begin(), recommendations.end(), [](const Recommendation &left, const Recommendation &right) {
        return left.score > right.score;
    });

    if (static_cast<int>(recommendations.size()) > limit) {
        recommendations.resize(limit);
    }
    return recommendations;
}

}
