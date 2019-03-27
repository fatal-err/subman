#ifndef FORMAT_SUBRIP_H
#define FORMAT_SUBRIP_H

#include "../document.h"
#include <regex>
#include <string>

namespace subman {
namespace formats {

class subrip {
public:
  subrip() = delete;
  static subman::document read(std::istream &stream) noexcept(false);
  static void write(subman::document const &sub,
                    std::ostream &out) noexcept(false);
};

} // namespace formats
} // namespace subman

#endif // FORMAT_SUBRIP_H
