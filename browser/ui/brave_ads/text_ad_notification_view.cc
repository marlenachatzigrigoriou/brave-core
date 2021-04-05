/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/brave_ads/text_ad_notification_view.h"

#include <memory>
#include <string>

#include "base/bind.h"
#include "base/strings/string16.h"
#include "brave/browser/ui/brave_ads/ad_notification_control_buttons_view.h"
#include "brave/browser/ui/brave_ads/ad_notification_header_view.h"
#include "brave/browser/ui/brave_ads/ad_notification_popup.h"
#include "brave/browser/ui/brave_ads/font_util.h"
#include "brave/browser/ui/brave_ads/insets_util.h"
#include "brave/components/brave_ads/browser/ad_notification.h"
#include "ui/gfx/color_palette.h"
#include "ui/gfx/font.h"
#include "ui/gfx/font_list.h"
#include "ui/gfx/geometry/insets.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/text_constants.h"
#include "ui/views/border.h"
#include "ui/views/controls/label.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"

namespace brave_ads {

namespace {

const int kNotificationWidth = 344;
const int kNotificationHeight = 88;

constexpr gfx::Insets kContainerViewInsideBorderInsets(
    /* top */ 0,
    /* left */ 10,
    /* bottom */ 10,
    /* right */ 10);

constexpr gfx::Insets kBodyViewBorderInsets(0);

const gfx::ElideBehavior kTitleElideBehavior = gfx::ELIDE_TAIL;

const int kBodyMaximumLines = 2;
const int kBodyFontSize = 13;
const gfx::Font::FontStyle kBodyFontStyle = gfx::Font::NORMAL;
const gfx::Font::Weight kBodyFontWeight = gfx::Font::Weight::NORMAL;
const int kBodyLineSpacing = 0;
constexpr SkColor kBodyColor = SkColorSetRGB(0x75, 0x75, 0x75);
const gfx::HorizontalAlignment kBodyHorizontalAlignment = gfx::ALIGN_LEFT;
const gfx::VerticalAlignment kBodyVerticalAlignment = gfx::ALIGN_TOP;
const gfx::ElideBehavior kBodyElideBehavior = gfx::ELIDE_TAIL;
constexpr gfx::Insets kBodyBorderInsets(0);

}  // namespace

TextAdNotificationView::TextAdNotificationView(
    const AdNotification& ad_notification)
    : AdNotificationView(ad_notification) {
  SetSize(gfx::Size(kNotificationWidth, kNotificationHeight));

  UpdateForAdNotification(ad_notification);
}

TextAdNotificationView::~TextAdNotificationView() = default;

void TextAdNotificationView::UpdateForAdNotification(
    const AdNotification& ad_notification) {
  AdNotificationView::UpdateForAdNotification(ad_notification);

  CreateView(ad_notification);

  Layout();
  SchedulePaint();
}

void TextAdNotificationView::OnThemeChanged() {
  AdNotificationView::OnThemeChanged();
}

///////////////////////////////////////////////////////////////////////////////

void TextAdNotificationView::CreateView(const AdNotification& ad_notification) {
  SetLayoutManager(std::make_unique<views::BoxLayout>(
      views::BoxLayout::Orientation::kVertical, gfx::Insets()));

  // Header
  AdNotificationHeaderView* header_view = CreateHeaderView(ad_notification);
  AdNotificationControlButtonsView* control_buttons_view =
      new AdNotificationControlButtonsView(this);
  header_view->AddChildView(control_buttons_view);
  AddChildView(header_view);

  // Container
  views::View* container_view = new views::View();
  views::BoxLayout* box_layout =
      container_view->SetLayoutManager(std::make_unique<views::BoxLayout>(
          views::BoxLayout::Orientation::kHorizontal,
          kContainerViewInsideBorderInsets));

  box_layout->set_cross_axis_alignment(
      views::BoxLayout::CrossAxisAlignment::kStart);

  AddChildView(container_view);

  // Body
  views::View* body_view = CreateBodyView(ad_notification);
  container_view->AddChildView(body_view);
  box_layout->SetFlexForView(body_view, 1);
}

AdNotificationHeaderView* TextAdNotificationView::CreateHeaderView(
    const AdNotification& ad_notification) {
  const int width = View::width();
  AdNotificationHeaderView* view = new AdNotificationHeaderView(width);

  view->SetTitle(ad_notification.title());
  view->SetTitleElideBehavior(kTitleElideBehavior);

  return view;
}

views::View* TextAdNotificationView::CreateBodyView(
    const AdNotification& ad_notification) {
  views::View* view = new views::View();

  view->SetLayoutManager(std::make_unique<views::BoxLayout>(
      views::BoxLayout::Orientation::kVertical, gfx::Insets()));

  view->SetBorder(views::CreateEmptyBorder(kBodyViewBorderInsets));

  views::Label* label = CreateBodyLabel(ad_notification);
  view->AddChildView(label);

  return view;
}

views::Label* TextAdNotificationView::CreateBodyLabel(
    const AdNotification& ad_notification) {
  const base::string16 body = ad_notification.body();

  views::Label* label = new views::Label(body);

  const gfx::FontList font_list =
      GetFontList(kBodyFontSize, kBodyFontStyle, kBodyFontWeight);
  label->SetFontList(font_list);

  label->SetEnabledColor(kBodyColor);
  label->SetBackgroundColor(SK_ColorTRANSPARENT);

  label->SetHorizontalAlignment(kBodyHorizontalAlignment);
  label->SetVerticalAlignment(kBodyVerticalAlignment);

  label->SetElideBehavior(kBodyElideBehavior);

  const int line_height = font_list.GetHeight() + kBodyLineSpacing;
  label->SetLineHeight(line_height);
  label->SetMaxLines(kBodyMaximumLines);
  label->SetMultiLine(true);
  label->SetAllowCharacterBreak(true);

  gfx::Insets border_insets = kBodyBorderInsets;
  AdjustInsetsForFontList(&border_insets, font_list);
  label->SetBorder(views::CreateEmptyBorder(border_insets));

  const int width = View::width() - GetInsets().width();
  label->SizeToFit(width);

  label->SetVisible(true);

  return label;
}

}  // namespace brave_ads
