/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_BROWSER_AD_NOTIFICATION_UTIL_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_BROWSER_AD_NOTIFICATION_UTIL_H_

#include <memory>

namespace message_center {
class Notification;
}  // namespace message_center

namespace ads {
struct AdNotificationInfo;
}  // namespace ads

namespace brave_ads {

class AdNotification;

constexpr char kAdNotificationUrlPrefix[] = "https://www.brave.com/ads/?";

std::unique_ptr<message_center::Notification> CreateLegacyAdNotification(
    const ads::AdNotificationInfo& ad_notification);

AdNotification CreateAdNotification(
    const ads::AdNotificationInfo& ad_notification);

}  // namespace brave_ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_BROWSER_AD_NOTIFICATION_UTIL_H_
