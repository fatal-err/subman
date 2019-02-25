#ifndef SUBRIP_H
#define SUBRIP_H

#include "../subtitle.h"
#include <string>

namespace subman {
namespace formats {

class subrip {
public:
  subrip() = delete;
  static std::string to_string(subman::duration const &timestamps);
  static subman::duration to_duration(std::string const &str);
  static subman::subtitle to_subtitle(std::ifstream &stream);
  static void write(subman::subtitle const &sub, std::ostream &out);
};


} // namespace formats
} // namespace subman

#endif // SUBRIP_H
