/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/browser/ad_notification_util.h"

#include <string>

#include "base/strings/string16.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "bat/ads/ad_notification_info.h"
#include "brave/components/brave_ads/browser/ad_notification.h"
#include "ui/message_center/public/cpp/notification.h"
#include "ui/message_center/public/cpp/notification_types.h"
#include "ui/message_center/public/cpp/notifier_id.h"

namespace brave_ads {

std::unique_ptr<message_center::Notification> CreateLegacyAdNotification(
    const ads::AdNotificationInfo& ad_notification) {
  base::string16 title;
  if (base::IsStringUTF8(ad_notification.title)) {
    title = base::UTF8ToUTF16(ad_notification.title);
  }

  base::string16 body;
  if (base::IsStringUTF8(ad_notification.body)) {
    body = base::UTF8ToUTF16(ad_notification.body);
  }

  message_center::RichNotificationData notification_data;
  notification_data.context_message = base::ASCIIToUTF16(" ");

  const std::string url = kAdNotificationUrlPrefix + ad_notification.uuid;

  std::unique_ptr<message_center::Notification> notification =
      std::make_unique<message_center::Notification>(
          message_center::NOTIFICATION_TYPE_SIMPLE, ad_notification.uuid, title,
          body, gfx::Image(), base::string16(), GURL(url),
          message_center::NotifierId(
              message_center::NotifierType::SYSTEM_COMPONENT,
              "service.ads_service"),
          notification_data, nullptr);

#if !defined(OS_MAC) || defined(OFFICIAL_BUILD)
  // set_never_timeout uses an XPC service which requires signing so for now we
  // don't set this for macos dev builds
  notification->set_never_timeout(true);
#endif

  return notification;
}

AdNotification CreateAdNotification(
    const ads::AdNotificationInfo& ad_notification) {
  base::string16 title;
  if (base::IsStringUTF8(ad_notification.title)) {
    title = base::UTF8ToUTF16(ad_notification.title);
  }

  base::string16 body;
  if (base::IsStringUTF8(ad_notification.body)) {
    body = base::UTF8ToUTF16(ad_notification.body);
  }

  return AdNotification(ad_notification.uuid, title, body, nullptr);
}

}  // namespace brave_ads
