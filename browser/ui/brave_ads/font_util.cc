/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/brave_ads/font_util.h"

#include "ui/gfx/font_list.h"

namespace brave_ads {

gfx::FontList GetFontList(const int size,
                          const gfx::Font::FontStyle style,
                          const gfx::Font::Weight weight) {
  const gfx::Font default_font;
  const int font_size_delta = size - default_font.GetFontSize();

  const gfx::Font font = default_font.Derive(font_size_delta, style, weight);

  return gfx::FontList(font);
}

}  // namespace brave_ads
