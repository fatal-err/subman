#ifndef STYLEDSTRING_H
#define STYLEDSTRING_H

#include <string>
#include <tuple>
#include <vector>

namespace subman {

struct range {
  size_t start = 0, finish = 0;

  bool operator<(range const &r) const noexcept;
  bool operator>(range const &r) const noexcept;
  bool operator>=(range const &r) const noexcept;
  bool operator<=(range const &r) const noexcept;
  bool operator==(range const &r) const noexcept;
  bool operator!=(range const &r) const noexcept;

  bool in_between(range const &r) const noexcept;
  bool is_collided(range const &r) const noexcept;
};

struct attr {
  range pos;
  std::string name;
  std::string value;

  bool operator==(attr const &a) const noexcept;
  bool operator!=(attr const &a) const noexcept;
  bool operator<(attr const &a) const noexcept;
  bool operator>(attr const &a) const noexcept;
  bool operator>=(attr const &a) const noexcept;
  bool operator<=(attr const &a) const noexcept;
};

class styledstring {
  std::vector<attr> attrs;
  std::string content;

public:
  styledstring() = default;
  styledstring(styledstring const &) = default;
  styledstring(styledstring &&sstr) = default;
  styledstring &operator=(styledstring const &) = default;
  styledstring &operator=(styledstring &&) = default;

  template <class Format>
  auto styled() const noexcept(noexcept(Format::paint_style))
      -> decltype(Format::paint_style);

  static styledstring &&add(std::string &&, styledstring &&) noexcept;
  styledstring operator+(styledstring &&sstr) const noexcept;
  styledstring operator+(styledstring const &sstr) const noexcept;
  styledstring operator+(std::string &&str) const noexcept;
  styledstring operator+(std::string const &str) const noexcept;
  styledstring &operator+=(std::string const &str) & noexcept;
  styledstring &&operator+=(std::string &&str) && noexcept;
  styledstring &operator+=(styledstring const &sstr) & noexcept;
  styledstring &&operator+=(styledstring &&sstr) && noexcept;

  styledstring substr(size_t const &a, size_t const &b) const noexcept;
  void shift_ranges(int64_t const &shift) noexcept;
  void clear() noexcept;

  void bold(range &&r) noexcept(false);
  void bold(range const &r) noexcept(false);

  void underline(range &&r) noexcept(false);
  void underline(range const &r) noexcept(false);

  void italic(range &&r) noexcept(false);
  void italic(range const &r) noexcept(false);

  void fontsize(range &&r, std::string &&_fontsize) noexcept(false);
  void fontsize(range const &r, std::string &&_fontsize) noexcept(false);
  void fontsize(range &&r, std::string const &_fontsize) noexcept(false);
  void fontsize(range const &r, std::string const &_fontsize) noexcept(false);

  void color(range &&r, std::string &&_color) noexcept(false);
  void color(range const &r, std::string &&_color) noexcept(false);
  void color(range &&r, std::string const &_color) noexcept(false);
  void color(range const &r, std::string const &_color) noexcept(false);

  auto get_attrs() const noexcept -> std::vector<attr> const & { return attrs; }
  auto get_content() const noexcept -> std::string const & { return content; }
};

styledstring operator+(std::string const &str,
                       styledstring const &sstr) noexcept;
styledstring operator+(std::string &&str, styledstring const &sstr) noexcept;
styledstring &&operator+(std::string const &str, styledstring &&sstr) noexcept;
styledstring &&operator+(std::string &&str, styledstring &&sstr) noexcept;

} // namespace subman
#endif // STYLEDSTRING_H
