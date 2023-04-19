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
#include <cassert>

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
  puts("In register");
  User user = {password, school, phone};
  std::unique_lock lck(user_mtx);
  if (users.emplace(username, user).second)
    return "注册成功";
  else
    return "用户名已被注册";
}

std::string const
do_login(std::string const &username, std::string const &password) {
  puts("In login");
  // 作业要求2：把这个登录计时器改成基于 chrono 的
  // long now = time(NULL);   // C 语言当前时间
  auto now = std::chrono::steady_clock::now();
  if (has_login.find(username) != has_login.end()) {
    // auto sec = now - has_login.at(username);  // C 语言算时间差
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - has_login.at(username));
    return std::to_string((double)dt.count() * 0.001) + "秒内登录过";
  }
  has_login[username] = now;

  if (users.find(username) == users.end())
    return "用户名错误";
  if (users.at(username).password != password)
    return "密码错误";
  return "登录成功";
}

#include <cmath>
std::string const
do_queryuser(std::string const &username) {
  /*
  user.at(username) cause a bug!
  */
  puts("In query");
  std::cout << username << std::endl;
  float res = 0;
  // use to delay
  for(int i = 0; i < 1000; i++){
    res += std::sin(i);
  }
  // auto &user = users.at(username);
  // puts("Got the user");
  std::stringstream ss;
  ss << "用户名: " << username << std::endl;
  ss << "电话: " << std::to_string(res) << std::endl;
  // ss << "学校:" << user.school << std::endl;
  // ss << "电话: " << user.phone << std::endl;
  return ss.str();
}

class ThreadPool {
public:
  ThreadPool() = default;

  ThreadPool(size_t queue_size) : M_queue{queue_size}, M_running{false} {}

  void
  start(size_t nthread){
    if(M_running) return;

    M_running = true;
    M_pool.reserve(nthread);
    for(size_t i = 0; i < nthread; i++){
      auto fut = std::async(std::launch::async, [this]
                            { worker(); });
      M_pool.push_back(std::move(fut));
    }
  }

  ~ThreadPool() {
    {
      std::unique_lock lck{M_mtx};
      M_running = false;
      M_nfull.notify_all();
      M_nempty.notify_all();
    }
    
    for (auto &fut : M_pool) {
      fut.get();
    }
  }

  void 
  create(std::function<void()> &&task) {
    // 作业要求3：如何让这个线程保持在后台执行不要退出？
    // 提示：改成 async 和 future 且用法正确也可以加分
    std::unique_lock lck{M_mtx};
    // puts("In producer, Got lock");
    // std::cout << " Full? " << M_queue.full() << std::endl;
    M_nfull.wait(lck, [this]
                 { return !M_running || !M_queue.full(); });
    assert(!M_queue.full());

    // puts("Ready to push_back");
    M_queue.push_back(std::move(task));
    M_nempty.notify_one();
  }

private:
  using task = std::function<void()>;
  void 
  worker(){
    while(true){
      puts("In worker loop");
      task t;
      {
        // puts("Try lock");
        std::unique_lock lck{M_mtx};
        // puts("Got lock");
        std::cout << " Empty? " << M_queue.empty() << std::endl;
        M_nempty.wait(lck, [this]
                      { return !M_running || !M_queue.empty(); });
        assert(!M_queue.empty());

        t = std::move(M_queue.front());
        M_queue.pop_front();
        M_nfull.notify_one();
      }
      puts("Ready to run task");
      t(); 
      puts("Finish task");
    }
  }
  
  std::mutex M_mtx;
  std::condition_variable M_nfull, M_nempty;  
  boost::circular_buffer<task> M_queue;
  std::vector<std::future<void>> M_pool;
  bool M_running;
};


namespace test { // 测试用例？出水用力！
  std::string username[] = {"张心欣", "王鑫磊", "彭于斌", "胡原名"};
  // std::string username[] = {"zxx", "wxl", "pyb", "hym"};
  std::string password[] = {"hellojob", "anti-job42", "cihou233", "reCihou_!"};
  std::string school[] = {"九百八十五大鞋", "浙江大鞋", "剑桥大鞋", "麻绳理工鞋院"};
  std::string phone[] = {"110", "119", "120", "12315"};
}

int main() {
  ThreadPool tpool(20);
  tpool.start(4);

  for (int i = 0; i < 262144; i++) {
    tpool.create([&]
                 { std::cout << do_register(test::username[rand() % 4], test::password[rand() % 4], test::school[rand() % 4], test::phone[rand() % 4]) << std::endl; });
    tpool.create([&]
                 { std::cout << do_login(test::username[rand() % 4], test::password[rand() % 4]) << std::endl; });
    tpool.create([&]
                 { std::cout << do_queryuser(test::username[rand() % 4]) << std::endl; });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // 作业要求4：等待 tpool 中所有线程都结束后再退出
  return 0;
}
