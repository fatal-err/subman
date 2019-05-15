#ifndef SUBTITLE_H
#define SUBTITLE_H

#include "duration.h"
#include "styledstring.h"
#include "subtitle.h"
#include <functional>
#include <set>
#include <vector>

/**
 * This file should be free of any format specific subtitle.
 * Here's a completely general thing that every formats in this project
 * will try to convert inputs and outputs to this general format, so
 * we would only need to do things once.
 */

namespace subman {

  enum class merge_method_direction {
    TOP_TO_BOTTOM,
    BOTTOM_TO_TOP,
    LEFT_TO_RIGHT,
    RIGHT_TO_LEFT
  };

  using merge_method_function_t = std::function<void(styledstring&)>;
  struct merge_method {
    std::vector<merge_method_function_t> functions = {};
    merge_method_direction direction = merge_method_direction::TOP_TO_BOTTOM;
    size_t gap = 100; // the gap between timestamps

    static merge_method_function_t
    fontsize(std::string const& fontsize) noexcept;
    static merge_method_function_t color(std::string const& color) noexcept;
    static merge_method_function_t bold() noexcept;
    static merge_method_function_t underline() noexcept;
    static merge_method_function_t italic() noexcept;
  };

  /**
   * @brief The subtitle class
   * use put_subtitle insead of directly modifing the subtitles
   */
  struct document {
    std::set<subtitle> subtitles;

    document() = default;
    void put_subtitle(subtitle const& v, merge_method const& mm = {});
    void put_subtitle(subtitle&& v, merge_method const& mm = {});

    void replace_subtitle(decltype(subtitles)::iterator it,
                          subtitle const& replacement);
    void replace_subtitle(decltype(subtitles)::iterator it,
                          subtitle&& replacement);

    void gap(size_t g);
    void shift(int64_t s);
  };

  /**
   * @brief merge two subtitles together
   * @param sub1
   * @param sub2
   * @return
   */
  document merge(document const& sub1,
                 document const& sub2,
                 merge_method const& mm = {});

} // namespace subman

#endif // SUBTITLE_H
