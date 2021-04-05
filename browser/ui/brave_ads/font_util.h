/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_UI_BRAVE_ADS_FONT_UTIL_H_
#define BRAVE_BROWSER_UI_BRAVE_ADS_FONT_UTIL_H_

#include "ui/gfx/font.h"

namespace gfx {
class FontList;
}  // namespace gfx

namespace brave_ads {

gfx::FontList GetFontList(const int size,
                          const gfx::Font::FontStyle style,
                          const gfx::Font::Weight weight);

}  // namespace brave_ads

#endif  // BRAVE_BROWSER_UI_BRAVE_ADS_FONT_UTIL_H_
