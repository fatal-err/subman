#ifndef STYLEDSTRING_H
#define STYLEDSTRING_H

#include <list>
#include <string>
#include <tuple>

namespace subman {

struct range {
  size_t start = 0, finish = 0;

  // TODO: Do we really need these?
  range(size_t &&start, size_t &&finish) noexcept;
  range(size_t const &start, size_t const &finish) noexcept;
  range(size_t const &start, size_t &&finish) noexcept;
  range(size_t &&start, size_t const &finish) noexcept;
  range &operator=(range r) noexcept;

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
  attr(range &&pos, std::string &&name, std::string &&value) noexcept;
  attr(range const &pos, std::string &&name, std::string &&value) noexcept;
  attr(range const &pos, std::string const &name, std::string &&value) noexcept;
  attr(range const &pos, std::string &&name, std::string const &value) noexcept;
  attr(range &&pos, std::string const &name, std::string const &value) noexcept;
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
  std::list<attr> attrs;
  std::string content;

public:
  styledstring() = default;
  styledstring(styledstring const &) = default;
  styledstring(styledstring &&sstr) = default;
  styledstring &operator=(styledstring const &) = default;
  styledstring &operator=(styledstring &&) = default;

  template <class Format>
  auto styled() const noexcept(noexcept(Format::paint_style))
      -> decltype(Format::paint_style);

  void replace_attr(decltype(attrs)::iterator &old_iter,
                    attr &&new_attr) noexcept;
  void replace_attr(decltype(attrs)::iterator &old_iter,
                    attr const &new_attr) noexcept;
  void replace_attr(const decltype(attrs)::const_iterator &old_iter,
                    attr &&new_attr) noexcept;
  void replace_attr(const decltype(attrs)::const_iterator &old_iter,
                    attr const &new_attr) noexcept;
  void replace_attr(attr const &old_attr, attr &&new_attr) noexcept;
  void replace_attr(attr const &old_attr, attr const &new_attr) noexcept;

  static styledstring &&add(std::string &&, styledstring &&) noexcept;
  styledstring operator+(styledstring &&sstr) const noexcept;
  styledstring operator+(styledstring const &sstr) const noexcept;
  styledstring operator+(std::string &&str) const noexcept;
  styledstring operator+(std::string const &str) const noexcept;
  styledstring &operator+=(std::string const &str) & noexcept;
  styledstring &&operator+=(std::string &&str) && noexcept;
  styledstring &operator+=(styledstring const &sstr) & noexcept;
  styledstring &&operator+=(styledstring &&sstr) && noexcept;

  styledstring substr(size_t const &a, size_t const &b) const noexcept;
  void shift_ranges(int64_t const &shift) noexcept;
  void clear() noexcept;

  void put_attribute(attr const &a) noexcept(false);
  void put_attribute(attr &&a) noexcept(false);

  void bold(range &&r) noexcept(false);
  void bold(range const &r) noexcept(false);

  void underline(range &&r) noexcept(false);
  void underline(range const &r) noexcept(false);

  void italic(range &&r) noexcept(false);
  void italic(range const &r) noexcept(false);

  void fontsize(range &&r, std::string &&_fontsize) noexcept(false);
  void fontsize(range const &r, std::string &&_fontsize) noexcept(false);
  void fontsize(range &&r, std::string const &_fontsize) noexcept(false);
  void fontsize(range const &r, std::string const &_fontsize) noexcept(false);

  void color(range &&r, std::string &&_color) noexcept(false);
  void color(range const &r, std::string &&_color) noexcept(false);
  void color(range &&r, std::string const &_color) noexcept(false);
  void color(range const &r, std::string const &_color) noexcept(false);

  auto cget_attrs() const noexcept -> std::list<attr> const & { return attrs; }
  auto get_attrs() noexcept -> std::list<attr> & { return attrs; }
  auto cget_content() const noexcept -> std::string const & { return content; }
  auto get_content() noexcept -> std::string & { return content; }
};

} // namespace subman

subman::styledstring operator+(std::string const &str,
                       subman::styledstring const &sstr) noexcept;
subman::styledstring operator+(std::string &&str, subman::styledstring const &sstr) noexcept;
subman::styledstring &&operator+(std::string const &str, subman::styledstring &&sstr) noexcept;
subman::styledstring &&operator+(std::string &&str, subman::styledstring &&sstr) noexcept;

#endif // STYLEDSTRING_H
