/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcs/emp_games/common/Constants.h"
#include "fbpcs/emp_games/pcf2_attribution/Constants.h"

namespace pcf2_attribution {

template <bool usingBatch>
struct Touchpoint {
  ConditionalVector<int64_t, usingBatch> id;
  ConditionalVector<bool, usingBatch> isClick;
  ConditionalVector<uint64_t, usingBatch> ts;
};

template <bool usingBatch>
using TouchpointT = ConditionalVector<Touchpoint<usingBatch>, !usingBatch>;

template <
    int schedulerId,
    bool usingBatch,
    common::InputEncryption inputEncryption>
struct PrivateTouchpoint {
  ConditionalVector<int64_t, usingBatch> id;
  SecTimestamp<schedulerId, usingBatch> ts;

  explicit PrivateTouchpoint(const Touchpoint<usingBatch>& touchpoint)
      : id{touchpoint.id} {
    if constexpr (inputEncryption == common::InputEncryption::Xor) {
      typename SecTimestamp<schedulerId, usingBatch>::ExtractedInt extractedTs(
          touchpoint.ts);
      ts = SecTimestamp<schedulerId, usingBatch>(std::move(extractedTs));
    } else {
      ts = SecTimestamp<schedulerId, usingBatch>(
          touchpoint.ts, common::PUBLISHER);
    }
  }
};

// Used for privately sharing isClick for xor encrypted inputs
template <
    int schedulerId,
    bool usingBatch,
    common::InputEncryption inputEncryption>
struct PrivateIsClick {
  SecBit<schedulerId, usingBatch> isClick;

  explicit PrivateIsClick(const Touchpoint<usingBatch>& touchpoint) {
    if constexpr (inputEncryption == common::InputEncryption::Xor) {
      typename SecBit<schedulerId, usingBatch>::ExtractedBit extractedIsClick(
          touchpoint.isClick);
      isClick = SecBit<schedulerId, usingBatch>(std::move(extractedIsClick));
    } else {
      isClick = SecBit<schedulerId, usingBatch>(
          touchpoint.isClick, common::PUBLISHER);
    }
  }
};

// Used for parsing touchpoints from input CSV files
struct ParsedTouchpoint {
  int64_t id;
  bool isClick;
  uint64_t ts;

  /**
   * If both are clicks, or both are views, the earliest one comes first.
   * If one is a click but the other is a view, the view comes first.
   */
  bool operator<(const ParsedTouchpoint& tp) const {
    return (isClick == tp.isClick) ? (ts < tp.ts) : !isClick;
  }
};

} // namespace pcf2_attribution
