// -*- mode:c++ -*-
// Copyright 2020 tadashi9e
#ifndef COW_PTR_H
#define COW_PTR_H

#include <assert.h>
#include <memory>
#include <mutex>
#include <utility>

/**
 * Tiny copy-on-write pointer.
 *
 * Example:
 *
 * // assume managed object class as:
 * class X {
 *   std::string s;
 * };
 *
 * // Create 'a' with managed object.
 * CowPtr<X> a(std::make_shared<X>());
 *
 * // If you won't confused, you can write it as 'a->s = "this is a"'.
 * a.put()->s = "this is a";
 *
 * // Create 'b' which shares managed object of 'a'.
 * CowPtr<X> b(a);
 *
 * // Now, a->s and b->s are identical.
 *
 * // copy on write.
 * b.put()->s = "b";
 * // <---  a.get()->s == "a", b.get()->s == "b"
 */
template<typename T>
class CowPtr {
 public:
  /**
   * Constructs a CowPtr<T> with no managed object.
   */
  CowPtr() : mode_(COW_SHARED) {
  }
  /**
   * Constructs a CowPtr<T> which shares ownership of the object
   * managed by orig.
   */
  CowPtr(const CowPtr<T>& orig)
    : mode_(COW_SHARED) {
    std::lock_guard<std::mutex> lock(orig.mutex_);
    ptr_ = orig.ptr_;
    orig.mode_ = COW_SHARED;
  }
  /**
   * Move-constructs a CowPtr<T> from orig.
   */
  CowPtr(CowPtr&& orig) {
    std::lock_guard<std::mutex> lock(orig.mutex_);
    mode_ = orig.mode_;
    ptr_ = std::move(orig.ptr_);
  }
  /**
   * Constructs a CowPtr<T> which owns ptr as managed object.
   */
  explicit CowPtr(const std::shared_ptr<T>& ptr)
    : mode_(COW_PRIVATE), ptr_(ptr) {
  }
  /**
   * Move-constructs a CowPtr<T> which owns ptr as managed object.
   */
  explicit CowPtr(std::shared_ptr<T>&& ptr)
    : mode_(COW_PRIVATE), ptr_(std::move(ptr)) {
  }
  /**
   * Shares ownership of the object managed by orig.
   */
  CowPtr<T>& operator=(const CowPtr<T>& orig) {
    assert(&orig != this);
    do {
      if (!orig.mutex_.try_lock()) {
        continue;
      }
      if (!mutex_.try_lock()) {
        orig.mutex_.unlock();
        continue;
      }
    } while (0);
    mode_ = COW_SHARED;
    ptr_ = orig.ptr_;
    orig.mode_ = COW_SHARED;
    mutex_.unlock();
    orig.mutex_.unlock();
    return *this;
  }
  /**
   * Move-assigns a CowPtr<T> from orig.
   */
  CowPtr<T>& operator=(CowPtr<T>&& orig) {
    assert(&orig != this);
    do {
      if (!orig.mutex_.try_lock()) {
        continue;
      }
      if (!mutex_.try_lock()) {
        orig.mutex_.unlock();
        continue;
      }
    } while (0);
    mode_ = orig.mode_;
    ptr_ = std::move(orig.ptr_);
    mutex_.unlock();
    orig.mutex_.unlock();
    return *this;
  }
  /**
   * Assign ptr to managed object
   */
  void assign(const std::shared_ptr<T>& ptr) {
    std::lock_guard<std::mutex> lock(mutex_);
    mode_ = COW_PRIVATE;
    ptr_ = ptr;
  }
  /**
   * Move-assign ptr to managed object
   */
  void assign(std::shared_ptr<T>&& ptr) {
    std::lock_guard<std::mutex> lock(mutex_);
    mode_ = COW_PRIVATE;
    ptr_ = std::move(ptr);
  }
  /**
   * Checks if *this stores a non-null pointer as std::shared_ptr.
   */
  explicit operator bool() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return ptr_ ? true : false;
  }
  /**
   * Get the managed object pointer to read access.
   */
  std::shared_ptr<const T> get() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return ptr_;
  }
  /**
   * Get the managed object pointer to write access.
   */
  std::shared_ptr<T> put() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (mode_ == COW_SHARED) {
      ptr_ = std::make_shared<T>(*ptr_);
      mode_ = COW_PRIVATE;
    }
    return ptr_;
  }
#ifdef WITH_COW_OPERATOR_ACCESS
  /**
   * Dereferences pointer to the managed object.
   */
  const T& operator*() const {
    return *get();
  }
  /**
   * Dereferences pointer to the managed object member.
   */
  std::shared_ptr<const T> operator->() const {
    return get();
  }
  /**
   * Dereferences pointer to the managed object.
   */
  T& operator*() {
    return *put();
  }
  /**
   * Dereferences pointer to the managed object member.
   */
  std::shared_ptr<T> operator->() {
    return put();
  }
#endif  // WITH_COW_OPERATOR_ACCESS

 private:
  mutable std::mutex mutex_;
  /**
   * Sharing mode of managed object.
   */
  mutable enum {
    COW_SHARED,
    COW_PRIVATE
  } mode_;
  /**
   * Managed object.
   * Managed object may be shared with other CowPtr<T>.
   */
  mutable std::shared_ptr<T> ptr_;
};

#endif  // COW_PTR_H
