#ifndef UTILITIES_H
#define UTILITIES_H

#include "document.h"
#include <ostream>
#include <string>
#include <type_traits>

namespace subman {

// read from file
template <typename SubtitleType> subman::document load(std::istream &in);
subman::document load(std::string const &path);

// write to file
template <typename SubtitleType>
void write(std::ostream &out, const subman::document &doc);
void write(std::string const &path, const subman::document &doc);

}; // namespace subman

#endif // UTILITIES_H
