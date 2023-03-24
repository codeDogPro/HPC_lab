// 小彭老师作业05：假装是多线程 HTTP 服务器 - 富连网大厂面试官觉得很赞
#include <functional>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <thread>
#include <map>

#include <chrono>
#include <future>
#include <mutex>
#include <condition_variable>
#include <boost/circular_buffer.hpp>

struct User
{
  std::string password;
  std::string school;
  std::string phone;
};

std::map<std::string, User> users;

// 换成 std::chrono::seconds 之类的
std::map<std::string, std::chrono::steady_clock::time_point> has_login;

std::mutex user_mtx;

// 作业要求1：把这些函数变成多线程安全的
// 提示：能正确利用 shared_mutex 加分，用 lock_guard 系列加分
std::string const
do_register(std::string const &username, std::string const &password,
            std::string const &school, std::string const &phone) {
  User user = {password, school, phone};
  std::unique_lock lck(user_mtx);
  if (users.emplace(username, user).second)
    return "注册成功";
  else
    return "用户名已被注册";
}

std::string const
do_login(std::string const &username, std::string const &password) {
  // 作业要求2：把这个登录计时器改成基于 chrono 的
  // long now = time(NULL);   // C 语言当前时间
  auto now = std::chrono::steady_clock::now();
  if (has_login.find(username) != has_login.end()) {
    // auto sec = now - has_login.at(username);  // C 语言算时间差
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - has_login.at(username));
    return std::to_string(dt.count()) + "秒内登录过";
  }
  has_login[username] = now;

  if (users.find(username) == users.end())
    return "用户名错误";
  if (users.at(username).password != password)
    return "密码错误";
  return "登录成功";
}

std::string const
do_queryuser(std::string const &username) {
  auto &user = users.at(username);
  std::stringstream ss;
  ss << "用户名: " << username << std::endl;
  ss << "学校:" << user.school << std::endl;
  ss << "电话: " << user.phone << std::endl;
  return ss.str();
}

class ThreadPool {
public:
  ThreadPool() = default;

  ThreadPool(size_t nthread){

  }

  ~ThreadPool() {
    for (auto &fut : M_pool) {
      fut.get();
    }
  }

  void create(std::function<void()> start) {
    // 作业要求3：如何让这个线程保持在后台执行不要退出？
    // 提示：改成 async 和 future 且用法正确也可以加分
    // std::thread thr(start);
    auto fut = std::async(start);
    M_pool.push_back(std::move(fut));
  }

private:
  std::mutex M_csmer_mtx, M_wker_mtx;
  std::condition_variable M_csmer_cv, M_wker_cv;  
  boost::circular_buffer<std::function<void()>> M_queue;
  std::vector<std::future<void>> M_pool;
};

ThreadPool tpool;

namespace test { // 测试用例？出水用力！
  std::string username[] = {"张心欣", "王鑫磊", "彭于斌", "胡原名"};
  std::string password[] = {"hellojob", "anti-job42", "cihou233", "reCihou_!"};
  std::string school[] = {"九百八十五大鞋", "浙江大鞋", "剑桥大鞋", "麻绳理工鞋院"};
  std::string phone[] = {"110", "119", "120", "12315"};
}

int main() {
  for (int i = 0; i < 262144; i++) {
    tpool.create([&]
                 { std::cout << do_register(test::username[rand() % 4], test::password[rand() % 4], test::school[rand() % 4], test::phone[rand() % 4]) << std::endl; });
    tpool.create([&]
                 { std::cout << do_login(test::username[rand() % 4], test::password[rand() % 4]) << std::endl; });
    tpool.create([&]
                 { std::cout << do_queryuser(test::username[rand() % 4]) << std::endl; });
  }

  // 作业要求4：等待 tpool 中所有线程都结束后再退出
  return 0;
}
