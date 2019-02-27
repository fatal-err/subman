#include "styledstring.h"
#include <algorithm>

using subman::styledstring;

template <class Format>
auto styledstring::styled() const -> decltype(Format::paint_style) {
  return Format::paint_styled(this);
}

styledstring styledstring::operator+(styledstring &&sstr) const {
  styledstring tmp = *this;  // copy this
  tmp.content += sstr.content;

  sstr.shift_ranges(static_cast<int64_t>(tmp.content.size()));
  std::move(sstr.attrs.begin(), sstr.attrs.end(),
            std::back_inserter(tmp.attrs));

  return tmp;
}
styledstring styledstring::operator+(styledstring const &sstr) const {
  return operator+(styledstring{sstr});
}
styledstring styledstring::operator+(std::string &&str) const {
  styledstring tmp{*this};
  tmp.content += str;
  return tmp;
}
styledstring styledstring::operator+(std::string const &str) const {
  return operator+(std::string{str});
}

styledstring &&styledstring::add(std::string &&str, styledstring &&sstr) {
  sstr.shift_ranges(static_cast<int64_t>(str.size()));
  sstr.content = std::move(str) + sstr.content;
  return std::move(sstr);
}
styledstring &&operator+(std::string &&str, styledstring &&sstr) {
  return styledstring::add(std::move(str), std::move(sstr));
}
styledstring operator+(std::string const &str, styledstring const &sstr) {
  return styledstring::add(std::string{str}, styledstring{sstr});
}

styledstring operator+(std::string &&str, styledstring const &sstr) {
  return styledstring::add(std::move(str), styledstring{sstr});
}
styledstring operator+(std::string const &str, styledstring &&sstr) {
  return styledstring::add(std::string(str), std::move(sstr));
}

styledstring styledstring::substr(size_t const &a, size_t const &b) const {
  styledstring tmp{*this};
  auto len = tmp.content.size();
  tmp.content = tmp.content.substr(a, std::min(len, b));
  if (a > 0) {
    tmp.shift_ranges(static_cast<int64_t>(a) * -1);
  }
  return tmp;
}

void styledstring::shift_ranges(int64_t const &shift) {
  for (auto &attribute : attrs) {
    std::get<0>(attribute).first += shift;
    std::get<0>(attribute).second += shift;
  }
}

styledstring &&styledstring::operator+=(styledstring &&sstr) && {
  using std::begin;
  using std::end;
  content += sstr.content;
  sstr.shift_ranges(static_cast<int64_t>(content.size()));
  std::move(begin(sstr.attrs), end(sstr.attrs), std::back_inserter(attrs));
  return std::move(*this);
}
styledstring &styledstring::operator+=(std::string const &str) & {
  content += str;
  return *this;
}
styledstring &&styledstring::operator+=(std::string &&str) && {
  content += str;
  return std::move(*this);
}
styledstring &styledstring::operator+=(styledstring const &sstr) & {
  styledstring tmp{sstr};
  content += tmp.content;
  tmp.shift_ranges(static_cast<int64_t>(content.size()));
  std::move(std::begin(tmp.attrs), std::end(tmp.attrs),
            std::back_inserter(attrs));
  return *this;
}

void styledstring::clear() {
  content.clear();
  attrs.clear();
}

void styledstring::bold(styledstring::range &&r) {
  attrs.emplace_back(attr{std::move(r), "b", nullptr});
}
void styledstring::underline(styledstring::range &&r) {
  attrs.emplace_back(attr{std::move(r), "u", nullptr});
}
void styledstring::color(styledstring::range &&r, std::string &&_color) {
  attrs.emplace_back(attr{std::move(r), "color", std::move(_color)});
}
void styledstring::fontsize(styledstring::range &&r, std::string &&_fontsize) {
  attrs.emplace_back(attr{std::move(r), "fontsize", std::move(_fontsize)});
}
void styledstring::color(styledstring::range const &r, std::string &&_color) {
  color(range{r}, std::move(_color));
}
void styledstring::fontsize(styledstring::range const &r,
                            std::string &&_fontsize) {
  fontsize(range{r}, std::move(_fontsize));
}
void styledstring::color(styledstring::range &&r, std::string const &_color) {
  color(std::move(r), std::string{_color});
}
void styledstring::fontsize(styledstring::range &&r,
                            std::string const &_fontsize) {
  fontsize(std::move(r), std::string{_fontsize});
}

void styledstring::bold(styledstring::range const &r) { bold(range{r}); }
void styledstring::underline(styledstring::range const &r) {
  underline(range{r});
}
void styledstring::color(styledstring::range const &r,
                         std::string const &_color) {
  color(range{r}, std::string{_color});
}
void styledstring::fontsize(styledstring::range const &r,
                            std::string const &_fontsize) {
  fontsize(range{r}, std::string{_fontsize});
}
