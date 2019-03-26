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
styledstring &&operator+(std::string const &str, styledstring &&sstr) noexcept {
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

void styledstring::put_attribute(attr &&a) noexcept(false) {
  if (a.pos.start >= content.size() || a.pos.finish > content.size())
    throw std::invalid_argument(
        "The specified range is not valid in this context.");

  // we are promising that the subtitles will not collide with each other
  for (auto it = begin(attrs); it != attrs.end(); ++it) {
    if (!it->pos.is_collided(a.pos))
      continue;
    if (*it == a) { // it's useless to insert something that it's already there
      return;       // ignore the whole thing
    }
    if (it->pos == a.pos && it->name == a.name && it->value != a.value) {
      it->value = a.value;
      return; // done with it
    }
    if (it->name == a.name && it->value == a.value &&
        it->pos.is_collided(a.pos)) {
      auto _min = std::min(it->pos.start, a.pos.start);
      auto _max = std::max(it->pos.finish, a.pos.finish);
      it->pos.start = _min;
      it->pos.finish = _max;

      // TODO: this process may result in generating two collided attributes
      return; // done with this thing too
    }
  }

  // there's no collision
  attrs.emplace_back(std::move(a));
}

void styledstring::put_attribute(attr const &a) noexcept(false) {
  put_attribute(attr{a});
}

void styledstring::italic(range &&r) noexcept(false) {
  put_attribute(attr{std::move(r), "i", nullptr});
}
void styledstring::italic(range const &r) noexcept(false) {
  put_attribute(attr{const_cast<range &&>(r), "i", nullptr});
}
void styledstring::bold(range &&r) noexcept(false) {
  put_attribute(attr{std::move(r), "b", nullptr});
}
void styledstring::underline(range &&r) noexcept(false) {
  put_attribute(attr{std::move(r), "u", nullptr});
}
void styledstring::color(range &&r, std::string &&_color) noexcept(false) {
  put_attribute(attr{std::move(r), "color", std::move(_color)});
}
void styledstring::fontsize(range &&r,
                            std::string &&_fontsize) noexcept(false) {
  put_attribute(attr{std::move(r), "fontsize", std::move(_fontsize)});
}
void styledstring::color(range const &r, std::string &&_color) noexcept(false) {
  color(const_cast<range &&>(r), std::move(_color));
}
void styledstring::fontsize(range const &r,
                            std::string &&_fontsize) noexcept(false) {
  fontsize(const_cast<range &&>(r), std::move(_fontsize));
}
void styledstring::color(range &&r, std::string const &_color) noexcept(false) {
  color(std::move(r), std::string{_color});
}
void styledstring::fontsize(range &&r,
                            std::string const &_fontsize) noexcept(false) {
  fontsize(std::move(r), std::string{_fontsize});
}

void styledstring::bold(range const &r) noexcept(false) {
  bold(const_cast<range &&>(r));
}
void styledstring::underline(range const &r) noexcept(false) {
  underline(const_cast<range &&>(r));
}
void styledstring::color(range const &r,
                         std::string const &_color) noexcept(false) {
  color(range{r}, std::string{_color});
}
void styledstring::fontsize(range const &r,
                            std::string const &_fontsize) noexcept(false) {
  fontsize(range{r}, std::string{_fontsize});
}

void styledstring::replace_attr(decltype(attrs)::iterator &old_iter,
                                attr &&new_attr) noexcept {
  if (old_iter->pos == new_attr.pos) {
    // we don't need to change the order of the attrs' list
    old_iter->name = std::move(new_attr.name);
    old_iter->value = std::move(new_attr.value);
  } else {
    // we have to change the order of attrs' list since we are changing the
    // ranges in the attribute so it's just faster to remove existing one and
    // add the new one
    attrs.erase(old_iter);
    put_attribute(std::move(new_attr));
  }
}
void styledstring::replace_attr(decltype(attrs)::iterator &old_iter,
                                attr const &new_attr) noexcept {
  replace_attr(old_iter, attr{new_attr});
}
void styledstring::replace_attr(const decltype(attrs)::const_iterator &old_iter,
                                attr &&new_attr) noexcept {

  // we have to change the order of attrs' list since we are changing the
  // ranges in the attribute so it's just faster to remove existing one and
  // add the new one
  attrs.erase(old_iter);
  put_attribute(std::move(new_attr));
}
void styledstring::replace_attr(const decltype(attrs)::const_iterator &old_iter,
                                attr const &new_attr) noexcept {
  replace_attr(old_iter, attr{new_attr});
}
void styledstring::replace_attr(attr const &old_attr,
                                attr &&new_attr) noexcept {
  replace_attr(std::find(std::begin(attrs), std::end(attrs), old_attr),
               std::move(new_attr));
}
void styledstring::replace_attr(attr const &old_attr,
                                attr const &new_attr) noexcept {
  replace_attr(old_attr, attr{new_attr});
}

void subman::swap(attr &a, attr &b) noexcept {
  using std::swap;
  swap(a.pos, b.pos);
  swap(a.name, b.name);
  swap(a.value, b.value);
}

void subman::swap(range &a, range &b) noexcept {
  using std::swap;
  swap(a.start, b.start);
  swap(a.finish, b.finish);
}

// range (copy/move) constructors

range::range(size_t &&start, size_t &&finish) noexcept
    : start(std::move(start)), finish(std::move(finish)) {}
range::range(size_t const &start, size_t const &finish) noexcept
    : range(std::move(size_t{start}), std::move(size_t(finish))) {}
range::range(size_t const &start, size_t &&finish) noexcept
    : range{std::move(size_t{start}), std::move(finish)} {}
range::range(size_t &&start, size_t const &finish) noexcept
    : range{std::move(start), std::move(size_t{finish})} {}
range &range::operator=(range r) noexcept {
  using std::swap;
  swap(*this, r);
  return *this;
}

// attr (performance stuff)

attr::attr(range &&pos, std::string &&name, std::string &&value) noexcept
    : pos{std::move(pos)}, name{std::move(name)}, value{std::move(value)} {}
attr::attr(range const &pos, std::string &&name, std::string &&value) noexcept
    : attr(const_cast<range &&>(pos), std::move(name), std::move(value)) {}
attr::attr(range const &pos, std::string const &name,
           std::string &&value) noexcept
    : attr(range{pos}, std::string{name}, std::move(value)) {}
attr::attr(range const &pos, std::string &&name,
           std::string const &value) noexcept
    : attr{range{pos}, std::move(name), std::string{value}} {}
attr::attr(range &&pos, std::string const &name,
           std::string const &value) noexcept
    : attr{std::move(pos), std::string{name}, std::string{value}} {}
attr::attr(range const &pos, std::string const &name,
           std::string const &value) noexcept
    : attr{range{pos}, std::string{name}, std::string{value}} {}

attr::attr(attr const &a) noexcept : attr{a.pos, a.name, a.value} {}
attr::attr(attr &&a) noexcept
    : attr{std::move(a.pos), std::move(a.name), std::move(a.value)} {}

attr &attr::operator=(attr a) noexcept {
  using std::swap;
  swap(this->pos, a.pos);
  swap(this->name, a.name);
  swap(this->value, a.value);
  return *this;
}

bool styledstring::operator<(styledstring const &sstr) const noexcept {
  return content < sstr.content;
}
bool styledstring::operator>(styledstring const &sstr) const noexcept {
  return content > sstr.content;
}
bool styledstring::operator<=(styledstring const &sstr) const noexcept {
  return content <= sstr.content;
}
bool styledstring::operator>=(styledstring const &sstr) const noexcept {
  return content >= sstr.content;
}
bool styledstring::operator==(styledstring const &sstr) const noexcept {
  return content == sstr.content && attrs == sstr.attrs;
}
bool styledstring::operator!=(styledstring const &sstr) const noexcept {
  return content != sstr.content || attrs != sstr.attrs;
}
