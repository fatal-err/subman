// Created by moisrex on 9/14/20.

#ifndef SUBMAN_STATS_H
#define SUBMAN_STATS_H

#include "document.h"
#include "subtitle.h"

#include <vector>
#include <string>
#include <string_view>

namespace subman {

  struct word_type {
    std::string_view word;
    std::vector<std::string_view> examples;

    bool operator==(std::string_view) const noexcept;
  };

  struct stats {
    std::vector<word_type> words;

    void process(std::string_view content);
    void process(document const &doc);
    void process(subtitle const& sub);
  };
}

#endif // SUBMAN_STATS_H
