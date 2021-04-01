/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "components/payments/content/payment_request.h"

#include "base/bind.h"
#include "brave/browser/brave_rewards/rewards_service_factory.h"
#include "brave/components/brave_rewards/common/constants.h"
#include "brave/components/payments/content/buildflags/buildflags.h"
#include "content/public/browser/browser_context.h"
#include "third_party/blink/public/mojom/payments/payment_request.mojom.h"

using payments::mojom::PaymentErrorReason;

#if BUILDFLAG(ENABLE_PAY_WITH_BAT)
namespace payments {

void PaymentRequest::GetPublisherDetailsCallback(
    mojo::PendingRemote<mojom::PaymentRequestClient> client,
    std::vector<mojom::PaymentMethodDataPtr> method_data,
    mojom::PaymentDetailsPtr details,
    mojom::PaymentOptionsPtr options,
    const ledger::type::Result result,
    ledger::type::PublisherInfoPtr info) {
  if (!info || info->status == ledger::type::PublisherStatus::NOT_VERIFIED) {
    log_.Error(brave_rewards::errors::kInvalidPublisher);
    TerminateConnection();
    return;
  }

  Init_ChromiumImpl(std::move(client), std::move(method_data),
                    std::move(details), std::move(options));
}

void PaymentRequest::TerminateConnectionWithMessage(PaymentErrorReason reason,
                                                    std::string err) {
  client_->OnError(reason, err);
  TerminateConnection();
}

void PaymentRequest::Init(
    mojo::PendingRemote<mojom::PaymentRequestClient> client,
    std::vector<mojom::PaymentMethodDataPtr> method_data,
    mojom::PaymentDetailsPtr details,
    mojom::PaymentOptionsPtr options) {
  bool result = false;
  content::WebContents* contents = web_contents();
  content::BrowserContext* context = contents->GetBrowserContext();

  for (size_t i = 0; i < method_data.size(); i++) {
    if (method_data[i]->supported_method == brave_rewards::kBatPaymentMethod) {
      result = true;
      break;
    }
  }

  if (!result) {
    Init_ChromiumImpl(std::move(client), std::move(method_data),
                      std::move(details), std::move(options));
    return;
  }

  // For BAT payment method, every item should have an associated SKU token
  if (details->display_items.has_value()) {
    for (const mojom::PaymentItemPtr& display_item : *details->display_items) {
      if (!(display_item->sku.has_value())) {
        log_.Error(brave_rewards::errors::kInvalidData);
        TerminateConnection();
        return;
      }
    }
  } else {
    log_.Error(brave_rewards::errors::kInvalidData);
    TerminateConnection();
    return;
  }

  // BAT payment method only works in regular mode.
  if (context->IsOffTheRecord()) {
    log_.Error(brave_rewards::errors::kBraveRewardsNotEnabled);
    TerminateConnection();
    return;
  }

  // BAT payment method only works for verified publishers
  auto* service =
      brave_rewards::RewardsServiceFactory::FromBrowserContext(context);
  if (service->IsRewardsEnabled()) {
    service->GetPublisherInfo(
        contents->GetLastCommittedURL().GetOrigin().host(),
        base::BindOnce(&PaymentRequest::GetPublisherDetailsCallback,
                       weak_ptr_factory_.GetWeakPtr(), std::move(client),
                       std::move(method_data), std::move(details),
                       std::move(options)));
  } else {
    log_.Error(brave_rewards::errors::kBraveRewardsNotEnabled);
    TerminateConnection();
  }
}

}  // namespace payments

#define Init Init_ChromiumImpl
#endif
#include "../../../../../components/payments/content/payment_request.cc"

#if BUILDFLAG(ENABLE_PAY_WITH_BAT)
#undef Init
#endif