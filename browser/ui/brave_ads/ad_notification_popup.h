/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_UI_BRAVE_ADS_AD_NOTIFICATION_POPUP_H_
#define BRAVE_BROWSER_UI_BRAVE_ADS_AD_NOTIFICATION_POPUP_H_

#include <string>

#include "base/scoped_observation.h"
#include "brave/components/brave_ads/browser/ad_notification.h"
#include "ui/views/metadata/metadata_header_macros.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/views/widget/widget_observer.h"

class Profile;

namespace gfx {
class Point;
class Rect;
class Size;
}  // namespace gfx

namespace views {
class Widget;
}  // namespace views

namespace brave_ads {

class AdNotificationView;

// The widget delegate of an ad notification popup view. The view is owned by
// the widget
class AdNotificationPopup : public views::WidgetDelegateView,
                            public views::WidgetObserver {
 public:
  METADATA_HEADER(AdNotificationPopup);

  explicit AdNotificationPopup(Profile* profile,
                               const AdNotification& ad_notification);
  ~AdNotificationPopup() override;

  // Show the notification popup view for the given |profile| and
  // |ad_notification|
  static void Show(Profile* profile, const AdNotification& ad_notification);

  // Close the notification popup view for the given |notification_id|.
  // |by_user| is true if the notification popup was closed by the user,
  // otherwise false
  static void Close(const std::string& notification_id, const bool by_user);

  // User clicked the notification popup view for the given |notification_id|
  static void OnClick(const std::string& notification_id);

  // views::WidgetDelegateView:
  void OnDisplayChanged() override;
  void OnWorkAreaChanged() override;
  void OnFocus() override;

  // views::WidgetObserver:
  void OnWidgetCreated(views::Widget* widget) override;
  void OnWidgetDestroyed(views::Widget* widget) override;
  void OnWidgetBoundsChanged(views::Widget* widget,
                             const gfx::Rect& new_bounds) override;

 private:
  Profile* profile_;  // NOT OWNED

  void CreatePopup();

  AdNotification ad_notification_;
  AdNotification GetAdNotification() const;

  gfx::Point GetDefaultOriginForSize(const gfx::Size& size) const;
  gfx::Point GetOriginForSize(const gfx::Size& size) const;
  void SaveOrigin(const gfx::Point& origin) const;

  gfx::Rect CalculateBounds() const;

  base::ScopedObservation<views::Widget, views::WidgetObserver>
      widget_observation_{this};
  void CreateWidgetView();
  void FadeOutAndCloseWidgetView() const;
  void CloseWidgetView();

  AdNotificationView* ad_notification_view_ = nullptr;  // NOT OWNED

  bool IsWidgetValid() const;

  AdNotificationPopup(const AdNotificationPopup&) = delete;
  AdNotificationPopup& operator=(const AdNotificationPopup&) = delete;
};

}  // namespace brave_ads

#endif  // BRAVE_BROWSER_UI_BRAVE_ADS_AD_NOTIFICATION_POPUP_H_
