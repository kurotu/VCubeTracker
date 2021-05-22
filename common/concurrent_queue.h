#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

// https://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
template <typename Data>
class concurrent_queue {
 private:
  std::queue<Data> the_queue;
  mutable std::mutex the_mutex;
  std::condition_variable the_condition_variable;

 public:
  void push(Data const& data) {
    std::unique_lock lock(the_mutex);
    the_queue.push(data);
    lock.unlock();
    the_condition_variable.notify_one();
  }

  void pushWithMove(Data data) {
    std::unique_lock lock(the_mutex);
    the_queue.push(std::move(data));
    lock.unlock();
    the_condition_variable.notify_one();
  }

  bool empty() const {
    std::unique_lock lock(the_mutex);
    return the_queue.empty();
  }

  bool try_pop(Data& popped_value) {
    std::unique_lock lock(the_mutex);
    if (the_queue.empty()) {
      return false;
    }

    popped_value = the_queue.front();
    the_queue.pop();
    return true;
  }

  void wait_and_pop(Data& popped_value) {
    std::unique_lock lock(the_mutex);
    while (the_queue.empty()) {
      the_condition_variable.wait(lock);
    }

    popped_value = the_queue.front();
    the_queue.pop();
  }

  Data wait_and_pop() {
    std::unique_lock lock(the_mutex);
    while (the_queue.empty()) {
      the_condition_variable.wait(lock);
    }
    auto popped_value = std::move(the_queue.front());
    the_queue.pop();
    return popped_value;
  }

  size_t size() const {
    std::unique_lock lock(the_mutex);
    return the_queue.size();
  }
};
