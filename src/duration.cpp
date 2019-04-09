#include "duration.h"
#include <utility>

using namespace subman;

bool duration::operator<(duration const &p) const { return from < p.from; }
bool duration::operator>(duration const &p) const { return from > p.from; }
bool duration::operator>=(duration const &p) const { return from >= p.from; }
bool duration::operator<=(duration const &p) const { return from <= p.from; }
bool duration::operator==(duration const &p) const {
  return from == p.from && to == p.to;
}
bool duration::operator!=(const duration &p) const {
  return from != p.from || to != p.to;
}

bool duration::in_between(duration const &v) const {
  return from >= v.from && to <= v.to;
}

bool duration::has_collide_with(duration const &v) const {
  return (from >= v.from && from < v.to) || (v.from >= from && v.from < to);
}

duration::duration(decltype(from) const &_from, decltype(to) const &_to)
    : from(_from), to(_to) {}

void duration::reset() {
  from = 0;
  to = 0;
}

void duration::shift(int64_t n) {
  auto nn = static_cast<size_t>(n);
  from += from - nn > from ? 0 : nn;
  to += to - nn > to ? 0 : nn;
}

void swap(duration &a, duration &b) noexcept {
  using std::swap;
  swap(a.from, b.from);
  swap(a.to, b.to);
}
