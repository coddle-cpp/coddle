#pragma once

#define SER_PROPERTY(property) arch(#property, property)

#define SER_DEFINE_PROPERTIES() \
  enum { IsSerializableClass }; \
  template <typename Arch>      \
  void ser(Arch &arch) const    \
  {                             \
    SER_PROPERTY_LIST           \
  }                             \
  template <typename Arch>      \
  void deser(Arch &arch)        \
  {                             \
    SER_PROPERTY_LIST           \
  }
