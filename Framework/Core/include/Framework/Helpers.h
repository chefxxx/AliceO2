//
// Created by Mateusz Mikiciuk on 18/07/2025.
//

#ifndef HELPERS_H
#define HELPERS_H

#include <iterator>
#include <unordered_map>
#include "BucketPolicy.h"

template <typename TBucketPolicy, typename TIter>
concept is_bucket_policy = requires(TBucketPolicy policy, TIter iter) {
    { policy.getBucket(*iter) } -> std::same_as<int>;
    { policy.getMaximalBucketCount() } -> std::same_as<int>;
    policy.getValues(*iter);
};

template <typename TIter>
struct BucketIdx {
    BucketIdx(const int dataIdx, TIter iter, const int bucketIdx)
        : mBucketIdx(bucketIdx), mDataIdx(dataIdx), mIter(iter)
    {
    }

    int mBucketIdx;
    int mDataIdx;
    TIter mIter;
};

template <std::forward_iterator TIter, typename TBucketPolicy>
    requires is_bucket_policy<TBucketPolicy, TIter>
[[nodiscard]] auto groupData(
    TIter start, TIter end, TBucketPolicy bucketPolicy, const std::size_t minCatSize = 1
)
{
    int dataIdx        = 0;
    const int maxCount = bucketPolicy.getMaximalBucketCount();
    std::unordered_map<int, std::vector<BucketIdx<TIter>>> buckets;
    for (auto it = start; it != end; ++it) {
        const auto bucketNumber = bucketPolicy.getBucket(*it);
        buckets[bucketNumber].emplace_back(dataIdx++, it, bucketNumber);
    }

    std::vector<BucketIdx<TIter>> resultData;
    for (int i = 0; i < maxCount; ++i) {
        if (buckets.contains(i) && buckets[i].size() >= minCatSize) {
            resultData.insert(resultData.end(), buckets.at(i).begin(), buckets.at(i).end());
        }
    }
    return resultData;
}

#endif  // HELPERS_H
