#include <forward_list>
#include <thread>
#include <future>

#include <fmt/core.h>
#include <gtest/gtest.h>

#include "../forward_list.hpp"

#include <iostream>

class ListTest: public testing::Test {
  protected:
    void SetUp() override {
      list.PushFront(1);
      list.PushFront(2);
      list.PushFront(3);
      list.PushFront(4);
      list.PushFront(5);
      list.PushFront(6);
      list.PushFront(7);
      assert(list.Size() == sz);
    }
  ForwardList<int> list;
  const size_t sz = 7;
};


// OK
TEST(EmptyListTest, DefaultConstructor) {
  ForwardList<int> list;
  ASSERT_TRUE(list.IsEmpty()) << "Default list isn't empty!";
}

// OK
TEST(EmptyListTest, PushFrontSimple) {
  ForwardList<int> list;
  list.PushFront(2);
  ASSERT_EQ(list.Size(), 1);
  ASSERT_EQ(list.Front(), 2);
}

// OK
TEST(EmptyListTest, PopFrontSimple) {
  ForwardList<int> list;
  list.PushFront(1);
  list.PushFront(2);
  list.PopFront();

  ASSERT_EQ(list.Size(), 1);
  ASSERT_EQ(list.Front(), 1);
}

// OK
TEST(EmptyListTest, PopFrontEmptyList) {
  ForwardList<int> list;
  EXPECT_THROW({
    list.PopFront();
  }, std::runtime_error);
}

// OK
TEST(EmptyListTest, ConstructorSizeDefaultValues) {
  ForwardList<std::string> list(5);
  ASSERT_EQ(list.Size(), 5);
  while (list.Size()) {
    ASSERT_TRUE(list.Front().empty());
    list.PopFront();
  }
}

// OK
TEST(EmptyListTest, ConstructorWithInitList) {
  ForwardList<int> list{1, 2, 3, 4, 5, 6, 7, 8};
  ASSERT_EQ(list.Size(), 8);
  int iter = 1;
  while (!list.IsEmpty()) {
    ASSERT_EQ(list.Front(), iter++);
    list.PopFront();
  }
}

// NOT OK
TEST(EmptyListTest, Swap) {
  ForwardList<int> list;  // {5}
  list.PushFront(5);

  ForwardList<int> lst;  // {14, 15}
  lst.PushFront(15);
  lst.PushFront(14);

  size_t old_mp_size = list.Size();
  size_t old_dict_size = lst.Size();

  std::swap(list, lst);

  ASSERT_EQ(lst.Size(), old_mp_size);
  ASSERT_EQ(list.Size(), old_dict_size);

  ASSERT_EQ(lst.Front(), 5);

  // ASSERT_EQ(list.Front(), 15);  // После свопа список list равен {14, 15} 
  ASSERT_EQ(list.Front(), 14);

  list.PopFront();

  // ASSERT_EQ(list.Front(), 14);  // После PopFront список list равен {15}
  ASSERT_EQ(list.Front(), 15);
}

// NOT OK
TEST_F(ListTest, CopyConstructor) {
  ForwardList<int> lst = list;  // lst = {1, 2, 3, 4, 5, 6, 7}
  ASSERT_NE(&list, &lst) << "Copy constructor must do copy!\n";
  ASSERT_EQ(list.Size(), lst.Size());  // 7 == 7

  /*

  while (!lst.IsEmpty()) {  // На последней итерации list = {7} и lst = {7}
    ASSERT_EQ(list.Front(), lst.Front());  // 7 == 7
    list.PopFront();  // list = {} и lst = {7}
    ASSERT_NE(list.Front(), lst.Front());  // list.Front() --> UB, так как list пустой
    lst.PopFront();
  }

  */

  // Можно сделать такой тест:
  while (!lst.IsEmpty()) {
    ASSERT_EQ(list.Front(), lst.Front());  // Значения соответствующих элементов должны быть равны
    ASSERT_NE(&list.Front(), &lst.Front());  // Но указатели на соответствующие элементы должны быть разными
    list.PopFront();
    lst.PopFront();
  }
  ASSERT_TRUE(list.IsEmpty());  // В конце оба списка
  ASSERT_TRUE(lst.IsEmpty());   // должны быть пустыми
}

// OK
TEST_F(ListTest, CopyAssigment) {
  ForwardList<int> lst;  // lst = {4}
  lst.PushFront(4);
  list = lst;  // list = {4}
  ASSERT_NE(&list, &lst) << "Copy assigment must do copy!\n";
  ASSERT_EQ(list.Size(), lst.Size());
  while (!lst.IsEmpty()) {
    ASSERT_EQ(list.Front(), lst.Front());
    list.PopFront();
    lst.PopFront();
  }
  ASSERT_TRUE(list.IsEmpty());  // В конце оба списка
  ASSERT_TRUE(lst.IsEmpty());   // должны быть пустыми
}

