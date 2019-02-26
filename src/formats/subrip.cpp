#include "subrip.h"
#include <boost/lexical_cast.hpp>
#include <chrono>
#include <fstream>
#include <regex>

using namespace subman::formats;

std::string subrip::to_string(subman::duration const &timestamps) {
  auto from = timestamps.from, to = timestamps.to;
  std::stringstream buffer;
  long hour, min, sec, ns, tmp;
  tmp = from.count();
  hour = tmp / 60 / 60 / 1000;
  tmp -= hour * 60 * 60 * 1000;
  min = tmp / 60 / 1000;
  tmp -= min * 60 * 1000;
  sec = tmp / 1000;
  ns = tmp - sec * 1000;
  buffer << hour << ':' << min << ':' << sec << ',' << ns; // from
  buffer << " --> ";

  tmp = to.count();
  hour = tmp / 60 / 60 / 1000;
  tmp -= hour * 60 * 60 * 1000;
  min = tmp / 60 / 1000;
  tmp -= min * 60 * 1000;
  sec = tmp / 1000;
  ns = tmp - sec * 1000;
  buffer << hour << ':' << min << ':' << sec << ',' << ns; // from
  return buffer.str();
}

subman::duration subrip::to_duration(std::string const &str) {
  std::regex durstr(
      R"(\d+):(\d+):(\d+),?(\d+)\s*-+>\s*(\d+):(\d+):(\d+),?(\d+)");
  std::smatch match;
  if (std::regex_search(str, match, durstr)) {
    if (match.ready()) {
      auto from_ns =
          boost::lexical_cast<long>(match[1]) * 60 * 60 * 1000;   // hour
      from_ns += boost::lexical_cast<long>(match[2]) * 60 * 1000; // min
      from_ns += (boost::lexical_cast<long>(match[3]) * 1000 +
                  boost::lexical_cast<long>(match[4])) *
                 1000; // sec

      auto to_ns = boost::lexical_cast<long>(match[5]) * 60 * 60 * 1000; // hour
      to_ns += boost::lexical_cast<long>(match[6]) * 60 * 1000;          // min
      to_ns += (boost::lexical_cast<long>(match[7]) * 1000 +
                boost::lexical_cast<long>(match[8])) *
               1000; // sec
      return subman::duration{std::chrono::nanoseconds(from_ns),
                              std::chrono::nanoseconds(to_ns)};
    }
  }
  throw std::invalid_argument("bad string");
}

subman::subtitle subrip::to_subtitle(std::ifstream &stream) {
  using namespace subman;
  if (stream) {
    subtitle sub;
    duration dur;
    styledstring content;
    std::string line;
    while (std::getline(stream, line)) {
      if (line.empty()) {
        sub.put_verse(verse{content, dur});
        dur.reset();
        content.clear();
      } else {
        try {
          dur = to_duration(line);
        } catch (std::invalid_argument const &) {
          if (!dur.is_zero())
            content += line;
          // if it's not a valid duration, then it's a number or a blank
          // line which we just don't care.
        }
      }
    }
    return sub;
  } else {
    throw std::invalid_argument("Cannot read the content of the file.");
  }
}

void subrip::write(subman::subtitle const &sub, std::ostream &out) {
  if (!out)
    throw std::invalid_argument("Cannot write data into stream");
  int i = 1;
  for (auto v : sub.get_verses())
    out << (i++) << '\n'
        << subrip::to_string(v.timestamps).c_str() << '\n'
        << v.content.styled<subrip>().c_str() << '\n';
}

std::string subrip::paint_style(styledstring const &sstr) {
  using namespace std;
  std::stringstream str;
  auto len = sstr.content.size();
  auto starts = {begin(sstr.bolds), begin(sstr.italics), begin(sstr.colors),
                 begin(sstr.fontsizes), begin(sstr.underlineds)};
  for (size_t i = 0; i < len;) {
    auto j = min(starts);
    for (auto s : starts)
      if (s == j)
  }
  return str.str();
}
