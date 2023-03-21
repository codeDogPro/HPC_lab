#include <iostream>
#include <cassert>
#include <vector>
#include <variant>
#include "cpp_type_name.h"

// 请修复这个函数的定义：10 分
template <typename T>
std::ostream &operator<<(std::ostream &os, std::vector<T> const &a)
{
  os << "{";
  for (size_t i = 0; i < a.size(); i++)
  {
    os << a[i];
    if (i != a.size() - 1)
      os << ", ";
  }
  os << "}";
  return os;
}

// 请修复这个函数的定义：10 分
template <typename T1, typename T2>
auto operator+(std::vector<T1> const &a, std::vector<T2> const &b)
{
  // 请实现列表的逐元素加法！10 分
  using T0 = decltype(T1{} + T2{});
  std::vector<T0> res;
  size_t size = std::min(a.size(), b.size());
  for (size_t i = 0; i < size; i++)
    res.push_back(a[i] + b[i]);
  // 例如 {1, 2} + {3, 4} = {4, 6}
  return res;
}

template <class T1, class T2>
std::variant<T1, T2> 
operator+(std::variant<T1, T2> const &a, std::variant<T1, T2> const &b)
{
  // 请实现自动匹配容器中具体类型的加法！10 分
  using T = decltype(T1{} + T2{});
  if(std::is_same_v<T, T2>){
//  开摆了，真不懂这个怎么精确获取类型了
  }
  std::cout << cpp_type_name<T>() << std::endl;
  auto real_a = std::get<T>(a);
  auto real_b = std::get<T>(b);
  size_t size = std::min(real_a.size(), real_b.size());
  T res;
  for(size_t i = 0; i < size; i++)
    res.push_back(real_a[i] + real_b[i]);
  return res;
}

template <class T1, class T2>
std::ostream &
operator<<(std::ostream &os, std::variant<T1, T2> const &a)
{
  // 请实现自动匹配容器中具体类型的打印！10 分
  using T = decltype(T1{} + T2{});
  auto real_a = std::get<T>(a);
  std::cout << real_a;
  return os;
}

int main()
{
  std::vector<int> a = {1, 4, 2, 8, 5, 7};
  std::cout << a << std::endl;
  std::vector<double> b = {3.14, 2.718, 0.618};
  std::cout << b << std::endl;
  auto c = a + b;

  // 应该输出 1
  std::cout << std::is_same_v<decltype(c), std::vector<double>> << std::endl;

  // 应该输出 {4.14, 6.718, 2.618}
  std::cout << c << std::endl;

  std::variant<std::vector<int>, std::vector<double>> d = c;
  std::variant<std::vector<int>, std::vector<double>> e = a;
  d = d + e;
  // d = d + c + e;

  // 应该输出 {9.28, 17.436, 7.236}
  std::cout << d << std::endl;

  return 0;
}
