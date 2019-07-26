#ifndef SEARCH_H
#define SEARCH_H

#include <string>

namespace subman {

  class search {
  private:
    std::string keyword;

  public:
    search() = default;

    void set_keyword(std::string&& words) noexcept {
      keyword = std::move(words);
    }

    void set_keyword(std::string const& words) noexcept {
      keyword = words;
    }

    void find();
  };

} // namespace subman

#endif // SEARCH_H
