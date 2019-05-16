#include "duration.h"
#include <utility>

using namespace subman;

bool duration::operator<(duration const& p) const noexcept {
  return from < p.from;
}
bool duration::operator>(duration const& p) const noexcept {
  return from > p.from;
}
bool duration::operator>=(duration const& p) const noexcept {
  return from >= p.from;
}
bool duration::operator<=(duration const& p) const noexcept {
  return from <= p.from;
}
bool duration::operator==(duration const& p) const noexcept {
  return from == p.from && to == p.to;
}
bool duration::operator!=(const duration& p) const noexcept {
  return from != p.from || to != p.to;
}

bool duration::in_between(duration const& v) const noexcept {
  return from >= v.from && to <= v.to;
}

bool duration::has_collide_with(duration const& v) const noexcept {
  return (from >= v.from && from < v.to) || (v.from >= from && v.from < to);
}

duration::duration(decltype(from) const& _from, decltype(to) const& _to)
    : from(_from), to(_to) {
}

void duration::reset() noexcept {
  from = 0;
  to = 0;
}

void duration::shift(int64_t n) noexcept {
  if (n >= 0) {
    from += static_cast<size_t>(n);
    to += static_cast<size_t>(n);
  } else {
    if ((static_cast<int64_t>(from) - n) > static_cast<int64_t>(from))
      from = 0;
    else
      from -= static_cast<size_t>(n);
    if ((static_cast<int64_t>(to) - n) > static_cast<int64_t>(to))
      to = 0;
    else
      to -= static_cast<size_t>(n);
  }
}

void duration::shift(size_t n) noexcept {
  from += n;
  to += n;
}

void swap(duration& a, duration& b) noexcept {
  using std::swap;
  swap(a.from, b.from);
  swap(a.to, b.to);
}
