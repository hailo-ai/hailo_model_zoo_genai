/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file writefile_callback.hpp
 * @brief Callback for writing to a file and reporting results
 **/

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include <oatpp/data/stream/FileStream.hpp>

#include "controller/pull_callback.hpp"

class OutputFileStream: public oatpp::data::stream::FileOutputStream {
  public:
    OutputFileStream(
        const char* filename,
        std::string resource,
        const std::shared_ptr<PullReadCallback::EventQueue>& queue,
        const std::string& content_length
    );

    oatpp::v_io_size write(
        const void* data,
        v_buff_size count,
        oatpp::async::Action& action
    ) override;

  private:
    std::shared_ptr<PullReadCallback::EventQueue> m_queue;
    std::string m_resource;
    int64_t m_content_length;
    int64_t m_completed;
    int_fast32_t m_update_every;
    int_fast32_t m_count;
};
