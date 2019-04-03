#ifndef DURATION_H
#define DURATION_H

#include <chrono>

namespace subman {

/**
 * @brief The duration struct
 */
struct duration {
  uint64_t from{0};
  uint64_t to{0};

  // directly setting the values
  duration(decltype(from) const &from, decltype(to) const &to);

  // default constructor
  duration() = default;

  // copy constructor
  duration(duration const &d) : from(d.from), to(d.to) {}

  void reset();
  inline bool is_zero() const { return from == 0 && to == 0; }
  bool in_between(duration const &v) const;
  bool has_collide_with(duration const &v) const;
  bool operator<(duration const &) const;
  bool operator>(duration const &) const;
  bool operator>=(duration const &) const;
  bool operator<=(duration const &) const;
  bool operator==(duration const &) const;
  bool operator!=(duration const &) const;
};
} // namespace subman

#endif // DURATION_H
