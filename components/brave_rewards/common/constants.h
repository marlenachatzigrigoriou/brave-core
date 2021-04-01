/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_REWARDS_COMMON_CONSTANTS_H_
#define BRAVE_COMPONENTS_BRAVE_REWARDS_COMMON_CONSTANTS_H_

namespace brave_rewards {

extern const char kBatPaymentMethod[];

namespace errors {

extern const char kBatTransactionFailed[];
extern const char kBraveRewardsNotEnabled[];
extern const char kInsufficientBalance[];
extern const char kInvalidData[];
extern const char kInvalidPublisher[];
extern const char kInvalidRenderer[];
extern const char kRewardsNotInitialized[];
extern const char kTransactionCancelled[];
extern const char kUnverifiedUserWallet[];

}  // namespace errors

}  // namespace brave_rewards

#endif  // BRAVE_COMPONENTS_BRAVE_REWARDS_COMMON_CONSTANTS_H_
