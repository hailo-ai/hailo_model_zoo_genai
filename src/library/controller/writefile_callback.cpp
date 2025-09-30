/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file writefile_callback.cpp
 * @brief OutputFileStream implementation
 **/

#include "controller/writefile_callback.hpp"

#include <string>

#include <oatpp/data/stream/FileStream.hpp>

#include "config/static_config.hpp"
#include "controller/pull_callback.hpp"

OutputFileStream::OutputFileStream(
    const char* filename,
    std::string resource,
    const std::shared_ptr<PullReadCallback::EventQueue>& queue,
    const std::string& content_length
) :
    oatpp::data::stream::FileOutputStream(filename),
    m_resource(std::move(resource)),
    m_queue(queue),
    m_content_length(std::stoll(content_length)),
    m_completed(0),
    m_update_every(config::output_file_stream_update_every),
    m_count(0) {}

oatpp::v_io_size OutputFileStream::write(
    const void* data,
    v_buff_size count,
    oatpp::async::Action& action
) {
    auto result =
        oatpp::data::stream::FileOutputStream::write(data, count, action);
    m_completed += result;
    m_count += 1;
    if (m_count % m_update_every != 0) {
        return result;
    }
    m_queue->enqueue(
        PullEvent::PROGRESS,
        "pulling",
        m_resource,
        m_content_length,
        m_completed
    );

    return result;
}
