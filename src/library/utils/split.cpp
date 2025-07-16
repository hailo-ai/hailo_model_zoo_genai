/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file split.cpp
 * @brief SplitIterator implementation
 **/

#include "utils/split.hpp"

#include <string_view>

SplitIterator::SplitIterator(
    std::string_view str,
    std::string_view delimiter,
    std::size_t pos
) :
    m_str(str),
    m_delimiter(delimiter),
    m_pos(pos),
    m_end(false) {
    advance();
}

SplitIterator::reference SplitIterator::operator*() const {
    return m_current;
}

SplitIterator::pointer SplitIterator::operator->() const {
    return &m_current;
}

SplitIterator& SplitIterator::operator++() {
    advance();
    return *this;
}

SplitIterator SplitIterator::operator++(int) {
    SplitIterator tmp = *this;
    advance();
    return tmp;
}

bool SplitIterator::operator==(const SplitIterator& other) const {
    return m_pos == other.m_pos && m_end == other.m_end;
}

bool SplitIterator::operator!=(const SplitIterator& other) const {
    return !(*this == other);
}

void SplitIterator::advance() {
    if (m_pos == std::string_view::npos) {
        m_end = true;
        return;
    }

    auto next_pos = m_str.find(m_delimiter, m_pos);
    if (next_pos == std::string_view::npos) {
        m_current = m_str.substr(m_pos);
        m_pos = std::string_view::npos;
    } else {
        m_current = m_str.substr(m_pos, next_pos - m_pos);
        m_pos = next_pos + m_delimiter.length();
    }
}

SplitRange::SplitRange(std::string_view str, std::string_view delimiter) :
    m_str(str),
    m_delimiter(delimiter) {}

SplitIterator SplitRange::begin() const {
    return SplitIterator(m_str, m_delimiter);
}

SplitIterator SplitRange::end() const {
    return SplitIterator(m_str, m_delimiter, std::string_view::npos);
}
