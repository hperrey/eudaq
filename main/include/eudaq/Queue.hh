#ifndef EUDAQ_INCLUDED_Queue
#define EUDAQ_INCLUDED_Queue


#include "Exception.hh"

const std::size_t QUEUE_SIZE = 1000;

template <class T> 
class TSQueue { 
 public:
  explicit TSQueue(std::size_t capacity = QUEUE_SIZE) : capacity_(capacity) {}

  void push_front(T item) {
    std::unique_lock<std::mutex> lock(mutex_,  std::chrono::seconds (60));
    if (!lock.owns_lock()) EUDAQ_THROWX(eudaq::MutexLockException, "Cannot lock queue mutex for writing");
    not_full_.wait(lock, std::bind(&TSQueue<T>::is_not_full, this)); 
    container_.push_front(item);
    lock.unlock();
    not_empty_.notify_one();
  }

  void pop_back(T* pItem) {
    std::unique_lock<std::mutex> lock(mutex_);
    not_empty_.wait(lock, std::bind(&TSQueue<T>::is_not_empty, this)); 
    *pItem = container_.back();
    container_.pop_back();
    lock.unlock();
    not_full_.notify_one();
  }

 private:
  // explicitly disable copy and assignment operators
  TSQueue(const TSQueue&) = delete;
  TSQueue& operator=(const TSQueue&) = delete;

  bool is_not_empty() const { return container_.size() > 0; }
  bool is_not_full() const { return container_.size() < capacity_; }

  const std::size_t capacity_; 
  std::deque<T> container_;

  std::mutex mutex_; 
  std::condition_variable not_empty_; 
  std::condition_variable not_full_;
};

#endif
