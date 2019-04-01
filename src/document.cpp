#include "document.h"
#include <boost/lexical_cast.hpp>
#include <exception>
#include <tuple>

using namespace subman;

#include <iostream>

styledstring merge_styledstring(styledstring const &first,
                                styledstring const &second,
                                merge_method const &mm) {
  auto max_content = std::max(first, second);
  auto max_len = max_content.get_content().size();
  styledstring merged;
  switch (mm) {
  case merge_method::TOP_TO_BOTTOM:
    merged = first + "\n" + second;
    break;
  case merge_method::BOTTOM_TO_TOP:
    merged = second + "\n" + first;
    break;
  case merge_method::LEFT_TO_RIGHT:
    for (size_t i = 0, j = 0; i < max_len;
         j = i, i = max_content.get_content().find('\n', i)) {
      merged += first.substr(j, i) + " ---- " + second.substr(j, i);
    }
    break;
  case merge_method::RIGHT_TO_LEFT:
    for (size_t i = 0, j = 0; i < max_len;
         j = i, i = max_content.get_content().find('\n', i)) {
      merged += second.substr(j, i) + " ---- " + first.substr(j, i);
    }
    break;
  }
  std::cout << merged.cget_content() << std::endl;
  return merged;
}

void document::put_subtitle(subtitle &&v, merge_method const &mm) {
  auto place = subtitles.equal_range(v);
  auto collided_subtitle =
      (place.first != std::end(subtitles) &&
       v.timestamps.has_collide_with(place.first->timestamps))
          ? place.first
          : ((place.second != std::end(subtitles) &&
              v.timestamps.has_collide_with(place.second->timestamps))
                 ? place.second
                 : subtitles.end());

  // there is no collision between subtitles
  if (collided_subtitle == subtitles.end()) { // just insert the damn thing
    subtitles.emplace_hint(place.first, std::move(v));
    return;
  }

  // duplicated subtitles are ignored
  if (*collided_subtitle == v) {
    return;
  }

  // both subtitles are in the same time but with different content;
  // so we change the content just for that subtitle
  if (collided_subtitle->timestamps == v.timestamps) {
    collided_subtitle->content +=
        merge_styledstring(collided_subtitle->content, v.content, mm);
    return;
  }

  subtitle current_sub = *collided_subtitle;
  auto next_sub = ++collided_subtitle;
  --collided_subtitle;

  // when one of the subtitles are between the other one. doen't matter which
  auto ainb = v.timestamps.in_between(collided_subtitle->timestamps);
  auto bina = collided_subtitle->timestamps.in_between(v.timestamps);
  if (ainb || bina) {
    auto &outter = bina ? v : current_sub;
    auto &inner = bina ? current_sub : v;

    // removing collided_subtitle
    subtitles.erase(collided_subtitle);

    // first part
    if (outter.timestamps.from != inner.timestamps.from) {
      subtitles.emplace_hint(
          next_sub, outter.content,
          duration{outter.timestamps.from, inner.timestamps.from});
    }

    // the middle part
    subtitles.emplace_hint(
        next_sub, merge_styledstring(current_sub.content, v.content, mm),
        inner.timestamps);

    // the last part
    if (outter.timestamps.to != inner.timestamps.to) {
      subtitles.emplace_hint(
          next_sub, outter.content,
          duration{inner.timestamps.to, outter.timestamps.to});
    }

    return;
  }

  // when one subtitle has collision with the other one.
  // this part of the code is for when there's only one collison happening.
  if (next_sub == std::end(subtitles) ||
      !v.timestamps.has_collide_with(next_sub->timestamps)) {
    auto &first =
        v.timestamps <= collided_subtitle->timestamps ? v : *collided_subtitle;
    auto &second =
        v.timestamps > collided_subtitle->timestamps ? v : *collided_subtitle;

    // removing collided_subtitle
    subtitles.erase(collided_subtitle);

    // first part
    if (first.timestamps.from != second.timestamps.from) {
      subtitles.emplace_hint(
          next_sub, first.content,
          duration{first.timestamps.from, second.timestamps.from});
    }

    // middle part
    subtitles.emplace_hint(
        next_sub, merge_styledstring(current_sub.content, v.content, mm),
        duration{second.timestamps.from, first.timestamps.to});

    // the last part
    // we actually don't need this if statement. it's always true
    if (first.timestamps.to != second.timestamps.to) {
      subtitles.emplace_hint(
          next_sub, second.content,
          duration{first.timestamps.to, second.timestamps.to});
    }

    return;
  }

  // the rest of the times:
  // it means that we have collision with at least 2 other subtitles
  for (auto it = collided_subtitle;
       it != std::end(subtitles) &&
       v.timestamps.has_collide_with(it->timestamps);) {
    // this put_subtitle will remove the collided subtitle anyway
    // so we don't need to do that, but we have to be careful about the
    // iterator in the for loop. so we did this:
    auto current_timestamps = it->timestamps;
    it++;
    put_subtitle({v.content,
                  duration{std::min(v.timestamps.from, current_timestamps.from),
                           std::max(current_timestamps.to, v.timestamps.to)}},
                 mm);
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
