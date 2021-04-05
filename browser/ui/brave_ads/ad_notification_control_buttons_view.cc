/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/brave_ads/ad_notification_control_buttons_view.h"

#include <memory>

#include "base/bind.h"
#include "brave/app/vector_icons/vector_icons.h"
#include "brave/browser/ui/brave_ads/ad_notification_view.h"
#include "brave/browser/ui/brave_ads/padded_image_button.h"
#include "brave/browser/ui/brave_ads/padded_image_view.h"
#include "ui/compositor/layer.h"
#include "ui/gfx/color_palette.h"
#include "ui/gfx/paint_vector_icon.h"
#include "ui/views/background.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/metadata/metadata_impl_macros.h"

namespace brave_ads {

namespace {

const int kMinimumButtonHeight = 44;

const int kInfoButtonIconDipSize = 32;

constexpr SkColor kCloseButtonIconColor = SkColorSetRGB(0x4c, 0x36, 0xd2);
const int kCloseButtonIconDipSize = 18;

}  // namespace

AdNotificationControlButtonsView::AdNotificationControlButtonsView(
    AdNotificationView* ad_notification_view)
    : ad_notification_view_(ad_notification_view) {
  DCHECK(ad_notification_view_);

  CreateView();
}

AdNotificationControlButtonsView::~AdNotificationControlButtonsView() = default;

///////////////////////////////////////////////////////////////////////////////

void AdNotificationControlButtonsView::CreateView() {
  views::BoxLayout* box_layout =
      SetLayoutManager(std::make_unique<views::BoxLayout>(
          views::BoxLayout::Orientation::kHorizontal));

  box_layout->set_cross_axis_alignment(
      views::BoxLayout::CrossAxisAlignment::kStart);

  // Use layer to change the opacity
  SetPaintToLayer();
  layer()->SetFillsBoundsOpaquely(false);

  CreateInfoButton();
  CreateCloseButton();

  Layout();
}

void AdNotificationControlButtonsView::CreateInfoButton() {
  PaddedImageView* info_button =
      AddChildView(std::make_unique<PaddedImageView>());

  const gfx::ImageSkia image_skia = gfx::CreateVectorIcon(
      kBraveAdsInfoButtonIcon, kInfoButtonIconDipSize, SK_ColorTRANSPARENT);

  info_button->SetImage(image_skia);
}

void AdNotificationControlButtonsView::CreateCloseButton() {
  PaddedImageButton* close_button =
      AddChildView(std::make_unique<PaddedImageButton>(
          base::BindRepeating(&AdNotificationView::OnCloseButtonPressed,
                              base::Unretained(ad_notification_view_))));

  const gfx::ImageSkia image_skia = gfx::CreateVectorIcon(
      kBraveAdsCloseButtonIcon, kCloseButtonIconDipSize, kCloseButtonIconColor);

  close_button->SetImage(views::Button::STATE_NORMAL, image_skia);

  close_button->AdjustBorderInsetToFitHeight(kMinimumButtonHeight);
}

BEGIN_METADATA(AdNotificationControlButtonsView, views::View)
END_METADATA

}  // namespace brave_ads
