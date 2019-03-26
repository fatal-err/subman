#ifndef UTILITIES_H
#define UTILITIES_H

#include "document.h"
#include <ostream>
#include <string>

namespace subman {

// read from file
template <typename SubtitleType> subman::document load(std::istream &in);
subman::document load(std::string const &path);

// write to file
template <typename SubtitleType>
void write(const subman::document &doc, std::ostream &out);
void write(const subman::document &doc, std::string const &path, std::string format="auto");

}; // namespace subman

#endif // UTILITIES_H
