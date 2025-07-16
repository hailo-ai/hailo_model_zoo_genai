/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file split.hpp
 * @brief Class for "strtok" replacement
 **/

#pragma once

#include <iterator>
#include <string_view>

class SplitIterator {
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type = std::string_view;
    using difference_type = std::ptrdiff_t;
    using pointer = const std::string_view*;
    using reference = const std::string_view&;

    SplitIterator(
        std::string_view str,
        std::string_view delimiter,
        std::size_t pos = 0
    );

    reference operator*() const;

    pointer operator->() const;

    SplitIterator& operator++();

    SplitIterator operator++(int);

    bool operator==(const SplitIterator& other) const;

    bool operator!=(const SplitIterator& other) const;

  private:
    void advance();

    std::string_view m_str;
    std::string_view m_delimiter;
    std::size_t m_pos;
    std::string_view m_current;
    bool m_end;
};

class SplitRange {
  public:
    SplitRange(std::string_view str, std::string_view delimiter);
    SplitIterator begin() const;

    SplitIterator end() const;

  private:
    std::string_view m_str;
    std::string_view m_delimiter;
};
