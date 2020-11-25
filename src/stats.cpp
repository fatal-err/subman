// Created by moisrex on 9/14/20.

#include "stats.h"

void subman::stats::process(const subman::subtitle &sub) {
  process(sub.content.cget_content());
}

void subman::stats::process(const subman::document &doc) {
  for (auto const &sub : doc.subtitles) {
    process(sub);
  }
}
void subman::stats::process(std::string_view content) {
  auto start_it = content.begin();
  auto end_it = start_it;
  for (; end_it != content.end(); ++end_it) {
    switch (*end_it) {
    case ' ':
    case '\n':
    case '\t': {
      auto str = std::string_view{start_it, end_it};
      auto found = std::find(words.begin(), words.end(), str);
      if (found == words.end()) {
        words.push_back(word_type{
            .word = str,
            .examples{content}
        });
      } else {
        found->examples.push_back(content);
      }
      start_it = end_it;
      continue;
    }
    }
  }
}
bool subman::word_type::operator==(std::string_view str) const noexcept {
  return word == str;
}
