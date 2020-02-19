#pragma once
#include <type_traits>

namespace internal
{
  template <typename T>
  struct IsSerializableClass
  {
    template <typename, typename>
    class Checker;

    template <typename C>
    static std::true_type test(Checker<C, decltype(C::IsSerializableClass)> *);

    template <typename C>
    static std::false_type test(...);

    using Type = decltype(test<T>(nullptr));
    static const bool value = std::is_same_v<std::true_type, Type>;
  };

  template <typename T>
  inline constexpr bool IsSerializableClassV = IsSerializableClass<T>::value;
} // namespace internal
