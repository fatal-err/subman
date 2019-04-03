#ifndef STYLEDSTRING_H
#define STYLEDSTRING_H

#include <list>
#include <memory>
#include <string>
#include <tuple>

namespace subman {

struct range {
  size_t start = 0, finish = 0;

  range(size_t const &start, size_t const &finish) noexcept;

  bool operator<(range const &r) const noexcept;
  bool operator>(range const &r) const noexcept;
  bool operator>=(range const &r) const noexcept;
  bool operator<=(range const &r) const noexcept;
  bool operator==(range const &r) const noexcept;
  bool operator!=(range const &r) const noexcept;

  bool in_between(range const &r) const noexcept;
  bool is_collided(range const &r) const noexcept;

  friend void swap(range &a, range &b) noexcept;
};
void swap(subman::range &a, subman::range &b) noexcept;

struct attr {
  range pos; // pos is not mutable since it will be used in sorting in std::set
  std::string name;
  std::string value;

  // TODO: Do we really need these?
  attr() = default;
  attr(range const &pos, std::string &&name = "",
       std::string &&value = "") noexcept;
  attr(range const &pos, std::string const &name,
       std::string &&value = "") noexcept;
  attr(range const &pos, std::string &&name, std::string const &value) noexcept;
  attr(range const &pos, std::string const &name,
       std::string const &value) noexcept;

  attr(attr const &a) noexcept;
  attr(attr &&a) noexcept;

  attr &operator=(attr a) noexcept;

  bool operator==(attr const &a) const noexcept;
  bool operator!=(attr const &a) const noexcept;
  bool operator<(attr const &a) const noexcept;
  bool operator>(attr const &a) const noexcept;
  bool operator>=(attr const &a) const noexcept;
  bool operator<=(attr const &a) const noexcept;

  friend void swap(attr &a, attr &b) noexcept;
};
void swap(subman::attr &a, subman::attr &b) noexcept;

class styledstring {
  std::string content;
  std::list<attr> attrs;

public:
  styledstring() = default;
  styledstring(styledstring const &) = default;
  styledstring(styledstring &&sstr) = default;
  styledstring(decltype(content) &&content, decltype(attrs) &&attrs = {});
  styledstring(decltype(content) const &content, decltype(attrs) attrs = {});
  styledstring(decltype(content) &&content, decltype(attrs) const &attrs);
  styledstring(decltype(content) const &content, decltype(attrs) const &attrs);
  styledstring &operator=(styledstring const &) = default;
  styledstring &operator=(styledstring &&) = default;

  //  template <class Format>
  //  auto styled() const noexcept(noexcept(Format::paint_style))
  //      -> decltype(Format::paint_style);

  void replace_attr(decltype(attrs)::iterator &old_iter,
                    attr &&new_attr) noexcept(false);
  void replace_attr(decltype(attrs)::iterator &old_iter,
                    attr const &new_attr) noexcept(false);
  void replace_attr(const decltype(attrs)::const_iterator &old_iter,
                    attr &&new_attr) noexcept(false);
  void replace_attr(const decltype(attrs)::const_iterator &old_iter,
                    attr const &new_attr) noexcept(false);
  void replace_attr(attr const &old_attr, attr &&new_attr) noexcept(false);
  void replace_attr(attr const &old_attr, attr const &new_attr) noexcept(false);

  static styledstring &&add(std::string &&, styledstring &&) noexcept;
  styledstring operator+(styledstring &&sstr) const noexcept;
  styledstring operator+(styledstring const &sstr) const noexcept;
  styledstring operator+(std::string const &str) const noexcept;
  styledstring &operator+=(std::string const &str) & noexcept;
  styledstring &operator+=(styledstring const &sstr) & noexcept;
  styledstring &&operator+=(styledstring &&sstr) && noexcept;
  void append_line(styledstring &&line);
  void append_line(styledstring const &line);
  void append_line(std::string &&line);
  void append_line(std::string const &line);
  void trim() noexcept;

  bool operator<(styledstring const &sstr) const noexcept;
  bool operator>(styledstring const &sstr) const noexcept;
  bool operator<=(styledstring const &sstr) const noexcept;
  bool operator>=(styledstring const &sstr) const noexcept;
  bool operator==(styledstring const &sstr) const noexcept;
  bool operator!=(styledstring const &sstr) const noexcept;

  styledstring substr(size_t const &a, size_t const &b) const noexcept;
  void shift_ranges(int64_t const &shift) noexcept;
  void clear() noexcept;

  void put_attribute(attr const &a) noexcept;
  void put_attribute(attr &&a) noexcept;

  void bold(range const &r) noexcept;
  void bold() noexcept;
  void underline(range const &r) noexcept;
  void underline() noexcept;
  void italic(range const &r) noexcept;
  void italic() noexcept;
  void fontsize(range const &r, std::string &&fontsize) noexcept;
  void fontsize(std::string &&fontsize) noexcept;
  void fontsize(range const &r, std::string const &fontsize) noexcept;
  void fontsize(std::string const &fontsize) noexcept;
  void color(range const &r, std::string &&_color) noexcept;
  void color(std::string &&color) noexcept;
  void color(range const &r, std::string const &_color) noexcept;
  void color(std::string const &_color) noexcept;

  auto cget_attrs() const noexcept -> std::list<attr> const & { return attrs; }
  auto get_attrs() noexcept -> std::list<attr> & { return attrs; }
  auto cget_content() const noexcept -> std::string const & { return content; }
  auto get_content() noexcept -> std::string & { return content; }
};

} // namespace subman

subman::styledstring operator+(std::string const &str,
                               subman::styledstring const &sstr) noexcept;
subman::styledstring operator+(std::string &&str,
                               subman::styledstring const &sstr) noexcept;
subman::styledstring &&operator+(std::string const &str,
                                 subman::styledstring &&sstr) noexcept;
subman::styledstring &&operator+(std::string &&str,
                                 subman::styledstring &&sstr) noexcept;

#endif // STYLEDSTRING_H
