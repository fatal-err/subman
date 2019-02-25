#ifndef STYLEDSTRING_H
#define STYLEDSTRING_H

#include <vector>
#include <string>
#include <map>

namespace subman {

struct styledstring {
  typedef std::pair<size_t, size_t> range;

  std::vector<range> underlined;
  std::vector<range> bolds;
  std::vector<range> italics;
  std::map<range, std::string> colors;
  std::map<range, std::string> fontsizes;
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
};

styledstring operator+(std::string const &str, styledstring const &sstr);
styledstring operator+(std::string &&str, styledstring const &sstr);
styledstring &&operator+(std::string const &str, styledstring &&sstr);
styledstring &&operator+(std::string &&str, styledstring &&sstr);

} // namespace subman
#endif // STYLEDSTRING_H
