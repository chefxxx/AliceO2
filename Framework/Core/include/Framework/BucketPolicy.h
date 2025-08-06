//
// Created by mshamrai on 7/17/25.
//

#ifndef BUCKETPOLICY_H
#define BUCKETPOLICY_H

#include <gtest/gtest_prod.h>
#include <algorithm>
#include <array>
#include <tuple>
#include <type_traits>
#include <vector>

template <typename T1, typename T2>
concept is_less_comparable = requires(T1 t, T2 u) {
    { t < u } -> std::same_as<bool>;
};

template <typename TElement, typename... TCallables>
concept is_function_callable_on_element = (std::invocable<TCallables, TElement> && ...);

template <typename TElement, typename... TCallables>
concept is_function_bucketable_on_doubles =
    is_function_callable_on_element<TElement, TCallables...> &&
    (std::is_convertible_v<std::invoke_result_t<TCallables, TElement>, double> && ...);

template <typename T1, typename T2>
    requires is_less_comparable<T1, T2>
int findIndex(std::vector<T1> const &data, T2 const &value, const bool ignoreOverflows)
{
    const auto tmp =
        static_cast<int>(distance(data.begin(), std::upper_bound(data.begin(), data.end(), value)));
    return ignoreOverflows && (tmp == 0 || tmp == static_cast<int>(data.size())) ? -1 : tmp;
}

template <typename... TCallables>
struct BucketPolicy final {
    BucketPolicy(
        std::tuple<TCallables...> const &callables,
        std::array<std::vector<double>, sizeof...(TCallables)> const &bucketsRanges,
        const bool ignoreOverflows = true
    )
        : mCallables(callables), mBucketsRanges(bucketsRanges), ignoreOverflows(ignoreOverflows)
    {
    }

    template <typename TElement>
        requires is_function_bucketable_on_doubles<TElement, TCallables...>
    [[nodiscard]] int getBucket(TElement const &element)
    {
        auto values  = getValues(element);
        auto indices = getUpperIndicesForTuple(values);
        auto bucket  = calculateBucketAtIndices(indices);
        return bucket;
    }

    [[nodiscard]] int getMaximalBucketCount() const
    {
        return [&]<std::size_t... I>(std::index_sequence<I...>) {
            return (1 * ... * (std::get<I>(mBucketsRanges).size() + 1));
        }(std::make_index_sequence<sizeof...(TCallables)>{});
    }

    template <typename TElement>
    [[nodiscard]] auto getValues(TElement const &element)
    {
        return std::make_tuple(std::get<TCallables>(mCallables)(element)...);
    }

    template <typename... Types>
    [[nodiscard]] auto getUpperIndicesForTuple(std::tuple<Types...> const &values)
    {
        return [&]<std::size_t... I>(std::index_sequence<I...>) {
            return std::make_tuple(
                findIndex(std::get<I>(mBucketsRanges), std::get<I>(values), ignoreOverflows)...
            );
        }(std::make_index_sequence<sizeof...(TCallables)>{});
    }

    template <typename... TIndices>
    [[nodiscard]] int calculateBucketAtIndices(std::tuple<TIndices...> const &indices)
    {
        constexpr auto N = sizeof...(TIndices);
        auto indexSeq    = std::make_index_sequence<N - 1>();
        return ignoreOverflows && checkUnderOverflows(indices)
                   ? -1
                   : [&]<size_t... I>(std::index_sequence<I...>) {
                         return (
                             std::get<0>(indices) + ... +
                             (std::get<I + 1>(indices) *
                              (1 * ... * (std::get<I>(mBucketsRanges).size() + 1)))
                         );
                     }(indexSeq);
    }

    private:
    /*                  --- TEST SECTION ---                    */
    FRIEND_TEST(BinarySearchTest, oneElementVector_indexOne);
    FRIEND_TEST(GetUpperIndicesForTupleTest, fiveRangesInEachDimensionWithOverflows);
    FRIEND_TEST(GetUpperIndicesForTupleTest, fiveRangesInEachDimensionWithoutOverflows);
    FRIEND_TEST(GetUpperIndicesForTupleTest, sevenAndFiveRangesWithOverflows);
    FRIEND_TEST(GetUpperIndicesForTupleTest, manyDimensionsWithOverflows);
    FRIEND_TEST(CalculateBucketAtIndicesTest, fiveRangesInTwoDimensionsWithout);
    FRIEND_TEST(GetBucketTest, manyDimensionsWithOverflow);
    /*                  ---              ---                    */

    template <typename... Types>
    bool checkUnderOverflows(std::tuple<Types...> arg) const
    {
        return [&arg]<size_t... I>(std::index_sequence<I...>) {
            return ((std::get<I>(arg) == -1) || ...);
        }(std::make_index_sequence<sizeof...(Types)>{});
    }

    std::tuple<TCallables...> mCallables;
    std::array<std::vector<double>, sizeof...(TCallables)> mBucketsRanges;
    bool ignoreOverflows;
};

#endif  // BUCKETPOLICY_H
