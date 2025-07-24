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
/// \file Cell.h
/// \brief
///

#ifndef TRACKINGITSU_INCLUDE_CACELL_H_
#define TRACKINGITSU_INCLUDE_CACELL_H_

#include "ITStracking/Constants.h"
#include "GPUCommonDef.h"

namespace o2::its
{

class Cell final
{
 public:
  GPUhd() int getFirstClusterIndex() const { return mFirstClusterIndex; };
  GPUhd() int getSecondClusterIndex() const { return mSecondClusterIndex; };
  GPUhd() int getThirdClusterIndex() const { return mThirdClusterIndex; };
  GPUhd() int getFirstTrackletIndex() const { return mFirstTrackletIndex; };
  GPUhd() int getSecondTrackletIndex() const { return mSecondTrackletIndex; };
  GPUhd() int getLevel() const { return mLevel; };
  GPUhd() void setLevel(const int level) { mLevel = level; };
  GPUhd() int* getLevelPtr() { return &mLevel; }

 private:
  int mFirstClusterIndex{constants::UnusedIndex};
  int mSecondClusterIndex{constants::UnusedIndex};
  int mThirdClusterIndex{constants::UnusedIndex};
  int mFirstTrackletIndex{constants::UnusedIndex};
  int mSecondTrackletIndex{constants::UnusedIndex};
  int mLevel{constants::UnusedIndex};
};

class CellSeed final : public o2::track::TrackParCovF
{
 public:
  GPUhdDefault() CellSeed() = default;
  GPUhd() CellSeed(int innerL, int cl0, int cl1, int cl2, int trkl0, int trkl1, o2::track::TrackParCovF& tpc, float chi2) : o2::track::TrackParCovF(tpc), mChi2(chi2), mLevel(1)
  {
    setUserField(innerL);
    mClusters[innerL + 0] = cl0;
    mClusters[innerL + 1] = cl1;
    mClusters[innerL + 2] = cl2;
    mTracklets[0] = trkl0;
    mTracklets[1] = trkl1;
  }
  GPUhdDefault() CellSeed(const CellSeed&) = default;
  GPUhdDefault() ~CellSeed() = default;
  // GPUhdDefault() CellSeed(CellSeed&&) = default; TODO cannot use this yet since TrackPar only has device
  GPUhdDefault() CellSeed& operator=(const CellSeed&) = default;
  GPUhdDefault() CellSeed& operator=(CellSeed&&) = default;

  GPUhd() int getFirstClusterIndex() const { return mClusters[getUserField()]; };
  GPUhd() int getSecondClusterIndex() const { return mClusters[getUserField() + 1]; };
  GPUhd() int getThirdClusterIndex() const { return mClusters[getUserField() + 2]; };
  GPUhd() int getFirstTrackletIndex() const { return mTracklets[0]; };
  GPUhd() void setFirstTrackletIndex(int trkl) { mTracklets[0] = trkl; };
  GPUhd() int getSecondTrackletIndex() const { return mTracklets[1]; };
  GPUhd() void setSecondTrackletIndex(int trkl) { mTracklets[1] = trkl; };
  GPUhd() float getChi2() const { return mChi2; };
  GPUhd() void setChi2(float chi2) { mChi2 = chi2; };
  GPUhd() int getLevel() const { return mLevel; };
  GPUhd() void setLevel(int level) { mLevel = level; };
  GPUhd() int* getLevelPtr() { return &mLevel; }
  GPUhd() int* getClusters() { return mClusters; }
  GPUhd() int getCluster(int i) const { return mClusters[i]; }
  GPUhd() void printCell() const
  {
    printf("trkl: %d, %d\t lvl: %d\t chi2: %f\tcls: [%d | %d | %d | %d | %d | %d | %d]\n", mTracklets[0], mTracklets[1], mLevel, mChi2, mClusters[0], mClusters[1], mClusters[2], mClusters[3], mClusters[4], mClusters[5], mClusters[6]);
  }

 private:
  float mChi2 = -999.f;
  int mLevel = constants::UnusedIndex;
  int mTracklets[2] = {constants::UnusedIndex, constants::UnusedIndex};
  int mClusters[7] = {constants::UnusedIndex, constants::UnusedIndex, constants::UnusedIndex, constants::UnusedIndex, constants::UnusedIndex, constants::UnusedIndex, constants::UnusedIndex};
};

} // namespace o2::its

#endif /* TRACKINGITSU_INCLUDE_CACELL_H_ */
