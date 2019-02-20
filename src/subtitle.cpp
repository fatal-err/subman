#include "subtitle.h"
#include <boost/filesystem.hpp>
#include <exception>

using namespace subman;
using std::string;

subtitle::subtitle() {}

subtitle load(string const &filepath) {
  auto ext = boost::filesystem::extension(filepath);
  if ("srt" == ext) {

  } else {
    throw std::invalid_argument("unknown file type");
  }
}

subtitle merge(subtitle const &sub1, subtitle const &sub2) {
  subtitle subtitle;
  for (auto sub1_portion : sub1.portions) {
  }
}

bool portion::operator<(portion const &p) { return duration < p.duration; }
bool portion::operator>(portion const &p) { return duration > p.duration; }
bool portion::operator>=(portion const &p) { return duration >= p.duration; }
bool portion::operator<=(portion const &p) { return duration <= p.duration; }
bool portion::operator==(portion const &p) { return duration == p.duration; }
