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
    duration(decltype(from) const& from, decltype(to) const& to);

    // default constructor
    duration() = default;

    // copy constructor
    duration(duration const& d) : from(d.from), to(d.to) {
    }

    void reset() noexcept;
    void shift(int64_t n) noexcept;
    void shift(size_t n) noexcept;
    inline bool is_zero() const {
      return from == 0 && to == 0;
    }
    bool in_between(duration const& v) const noexcept;
    bool has_collide_with(duration const& v) const noexcept;
    bool operator<(duration const&) const noexcept;
    bool operator>(duration const&) const noexcept;
    bool operator>=(duration const&) const noexcept;
    bool operator<=(duration const&) const noexcept;
    bool operator==(duration const&) const noexcept;
    bool operator!=(duration const&) const noexcept;

    friend void swap(duration& a, duration& b) noexcept;
  };

  void swap(duration& a, duration& b) noexcept;

} // namespace subman

#endif // DURATION_H
