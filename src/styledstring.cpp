#include "styledstring.h"
#include <algorithm>

using subman::styledstring;

template <class Format>
auto styledstring::styled() const -> decltype(Format::paint_style) {
  return Format::paint_styled(this);
}

// styledstring::styledstring(styledstring &&sstr)
//    : underlined(std::move(sstr.underlined)), bolds(std::move(sstr.bolds)),
//      italics(std::move(sstr.italics)), colors(std::move(sstr.colors)),
//      fontsizes(std::move(sstr.fontsizes)), content(std::move(sstr.content))
//      {}

// styledstring::styledstring(styledstring const &sstr)
//    : underlined(sstr.underlined), bolds(sstr.bolds), italics(sstr.italics),
//      colors(sstr.colors), fontsizes(sstr.fontsizes) {}

// styledstring &styledstring::operator=(styledstring const &sstr) {
//  if (this != &sstr) {
//    underlined = sstr.underlined;
//    bolds = sstr.bolds;
//    italics = sstr.italics;
//    colors = sstr.colors;
//    fontsizes = sstr.fontsizes;
//  }
//  return *this;
//}
// styledstring &styledstring::operator=(styledstring &&sstr) {
//  *this = std::move(sstr);
//}
styledstring styledstring::operator+(styledstring &&sstr) const {
  using std::begin;
  using std::end;
  using std::get;
  using std::move;
  using t = std::tuple<std::vector<range>, std::vector<range>>;
  using mt = std::tuple<std::vector<attr>, std::vector<attr>>;

  styledstring tmp = *this; // copy this
  size_t first_string_length = tmp.content.size();
  tmp.content += sstr.content;

  // transform "bolds" and "underlines" and "italics"
  for (auto crange :
       {t{sstr.bolds, tmp.bolds}, t{sstr.underlineds, tmp.underlineds},
        t{sstr.italics, tmp.italics}}) {
    get<1>(crange).reserve(get<0>(crange).size() +
                           get<1>(crange).size()); // performance shit
    std::transform(begin(get<0>(crange)), end(get<0>(crange)),
                   std::back_inserter(get<1>(crange)), [&](range &r) {
                     r.first += first_string_length;
                     r.second += first_string_length;
                     return move(r);
                   });
  }

  // transforming "colors" and "fontsizes"
  for (auto crange :
       {mt{sstr.colors, tmp.colors}, mt{sstr.fontsizes, tmp.fontsizes}}) {
    std::transform(begin(get<0>(crange)), end(get<0>(crange)),
                   std::back_inserter(get<1>(crange)),
                   [&](std::pair<range, std::string> &p) {
                     p.first.first += first_string_length;
                     p.first.second += first_string_length;
                     return move(p);
                   });
  }

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
  sstr.content = std::move(str) + sstr.content;
  sstr.shift_ranges(static_cast<long>(str.size()));
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

  auto len = static_cast<int64_t>(content.size());

  // transform "bolds" and "underlines" and "italics"
  for (auto const &crange : {bolds, underlineds, italics}) {
    for (auto r : crange) {
      r.first = static_cast<size_t>(
          std::max(0l, std::min(len, static_cast<int64_t>(r.first) + shift)));
      r.second = static_cast<size_t>(
          std::max(0l, std::min(len, static_cast<int64_t>(r.second) + shift)));
    }
  }

  // transforming "colors" and "fontsizes"
  for (auto const &crange : {colors, fontsizes}) {
    for (auto r : crange) {
      r.first.first = static_cast<size_t>(std::max(
          0l, std::min(len, static_cast<int64_t>(r.first.first) + shift)));
      r.first.second = static_cast<size_t>(std::max(
          0l, std::min(len, static_cast<int64_t>(r.first.second) + shift)));
    }
  }
}

styledstring &&styledstring::operator+=(styledstring &&sstr) && {
  using std::begin;
  using std::end;
  content += sstr.content;
  sstr.shift_ranges(static_cast<int64_t>(content.size()));
  std::move(begin(sstr.italics), end(sstr.italics),
            std::back_inserter(italics));
  std::move(begin(sstr.bolds), end(sstr.bolds), std::back_inserter(bolds));
  std::move(begin(sstr.underlineds), end(sstr.underlineds),
            std::back_inserter(underlineds));
  std::move(begin(sstr.colors), end(sstr.colors), std::back_inserter(colors));
  std::move(begin(sstr.fontsizes), end(sstr.fontsizes),
            std::back_inserter(fontsizes));
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
  std::move(std::begin(tmp.italics), std::end(tmp.italics),
            std::back_inserter(italics));
  std::move(std::begin(tmp.bolds), std::end(tmp.bolds),
            std::back_inserter(bolds));
  std::move(std::begin(tmp.underlineds), std::end(tmp.underlineds),
            std::back_inserter(underlineds));
  std::move(std::begin(tmp.colors), std::end(tmp.colors),
            std::back_inserter(colors));
  std::move(std::begin(tmp.fontsizes), std::end(tmp.fontsizes),
            std::back_inserter(fontsizes));
  return *this;
}

void styledstring::clear() {
  content.clear();
  italics.clear();
  bolds.clear();
  underlineds.clear();
  colors.clear();
  fontsizes.clear();
}
