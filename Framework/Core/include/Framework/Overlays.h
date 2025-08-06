//
// Created by Mateusz Mikiciuk on 24/07/2025.
//

// clang-format off
//#ifdef O2_OVERLAY

#ifndef O2OVERLAYS_H
#define O2OVERLAYS_H

#include <cstdint>
#include <iterator>
#include <memory>
#include <tuple>
#include "BucketPolicy.h"
#include "Framework/ASoA.h"

namespace o2::framework
{

template <typename Type>
struct myCallable {
    template <typename TIter>
    auto operator()(TIter const& it)
    {
        return soa::row_helpers::getColumnValue<typename Type::type, TIter, Type>(it);
    }
};

template <typename... Types>
struct ColumnBinningPolicy2 {
    using BucketType = BucketPolicy<myCallable<Types>...>;

    BucketType myBucket;

    ColumnBinningPolicy2(
        std::array<std::vector<double>, sizeof...(Types)> bins, bool ignoreOverflows
    )
        : myBucket(std::make_tuple(myCallable<Types>()...), bins, ignoreOverflows)
    {
    }

    template <typename TIter>
    auto getBinningValues(TIter& rowIterator, uint64_t globalIndex = -1) const
    {
        if (globalIndex != -1) {
            rowIterator.setCursor(globalIndex);
        }
        return myBucket.getValues(*rowIterator);
    }

    template <typename Table>
    auto getBinningValues(
        typename Table::iterator rowIterator, Table& table, uint64_t globalIndex = -1
    ) const
    {
        return getBinningValues(rowIterator, globalIndex);
    }

    int getBin(std::tuple<typename Types::type...> const& data) const
    {
        auto indices = myBucket.getUpperIndicesForTuple(data);
        auto bucket  = myBucket.calculateBucketAtIndices(indices);
        return bucket;
    }
};

template <typename, typename...>
struct FlexibleBinningPolicy2;

template <typename... Types, typename... TCallables>
struct FlexibleBinningPolicy2<std::tuple<TCallables...>, Types...> {
    using BucketType = BucketPolicy<myCallable<Types>...>;

    BucketType myBucket;

    FlexibleBinningPolicy2(
        std::tuple<TCallables...> const &callables, std::array<std::vector<double>, sizeof...(Types)> bins, bool ignoreOverflows
    )
        : myBucket(std::tuple_cat(callables, std::make_tuple(myCallable<Types>()...)), bins, ignoreOverflows)
    {
    }

    template <typename TIter>
    auto getBinningValues(TIter& rowIterator, uint64_t globalIndex = -1) const
    {
        if (globalIndex != -1) {
            rowIterator.setCursor(globalIndex);
        }
        return myBucket.getValues(*rowIterator);
    }

    template <typename Table>
    auto getBinningValues(
        typename Table::iterator rowIterator, Table& table, uint64_t globalIndex = -1
    ) const
    {
        return getBinningValues(rowIterator, globalIndex);
    }

    template <typename... TypesAndLambdas>
    int getBin(std::tuple<TypesAndLambdas...> const& data) const
    {
        auto indices = myBucket.getUpperIndicesForTuple(data);
        auto bucket  = myBucket.calculateBucketAtIndices(indices);
        return bucket;
    }
};

}  // namespace o2::framework

#endif  // O2OVERLAYS_H
//#endif
