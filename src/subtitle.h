#ifndef VERSE_H
#define VERSE_H

#include "duration.h"
#include "styledstring.h"

namespace subman {

/**
 * @brief The subtitle struct
 */
struct subtitle {
  mutable styledstring content;
  duration timestamps;

  // copy constructor
  subtitle(subtitle const &v) : content(v.content), timestamps(v.timestamps) {}
  subtitle(styledstring content, duration const &timestamps);

  bool operator<(subtitle const &) const;
  bool operator>(subtitle const &) const;
  bool operator==(subtitle const &) const;
  bool operator!=(subtitle const &) const;
  bool operator>=(subtitle const &) const;
  bool operator<=(subtitle const &) const;
};

} // namespace subman

#endif // VERSE_H
