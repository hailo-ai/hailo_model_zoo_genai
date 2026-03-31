/**
 * Copyright (c) 2019-2026 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file pull_callback.hpp
 * @brief Callback for downloading file
 **/

#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <thread>
#include <queue>

#include <eventpp/eventqueue.h>
#include <oatpp/data/mapping/ObjectMapper.hpp>
#include <oatpp/data/stream/Stream.hpp>

#include "dto/DTOs.hpp"

namespace hailo_ollama
{

enum class PullEvent {
    DONE,
    PROGRESS,
    PULL_ERROR
};

struct PullResponseData {
};

class PullReadCallback : public oatpp::data::stream::ReadCallback
{
public:
    using EventQueue = eventpp::EventQueue<PullEvent,
        void(const std::string &status, const std::string &digest, int64_t total, int64_t completed)>;

    PullReadCallback(const std::shared_ptr<oatpp::data::mapping::ObjectMapper> &object_mapper,
        const std::shared_ptr<EventQueue> &queue, std::thread &&download_thread);

    virtual ~PullReadCallback();

    oatpp::v_io_size read(void *buffer, v_buff_size bufferSize, oatpp::async::Action &action) override;

private:
    void setup_listeners() noexcept;

    std::shared_ptr<oatpp::data::mapping::ObjectMapper> m_object_mapper;
    std::shared_ptr<EventQueue> m_queue;
    std::thread m_download_thread;
    std::queue<std::string> m_pending_responses;
};

} // namespace hailo_ollama
