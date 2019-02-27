#ifndef STYLEDSTRING_H
#define STYLEDSTRING_H

#include <string>
#include <tuple>
#include <vector>

namespace subman {

class styledstring {
 public:
  using range = std::pair<size_t, size_t>;
  using attr = std::tuple<range, std::string, std::string>;

 protected:
  std::vector<attr> attrs;
  std::string content;

 public:
  styledstring() = default;
  styledstring(styledstring const &) = default;
  styledstring(styledstring &&sstr) = default;
  styledstring &operator=(styledstring const &) = default;
  styledstring &operator=(styledstring &&) = default;

  template <class Format>
  auto styled() const -> decltype(Format::paint_style);
  static styledstring &&add(std::string &&, styledstring &&);
  styledstring operator+(styledstring &&sstr) const;
  styledstring operator+(styledstring const &sstr) const;
  styledstring operator+(std::string &&str) const;
  styledstring operator+(std::string const &str) const;
  styledstring &operator+=(std::string const &str) &;
  styledstring &&operator+=(std::string &&str) &&;
  styledstring &operator+=(styledstring const &sstr) &;
  styledstring &&operator+=(styledstring &&sstr) &&;

  styledstring substr(size_t const &a, size_t const &b) const;
  void shift_ranges(long const &shift);
  void clear();

  void bold(range &&r);
  void bold(range const &r);

  void underline(range &&r);
  void underline(range const &r);
  
  void fontsize(range &&r, std::string &&_fontsize);
  void fontsize(range const &r, std::string &&_fontsize);
  void fontsize(range &&r, std::string const &_fontsize);
  void fontsize(range const&r, std::string const &_fontsize);
  
  void color(range &&r, std::string &&_color);
  void color(range const &r, std::string &&_color);
  void color(range &&r, std::string const &_color);
  void color(range const &r, std::string const &_color);
};

styledstring operator+(std::string const &str, styledstring const &sstr);
styledstring operator+(std::string &&str, styledstring const &sstr);
styledstring &&operator+(std::string const &str, styledstring &&sstr);
styledstring &&operator+(std::string &&str, styledstring &&sstr);

}  // namespace subman
#endif  // STYLEDSTRING_H
