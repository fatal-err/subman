#include "subtitle.h"
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <exception>
#include <regex>
#include <tuple>

using namespace subman;

subtitle::subtitle() {}

void subtitle::put_verse(verse &&v, merge_method const &mm) {
  auto place = verses.equal_range(v);
  auto collided_verse =
      (place.first->timestamps.has_cllide_with(v.timestamps))
          ? place.first
          : place.second->timestamps.has_cllide_with(v.timestamps)
                ? place.second
                : verses.end();

  if (collided_verse == verses.end()) { // just insert the damn thing
    verses.insert(std::move(v));
    return;
  }

  std::vector<decltype(place.first)> collided_verses;
  for (auto current_verse = place.first;
       v.timestamps.in_between((current_verse)->timestamps); ++current_verse) {
    collided_verses.push_back(current_verse);
  }

  // taking care of the remaining first and last part after the for loop:
  decltype(collided_verse) current_verse;
  auto len = collided_verses.size();
  current_verse = collided_verses[0];
  if (v.timestamps.from != current_verse->timestamps.from)
    verses.insert(
        verse{current_verse->content,
              duration{current_verse->timestamps.from, v.timestamps.from}});
  current_verse = collided_verses[len];
  if (v.timestamps.to != current_verse->timestamps.to)
    verses.insert(
        verse{current_verse->content,
              duration{v.timestamps.to, current_verse->timestamps.to}});

  for (size_t i = 0; i < len; i++) {
    current_verse = collided_verses[i];

    styledstring content;
    switch (mm) {
    case merge_method::TOP_TO_BOTTOM:
      content = current_verse->content + "\n" + v.content;
      break;
    case merge_method::BOTTOM_TO_TOP:
      content = v.content + "\n" + current_verse->content;
      break;
    case merge_method::LEFT_TO_RIGHT:
      auto max_content = std::max(v.content, current_verse->content);
      size_t len = max_content.content.size();
      std::string line;
      for (size_t i = 0, j = 0; i < len;
           j = i, i = max_content.content.find('\n', i)) {
        content += current_verse->content.substr(j, i) + " ---- " +
                   v.content.substr(j, i);
      }
      break;
    case merge_method::RIGHT_TO_LEFT:

      break;
    }

    // taking care of the collided parts
    verses.insert(verse{
        content,
        duration{std::max(v.timestamps.from, current_verse->timestamps.from),
                 std::min(v.timestamps.to, current_verse->timestamps.to)}});

    if (i != len && collided_verse->timestamps.to !=
                        collided_verses[i + 1]->timestamps.from) {
      // if it's not the last one, then we take care of the middle part (the
      // gap, if it exists)
      verses.insert(
          verse{v.content, duration{collided_verse->timestamps.to,
                                    collided_verses[i + 1]->timestamps.from}});
    }

    // removing the current_verse from the verses set
    verses.erase(current_verse);
  }
}

void subtitle::put_verse(const verse &v) { put_verse(verse(v)); }

subtitle merge(subtitle const &sub1, subtitle const &sub2) {
  subtitle new_sub = sub1;
  for (auto &v : sub2.get_verses()) {
    new_sub.put_verse(v);
  }
  return new_sub;
}
