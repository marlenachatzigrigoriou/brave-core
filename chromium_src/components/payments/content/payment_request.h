/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_COMPONENTS_PAYMENTS_CONTENT_PAYMENT_REQUEST_H_
#define BRAVE_CHROMIUM_SRC_COMPONENTS_PAYMENTS_CONTENT_PAYMENT_REQUEST_H_

#include "brave/vendor/bat-native-ledger/include/bat/ledger/mojom_structs.h"
#include "components/payments/content/initialization_task.h"
#include "components/payments/content/payment_handler_host.h"
#include "components/payments/content/payment_request_display_manager.h"
#include "components/payments/content/payment_request_spec.h"
#include "components/payments/content/payment_request_state.h"
#include "components/payments/content/service_worker_payment_app.h"
#include "third_party/blink/public/mojom/payments/payment_request.mojom.h"

using payments::mojom::PaymentErrorReason;

#if !defined(OS_ANDROID)
#define BRAVE_PAYMENT_METHOD_UTIL_FUNCTIONS                               \
  void TerminateConnectionWithMessage(                                    \
      payments::mojom::PaymentErrorReason reason, std::string err);       \
  void GetPublisherDetailsCallback(                                       \
      mojo::PendingRemote<mojom::PaymentRequestClient> client,            \
      std::vector<mojom::PaymentMethodDataPtr> method_data,               \
      mojom::PaymentDetailsPtr details, mojom::PaymentOptionsPtr options, \
      const ledger::type::Result result, ledger::type::PublisherInfoPtr info);
#endif

#define Init                                                                 \
  Init_ChromiumImpl(mojo::PendingRemote<mojom::PaymentRequestClient> client, \
                    std::vector<mojom::PaymentMethodDataPtr> method_data,    \
                    mojom::PaymentDetailsPtr details,                        \
                    mojom::PaymentOptionsPtr options);                       \
  void Init
#include "../../../../../components/payments/content/payment_request.h"
#undef Init

#if !defined(OS_ANDROID)
#undef BRAVE_PAYMENT_METHOD_UTIL_FUNCTIONS
#endif

#endif  // BRAVE_CHROMIUM_SRC_COMPONENTS_PAYMENTS_CONTENT_PAYMENT_REQUEST_H_
