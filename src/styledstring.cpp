#include "styledstring.h"
#include <algorithm>

using namespace subman;


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
  typedef std::tuple<std::vector<range>, std::vector<range>> t;
  typedef std::tuple<std::map<range, std::string>, std::map<range, std::string>>
      mt;

  styledstring tmp = *this; // copy this
  size_t first_string_length = tmp.content.size();
  tmp.content += move(sstr.content);

  // transform "bolds" and "underlines" and "italics"
  for (auto crange :
       {t{sstr.bolds, tmp.bolds}, t{sstr.underlined, tmp.underlined},
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
  tmp.content += std::move(str);
  return tmp;
}
styledstring styledstring::operator+(std::string const &str) const {
  return operator+(std::string{str});
}

styledstring &&styledstring::add(std::string &&str, styledstring &&sstr) {
  typedef std::tuple<std::vector<styledstring::range>> t;
  sstr.content = std::move(str) + sstr.content;
  auto s = str.size();
  for (auto crange : {sstr.underlined, sstr.bolds, sstr.italics}) {
    for (auto r : crange) {
      r.first += s;
      r.second += s;
    }
  }
  for (auto crange : {sstr.colors, sstr.fontsizes}) {
    for (auto r : crange) {
      styledstring::range q = r.first;
      styledstring::range key = r.first;
      q.first += s;
      q.second += s;
      crange[q] = std::move(r.second);
      crange.erase(key);
    }
  }
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
