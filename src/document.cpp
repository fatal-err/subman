#include "document.h"
#include <boost/lexical_cast.hpp>
#include <exception>
#include <tuple>

using namespace subman;

#include <iostream>

styledstring merge_styledstring(styledstring const &first, styledstring second,
                                merge_method const &mm) {
  auto max_content = std::max(first, second);
  auto max_len = max_content.get_content().size();
  styledstring merged;

  // doing the functions:
  for (auto &func : mm.functions)
    func(second);

  // directions:
  switch (mm.direction) {
  case merge_method_direction::TOP_TO_BOTTOM:
    merged = first + "\n" + second;
    break;
  case merge_method_direction::BOTTOM_TO_TOP:
    merged = second + "\n" + first;
    break;
  case merge_method_direction::LEFT_TO_RIGHT:
    for (size_t i = 0, j = 0; i < max_len;
         j = i, i = max_content.get_content().find('\n', i)) {
      merged += first.substr(j, i) + " ---- " + second.substr(j, i);
    }
    break;
  case merge_method_direction::RIGHT_TO_LEFT:
    for (size_t i = 0, j = 0; i < max_len;
         j = i, i = max_content.get_content().find('\n', i)) {
      merged += second.substr(j, i) + " ---- " + first.substr(j, i);
    }
    break;
  }
  return merged;
}

void document::put_subtitle(subtitle &&v, merge_method const &mm) {
  auto lower_bound = subtitles.lower_bound(v);
  auto end = std::end(subtitles);
  auto begin = std::begin(subtitles);
  auto collided_subtitle = end;

  if (lower_bound != end &&
      lower_bound->timestamps.has_collide_with(v.timestamps))
    collided_subtitle = lower_bound;

  if (collided_subtitle != begin &&
      std::prev(collided_subtitle)->timestamps.has_collide_with(v.timestamps))
    collided_subtitle--;

  //  std::cout << std::boolalpha << v.content.cget_content() << "++++"
  //            << v.timestamps.from.count() << "-" << v.timestamps.to.count()
  //            << "........." << (collided_subtitle == std::end(subtitles))
  //            << "......."
  //            << (collided_subtitle != begin &&
  //                std::prev(collided_subtitle)
  //                    ->timestamps.has_collide_with(v.timestamps))
  //            << std::endl;

  // there is no collision between subtitles
  if (collided_subtitle == end) {
    // just insert the damn thing
    assert(v.timestamps.from != v.timestamps.to);
    subtitles.emplace_hint(lower_bound, std::move(v));
    return;
  }

  assert(v.timestamps.to != v.timestamps.from);
  std::cout << collided_subtitle->content.get_content() << std::endl;
  assert(collided_subtitle->timestamps.to !=
         collided_subtitle->timestamps.from);
  // duplicated subtitles are ignored
  if (*collided_subtitle == v) {
    return;
  }

  // both subtitles are in the same time but with different content;
  // so we change the content just for that subtitle
  if (collided_subtitle->timestamps == v.timestamps) {
    std::cout << "merge 1" << std::endl;
    assert(collided_subtitle->timestamps.from !=
           collided_subtitle->timestamps.to);
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

    std::cout << "merge 2  === " << outter.timestamps.from << "-"
              << outter.timestamps.to << " --- " << inner.timestamps.from << "-"
              << inner.timestamps.to << "--------- "
              << outter.content.get_content() << "-----"
              << inner.content.get_content() << std::endl;

    assert(outter.timestamps.from != inner.timestamps.from);
    assert(inner.timestamps.from != inner.timestamps.to);
    assert(outter.timestamps.from != outter.timestamps.to);
    assert(inner.timestamps.to != outter.timestamps.to);

    // we just don't care if the new subtitle is the same as the other one that
    // already exists and it's timestamps is just almost the same.
    if (ainb && inner.content.cget_content() == outter.content.cget_content())
      return;

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
  if (next_sub == end || !v.timestamps.has_collide_with(next_sub->timestamps)) {
    auto &first =
        v.timestamps <= collided_subtitle->timestamps ? v : *collided_subtitle;
    auto &second =
        v.timestamps > collided_subtitle->timestamps ? v : *collided_subtitle;

    assert(first.timestamps.from != first.timestamps.to);
    assert(second.timestamps.from != second.timestamps.to);
    assert(first.timestamps.from != second.timestamps.from);
    assert(second.timestamps.from != first.timestamps.to);
    assert(first.timestamps.to != second.timestamps.to);

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
  if (v.timestamps < collided_subtitle->timestamps) {
    // inserting the first part
    std::cout << "first part: " << v.content.get_content() << std::endl;
    assert(v.timestamps.from != collided_subtitle->timestamps.from);
    subtitles.emplace_hint(
        collided_subtitle, v.content,
        duration{v.timestamps.from, collided_subtitle->timestamps.from});
  }
  auto it = collided_subtitle;
  std::vector<subtitle> subtitle_registery;
  while (it != end && v.timestamps.has_collide_with(it->timestamps)) {
    // this put_subtitle will remove the collided subtitle anyway
    // so we don't need to do that, but we have to be careful about the
    // iterator in the for loop. so we did this:
    auto next = std::next(it);
    auto from = std::max(v.timestamps.from, it->timestamps.from);
    auto to = next == end ? std::min(v.timestamps.to, it->timestamps.to)
                          : std::min(v.timestamps.to, next->timestamps.from);
    assert(from != to);
    subtitle_registery.emplace_back(
        merge_styledstring(it->content, v.content, mm), duration{from, to});
    it++;

    std::cout << v.content.cget_content() << "\n\n";
  }

  for (auto &sub : subtitle_registery) {
    assert(sub.timestamps.from != sub.timestamps.to);
    put_subtitle(std::move(sub), mm);
  }

  if (it != end && v.timestamps.to > it->timestamps.to) {
    // inserting the last remmaning part
    std::cout << "last part: " << v.content.get_content() << std::endl;
    assert(it->timestamps.to != v.timestamps.to);
    subtitles.emplace_hint(collided_subtitle, v.content,
                           duration{it->timestamps.to, v.timestamps.to});
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
