#include "verse.h"

using namespace subman;

verse::verse(styledstring &&content, duration &&timestamps)
    : content(std::move(content)), timestamps(std::move(timestamps)) {}
verse::verse(styledstring const &content, duration const &timestamps)
    : content(content), timestamps(timestamps) {}
verse::verse(styledstring &&content, duration const &timestamps)
    : content(std::move(content)), timestamps(timestamps) {}
verse::verse(styledstring const &content, duration &&timestamps)
    : content(content), timestamps(std::move(timestamps)) {}

bool verse::operator<(verse const &v) const {
  return timestamps < v.timestamps;
}
bool verse::operator>(verse const &v) const {
  return timestamps > v.timestamps;
}
bool verse::operator==(verse const &v) const {
  return timestamps == v.timestamps;
}
bool verse::operator!=(verse const &v) const {
  return timestamps != v.timestamps;
}
bool verse::operator>=(verse const &v) const {
  return timestamps >= v.timestamps;
}
bool verse::operator<=(verse const &v) const {
  return timestamps <= v.timestamps;
}
