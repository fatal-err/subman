#include "styledstring.h"
#include <algorithm>

using subman::attr;
using subman::range;
using subman::styledstring;

bool range::operator<(range const &r) const noexcept { return start < r.start; }
bool range::operator>(range const &r) const noexcept { return start > r.start; }
bool range::operator>=(range const &r) const noexcept {
  return start >= r.start;
}
bool range::operator<=(range const &r) const noexcept {
  return start <= r.start;
}
bool range::operator==(range const &r) const noexcept {
  return start == r.start && finish == r.finish;
}
bool range::operator!=(range const &r) const noexcept { return !(*this == r); }
bool range::in_between(range const &r) const noexcept {
  return start >= r.start && finish <= r.finish;
}
bool range::is_collided(range const &r) const noexcept {
  return (start >= r.start && start < r.finish) ||
         (finish >= r.start && finish < r.finish);
}

bool attr::operator==(attr const &a) const noexcept {
  return a.pos == pos && name == a.name && a.value == value;
}
bool attr::operator!=(attr const &a) const noexcept { return !(a == *this); }
bool attr::operator<(attr const &a) const noexcept { return pos < a.pos; }
bool attr::operator>(attr const &a) const noexcept { return pos > a.pos; }
bool attr::operator>=(attr const &a) const noexcept { return pos >= a.pos; }
bool attr::operator<=(attr const &a) const noexcept { return pos <= a.pos; }

template <class Format>
auto styledstring::styled() const noexcept(noexcept(Format::paint_style))
    -> decltype(Format::paint_style) {
  return Format::paint_styled(this);
}

styledstring styledstring::operator+(styledstring &&sstr) const noexcept {
  styledstring tmp = *this; // copy this
  tmp.content += sstr.content;

  sstr.shift_ranges(static_cast<int64_t>(tmp.content.size()));
  std::move(sstr.attrs.begin(), sstr.attrs.end(),
            std::back_inserter(tmp.attrs));

  return tmp;
}
styledstring styledstring::operator+(styledstring const &sstr) const noexcept {
  return operator+(styledstring{sstr});
}
styledstring styledstring::operator+(std::string &&str) const noexcept {
  styledstring tmp{*this};
  tmp.content += str;
  return tmp;
}
styledstring styledstring::operator+(std::string const &str) const noexcept {
  return operator+(std::string{str});
}

styledstring &&styledstring::add(std::string &&str,
                                 styledstring &&sstr) noexcept {
  sstr.shift_ranges(static_cast<int64_t>(str.size()));
  sstr.content = std::move(str) + sstr.content;
  return std::move(sstr);
}
styledstring &&operator+(std::string &&str, styledstring &&sstr) noexcept {
  return styledstring::add(std::move(str), std::move(sstr));
}
styledstring operator+(std::string const &str,
                       styledstring const &sstr) noexcept {
  return styledstring::add(std::string{str}, styledstring{sstr});
}

styledstring operator+(std::string &&str, styledstring const &sstr) noexcept {
  return styledstring::add(std::move(str), styledstring{sstr});
}
styledstring operator+(std::string const &str, styledstring &&sstr) noexcept {
  return styledstring::add(std::string(str), std::move(sstr));
}

styledstring styledstring::substr(size_t const &a, size_t const &b) const
    noexcept {
  styledstring tmp{*this};
  auto len = tmp.content.size();
  tmp.content = tmp.content.substr(a, std::min(len, b));
  if (a > 0) {
    tmp.shift_ranges(static_cast<int64_t>(a) * -1);
  }
  return tmp;
}

void styledstring::shift_ranges(int64_t const &shift) noexcept {
  for (auto &attribute : attrs) {
    attribute.pos.start += static_cast<size_t>(shift);
    attribute.pos.finish += static_cast<size_t>(shift);
  }
}

styledstring &&styledstring::operator+=(styledstring &&sstr) && noexcept {
  using std::begin;
  using std::end;
  content += sstr.content;
  sstr.shift_ranges(static_cast<int64_t>(content.size()));
  std::move(begin(sstr.attrs), end(sstr.attrs), std::back_inserter(attrs));
  return std::move(*this);
}
styledstring &styledstring::operator+=(std::string const &str) & noexcept {
  content += str;
  return *this;
}
styledstring &&styledstring::operator+=(std::string &&str) && noexcept {
  content += str;
  return std::move(*this);
}
styledstring &styledstring::operator+=(styledstring const &sstr) & noexcept {
  styledstring tmp{sstr};
  content += tmp.content;
  tmp.shift_ranges(static_cast<int64_t>(content.size()));
  std::move(std::begin(tmp.attrs), std::end(tmp.attrs),
            std::back_inserter(attrs));
  return *this;
}

void styledstring::clear() noexcept {
  content.clear();
  attrs.clear();
}

void styledstring::bold(range &&r) noexcept(false) {
  if (r.start >= content.size() || r.finish > content.size())
    throw std::invalid_argument("The specified range is not right.");
  attrs.emplace_back(attr{std::move(r), "b", nullptr});
}
void styledstring::underline(range &&r) noexcept(false) {
  attrs.emplace_back(attr{std::move(r), "u", nullptr});
}
void styledstring::color(range &&r, std::string &&_color) noexcept(false) {
  attrs.emplace_back(attr{std::move(r), "color", std::move(_color)});
}
void styledstring::fontsize(range &&r,
                            std::string &&_fontsize) noexcept(false) {
  attrs.emplace_back(attr{std::move(r), "fontsize", std::move(_fontsize)});
}
void styledstring::color(range const &r, std::string &&_color) noexcept(false) {
  color(range{r}, std::move(_color));
}
void styledstring::fontsize(range const &r,
                            std::string &&_fontsize) noexcept(false) {
  fontsize(range{r}, std::move(_fontsize));
}
void styledstring::color(range &&r, std::string const &_color) noexcept(false) {
  color(std::move(r), std::string{_color});
}
void styledstring::fontsize(range &&r,
                            std::string const &_fontsize) noexcept(false) {
  fontsize(std::move(r), std::string{_fontsize});
}

void styledstring::bold(range const &r) noexcept(false) { bold(range{r}); }
void styledstring::underline(range const &r) noexcept(false) {
  underline(range{r});
}
void styledstring::color(range const &r,
                         std::string const &_color) noexcept(false) {
  color(range{r}, std::string{_color});
}
void styledstring::fontsize(range const &r,
                            std::string const &_fontsize) noexcept(false) {
  fontsize(range{r}, std::string{_fontsize});
}
