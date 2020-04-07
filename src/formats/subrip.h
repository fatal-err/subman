#ifndef FORMAT_SUBRIP_H
#define FORMAT_SUBRIP_H

#include "../document.h"
#include <regex>
#include <string>

namespace subman::formats {
    std::string paint_style(styledstring sstr) noexcept;

    class subrip {
    public:
      subrip() = delete;
      static subman::document read(std::istream& stream) noexcept(false);
      static void write(subman::document const& sub,
                        std::ostream& out) noexcept(false);
    };

} // namespace subman::formats

#endif // FORMAT_SUBRIP_H
