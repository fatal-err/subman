#include "subtitle.h"

using namespace subman;

subtitle::subtitle(styledstring content, duration const &timestamps)
    : content(std::move(content)), timestamps(timestamps) {}

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
