#include "subtitle.h"

using namespace subman;

subtitle::subtitle(styledstring &&content, duration &&timestamps)
    : content(std::move(content)), timestamps(std::move(timestamps)) {}
subtitle::subtitle(styledstring const &content, duration const &timestamps)
    : content(content), timestamps(timestamps) {}
subtitle::subtitle(styledstring &&content, duration const &timestamps)
    : content(std::move(content)), timestamps(timestamps) {}
subtitle::subtitle(styledstring const &content, duration &&timestamps)
    : content(content), timestamps(std::move(timestamps)) {}

bool subtitle::operator<(subtitle const &v) const {
  return timestamps < v.timestamps;
}
bool subtitle::operator>(subtitle const &v) const {
  return timestamps > v.timestamps;
}
bool subtitle::operator==(subtitle const &v) const {
  return timestamps == v.timestamps;
}
bool subtitle::operator!=(subtitle const &v) const {
  return timestamps != v.timestamps;
}
bool subtitle::operator>=(subtitle const &v) const {
  return timestamps >= v.timestamps;
}
bool subtitle::operator<=(subtitle const &v) const {
  return timestamps <= v.timestamps;
}
