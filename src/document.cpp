#include "document.h"
#include <boost/lexical_cast.hpp>
#include <exception>
#include <tuple>

using namespace subman;

merge_method_function_t
merge_method::color(std::string const& _color) noexcept {
  return [&](styledstring& sstr) {
    sstr.color(subman::range{0, sstr.cget_content().size()}, _color);
  };
}
merge_method_function_t
merge_method::fontsize(std::string const& _fontsize) noexcept {
  return [&](styledstring& sstr) { sstr.fontsize(_fontsize); };
}

merge_method_function_t merge_method::bold() noexcept {
  return [](styledstring& sstr) { sstr.bold(); };
}
merge_method_function_t underline() noexcept {
  return [](styledstring& sstr) { sstr.underline(); };
}
merge_method_function_t italic() noexcept {
  return [](styledstring& sstr) { sstr.italic(); };
}

styledstring merge_styledstring(styledstring const& first,
                                styledstring second,
                                merge_method const& mm) noexcept {
  // if (first.cget_content().find(second.cget_content()) != std::string::npos)
  // {
  //   return second;
  // } else if (second.cget_content().find(first.cget_content()) !=
  // std::string::npos) {
  //   return first;
  // }
  auto const& max_content = std::max(first, second);
  auto max_len = max_content.cget_content().size();
  styledstring merged;

  // doing the functions:
  for (auto& func : mm.functions)
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
         j = i, i = max_content.cget_content().find('\n', i)) {
      merged += first.substr(j, i) + " ---- " + second.substr(j, i);
    }
    break;
  case merge_method_direction::RIGHT_TO_LEFT:
    for (size_t i = 0, j = 0; i < max_len;
         j = i, i = max_content.cget_content().find('\n', i)) {
      merged += second.substr(j, i) + " ---- " + first.substr(j, i);
    }
    break;
  }
  return merged;
}

