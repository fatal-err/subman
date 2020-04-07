#include "subrip.h"
#include "../utilities.h"
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <list>
#include <regex>
#include <tuple>

using namespace subman::formats;
using subman::styledstring;

styledstring transpile_html(std::string&& line) {
  static const std::regex tag_regex{"<([^>]+)>"};
  static const std::regex tag_name_regex{"^/?[A-z_-]+"};
  static const std::regex attributes_regex{
      R"(([A-z_-]+)\s+(('[^']*')|(\"[^\"]*\")|([^\s])))"};
  styledstring sstr;

  std::smatch tag_matches, tag_name_matcher, attrs_matcher;
  std::string data, tag_name, attr_name, value;
  size_t position = 0;
  decltype(std::rbegin(sstr.get_attrs())) it;
  std::string::const_iterator line_iter{line.cbegin()}, attr_data_iter;
  bool has_found_something = false;
  while (
      std::regex_search(line_iter, std::cend(line), tag_matches, tag_regex)) {
    has_found_something = true;
    data = tag_matches[1];
    if (!std::regex_search(data, tag_name_matcher, tag_name_regex)) {
      continue;
    }
    position = sstr.cget_content().size() +
               static_cast<size_t>(tag_matches.position());
    tag_name = tag_name_matcher[0];
    boost::to_lower(tag_name);
    if ('/' == data[0]) { // the end of a tag
      tag_name =
          tag_name.substr(1); // removing the first slash in the beggining
      for (it = std::rbegin(sstr.get_attrs());
           it != std::rend(sstr.get_attrs());
           it++) {
        if (it->name == tag_name && it->pos.finish == line.size()) {
          it->pos.finish = position;
        }
      }
    } else { // it's a new tag
      subman::range pos{position, line.size()};
      if ("i" == tag_name)
        sstr.italic(pos);
      else if ("b" == tag_name)
        sstr.bold(pos);
      else if ("u" == tag_name)
        sstr.underline(pos);
      else if ("font" == tag_name) {
        std::string attrs_data = tag_name_matcher.suffix().str();
        attr_data_iter = std::begin(attrs_data);
        while (std::regex_search(attr_data_iter,
                                 std::cend(attrs_data),
                                 attrs_matcher,
                                 attributes_regex)) {
          attr_name = attrs_matcher[1];
          value = attrs_matcher[2];
          if ('\'' == value[0] || '"' == value[0])
            value = value.substr(1, value.size() - 1);
          if ("size" == attr_name)
            sstr.fontsize(pos, value);
          else if ("color" == attr_name)
            sstr.color(pos, value);
          attr_data_iter = attrs_matcher.prefix().first;
        }
      }
    }
    line_iter = tag_matches.suffix().first;
    sstr.get_content() += tag_matches.prefix().str();
  }

  // last pieces of the subtitle
  if (!has_found_something)
    sstr.get_content() = line;
  else
    sstr.get_content() += tag_matches.suffix().str();
  return sstr;
}

std::string to_string(subman::duration const& timestamps) noexcept {
  auto from = timestamps.from, to = timestamps.to;
  std::stringstream buffer;
  uint64_t hour, min, sec, ns, tmp;
  tmp = from;
  hour = tmp / 60 / 60 / 1000;
  tmp -= hour * 60 * 60 * 1000;
  min = tmp / 60 / 1000;
  tmp -= min * 60 * 1000;
  sec = tmp / 1000;
  ns = tmp - sec * 1000;
  buffer << std::setfill('0') << std::setw(2) << hour << std::setw(1) << ':'
         << std::setw(2) << min << std::setw(1) << ':' << std::setw(2) << sec
         << std::setw(1) << ',' << std::setw(3) << ns; // from
  buffer << " --> ";

  tmp = to;
  hour = tmp / 60 / 60 / 1000;
  tmp -= hour * 60 * 60 * 1000;
  min = tmp / 60 / 1000;
  tmp -= min * 60 * 1000;
  sec = tmp / 1000;
  ns = tmp - sec * 1000;
  buffer << std::setfill('0') << std::setw(2) << hour << std::setw(1) << ':'
         << std::setw(2) << min << std::setw(1) << ':' << std::setw(2) << sec
         << std::setw(1) << ',' << std::setw(3) << ns; // to
  return buffer.str();
}

