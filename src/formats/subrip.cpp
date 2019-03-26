#include "subrip.h"
#include <boost/lexical_cast.hpp>
#include <chrono>
#include <fstream>
#include <regex>
#include <tuple>
#include <algorithm>

using namespace subman::formats;
using subman::styledstring;

std::string subrip::to_string(subman::duration const &timestamps) noexcept {
  auto from = timestamps.from, to = timestamps.to;
  std::stringstream buffer;
  int64_t hour, min, sec, ns, tmp;
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

subman::duration subrip::to_duration(std::string const &str) noexcept(false) {
  std::regex durstr(
      R"(\d+):(\d+):(\d+),?(\d+)\s*-+>\s*(\d+):(\d+):(\d+),?(\d+)");
  std::smatch match;
  if (std::regex_search(str, match, durstr)) {
    if (match.ready()) {
      auto from_ns =
          boost::lexical_cast<int64_t>(match[1]) * 60 * 60 * 1000;   // hour
      from_ns += boost::lexical_cast<int64_t>(match[2]) * 60 * 1000; // min
      from_ns += (boost::lexical_cast<int64_t>(match[3]) * 1000 +
                  boost::lexical_cast<int64_t>(match[4])) *
                 1000; // sec

      auto to_ns =
          boost::lexical_cast<int64_t>(match[5]) * 60 * 60 * 1000; // hour
      to_ns += boost::lexical_cast<int64_t>(match[6]) * 60 * 1000; // min
      to_ns += (boost::lexical_cast<int64_t>(match[7]) * 1000 +
                boost::lexical_cast<int64_t>(match[8])) *
               1000; // sec
      return subman::duration{std::chrono::nanoseconds(from_ns),
                              std::chrono::nanoseconds(to_ns)};
    }
  }
  throw std::invalid_argument("bad string");
}

subman::document subrip::read(std::istream &stream) noexcept(false) {
  using subman::styledstring;
  if (stream) {
    document sub;
    duration dur;
    styledstring content;
    std::string line;
    while (std::getline(stream, line)) {
      if (line.empty()) {
        sub.put_verse(subtitle{content, dur});
        dur.reset();
        content.clear();
      } else {
        try {
          dur = to_duration(line);
        } catch (std::invalid_argument const &) {
          if (!dur.is_zero()) {
            content += line;
          }
          // if it's not a valid duration, then it's a number or a blank
          // line which we just don't care.
        }
      }
    }
    return sub;
  }
  throw std::invalid_argument("Cannot read the content of the file.");
}

void subrip::write(subman::document const &sub,
                   std::ostream &out) noexcept(false) {
  if (!out) {
    throw std::invalid_argument("Cannot write data into stream");
  }
  int i = 1;
  for (auto v : sub.get_verses())
    out << (i++) << '\n'
        << subrip::to_string(v.timestamps).c_str() << '\n'
        << paint_style(v.content) << '\n';
}

std::string subrip::paint_style(styledstring sstr) noexcept {
  using std::begin;
  using std::end;
  using std::move;
  auto &attrs = sstr.get_attrs();
  std::string const &content = sstr.cget_content();
  if (attrs.empty()) {
    return content;
  }

  std::sort(begin(attrs), end(attrs));

  // the content that will be styled and returned
  std::string ncontent = content.substr(0, begin(attrs)->pos.start);
  std::string _start, _end;
  for (auto it = begin(attrs); it != end(attrs); it++) {
    auto const &attribute = *it;
    if (attribute.name == "b" || attribute.name == "u" ||
        attribute.name == "i") {
      _start = "<" + attribute.name + ">";
      _end = "</" + attribute.name + ">";
    } else if (attribute.name == "color") {
      _start = "<font color=\"" + attribute.value +
               "\">"; // TODO: make sure the value is correct
      _end = "</font>";
    } else if (attribute.name == "fontsize") {
      _start = "<font size=\"" + attribute.value +
               "\">"; // TODO: make sure the value is correct
      _end = "</font>";
    }
    ncontent.append(std::move(_start));
    ncontent.append(content.substr(attribute.pos.start, attribute.pos.finish));
    ncontent.append(std::move(_end));
  }

  return ncontent;
}
