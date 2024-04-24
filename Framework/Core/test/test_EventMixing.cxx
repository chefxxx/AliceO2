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

#include "Framework/ASoAHelpers.h"
#include "Framework/GroupedCombinations.h"
#include "Framework/TableBuilder.h"
#include "Framework/AnalysisDataModel.h"
#include <catch_amalgamated.hpp>
#include <random>
#include <vector>
#include <list>
#include <iostream>

using namespace o2::framework;
using namespace o2::soa;

constexpr uint64_t testSize = 4194304;
constexpr int numEventsToMix = 5;

TEST_CASE("EventMixingNaiveCollisionsPairsSameCategories")
{
  // Seed with a real random value, if available
  std::default_random_engine e1(1234567891);
  std::uniform_real_distribution<float> uniform_dist(0.f, 1.f);
  std::uniform_real_distribution<float> uniform_dist_x(-0.065f, 0.073f);
  std::uniform_real_distribution<float> uniform_dist_y(-0.320f, 0.360f);
  std::uniform_int_distribution<int> uniform_dist_int(0, 5);

  std::vector<double> xBins{VARIABLE_WIDTH, -0.064, -0.062, -0.060, 0.066, 0.068, 0.070, 0.072};
  std::vector<double> yBins{VARIABLE_WIDTH, -0.320, -0.301, -0.300, 0.330, 0.340, 0.350, 0.360};
  using BinningType = ColumnBinningPolicy<o2::aod::collision::PosX, o2::aod::collision::PosY>;
  BinningType binningOnPositions{{xBins, yBins}, true}; // true is for 'ignore overflows' (true by default)

  TableBuilder colBuilder;
  auto rowWriterCol = colBuilder.cursor<o2::aod::Collisions>();
  for (auto i = 0; i < testSize; ++i) {
    float x = uniform_dist_x(e1);
    float y = uniform_dist_y(e1);
    rowWriterCol(0, uniform_dist_int(e1),
                 x, y, uniform_dist(e1),
                 uniform_dist(e1), uniform_dist(e1), uniform_dist(e1),
                 uniform_dist(e1), uniform_dist(e1), uniform_dist(e1),
                 uniform_dist_int(e1), uniform_dist(e1),
                 uniform_dist_int(e1),
                 uniform_dist(e1), uniform_dist(e1));
  }
  auto tableCol = colBuilder.finalize();
  o2::aod::Collisions collisions{tableCol};

  int64_t colCount = 0;
  int nBinsTot = (xBins.size() - 2) * (yBins.size() - 2);

  colCount = 0;
  std::vector<std::list<o2::aod::Collisions::iterator>> mixingBufferVector;
  for (int i = 0; i < nBinsTot; i++) {
    mixingBufferVector.push_back(std::list<o2::aod::Collisions::iterator>());
  }
  for (auto& col1 : collisions) {
    int bin = binningOnPositions.getBin({col1.posX(), col1.posY()});
    if (bin == -1) {
      continue;
    }
    auto& mixingBuffer = mixingBufferVector[bin];
    if (mixingBuffer.size() > 0) {
      for (auto& col2 : mixingBuffer) {
        colCount++;
      }
      if (mixingBuffer.size() >= numEventsToMix - 1) {
        mixingBuffer.pop_back();
      }
    }
    mixingBuffer.push_front(col1);
  }
}

TEST_CASE("EventMixingCombGenCollisionsPairsSameCategories")
{
  std::cout << "Start EventMixingCombGenCollisionsPairsSameCategories" << std::endl;
  // Seed with a real random value, if available
  std::default_random_engine e1(1234567891);
  std::uniform_real_distribution<float> uniform_dist(0.f, 1.f);
  std::uniform_real_distribution<float> uniform_dist_x(-0.065f, 0.073f);
  std::uniform_real_distribution<float> uniform_dist_y(-0.320f, 0.360f);
  std::uniform_int_distribution<int> uniform_dist_int(0, 5);

  std::vector<double> xBins{VARIABLE_WIDTH, -0.064, -0.062, -0.060, 0.066, 0.068, 0.070, 0.072};
  std::vector<double> yBins{VARIABLE_WIDTH, -0.320, -0.301, -0.300, 0.330, 0.340, 0.350, 0.360};
  using BinningType = ColumnBinningPolicy<o2::aod::collision::PosX, o2::aod::collision::PosY>;
  BinningType binningOnPositions{{xBins, yBins}, true}; // true is for 'ignore overflows' (true by default)

  TableBuilder colBuilder;
  auto rowWriterCol = colBuilder.cursor<o2::aod::Collisions>();
  for (auto i = 0; i < testSize; ++i) {
    float x = uniform_dist_x(e1);
    float y = uniform_dist_y(e1);
    rowWriterCol(0, uniform_dist_int(e1),
                 x, y, uniform_dist(e1),
                 uniform_dist(e1), uniform_dist(e1), uniform_dist(e1),
                 uniform_dist(e1), uniform_dist(e1), uniform_dist(e1),
                 uniform_dist_int(e1), uniform_dist(e1),
                 uniform_dist_int(e1),
                 uniform_dist(e1), uniform_dist(e1));
  }
  auto tableCol = colBuilder.finalize();
  o2::aod::Collisions collisions{tableCol};

  int64_t colCount = 0;

  colCount = 0;
  for (auto& [c1, c2] : combinations(CombinationsBlockStrictlyUpperSameIndexPolicy(binningOnPositions, numEventsToMix - 1, -1, collisions, collisions))) {
    colCount++;
    int bin = binningOnPositions.getBin({c1.posX(), c1.posY()});
  }
  std::cout << "End EventMixingCombGenCollisionsPairsSameCategories" << std::endl;
}
