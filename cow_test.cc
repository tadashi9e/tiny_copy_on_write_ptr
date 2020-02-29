// -*- mode:c++ -*-
// Copyright 2020 tadashi9e
#include "cow_ptr.h"
#include <iostream>
#include <map>
#include <sstream>
#include <string>
// ----------------------------------------------------------------------
void test_str() {
  std::cout << "==== std::string ====" << std::endl;
  std::cout << "---- initial (get() allocates managed object)" << std::endl;

  // CowPtr<std::string> p1(std::make_shared<std::string>());
  CowPtr<std::string> p1;
  p1.assign(std::make_shared<std::string>());

  *(p1.put()) = "this is p1";
  std::cout << "p1=" << *(p1.get()) << std::endl;
  std::cout << "---- divide" << std::endl;
  CowPtr<std::string> p2(p1);
  std::cout << "p1=" << *(p1.get()) << std::endl;
  std::cout << "p2=" << *(p2.get()) << std::endl;
  std::cout << "---- put (copy-on-write)" << std::endl;
  *(p2.put()) = "this is p2";
  std::cout << "p1=" << *(p1.get()) << std::endl;
  std::cout << "p2=" << *(p2.get()) << std::endl;
}
// ----------------------------------------------------------------------
struct Test {
  typedef CowPtr<Test> ptr;
  typedef CowPtr<Test> const const_ptr;
  std::string s;
  std::string str() const {
    std::stringstream ss;
    ss << "Test[" << s << "(" << &s << ")]";
    return ss.str();
  }
};

void test() {
  std::cout << "==== simple object ====" << std::endl;
  std::cout << "---- initial (get() allocates managed object)" << std::endl;
  Test::ptr t1(std::make_shared<Test>());
  std::cout << "t1=" << t1.get()->str() << std::endl;
  std::cout << "---- put" << std::endl;
  t1.put()->s = "test1";
  // t1.put()->s = "test1";
  std::cout << "t1=" << t1.get()->str() << std::endl;
  std::cout << "---- divide (shares same managed object)" << std::endl;
  Test::ptr t2(t1);
  std::cout << "t1=" << t1.get()->str() << std::endl;
  std::cout << "t2=" << t1.get()->str() << std::endl;
  std::cout << "---- put (copy-on-write)" << std::endl;
  t2.put()->s = "test2";
  // t2.put()->s = "test1";
  std::cout << "t1=" << t1.get()->str() << std::endl;
  std::cout << "t2=" << t2.get()->str() << std::endl;
}
// ----------------------------------------------------------------------
struct List {
  typedef CowPtr<List> ptr;
  ptr next;
  std::string s;
  std::string str() const {
    std::stringstream ss;
    ss << s << "(" << this << ")";
    if (next) {
      ss << ", ";
      ss << next.get()->str();
    }
    return ss.str();
  }
};

void list_test() {
  std::cout << "==== linked list object ====" << std::endl;
  std::cout << "---- initial (build linked list)" << std::endl;
  List::ptr head(std::make_shared<List>());
  List::ptr* p = &head;
  for (int i = 0; i < 10; ++i) {
    p->put()->s = std::string(1, 'a' + i);
    p->put()->next = std::move(List::ptr(std::make_shared<List>()));
    p = &(p->put()->next);
  }
  std::cout << head.get()->str() << std::endl;
  std::cout << "---- divide (shares same managed object)" << std::endl;
  List::ptr head2(head);
  std::cout << head.get()->str() << std::endl;
  std::cout << head2.get()->str() << std::endl;
  std::cout << "---- put (copy-on-write)" << std::endl;
  p = &head2;
  for (int i = 0; i < 5; ++i) {
    p = &(p->put()->next);
  }
  p->put()->s = "X";
  std::cout << head.get()->str() << std::endl;
  std::cout << head2.get()->str() << std::endl;
}
// ----------------------------------------------------------------------
typedef std::map<int, Test::ptr> map_test_t;

void dump_map(const map_test_t& m) {
  for (auto x : m) {
    std::cout << x.first << ":" << x.second.get()->str() << " ";
  }
  std::cout << std::endl;
}

void map_test() {
  std::cout << "==== map ====" << std::endl;
  std::cout << "---- initial" << std::endl;
  map_test_t m0;
  for (int i = 0; i < 6; ++i) {
    Test::ptr p(std::make_shared<Test>());
    std::stringstream ss;
    ss << "test object " << i;
    p.put()->s = ss.str();
    m0[i] = p;
  }
  dump_map(m0);
  std::cout << "---- divide" << std::endl;
  map_test_t m1(m0);
  dump_map(m0);
  dump_map(m1);
  std::cout << "---- put (copy-on-write)" << std::endl;
  for (int i = 0; i < 6; ++i) {
    if (i % 2 == 0) {
      std::stringstream ss;
      ss << "TEST OBJECT " << i;
      m1[i].put()->s = ss.str();
    }
  }
  dump_map(m0);
  dump_map(m1);
}
// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  test_str();
  test();
  list_test();
  map_test();
}
