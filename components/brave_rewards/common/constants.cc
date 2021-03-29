/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_rewards/common/constants.h"

namespace brave_rewards {

const char kBatPaymentMethod[] = "bat";

namespace errors {

const char kBatTransactionFailed[] = "BAT transaction failed";
const char kBraveRewardsNotEnabled[] =
    "Brave rewards should be enabled to use Pay with BAT";
const char kInvalidData[] = "Invalid data in payment request";
const char kInvalidPublisher[] = "Unverified publisher";
const char kInvalidRenderer[] = "Renderer not found";
const char kMissingSKUTokens[] = "SKU tokens are missing";
const char kRewardsNotInitialized[] = "Brave rewards is not initialized";
const char kTransactionCancelled[] = "Transaction cancelled";
const char kUnavailableInPrivateMode[] =
    "BAT payment method cannot be used in private mode";
const char kUnverifiedUserWallet[] = "Unverified user wallet";
const char kInsufficientBalance[] = "Insufficient Balance";

}  // namespace errors

}  // namespace brave_rewards