std::unique_ptr<subman::duration> to_duration(std::string const& str) noexcept {
  static const std::regex durstr{
      "([0-9]+):([0-9]+):([0-9]+),?([0-9]+)\\s*-+>\\s*([0-9]+):"
      "([0-9]+):([0-9]+),?([0-9]+)"};
  std::smatch match;
  try {
    if (std::regex_search(str, match, durstr)) {
      if (match.ready()) {
        auto from_ns =
            boost::lexical_cast<uint64_t>(match[1]) * 60 * 60 * 1000;   // hour
        from_ns += boost::lexical_cast<uint64_t>(match[2]) * 60 * 1000; // min
        from_ns += boost::lexical_cast<uint64_t>(match[3]) * 1000 +
                   boost::lexical_cast<uint64_t>(match[4]); // sec and the rest

        auto to_ns =
            boost::lexical_cast<uint64_t>(match[5]) * 60 * 60 * 1000; // hour
        to_ns += boost::lexical_cast<uint64_t>(match[6]) * 60 * 1000; // min
        to_ns += boost::lexical_cast<uint64_t>(match[7]) * 1000 +
                 boost::lexical_cast<uint64_t>(match[8]); // sec and the rest
        return std::make_unique<subman::duration>(from_ns, to_ns);
      }
    }
  } catch (...) { // we return nullptr if anything happens.
  }
  return nullptr;
}

std::string subman::formats::paint_style(styledstring sstr) noexcept {
  using std::begin;
  using std::end;
  using std::move;
  auto& attrs = sstr.get_attrs();
  std::string const& content = sstr.cget_content();
  if (attrs.empty()) {
    return content;
  }

  attrs.sort();

  // the content that will be styled and returned
  std::string ncontent = content;
  std::string _start, _end;
  //size_t shift_start, shift_end;
  auto size = content.size();
  auto attrs_end = std::end(attrs);
  // std::vector<std::pair<subman::range, subman::range>> shifts;
  for (auto it = begin(attrs); it != attrs_end; it++) {
    auto const& attribute = *it;
    if (attribute.name == "b" || attribute.name == "u" ||
        attribute.name == "i") {
      _start = "<" + attribute.name + ">";
      _end = "</" + attribute.name + ">";
    } else if (attribute.name == "color") {
      _start = "<font color=\"" + attribute.value + "\">";
      _end = "</font>";
    } else if (attribute.name == "fontsize") {
      _start = "<font size=\"" + attribute.value + "\">";
      _end = "</font>";
    }
    // auto finish = std::min(attribute.pos.finish, ncontent.size());
//    shift_start = 0;
//    shift_end = 0;
//    for (auto const& s : shifts) {
//      if (s.first.start <= attribute.pos.start)
//        shift_start += s.second.start;
//      if (s.first.finish <= attribute.pos.start)
//        shift += s.second.finish;
//    }
    std::stringstream res;
    res << _start << ncontent.substr(attribute.pos.start, attribute.pos.finish - attribute.pos.start) << _end;
    ncontent.replace(
        attribute.pos.start /* + shift */,
        attribute.pos.finish - attribute.pos.start,
        res.str());
//    shifts.emplace_back(attribute.pos,
//                        subman::range{_start.size(), _end.size()});

      // shifting the attributes
      it->pos.finish += _start.size();
      for (auto nit = std::next(it); nit != end(attrs); ++nit) {
        if (nit->pos.start > attribute.pos.start) {
          nit->pos.start += _start.size();
        }
        if (nit->pos.start >= attribute.pos.finish) {
          nit->pos.start += _end.size();
        }

        if (nit->pos.finish > attribute.pos.start) {
          nit->pos.finish += _start.size();
        }
        if (nit->pos.finish > attribute.pos.finish) {
          nit->pos.finish += _end.size();
        }
      }
  }

  return ncontent;
}

subman::document subrip::read(std::istream& stream) noexcept(false) {
  using subman::styledstring;
  if (stream) {
    document sub;
    std::unique_ptr<duration> dur;
    std::string content;
    std::string line;
    while (std::getline(stream, line)) {
      boost::trim(line);
      if (line.empty()) {
        if (dur && !content.empty()) {
          sub.put_subtitle(subtitle{transpile_html(std::move(content)), *dur});
        }
        dur = nullptr;
        content.clear();
      } else {
        if (auto ndur = to_duration(line)) {
          dur.reset(nullptr);
          dur = std::move(ndur);
        } else if (dur && !dur->is_zero()) {

          // transpile the html tags and add to the content
          content.append(content.empty() ? line : '\n' + line);
        }
        // if it's not a valid duration, then it's a number or a blank
        // line which we just don't care.
      }
    }

    // we repeat this because last subtitle may not have an empty line
    if (line.empty()) {
      if (dur && !content.empty()) {
        sub.put_subtitle(subtitle{transpile_html(std::move(content)), *dur});
      }
    }
    return sub;
  }
  throw std::invalid_argument("Cannot read the content of the file.");
}

void subrip::write(subman::document const& sub,
                   std::ostream& out) noexcept(false) {
  if (!out) {
    throw std::invalid_argument("Cannot write data into stream");
  }
  int i = 1;
  for (const auto& v : sub.subtitles)
    out << (i++) << '\n'
        << to_string(v.timestamps).c_str() << '\n'
        << subman::formats::paint_style(v.content) << "\n\n";
}
