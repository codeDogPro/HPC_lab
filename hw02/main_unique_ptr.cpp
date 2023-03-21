/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>
#include <cassert>

struct Node {
  // 这两个指针会造成什么问题？请修复
  std::unique_ptr<Node> next;
  std::unique_ptr<Node> prev;
  // 如果能改成 unique_ptr 就更好了!

  int value;

  // 这个构造函数有什么可以改进的？
  // Node(int val) {
  //     value = val;
  // }
  Node(int val) : value(val) {}

  void insert(int val) {
    auto node = std::make_unique<Node>(val);
    node->next = std::move(next);
    node->prev = std::move(prev);
    if (prev)
      prev->next = std::move(node);
    if (next)
      next->prev = std::move(node);
  }

  void erase() {
    if (prev)
      prev->next = std::move(next);
    if (next)
      next->prev = std::move(prev);
  }

  ~Node() {
    printf("~Node()\n"); // 应输出多少次？为什么少了？
  }
};

struct List {
  std::unique_ptr<Node> head;

  List() = default;

  List(List const &other) {
    printf("List 被拷贝!!\n");
    // head = other.head; // 这是浅拷贝！
                       // 请实现拷贝构造函数为 **深拷贝**
  }

  List &operator=(List const &) = delete; // 为什么删除拷贝赋值函数也不出错？

  List(List &&) = default;
  List &operator=(List &&) = default;

  Node *front() const {
    return head.get();
  }

  int pop_front() {
    int ret = head->value;
    head = std::move(head->next);
    return ret;
  }

  void push_front(int value) {
    auto node = std::make_unique<Node>(value);
    node->next = std::move(head);
    if (head){
      puts("Head is not empty!");
      head->prev = std::move(node);
    }
    assert(node);
    head = std::move(node);
    assert(head);
  }

  Node *at(size_t index) const {
    auto curr = front();
    for (size_t i = 0; i < index; i++)
    {
      curr = curr->next.get();
    }
    return curr;
  }
};

void print(const List& lst) { // 有什么值得改进的？
  printf("[");
  for (auto curr = lst.front(); curr; curr = curr->next.get())
  {
    printf(" %d", curr->value);
  }
  printf(" ]\n");
}

int main() {
  List a;

  a.push_front(7);
  a.push_front(5);
  print(a); // [ 5 7]
  a.push_front(8);
  a.push_front(2);
  a.push_front(9);
  print(a); // [ 9 2 8 5 7]
  a.push_front(4);
  a.push_front(1);

  print(a); // [ 1 4 9 2 8 5 7 ]

  a.pop_front();

  a.at(2)->erase();

  print(a); // [ 1 4 2 8 5 7 ]

  List b = a;

  a.at(3)->erase();

  print(a); // [ 1 4 2 5 7 ]
  print(b); // [ 1 4 2 8 5 7 ]

  b = {};
  a = {};

  return 0;
}
