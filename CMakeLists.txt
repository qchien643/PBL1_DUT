cmake_minimum_required(VERSION 3.16)

project(CasualDiningOrderingMvp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(restaurant_mvp
    src/main.cpp
    src/shared/utils.cpp
    src/infrastructure/file_database.cpp
    src/policies/business_policies.cpp
    src/modules/table_session/table_session_service.cpp
    src/modules/menu_inventory/menu_inventory_service.cpp
    src/modules/order_management/order_management_service.cpp
    src/modules/kitchen_fulfillment/kitchen_fulfillment_service.cpp
    src/modules/payment_billing/payment_billing_service.cpp
    src/modules/recommendation_ai_ml/recommendation_service.cpp
    src/modules/reporting_audit/reporting_audit_service.cpp
    src/server/json_helpers.cpp
    src/server/simple_http_server.cpp
    src/server/server_app.cpp
    src/console/console_io.cpp
    src/console/ui_helpers.cpp
    src/console/console_views.cpp
    src/console/manager_console.cpp
    src/console/cashier_console.cpp
    src/console/customer_console.cpp
    src/console/kitchen_console.cpp
    src/tests/business_scenario_tests.cpp
)

if(WIN32)
    target_link_libraries(restaurant_mvp PRIVATE ws2_32)
endif()