void document::put_subtitle(subtitle&& v, merge_method const& mm) noexcept {
  auto lower_bound = subtitles.lower_bound(v);
  auto end = std::end(subtitles);
  auto begin = std::begin(subtitles);
  auto collided_subtitle = end;

  if (lower_bound != end &&
      lower_bound->timestamps.has_collide_with(v.timestamps))
    collided_subtitle = lower_bound;

  if (lower_bound != begin) {
    auto before_lower_bound = std::prev(lower_bound);
    if (before_lower_bound->timestamps.has_collide_with(v.timestamps))
      collided_subtitle = before_lower_bound;
  }

  if (collided_subtitle != begin &&
      std::prev(collided_subtitle)->timestamps.has_collide_with(v.timestamps))
    collided_subtitle--;

  // there is no collision between subtitles
  if (collided_subtitle == end) {
    // just insert the damn thing
    subtitles.emplace_hint(lower_bound, std::move(v));
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

  auto next_sub = std::next(collided_subtitle);

  // when one of the subtitles are between the other one. doen't matter which
  auto ainb = v.timestamps.in_between(collided_subtitle->timestamps);
  auto bina = collided_subtitle->timestamps.in_between(v.timestamps);
  if (ainb || bina) {
    auto outter = bina ? v : *collided_subtitle;
    auto inner = bina ? *collided_subtitle : v;

    // we just don't care if the new subtitle is the same as the other one that
    // already exists and it's timestamps is just almost the same.
    if (ainb && inner.content.cget_content() == outter.content.cget_content())
      return;

    auto merged = merge_styledstring(collided_subtitle->content, v.content, mm);

    // removing collided_subtitle
    subtitles.erase(collided_subtitle);

    // first part
    if (outter.timestamps.from != inner.timestamps.from) {
      subtitles.emplace_hint(
          next_sub,
          outter.content,
          duration{outter.timestamps.from, inner.timestamps.from});
    }

    // the middle part
    subtitles.emplace_hint(next_sub, std::move(merged), inner.timestamps);

    // the last part
    if (outter.timestamps.to != inner.timestamps.to) {
      subtitles.emplace_hint(
          next_sub,
          outter.content,
          duration{inner.timestamps.to, outter.timestamps.to});
    }

    return;
  }

  // when one subtitle has collision with the other one.
  // this part of the code is for when there's only one collison happening.
  if (next_sub == end || !v.timestamps.has_collide_with(next_sub->timestamps)) {
    auto first =
        v.timestamps <= collided_subtitle->timestamps ? v : *collided_subtitle;
    auto second =
        v.timestamps > collided_subtitle->timestamps ? v : *collided_subtitle;

    auto merged = merge_styledstring(collided_subtitle->content, v.content, mm);

    // removing collided_subtitle
    subtitles.erase(collided_subtitle);

    // first part
    if (first.timestamps.from != second.timestamps.from) {
      // put_subtitle({
      //   first.content,
      //   duration{first.timestamps.from, second.timestamps.from}}, mm);
      subtitles.emplace_hint(
          next_sub,
          first.content,
          duration{first.timestamps.from, second.timestamps.from});
    }

    // middle part
    subtitles.emplace_hint(
        next_sub,
        std::move(merged),
        duration{second.timestamps.from, first.timestamps.to});

    // the last part
    // we actually don't need this if statement. it's always true
    if (first.timestamps.to != second.timestamps.to) {
      subtitles.emplace_hint(
          next_sub,
          second.content,
          duration{first.timestamps.to, second.timestamps.to});
    }

    return;
  }

  // the rest of the times:
  // it means that we have collision with at least 2 other subtitles
  if (v.timestamps < collided_subtitle->timestamps) {
    // inserting the first part
    subtitles.emplace_hint(
        collided_subtitle,
        v.content,
        duration{v.timestamps.from, collided_subtitle->timestamps.from});
  }
  auto it = collided_subtitle;
  std::vector<subtitle> subtitle_registery;
  for (; it != end && v.timestamps.has_collide_with(it->timestamps); ++it) {
    // this put_subtitle will remove the collided subtitle anyway
    // so we don't need to do that, but we have to be careful about the
    // iterator in the for loop. so we did this:
    auto next = std::next(it);
    auto from = std::max(v.timestamps.from, it->timestamps.from);
    auto to = next == end ? std::min(v.timestamps.to, it->timestamps.to)
                          : std::min(v.timestamps.to, next->timestamps.from);

    // we are not going to merge the settings here. that was a miskate I made
    subtitle_registery.emplace_back(v.content, duration{from, to});
  }

  for (auto& sub : subtitle_registery) {
    put_subtitle(std::move(sub), mm);
  }

  if (it != end && v.timestamps.to > it->timestamps.to) {
    // inserting the last remmaning part
    subtitles.emplace_hint(collided_subtitle,
                           v.content,
                           duration{it->timestamps.to, v.timestamps.to});
  }
}

void document::put_subtitle(const subtitle& v,
                            merge_method const& mm) noexcept {
  put_subtitle(subtitle{v}, mm);
}

void document::replace_subtitle(decltype(subtitles)::iterator it,
                                subtitle const& replacement) noexcept {
  replace_subtitle(it, subtitle{replacement});
}
void document::replace_subtitle(decltype(subtitles)::iterator it,
                                subtitle&& replacement) noexcept {
  if (it != std::end(subtitles)) {
    auto next = std::next(it);
    subtitles.erase(it);
    subtitles.emplace_hint(next, std::move(replacement));
  }
}
document subman::merge(document const& sub1,
                       document const& sub2,
                       merge_method const& mm) noexcept {
  document new_sub = sub1;
  for (auto& v : sub2.subtitles) {
    new_sub.put_subtitle(v, mm);
  }
  return new_sub;
}

// shifting stuff
// we could just shift stuff when we were loading things; but in that
// situation we had to do it in every single format. so we do it here, it's
// not as performant as it should, but we'll be writing this once.
void document::shift(size_t s) noexcept {
  shift(static_cast<int64_t>(s));
}

void document::shift(int64_t s) noexcept {
  std::vector<subman::subtitle> subs(subtitles.begin(), subtitles.end());
  if (s < 0) {
    for (auto& sub : subs)
      sub.timestamps.shift(s);
  } else {
    for (auto& sub : subs)
      sub.timestamps.shift(static_cast<size_t>(s));
  }
  subtitles = decltype(subtitles)(subs.begin(), subs.end());
}

#include <iostream>
void document::gap(size_t gdiff) noexcept {
  size_t diff;
  auto finishline = std::prev(std::end(subtitles));
  decltype(finishline) next;
  size_t each;
  for (auto it = std::begin(subtitles); it != finishline; it++) {
    next = std::next(it);
    if (next != finishline) {
      diff = static_cast<size_t>(static_cast<int64_t>(next->timestamps.from) -
                                 static_cast<int64_t>(it->timestamps.to));
      if (diff < gdiff) {
        each = (gdiff - diff) / 2;
        auto current_item = *it;
        auto next_item = *next;
        current_item.timestamps.to -= each;
        next_item.timestamps.from += each;
        replace_subtitle(it, std::move(current_item));
        replace_subtitle(next, std::move(next_item));
      }
    }
  }
}
