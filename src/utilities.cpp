#include "utilities.h"
// read from file
template <typename SubtitleType> subman::document load(std::istream &in) {
  return SubtitleType::read(in);
}
subman::document load(std::string const &path) {}

// write to file
template <typename SubtitleType>
void write(std::ostream &out, const subman::document &doc) {}
void write(std::string const &path, const subman::document &doc) {}
