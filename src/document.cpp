#include "document.h"
#include <boost/lexical_cast.hpp>
#include <exception>
#include <tuple>

using namespace subman;

void document::put_subtitle(subtitle &&v, merge_method const &mm) {
  auto place = subtitles.equal_range(v);
  auto collided_subtitle =
      (place.first->timestamps.has_cllide_with(v.timestamps))
          ? place.first
          : place.second->timestamps.has_cllide_with(v.timestamps)
                ? place.second
                : subtitles.end();

  if (collided_subtitle == subtitles.end()) { // just insert the damn thing
    subtitles.emplace(std::move(v));
    return;
  }

  // taking care of the first part of the subtitle
  if (v.timestamps.from != place.first->timestamps.from)
    subtitles.insert(
        subtitle{place.first->content,
                 duration{place.first->timestamps.from, v.timestamps.from}});

  styledstring content;
  styledstring max_content;
  size_t max_len;
  auto it = place.first;
  for (; it != end(subtitles) && v.timestamps.in_between(it->timestamps);
       ++it) {
    max_content = std::max(v.content, it->content);
    max_len = max_content.get_content().size();
    switch (mm) {
    case merge_method::TOP_TO_BOTTOM:
      content = it->content + "\n" + v.content;
      break;
    case merge_method::BOTTOM_TO_TOP:
      content = v.content + "\n" + it->content;
      break;
    case merge_method::LEFT_TO_RIGHT:
      for (size_t i = 0, j = 0; i < max_len;
           j = i, i = max_content.get_content().find('\n', i)) {
        content += it->content.substr(j, i) + " ---- " + v.content.substr(j, i);
      }
      break;
    case merge_method::RIGHT_TO_LEFT:
      for (size_t i = 0, j = 0; i < max_len;
           j = i, i = max_content.get_content().find('\n', i)) {
        content += v.content.substr(j, i) + " ---- " + it->content.substr(j, i);
      }
      break;
    }

    // taking care of the collided parts
    subtitles.insert(subtitle{
        content, duration{std::max(v.timestamps.from, it->timestamps.from),
                          std::min(v.timestamps.to, it->timestamps.to)}});

    if (it != end(subtitles) &&
        collided_subtitle->timestamps.to != (++it--)->timestamps.from) {
      // if it's not the last one, then we take care of the middle part (the
      // gap, if it exists)
      subtitles.insert(
          subtitle{v.content, duration{collided_subtitle->timestamps.to,
                                       (++it--)->timestamps.from}});
    }

    // taking care of the last remaining piece of subtitle
    ++it;
    if (it != end(subtitles) && !v.timestamps.in_between(it->timestamps) &&
        v.timestamps.to != it->timestamps.to) {
      subtitles.insert(
          subtitle{it->content, duration{v.timestamps.to, it->timestamps.to}});
    }
    --it;

    // removing the current_verse from the verses set
    subtitles.erase(it);
    content.clear();
    max_content.clear();
  }
}

void document::put_subtitle(const subtitle &v, merge_method const &mm) {
  put_subtitle(subtitle{v}, mm);
}

document subman::merge(document const &sub1, document const &sub2,
                       merge_method const &mm) {
  document new_sub = sub1;
  for (auto &v : sub2.get_subtitles()) {
    new_sub.put_subtitle(v, mm);
  }
  return new_sub;
}
