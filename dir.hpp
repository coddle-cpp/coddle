#pragma once
#include <iostream>
#include <stdexcept>
#include <string>
#include <dirent.h>

class Dir
{
public:
  class Error: public std::runtime_error
  {
  public:
    Error(const std::string &what);
  };
  class iterator;
  class Entry
  {
    friend class Dir::iterator;
  public:
    enum Type
    {
      BlockDevice = DT_BLK,
      CharacterDevice = DT_CHR,
      Directory = DT_DIR,
      Fifo = DT_FIFO,
      Link = DT_LNK,
      Regular = DT_REG,
      UnixSocket = DT_SOCK,
      Unknown = DT_UNKNOWN
    };
    Entry(dirent *);
    ino_t ino() const;
    Type type() const;
    std::string name() const;
    operator std::string() const;
  private:
    dirent *dirent_;
  };
  class iterator
  {
  public:
    iterator();
    iterator(const std::string &dirName);
    iterator(const iterator &);
    iterator &operator=(const iterator &);
    ~iterator();
    Entry operator*();
    Entry *operator->();
    bool operator!=(const iterator &);
    bool operator==(const iterator &);
    iterator &operator++();
    iterator operator++(int);
  private:
    const std::string *dirName_;
    DIR *dir_;
    Entry currentEntry_;
    size_t cnt_;
  };
  Dir(const std::string &dir);
  iterator begin() const;
  iterator end() const;
private:
  std::string dir_;
};

std::ostream &operator<<(std::ostream &, const Dir::Entry &);

namespace std
{
template <>
struct iterator_traits<Dir::iterator> {
  typedef ptrdiff_t difference_type;
  typedef Dir::Entry value_type;
  typedef const Dir::Entry &reference;
  typedef const Dir::Entry *pointer;
  typedef forward_iterator_tag iterator_category;
};
}
