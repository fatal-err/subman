#ifndef VERSE_H
#define VERSE_H

#include "styledstring.h"
#include "duration.h"

namespace subman {


/**
 * @brief The verse struct
 */
struct verse {
  styledstring content;
  duration timestamps;

  // copy constructor
  verse(verse const &v) : content(v.content), timestamps(v.timestamps) {}

  verse(styledstring &&content, duration &&timestamps);
  verse(styledstring &&content, duration const &timestamps);
  verse(styledstring const &content, duration &&timestamps);
  verse(styledstring const &content, duration const &timestamps);

  bool operator<(verse const &) const;
  bool operator>(verse const &) const;
  bool operator==(verse const &) const;
  bool operator!=(verse const &) const;
  bool operator>=(verse const &) const;
  bool operator<=(verse const &) const;
};

} // namespace subman

#endif // VERSE_H
