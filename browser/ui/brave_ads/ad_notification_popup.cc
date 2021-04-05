/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/brave_ads/ad_notification_popup.h"

#include <map>
#include <memory>
#include <utility>

#include "base/bind.h"
#include "brave/browser/profiles/profile_util.h"
#include "brave/browser/ui/brave_ads/ad_notification_view.h"
#include "brave/browser/ui/brave_ads/ad_notification_view_factory.h"
#include "brave/browser/ui/brave_ads/bounds_util.h"
#include "brave/components/brave_ads/common/pref_names.h"
#include "build/build_config.h"
#include "components/prefs/pref_service.h"
#include "ui/display/screen.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/size.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/metadata/metadata_impl_macros.h"
#include "ui/views/widget/widget.h"

#if defined(OS_WIN)
#include "ui/views/widget/desktop_aura/desktop_native_widget_aura.h"
#endif

namespace brave_ads {

namespace {

// TODO(https://github.com/brave/brave-browser/issues/14957): Decouple
// AdNotificationPopup management to NotificationPopupCollection
std::map<std::string, AdNotificationPopup*> g_ad_notification_popups;

const int kPopupPadding = 10;

}  // namespace

AdNotificationPopup::AdNotificationPopup(Profile* profile,
                                         const AdNotification& ad_notification)
    : profile_(profile), ad_notification_(ad_notification) {
  DCHECK(profile_);

  CreatePopup();
}

AdNotificationPopup::~AdNotificationPopup() = default;

// static
void AdNotificationPopup::Show(Profile* profile,
                               const AdNotification& ad_notification) {
  DCHECK(profile);

  const std::string& id = ad_notification.id();

  DCHECK(!g_ad_notification_popups[id]);
  g_ad_notification_popups[id] =
      new AdNotificationPopup(profile, ad_notification);

  AdNotificationDelegate* delegate = ad_notification.delegate();
  if (delegate) {
    delegate->OnShow();
  }
}

// static
void AdNotificationPopup::Close(const std::string& notification_id,
                                const bool by_user) {
  DCHECK(!notification_id.empty());

  if (!g_ad_notification_popups[notification_id]) {
    return;
  }

  AdNotificationPopup* popup = g_ad_notification_popups[notification_id];
  DCHECK(popup);

  const AdNotification ad_notification = popup->GetAdNotification();
  AdNotificationDelegate* delegate = ad_notification.delegate();
  if (delegate) {
    delegate->OnClose(by_user);
  }

  popup->FadeOutAndCloseWidgetView();
}

// static
void AdNotificationPopup::OnClick(const std::string& notification_id) {
  DCHECK(!notification_id.empty());

  DCHECK(g_ad_notification_popups[notification_id]);
  AdNotificationPopup* popup = g_ad_notification_popups[notification_id];
  DCHECK(popup);

  const AdNotification ad_notification = popup->GetAdNotification();
  AdNotificationDelegate* delegate = ad_notification.delegate();
  if (delegate) {
    delegate->OnClick();
  }

  popup->FadeOutAndCloseWidgetView();
}

void AdNotificationPopup::OnDisplayChanged() {
  OnWorkAreaChanged();
}

void AdNotificationPopup::OnWorkAreaChanged() {
  if (!IsWidgetValid()) {
    return;
  }

  gfx::Rect bounds = GetWidget()->GetWindowBoundsInScreen();
  const gfx::NativeView native_view = GetWidget()->GetNativeView();
  AdjustBoundsToFitWorkAreaForNativeView(&bounds, native_view);

  GetWidget()->SetBounds(bounds);
}

void AdNotificationPopup::OnFocus() {
  // This view is just a container, so advance focus to the underlying
  // AdNotificationView
  DCHECK(ad_notification_view_);
  GetFocusManager()->SetFocusedView(ad_notification_view_);
}

void AdNotificationPopup::OnWidgetCreated(views::Widget* widget) {
  DCHECK(widget);

  gfx::Rect bounds = widget->GetWindowBoundsInScreen();
  const gfx::NativeView native_view = widget->GetNativeView();
  AdjustBoundsToFitWorkAreaForNativeView(&bounds, native_view);

  widget->SetBounds(bounds);
}

void AdNotificationPopup::OnWidgetDestroyed(views::Widget* widget) {
  DCHECK(widget);

  DCHECK(widget_observation_.IsObservingSource(widget));
  widget_observation_.Reset();
}

void AdNotificationPopup::OnWidgetBoundsChanged(views::Widget* widget,
                                                const gfx::Rect& new_bounds) {
  DCHECK(widget);

  const gfx::Point origin = new_bounds.origin();
  SaveOrigin(origin);
}

///////////////////////////////////////////////////////////////////////////////

void AdNotificationPopup::CreatePopup() {
  SetLayoutManager(std::make_unique<views::FillLayout>());

  ad_notification_view_ =
      AddChildView(AdNotificationViewFactory::Create(ad_notification_));

  SetNotifyEnterExitOnChild(true);

  CreateWidgetView();
}

AdNotification AdNotificationPopup::GetAdNotification() const {
  return ad_notification_;
}

gfx::Point AdNotificationPopup::GetDefaultOriginForSize(
    const gfx::Size& size) const {
  const gfx::Rect work_area =
      display::Screen::GetScreen()->GetPrimaryDisplay().work_area();

  int x = 0;
  int y = 0;

#if defined(OS_WIN)
  // Bottom right to the left of Windows native notifications
  const int kNativeNotificationWidth = 344;
  x = work_area.right() - kNativeNotificationWidth -
      (size.width() + kPopupPadding);
  y = work_area.bottom() - (size.height() + kPopupPadding);
#elif defined(OS_MAC)
  // Top right to the left of macOS native notifications
  const int kNativeNotificationWidth = 344;
  x = work_area.right() - kNativeNotificationWidth -
      (size.width() + kPopupPadding);
  y = work_area.y() + kPopupPadding;
#elif defined(OS_LINUX)
  // Bottom center
  x = (work_area.width() - size.width()) / 2;
  y = work_area.bottom() - (size.height() + kPopupPadding);
#endif

  return gfx::Point(x, y);
}

gfx::Point AdNotificationPopup::GetOriginForSize(const gfx::Size& size) const {
  if (!profile_->GetPrefs()->HasPrefPath(
          prefs::kAdNotificationLastScreenPositionX) ||
      !profile_->GetPrefs()->HasPrefPath(
          prefs::kAdNotificationLastScreenPositionY)) {
    return GetDefaultOriginForSize(size);
  }

  const int x = profile_->GetPrefs()->GetInteger(
      prefs::kAdNotificationLastScreenPositionX);
  const int y = profile_->GetPrefs()->GetInteger(
      prefs::kAdNotificationLastScreenPositionY);
  return gfx::Point(x, y);
}

void AdNotificationPopup::SaveOrigin(const gfx::Point& origin) const {
  profile_->GetPrefs()->SetInteger(prefs::kAdNotificationLastScreenPositionX,
                                   origin.x());
  profile_->GetPrefs()->SetInteger(prefs::kAdNotificationLastScreenPositionY,
                                   origin.y());
}

gfx::Rect AdNotificationPopup::CalculateBounds() const {
  DCHECK(ad_notification_view_);
  const gfx::Size size = ad_notification_view_->size();
  DCHECK(!size.IsEmpty());

  const gfx::Point origin = GetOriginForSize(size);

  return gfx::Rect(origin, size);
}

void AdNotificationPopup::CreateWidgetView() {
  // The widget instance is owned by its NativeWidget. For more details see
  // ui/views/widget/widget.h
  views::Widget::InitParams params(views::Widget::InitParams::TYPE_POPUP);
  params.z_order = ui::ZOrderLevel::kFloatingWindow;
  params.type = views::Widget::InitParams::TYPE_WINDOW_FRAMELESS;
  params.delegate = this;
  params.name = "BraveAdsNotificationPopup";
  params.opacity = views::Widget::InitParams::WindowOpacity::kTranslucent;
  params.bounds = CalculateBounds();

  views::Widget* widget = new views::Widget();
  widget->set_focus_on_creation(false);
  widget_observation_.Observe(widget);

#if defined(OS_WIN)
  // We want to ensure that this toast always goes to the native desktop,
  // not the Ash desktop (since there is already another toast contents view
  // there
  if (!params.parent) {
    DCHECK(!params.native_widget);
    params.native_widget = new views::DesktopNativeWidgetAura(widget);
  }
#endif

  widget->Init(std::move(params));

  widget->ShowInactive();
}

void AdNotificationPopup::FadeOutAndCloseWidgetView() const {
  const std::string notification_id = ad_notification_.id();

  // Destroy the widget when done. The observer deletes itself on completion
  DCHECK(ad_notification_view_);
  ad_notification_view_->FadeOut(base::BindOnce(
      [](const std::string& notification_id) {
        DCHECK(!notification_id.empty());

        DCHECK(g_ad_notification_popups[notification_id]);
        AdNotificationPopup* popup = g_ad_notification_popups[notification_id];
        DCHECK(popup);

        g_ad_notification_popups.erase(notification_id);

        popup->CloseWidgetView();
      },
      notification_id));
}

void AdNotificationPopup::CloseWidgetView() {
  if (!GetWidget()) {
    DeleteDelegate();
    return;
  }

  if (GetWidget()->IsClosed()) {
    return;
  }

  GetWidget()->CloseNow();
}

bool AdNotificationPopup::IsWidgetValid() const {
  return GetWidget() && !GetWidget()->IsClosed();
}

BEGIN_METADATA(AdNotificationPopup, views::WidgetDelegateView)
END_METADATA

}  // namespace brave_ads
