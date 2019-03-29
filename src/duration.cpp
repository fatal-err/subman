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

bool duration::has_cllide_with(duration const &v) const {
  return (to >= v.from && to <= v.to) || (from >= v.from && from <= v.to);
}

duration::duration(std::chrono::nanoseconds const &from,
                   std::chrono::nanoseconds const &to)
    : from(from), to(to) {}

void duration::reset() {
  from = std::chrono::nanoseconds{0};
  to = std::chrono::nanoseconds{0};
}
