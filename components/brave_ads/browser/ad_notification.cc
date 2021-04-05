/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/browser/ad_notification.h"

namespace brave_ads {

AdNotification::AdNotification(const std::string& id,
                               const base::string16& title,
                               const base::string16& body,
                               scoped_refptr<AdNotificationDelegate> delegate)
    : id_(id), title_(title), body_(body), delegate_(std::move(delegate)) {}

AdNotification::AdNotification(scoped_refptr<AdNotificationDelegate> delegate,
                               const AdNotification& other)
    : AdNotification(other) {
  delegate_ = delegate;
}

AdNotification::AdNotification(const std::string& id,
                               const AdNotification& other)
    : AdNotification(other) {
  id_ = id;
}

AdNotification::AdNotification(const AdNotification& other) = default;

AdNotification& AdNotification::operator=(const AdNotification& other) =
    default;

AdNotification::~AdNotification() = default;

}  // namespace brave_ads
