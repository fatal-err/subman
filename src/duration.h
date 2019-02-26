#ifndef DURATION_H
#define DURATION_H

#include <chrono>

namespace subman {

/**
 * @brief The duration struct
 */
struct duration {
  std::chrono::nanoseconds from{0};
  std::chrono::nanoseconds to{0};

  // directly setting the values
  duration(std::chrono::nanoseconds &&from, std::chrono::nanoseconds &&to);
  duration(std::chrono::nanoseconds const &from,
           std::chrono::nanoseconds const &to);

  // default constructor
  duration() = default;

  // copy constructor
  duration(duration const &d) : from(d.from), to(d.to) {}

  void reset();
  inline bool is_zero() const {
    return from == std::chrono::nanoseconds(0) &&
           to == std::chrono::nanoseconds(0);
  }
  bool in_between(duration const &v) const;
  bool has_cllide_with(duration const &v) const;
  bool operator<(duration const &) const;
  bool operator>(duration const &) const;
  bool operator>=(duration const &) const;
  bool operator<=(duration const &) const;
  bool operator==(duration const &) const;
  bool operator!=(duration const &) const;
};
} // namespace subman

#endif // DURATION_H
