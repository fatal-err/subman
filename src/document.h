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
    void put_subtitle(subtitle const& v, merge_method const& mm = {}) noexcept;
    void put_subtitle(subtitle&& v, merge_method const& mm = {}) noexcept;

    void replace_subtitle(decltype(subtitles)::iterator it,
                          subtitle const& replacement) noexcept;
    void replace_subtitle(decltype(subtitles)::iterator it,
                          subtitle&& replacement) noexcept;

    void gap(size_t g) noexcept;
    void shift(size_t s) noexcept;
    void shift(int64_t s) noexcept;

    /**
     * @brief returns a new document that their subtitles match the specified keyword
     * @param keyword
     * @return subman::document
     */
    document matches(std::string const& keyword) const noexcept;

    /**
     * @brief returns a new document containing only subtitles that their content contains the specified keyword
     * @param keyword
     * @return subman::document
     */
    document contains(std::string const& keyword) const noexcept;
    document regex(std::string const& pattern ) const noexcept;
  };

  /**
   * @brief merge two subtitles together
   * @param sub1
   * @param sub2
   * @return
   */
  document merge(document const& sub1,
                 document const& sub2,
                 merge_method const& mm = {}) noexcept;
  void merge_in_place(document& sub1,
                                  document const& sub2,
                                  merge_method const& mm = {}) noexcept;

} // namespace subman

#endif // SUBTITLE_H
