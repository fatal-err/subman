#ifndef FORMAT_SUBRIP_H
#define FORMAT_SUBRIP_H

#include "../document.h"
#include <regex>
#include <string>

namespace subman {
namespace formats {

const std::regex durstr{
    "([0-9]+):([0-9]+):([0-9]+),?([0-9]+)\\s*-+>\\s*([0-9]+):"
    "([0-9]+):([0-9]+),?([0-9]+)"};
class subrip {
public:
  subrip() = delete;
  static std::string to_string(subman::duration const &timestamps) noexcept;
  static std::unique_ptr<subman::duration>
  to_duration(std::string const &str) noexcept;
  static subman::document read(std::istream &stream) noexcept(false);
  static void write(subman::document const &sub,
                    std::ostream &out) noexcept(false);
  static std::string paint_style(styledstring sstr) noexcept;
};

} // namespace formats
} // namespace subman

#endif // FORMAT_SUBRIP_H
