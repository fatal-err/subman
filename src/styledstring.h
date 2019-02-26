#ifndef STYLEDSTRING_H
#define STYLEDSTRING_H

#include <map>
#include <string>
#include <vector>

namespace subman {

struct styledstring {
  typedef std::pair<size_t, size_t> range;
  typedef std::pair<range, std::string> attr;

  std::vector<range> underlineds;
  std::vector<range> bolds;
  std::vector<range> italics;
  std::vector<attr> colors;
  std::vector<attr> fontsizes;
  std::string content;

  styledstring() = default;
  styledstring(styledstring const &) = default;
  styledstring(styledstring &&sstr) = default;
  styledstring &operator=(styledstring const &) = default;
  styledstring &operator=(styledstring &&) = default;

  template <class Format> auto styled() const -> decltype(Format::paint_style);
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
};

styledstring operator+(std::string const &str, styledstring const &sstr);
styledstring operator+(std::string &&str, styledstring const &sstr);
styledstring &&operator+(std::string const &str, styledstring &&sstr);
styledstring &&operator+(std::string &&str, styledstring &&sstr);

} // namespace subman
#endif // STYLEDSTRING_H
