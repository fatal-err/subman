#ifndef SUBTITLE_H
#define SUBTITLE_H

#include "duration.h"
#include "styledstring.h"
#include "verse.h"
#include <set>

/**
 * This file should be free of any format specific subtitle.
 * Here's a completely general thing that every formats in this project
 * will try to convert inputs and outputs to this general format, so
 * we would only need to do things once.
 */

namespace subman {

enum class merge_method {
  TOP_TO_BOTTOM,
  BOTTOM_TO_TOP,
  LEFT_TO_RIGHT,
  RIGHT_TO_LEFT
};

/**
 * @brief The subtitle class
 */
class document {
  std::set<subtitle> verses;

public:
  document();
  void put_verse(subtitle const &v);
  void put_verse(
      subtitle &&v,
      merge_method const &mm = merge_method::TOP_TO_BOTTOM); // move semantic
  std::set<subtitle> get_verses() const { return verses; }
};

/**
 * @brief merge two subtitles together
 * @param sub1
 * @param sub2
 * @return
 */
document merge(document const &sub1, document const &sub2,
               merge_method const &mm = merge_method::TOP_TO_BOTTOM);

} // namespace subman

#endif // SUBTITLE_H
