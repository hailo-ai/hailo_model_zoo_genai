/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
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

#include "dto/DTOs.hpp"
#include "oatpp/Types.hpp"

PullReadCallback::PullReadCallback(
    const std::shared_ptr<oatpp::data::mapping::ObjectMapper>& object_mapper,
    const std::shared_ptr<EventQueue>& queue,
    std::thread&& download_thread
) :
    m_object_mapper(object_mapper),
    m_queue(queue),
    m_download_thread(std::move(download_thread)) {}

oatpp::v_io_size PullReadCallback::read(
    void* buffer,
    v_buff_size bufferSize,
    oatpp::async::Action& action
) {
    (void)action;

    oatpp::v_io_size result_size = 0;
    m_queue->appendListener(
        PullEvent::DONE,
        [this, &result_size](
            const std::string&,
            const std::string&,
            int64_t,
            int64_t
        ) {
            m_download_thread.join();
            result_size = 0;
        }
    );
    m_queue->appendListener(
        PullEvent::PROGRESS,
        [this, buffer, &result_size, bufferSize](
            const std::string& status,
            const std::string& digest,
            int64_t total,
            int64_t completed
        ) {
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
            const auto result =
                m_object_mapper->writeToString(response).getValue("") + "\r\n";
            if (result.size() > bufferSize) {
                throw std::runtime_error("Buffer too small");
            }
            std::memcpy(buffer, result.data(), result.size());
            result_size = result.size();
        }
    );

    while (true) {
        m_queue->wait();
        if (m_queue->processOne()) {
            break;
        }
    }

    return result_size;
}
