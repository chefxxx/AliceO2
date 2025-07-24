// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.
///
/// \file BoundedAllocator.h
/// \brief
///

#ifndef TRACKINGITSU_INCLUDE_BOUNDEDALLOCATOR_H_
#define TRACKINGITSU_INCLUDE_BOUNDEDALLOCATOR_H_

#include <limits>
#include <memory_resource>
#include <atomic>
#include <new>
#include <vector>

#include "GPUCommonLogger.h"

namespace o2::its
{

class BoundedMemoryResource final : public std::pmr::memory_resource
{
 public:
  class MemoryLimitExceeded final : public std::bad_alloc
  {
   public:
    MemoryLimitExceeded(size_t attempted, size_t used, size_t max)
      : mAttempted(attempted), mUsed(used), mMax(max) {}
    const char* what() const noexcept final
    {
      static thread_local char msg[256];
      if (mAttempted != 0) {
        snprintf(msg, sizeof(msg),
                 "Reached set memory limit (attempted: %zu, used: %zu, max: %zu)",
                 mAttempted, mUsed, mMax);
      } else {
        snprintf(msg, sizeof(msg),
                 "New set maximum below current used (newMax: %zu, used: %zu)",
                 mMax, mUsed);
      }
      return msg;
    }

   private:
    size_t mAttempted{0}, mUsed{0}, mMax{0};
  };

  BoundedMemoryResource(size_t maxBytes = std::numeric_limits<size_t>::max(), std::pmr::memory_resource* upstream = std::pmr::get_default_resource())
    : mMaxMemory(maxBytes), mUpstream(upstream) {}

  void* do_allocate(size_t bytes, size_t alignment) final
  {
    size_t new_used{0}, current_used{mUsedMemory.load(std::memory_order_relaxed)};
    do {
      new_used = current_used + bytes;
      if (new_used > mMaxMemory) {
        ++mCountThrow;
        throw MemoryLimitExceeded(new_used, current_used, mMaxMemory);
      }
    } while (!mUsedMemory.compare_exchange_weak(current_used, new_used,
                                                std::memory_order_acq_rel,
                                                std::memory_order_relaxed));
    return mUpstream->allocate(bytes, alignment);
  }

  void do_deallocate(void* p, size_t bytes, size_t alignment) final
  {
    mUpstream->deallocate(p, bytes, alignment);
    mUsedMemory.fetch_sub(bytes, std::memory_order_relaxed);
  }

  bool do_is_equal(const std::pmr::memory_resource& other) const noexcept final
  {
    return this == &other;
  }

  size_t getUsedMemory() const noexcept { return mUsedMemory.load(); }
  size_t getMaxMemory() const noexcept { return mMaxMemory; }
  void setMaxMemory(size_t max)
  {
    if (mUsedMemory > max) {
      ++mCountThrow;
      throw MemoryLimitExceeded(0, mUsedMemory, max);
    }
    mMaxMemory = max;
  }

  void print() const
  {
#if !defined(GPUCA_GPUCODE_DEVICE)
    constexpr double GB{1024 * 1024 * 1024};
    auto throw_ = mCountThrow.load(std::memory_order_relaxed);
    auto used = static_cast<double>(mUsedMemory.load(std::memory_order_relaxed));
    LOGP(info, "maxthrow={} maxmem={:.2f} GB used={:.2f} ({:.2f}%)",
         throw_, (double)mMaxMemory / GB, used / GB, 100. * used / (double)mMaxMemory);
#endif
  }

 private:
  size_t mMaxMemory{std::numeric_limits<size_t>::max()};
  std::atomic<size_t> mCountThrow{0};
  std::atomic<size_t> mUsedMemory{0};
  std::pmr::memory_resource* mUpstream;
};

template <typename T>
using bounded_vector = std::pmr::vector<T>;

template <typename T>
void deepVectorClear(std::vector<T>& vec)
{
  std::vector<T>().swap(vec);
}

template <typename T>
inline void deepVectorClear(bounded_vector<T>& vec, BoundedMemoryResource* bmr = nullptr)
{
  vec.~bounded_vector<T>();
  if (bmr == nullptr) {
    auto alloc = vec.get_allocator().resource();
    new (&vec) bounded_vector<T>(alloc);
  } else {
    new (&vec) bounded_vector<T>(bmr);
  }
}

template <typename T>
void deepVectorClear(std::vector<bounded_vector<T>>& vec, BoundedMemoryResource* bmr = nullptr)
{
  for (auto& v : vec) {
    deepVectorClear(v, bmr);
  }
}

template <typename T, size_t S>
void deepVectorClear(std::array<bounded_vector<T>, S>& arr, BoundedMemoryResource* bmr = nullptr)
{
  for (size_t i{0}; i < S; ++i) {
    deepVectorClear(arr[i], bmr);
  }
}

template <typename T>
void clearResizeBoundedVector(bounded_vector<T>& vec, size_t size, BoundedMemoryResource* bmr, T def = T())
{
  vec.~bounded_vector<T>();
  new (&vec) bounded_vector<T>(size, def, bmr);
}

template <typename T>
void clearResizeBoundedVector(std::vector<bounded_vector<T>>& vec, size_t size, BoundedMemoryResource* bmr)
{
  vec.clear();
  vec.reserve(size);
  for (size_t i{0}; i < size; ++i) {
    vec.emplace_back(bmr);
  }
}

template <typename T, size_t S>
void clearResizeBoundedArray(std::array<bounded_vector<T>, S>& arr, size_t size, BoundedMemoryResource* bmr, T def = T())
{
  for (size_t i{0}; i < S; ++i) {
    clearResizeBoundedVector(arr[i], size, bmr, def);
  }
}

template <typename T>
std::vector<T> toSTDVector(const bounded_vector<T>& b)
{
  std::vector<T> t(b.size());
  std::copy(b.cbegin(), b.cend(), t.begin());
  return t;
}

} // namespace o2::its

#endif
