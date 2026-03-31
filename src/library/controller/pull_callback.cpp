/**
 * Copyright (c) 2019-2026 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file pull_callback.cpp
 * @brief PullReadCallback implementation
 **/

#include "controller/pull_callback.hpp"

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

#include <oatpp/data/mapping/ObjectMapper.hpp>
#include <oatpp/data/stream/Stream.hpp>

#include "config/static_config.hpp"
#include "dto/DTOs.hpp"
#include "oatpp/Types.hpp"

namespace hailo_ollama
{

const std::string END_OF_STREAM_RESPONSE = "";

PullReadCallback::PullReadCallback(const std::shared_ptr<oatpp::data::mapping::ObjectMapper> &object_mapper,
    const std::shared_ptr<EventQueue> &queue, std::thread &&download_thread)
    : m_object_mapper(object_mapper), m_queue(queue), m_download_thread(std::move(download_thread)), 
      m_pending_responses()
{
    setup_listeners();
}

PullReadCallback::~PullReadCallback()
{
    if (m_download_thread.joinable()) {
        m_download_thread.join();
    }
}

void PullReadCallback::setup_listeners() noexcept
{
    m_queue->appendListener(PullEvent::DONE, [this](const std::string&, const std::string&, int64_t, int64_t)
        {
            if (m_download_thread.joinable()) {
                m_download_thread.join();
            }
            m_pending_responses.push(END_OF_STREAM_RESPONSE);
        }
    );

    m_queue->appendListener(PullEvent::PULL_ERROR, [this](const std::string &error_message, const std::string&, int64_t, int64_t)
        {
            if (m_download_thread.joinable()) {
                m_download_thread.join();
            }
            auto response = ErrorResponse::createShared();
            response->error = error_message;
            auto response_string = m_object_mapper->writeToString(response).getValue("") + config::NDJSON_LINE_TERMINATOR;
            m_pending_responses.push(std::move(response_string));
            m_pending_responses.push(END_OF_STREAM_RESPONSE);
        }
    );

    m_queue->appendListener(PullEvent::PROGRESS, [this]( const std::string &status, const std::string &digest,
        int64_t total, int64_t completed)
        {
            auto response = PullResponse::createShared();
            response->status = status;
            if (!digest.empty()) {
                response->digest = digest;
            }
            if (total >= 0) {
                response->total = total;
            }
            if (completed >= 0) {
                response->completed = completed;
            }
            auto response_string = m_object_mapper->writeToString(response).getValue("") + config::NDJSON_LINE_TERMINATOR;
            m_pending_responses.push(std::move(response_string));
        }
    );
}

oatpp::v_io_size PullReadCallback::read(void *buffer, v_buff_size bufferSize, oatpp::async::Action &action)
{
    (void)action;

    // TODO: Consider moving to a separate thread and adding a lock on the pending responses queue.
    // TODO: This is a blocking wait. Consider adding a timeout.
    // Only wait for new events if we don't already have pending responses
    if (m_pending_responses.empty()) {
        m_queue->wait();
        m_queue->processOne();
    }

    auto response = m_pending_responses.front();
    m_pending_responses.pop();
    if (response.size() > bufferSize) {
        throw std::runtime_error("Buffer too small");
    }

    std::memcpy(buffer, response.data(), response.size());
    return response.size();
}

} // namespace hailo_ollama
