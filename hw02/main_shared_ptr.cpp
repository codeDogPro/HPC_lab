/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>
#include <cassert>

struct Node {
  // 这两个指针会造成什么问题？请修复
  std::shared_ptr<Node> next;
  std::shared_ptr<Node> prev;
  // 如果能改成 unique_ptr 就更好了!

  int value;

  // 这个构造函数有什么可以改进的？
  // Node(int val) {
  //     value = val;
  // }
  Node(int val) : value(val), next(nullptr), prev(nullptr) {}

  void insert(int val) {
    auto node = std::make_shared<Node>(val);
    node->next = next;
    node->prev = prev;
    if (prev)
      prev->next = node;
    if (next)
      next->prev = node;
  }

  void erase() {
    if (prev)
      prev->next = next;
    if(next)
      next->prev = prev;
  }

  ~Node() {
    printf("~Node()\n"); // 应输出多少次？为什么少了？
  }
};

struct List {
  std::shared_ptr<Node> head;
  std::shared_ptr<Node> tail;

  List() = default;

  List(List const &other) {
    printf("List 被拷贝!!\n");
    auto tmp_tail = other.tail;
    while(tmp_tail){
      push_front(tmp_tail->value);
      tmp_tail = tmp_tail->prev;
    }
  }

  List &operator=(List const &) = delete; // 为什么删除拷贝赋值函数也不出错？

  List(List &&) = default;
  List &operator=(List &&) = default;

  Node *front() const {
    return head.get();
  }

  int pop_front() {
    int ret = head->value;
    head = head->next;
    return ret;
  }

  void push_front(int value) {
    auto node = std::make_shared<Node>(value);
    node->next = head;
    if (head){
      puts("head not nullptr");
      head->prev = node;
    }
    head = node; 
    if(!tail)
      tail = head;
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
  print(a);
  a.push_front(5);
  // printf("%d\n",a.head->value);
  // printf("%d\n",a.tail->prev->value);
  // if(!a.tail) puts("nullptr");
  // printf("%p\n",a.tail);
  print(a); // [ 5 7]
  // a.push_front(2);
  // a.push_front(9);
  // print(a); // [ 9 2 8 5 7]
  // a.push_front(4);
  // a.push_front(1);

  // print(a); // [ 1 4 9 2 8 5 7 ]

  // // a.pop_front();

  // a.at(2)->erase();

  // print(a); // [ 1 4 2 8 5 7 ]

  List b = a;
  print(b);

  a.push_front(6);
  print(a);
  print(b);
  a.at(2)->erase();

  print(a); // [ 1 4 2 5 7 ]
  print(b); // [ 1 4 2 8 5 7 ]

  // b = {};
  // a = {};

  return 0;
}
