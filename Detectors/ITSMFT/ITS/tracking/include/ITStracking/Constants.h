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
/// \file Constants.h
/// \brief
///

#ifndef TRACKINGITSU_INCLUDE_CONSTANTS_H_
#define TRACKINGITSU_INCLUDE_CONSTANTS_H_

#include "ITStracking/Definitions.h"

namespace o2::its::constants
{
constexpr float MB = 1024.f * 1024.f;
constexpr float GB = 1024.f * 1024.f * 1024.f;
constexpr bool DoTimeBenchmarks = true;
constexpr bool SaveTimeBenchmarks = false;

constexpr float Tolerance{1e-12}; // numerical tolerance
constexpr int ClustersPerCell{3};
constexpr int UnusedIndex{-1};
constexpr float Resolution{0.0005f};
constexpr float Radl = 9.36f; // Radiation length of Si [cm]
constexpr float Rho = 2.33f;  // Density of Si [g/cm^3]
} // namespace o2::its::constants

#endif /* TRACKINGITSU_INCLUDE_CONSTANTS_H_ */
