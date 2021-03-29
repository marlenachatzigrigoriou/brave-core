/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_CHROME_BROWSER_UI_VIEWS_PAYMENTS_PAYMENT_REQUEST_DIALOG_VIEW_H_
#define BRAVE_CHROMIUM_SRC_CHROME_BROWSER_UI_VIEWS_PAYMENTS_PAYMENT_REQUEST_DIALOG_VIEW_H_

#include "components/payments/content/payment_request_dialog.h"

#define ShowDialog           \
  ShowDialog_ChromiumImpl(); \
  virtual void ShowDialog

#define CloseDialog           \
  CloseDialog_ChromiumImpl(); \
  virtual void CloseDialog
#include "../../../../../../../chrome/browser/ui/views/payments/payment_request_dialog_view.h"
#undef CloseDialog
#undef ShowDialog

#endif  // BRAVE_CHROMIUM_SRC_CHROME_BROWSER_UI_VIEWS_PAYMENTS_PAYMENT_REQUEST_DIALOG_VIEW_H_