// NOT OK  -----  НИКОГДА НЕ ПРОХОДИТ
TEST_F(ListTest, SelfAssignment) {
  /*

  std::thread thread([&](){
    list = list;
  });
  auto future = std::async(std::launch::async, &std::thread::join, &thread);
  ASSERT_EQ(
    future.wait_for(std::chrono::seconds(1)),
    std::future_status::timeout
  ) << "There is infinity loop!\n";

  */

  // Задача лежит в потоке дольше секунды, но не из-за рекурсии
  ForwardList<int> copy = list;
  list = list;
  while (!list.IsEmpty()) {
    ASSERT_EQ(list.Front(), copy.Front());
    list.PopFront();
    copy.PopFront();
  }
}

// NOT OK
TEST_F(ListTest, RangeWithIteratorPreFix) {
  ASSERT_EQ(std::distance(list.Begin(), list.End()), sz) << 
                "Distanse between begin and end iterators ins't equal size";
  /*

  int iter = 1;
  for (auto it = list.Begin(); it != list.End(); ++it) {
    ASSERT_EQ(*it, iter++);
  }

  */

  // list = {7, 6, 5, 4, 3, 2, 1}, см. в начало файла
  int number = 7;
  for (auto it = list.Begin(); it != list.End(); ++it) {
    ASSERT_EQ(*it, number--);
  }
}

// NOT OK
TEST_F(ListTest, RangeWithIteratorPostFix) {
  ASSERT_EQ(std::distance(list.Begin(), list.End()), sz) << 
                "Distanse between begin and end iterators ins't equal size";
  /*

  int iter = 1;
  for (auto it = list.Begin(); it != list.End(); it++) {
    ASSERT_EQ(*it, iter++);
  }

  */
  int number = 7;
  for (auto it = list.Begin(); it != list.End(); it++) {
    ASSERT_EQ(*it, number--);
  }
}

// NOT OK
TEST_F(ListTest, EraseBegin) {
  // /*

  // int second_value = *(list.Begin()++);  // list = {7, 6, 5, 4, 3, 2, 1}, *list.Begin()  -->  7, *list.Begin()++  -->  7

  // */
  // int second_value = *(++list.Begin());  // second_value = 6

  // list.EraseAfter(list.Begin());  // list = {7, 5, 4, 3, 2, 1}
  // ASSERT_EQ(list.Size(), sz - 1);  // 6 = 6
  // /*
  
  // ASSERT_NE(*(list.Begin()++), second_value);  // *(list.Begin()++)  --> 7, second_value = 6
  
  // */
  // ASSERT_NE(*(++list.Begin()), second_value);


  // Я бы тест переделал. Хочется проверить, что другие итераторы не ломаются, а также элементы не меняются порядка
  auto it2 = ++list.Begin();
  auto it3 = it2;
  ++it3;
  int second_value = *it2;  // second_value = 6
  int third_value = *it3;  // third_value = 5

  list.EraseAfter(list.Begin());  // it2 теперь сломан, а вот it3 - нет
  ASSERT_EQ(list.Size(), sz - 1);
  
  ASSERT_NE(*(++list.Begin()), second_value);
  ASSERT_EQ(*(++list.Begin()), third_value);
  ASSERT_EQ(*it3, third_value);
}

// NOT OK
TEST_F(ListTest, EraseMedium) {
  auto it = list.Begin();  // *it = 7
  std::advance(it, list.Size() / 2);  // *it = 4
  list.EraseAfter(it);  // list = {7, 6, 5, 4, 2, 1}
  ASSERT_EQ(list.Size(), sz - 1);
  for (auto it = list.Begin(); it != list.End(); ++it) {
    // ASSERT_NE(*it, 4);  // Был удалён элемент со значением 3
    ASSERT_NE(*it, 3);
  }
}

// OK
TEST_F(ListTest, InsertMedium) {
  auto it = list.Begin();  // *it = 7
  std::advance(it, list.Size() / 2);  // *it = 4
  list.InsertAfter(it, 4);  // list = {7, 6, 5, 4, +4, 3, 2, 1}
  ASSERT_EQ(list.Size(), sz + 1);
  it = list.Begin();  // *it = 7
  std::advance(it, list.Size() / 2); // *it = 4 (новая четвёрка)
  ASSERT_EQ(*it, 4);
}

// OK
TEST_F(ListTest, Clear) {
  list.Clear();
  ASSERT_TRUE(list.IsEmpty());
  ASSERT_EQ(list.Size(), 0);
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}