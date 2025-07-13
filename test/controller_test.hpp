/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file controller_test.hpp
 * @brief Tests for MyController
 **/

#pragma once

#include <filesystem>

#include <oatpp-test/UnitTest.hpp>

class MyControllerTest: public oatpp::test::UnitTest {
  public:
    explicit MyControllerTest(const char* test_hef);

    void onRun() override;

  private:
    std::filesystem::path m_test_hef;
};
