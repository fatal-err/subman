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

const std::regex durstr{
    "([0-9]+):([0-9]+):([0-9]+),?([0-9]+)\\s*-+>\\s*([0-9]+):"
    "([0-9]+):([0-9]+),?([0-9]+)"};

styledstring transpile_html(std::string &&line) {
  std::list<subman::attr> lattrs;
  std::string clean_line;
  char separator;
  std::list<subman::attr>::iterator ender;
  std::string name;
  static enum { TAG_NAME_PHASE, ATTR_NAME_PHASE, ATTR_VALUE_PHASE } tag_status;
  for (auto ch = begin(line); ch != end(line); ch++) {
    switch (*ch) {
    case '<':
      tag_status = TAG_NAME_PHASE;
      lattrs.emplace_back(subman::range{clean_line.size(), line.size()});
      // replacing the correct finish position for the attributes:
      if ('/' == *(ch + 1)) {
        ch++;
        ender = end(lattrs);
        name = "";
        // find the name in something like this: </tagname>
        while (!((*ch > 'a' && *ch < 'z') || (*ch > 'A' && *ch < 'Z')) &&
               ch != end(line)) {
          name += {*ch, '\0'};
          ch++;
        }
        boost::to_lower(name);
        // finding the attribute that we have to close here:
        while (ender != --begin(lattrs) && (ender--)->name != name &&
               ender->pos.finish != line.size())
          ;
        if (ender != --begin(lattrs))
          ender->pos.finish = clean_line.size();
      }
      for (; ch != end(line) || '>' == *ch; ch++) {
        switch (tag_status) {
        case TAG_NAME_PHASE:
          switch (*ch) {
          case ' ':
          case '\n':
          case '\r':
          case '\t':
          case '\f':
          case '\v':
            // done with nameing
            tag_status = ATTR_NAME_PHASE;
            break;
          default:
            (--end(lattrs))->name.append({std::move(*ch), '\0'});
          }
          break;
        case ATTR_NAME_PHASE:
          switch (*ch) {
          case '=':
            tag_status = ATTR_VALUE_PHASE;
            break;
          case ' ':
          case '\n':
          case '\r':
          case '\t':
          case '\f':
          case '\v':
            // go to the next attribute if the attribute has no value
            if ("" != (--end(lattrs))->name) {
              lattrs.emplace_back();
            } // else: skip the first/last spaces
            // skip last spaces
            break;
          default:
            (--end(lattrs))->name.append({std::move(*ch), '\0'});
            break;
          }
          break;
        case ATTR_VALUE_PHASE:
          // skip the spaces if any:
          while ((' ' == *ch || '\r' == *ch || '\n' == *ch || '\t' == *ch ||
                  '\f' == *ch || '\v' == *ch) &&
                 ch++ != end(line))
            ;
          // fiding the separator and adding the the values to it's true place
          separator = ('"' == *ch || '\'' == *ch) ? *ch : ' ';
          while (((separator == ' ') ? ('>' != *ch) : (*ch != separator)) &&
                 *(ch - 1) != '\\')
            (--end(lattrs))->value.append({*ch, '\0'});
          break;
        }
      }
      break;
    default:
      clean_line.append({std::move(*ch), '\0'});
      break;
    }
  }
  return styledstring{std::move(clean_line), std::move(lattrs)};
}

std::string to_string(subman::duration const &timestamps) noexcept {
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
  buffer << std::setfill('0') << std::setw(2) << hour << std::setw(1) << ':'
         << std::setw(2) << min << std::setw(1) << ':' << std::setw(2) << sec
         << std::setw(1) << ',' << std::setw(3) << ns; // from
  buffer << " --> ";

  tmp = to.count();
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

std::unique_ptr<subman::duration> to_duration(std::string const &str) noexcept {
  std::smatch match;
  if (std::regex_search(str, match, durstr)) {
    if (match.ready()) {
      auto from_ns =
          boost::lexical_cast<int64_t>(match[1]) * 60 * 60 * 1000;   // hour
      from_ns += boost::lexical_cast<int64_t>(match[2]) * 60 * 1000; // min
      from_ns += boost::lexical_cast<int64_t>(match[3]) * 1000 +
                 boost::lexical_cast<int64_t>(match[4]); // sec and the rest

      auto to_ns =
          boost::lexical_cast<int64_t>(match[5]) * 60 * 60 * 1000; // hour
      to_ns += boost::lexical_cast<int64_t>(match[6]) * 60 * 1000; // min
      to_ns += boost::lexical_cast<int64_t>(match[7]) * 1000 +
               boost::lexical_cast<int64_t>(match[8]); // sec and the rest
      return std::make_unique<subman::duration>(
          std::chrono::nanoseconds(from_ns), std::chrono::nanoseconds(to_ns));
    }
  }
  return nullptr;
}

std::string paint_style(styledstring sstr) noexcept {
  using std::begin;
  using std::end;
  using std::move;
  auto &attrs = sstr.get_attrs();
  std::string const &content = sstr.cget_content();
  if (attrs.empty()) {
    return content;
  }

  attrs.sort();

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

#include <iostream>
subman::document subrip::read(std::istream &stream) noexcept(false) {
  using subman::styledstring;
  if (stream) {
    document sub;
    std::unique_ptr<duration> dur = nullptr;
    styledstring content;
    std::string line;
    while (std::getline(stream, line)) {
      boost::trim(line);
      if ("" == line) {
        if (dur && content.get_content().size() > 0) {
          sub.put_subtitle(subtitle{std::move(content), std::move(*dur)});
        }
        dur = nullptr;
        content.clear();
      } else {
        if (auto ndur = to_duration(line)) {
          dur.reset(nullptr);
          dur = std::move(ndur);
        } else if (dur && !dur->is_zero()) {

          // transpile the html tags and add to the content
          content.append_line(transpile_html(std::move(line)));
        }
        // if it's not a valid duration, then it's a number or a blank
        // line which we just don't care.
      }
    }
    std::cout << sub.get_subtitles().end()->content.cget_content() << std::endl;
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
  for (auto v : sub.get_subtitles())
    out << (i++) << '\n'
        << to_string(v.timestamps).c_str() << '\n'
        << paint_style(v.content) << "\n\n";
}
