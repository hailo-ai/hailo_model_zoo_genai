/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file tests.cpp
 * @brief Tests main
 **/

#include <iostream>

#include "controller_test.hpp"

void runTests(char* test_hef) {
    MyControllerTest test(test_hef);
    test.onRun();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: ./hollma-test <test_hef>\n";
        return 1;
    }
    oatpp::Environment::init();

    runTests(argv[1]);

    /* Print how much objects were created during app running, and what have left-probably leaked */
    /* Disable object counting for release builds using '-D OATPP_DISABLE_ENV_OBJECT_COUNTERS' flag
   * for better performance */
    std::cout << "\nEnvironment:\n";
    std::cout << "objectsCount = " << oatpp::Environment::getObjectsCount()
              << "\n";
    std::cout << "objectsCreated = " << oatpp::Environment::getObjectsCreated()
              << "\n\n";

    OATPP_ASSERT(oatpp::Environment::getObjectsCount() == 0);

    oatpp::Environment::destroy();

    return 0;
}
