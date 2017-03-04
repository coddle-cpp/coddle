#include "dir.hpp"
#include <cstring>

Dir::Error::Error(const std::string &what):
  std::runtime_error(what)
{}

Dir::Entry::Entry(dirent *e):
  dirent_(e)
{}

ino_t Dir::Entry::ino() const
{
  return dirent_->d_ino;
}

off_t Dir::Entry::off() const
{
  return dirent_->d_off;
}

Dir::Entry::Type Dir::Entry::type() const
{
  return static_cast<Type>(dirent_->d_type);
}

std::string Dir::Entry::name() const
{
  return dirent_->d_name;
}

Dir::Entry::operator std::string() const
{
  return name();
}

Dir::iterator::iterator():
  dirName_(nullptr),
  dir_(nullptr),
  currentEntry_(nullptr),
  cnt_(0)
{}

Dir::iterator::iterator(const std::string &dirName):
  dirName_(&dirName),
  dir_(opendir(dirName_->c_str())),
  currentEntry_(dir_ ? readdir(dir_) : nullptr),
  cnt_(0)
{
  if (!dir_)
    throw Dir::Error(std::string(strerror(errno)) + ": \"" + *dirName_ + "\"");
}

Dir::iterator::iterator(const iterator &iter):
  dirName_(iter.dirName_),
  dir_(dirName_ ? opendir(dirName_->c_str()) : nullptr),
  currentEntry_(dir_ ? readdir(dir_) : nullptr),
  cnt_(0)
{
  for (auto i = 0u; i < iter.cnt_; ++i)
    ++(*this);
}

Dir::iterator &Dir::iterator::operator=(const iterator &iter)
{
  if (dir_)
    closedir(dir_);
  dirName_ = iter.dirName_;
  dir_ = dirName_ ? opendir(dirName_->c_str()) : nullptr;
  currentEntry_ = dir_ ? readdir(dir_) : nullptr;
  cnt_ = 0;
  for (auto i = 0u; i < iter.cnt_; ++i)
    ++(*this);
  return *this;
}

Dir::iterator::~iterator()
{
  if (dir_)
    closedir(dir_);
}

Dir::Entry Dir::iterator::operator*()
{
  return currentEntry_;
}

Dir::Entry *Dir::iterator::operator->()
{
  return &currentEntry_;
}

bool Dir::iterator::operator!=(const iterator &y)
{
  return !(*this == y);
}

bool Dir::iterator::operator==(const iterator &y)
{
  return (currentEntry_.dirent_ == nullptr && y.currentEntry_.dirent_ == nullptr) ||
    (currentEntry_.dirent_ != nullptr && y.currentEntry_.dirent_ != nullptr && cnt_ == y.cnt_);
}

Dir::iterator &Dir::iterator::operator++()
{
  currentEntry_.dirent_ = readdir(dir_);
  ++cnt_;
  return *this;
}

Dir::iterator Dir::iterator::operator++(int)
{
  auto tmp = *this;
  ++*this;
  return tmp;
}

Dir::Dir(const std::string &dir):
  dir_(dir)
{}

Dir::iterator Dir::begin() const
{
  return iterator(dir_);
}

Dir::iterator Dir::end() const
{
  return iterator();
}

std::ostream &operator<<(std::ostream &strm, const Dir::Entry &entry)
{
  strm << entry.name();
  return strm;
}
