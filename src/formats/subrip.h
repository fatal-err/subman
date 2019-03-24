#ifndef FORMAT_SUBRIP_H
#define FORMAT_SUBRIP_H

#include "../document.h"
#include <string>

namespace subman {
namespace formats {

class subrip {
public:
  subrip() = delete;
  static std::string to_string(subman::duration const &timestamps) noexcept;
  static subman::duration to_duration(std::string const &str) noexcept(false);
  static subman::document to_subtitle(std::ifstream &stream) noexcept(false);
  static void write(subman::document const &sub,
                    std::ostream &out) noexcept(false);
  static std::string paint_style(styledstring sstr) noexcept;
};

} // namespace formats
} // namespace subman

#endif // FORMAT_SUBRIP_H
