#include "document.h"
#include <boost/lexical_cast.hpp>
#include <exception>
#include <tuple>

using namespace subman;

document::document() {}

void document::put_subtitle(subtitle &&v, merge_method const &mm) {
  auto place = subtitles.equal_range(v);
  auto collided_subtitle =
      (place.first->timestamps.has_cllide_with(v.timestamps))
          ? place.first
          : place.second->timestamps.has_cllide_with(v.timestamps)
                ? place.second
                : subtitles.end();

  if (collided_subtitle == subtitles.end()) { // just insert the damn thing
    subtitles.insert(std::move(v));
    return;
  }

  std::vector<decltype(place.first)> collided_verses;
  for (auto current_verse = place.first;
       v.timestamps.in_between((current_verse)->timestamps); ++current_verse) {
    collided_verses.push_back(current_verse);
  }

  // taking care of the remaining first and last part after the for loop:
  decltype(collided_subtitle) current_subtitle;
  auto len = collided_verses.size();
  current_subtitle = collided_verses[0];
  if (v.timestamps.from != current_subtitle->timestamps.from)
    subtitles.insert(subtitle{
        current_subtitle->content,
        duration{current_subtitle->timestamps.from, v.timestamps.from}});
  current_subtitle = collided_verses[len];
  if (v.timestamps.to != current_subtitle->timestamps.to)
    subtitles.insert(
        subtitle{current_subtitle->content,
                 duration{v.timestamps.to, current_subtitle->timestamps.to}});

  for (size_t i = 0; i < len; i++) {
    current_subtitle = collided_verses[i];

    styledstring content;
    auto max_content = std::max(v.content, current_subtitle->content);
    size_t len = max_content.get_content().size();
    switch (mm) {
    case merge_method::TOP_TO_BOTTOM:
      content = current_subtitle->content + "\n" + v.content;
      break;
    case merge_method::BOTTOM_TO_TOP:
      content = v.content + "\n" + current_subtitle->content;
      break;
    case merge_method::LEFT_TO_RIGHT:
      for (size_t i = 0, j = 0; i < len;
           j = i, i = max_content.get_content().find('\n', i)) {
        content += current_subtitle->content.substr(j, i) + " ---- " +
                   v.content.substr(j, i);
      }
      break;
    case merge_method::RIGHT_TO_LEFT:
      for (size_t i = 0, j = 0; i < len;
           j = i, i = max_content.get_content().find('\n', i)) {
        content += v.content.substr(j, i) + " ---- " +
                   current_subtitle->content.substr(j, i);
      }
      break;
    }

    // taking care of the collided parts
    subtitles.insert(subtitle{
        content,
        duration{std::max(v.timestamps.from, current_subtitle->timestamps.from),
                 std::min(v.timestamps.to, current_subtitle->timestamps.to)}});

    if (i != len && collided_subtitle->timestamps.to !=
                        collided_verses[i + 1]->timestamps.from) {
      // if it's not the last one, then we take care of the middle part (the
      // gap, if it exists)
      subtitles.insert(subtitle{
          v.content, duration{collided_subtitle->timestamps.to,
                              collided_verses[i + 1]->timestamps.from}});
    }

    // removing the current_verse from the verses set
    subtitles.erase(current_subtitle);
  }
}

void document::put_subtitle(const subtitle &v, merge_method const &mm) {
  put_subtitle(const_cast<subtitle &&>(v), mm);
}

document subman::merge(document const &sub1, document const &sub2,
                       merge_method const &mm) {
  document new_sub = sub1;
  for (auto &v : sub2.get_subtitles()) {
    new_sub.put_subtitle(v, mm);
  }
  return new_sub;
}
