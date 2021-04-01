/* Copyright 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/brave_rewards/checkout_dialog.h"

#include <memory>
#include <string>
#include <vector>

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/strings/string_number_conversions.h"
#include "base/values.h"
#include "brave/browser/brave_rewards/rewards_service_factory.h"
#include "brave/common/webui_url_constants.h"
#include "brave/components/brave_rewards/browser/rewards_service.h"
#include "brave/components/brave_rewards/browser/rewards_service_impl.h"
#include "brave/components/brave_rewards/common/constants.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/webui/constrained_web_dialog_ui.h"
#include "components/payments/content/payment_request.h"
#include "components/payments/core/payer_data.h"
#include "components/web_modal/web_contents_modal_dialog_host.h"
#include "content/public/browser/web_contents.h"
#include "third_party/blink/public/mojom/payments/payment_request.mojom.h"
#include "ui/web_dialogs/web_dialog_delegate.h"

using content::WebContents;
using content::WebUIMessageHandler;
using payments::PaymentRequest;
using payments::mojom::PaymentErrorReason;

namespace brave_rewards {

enum DialogCloseReason {
  Complete,
  InsufficientBalance,
  UnverifiedWallet,
  UserCancelled
};

constexpr int kDialogWidth = 548;
constexpr int kDialogMinHeight = 200;
constexpr int kDialogMaxHeight = 800;

CheckoutDialogDelegate::CheckoutDialogDelegate(
    base::Value params,
    base::WeakPtr<PaymentRequest> request)
    : params_(std::move(params)), request_(request) {}

CheckoutDialogDelegate::~CheckoutDialogDelegate() = default;

ui::ModalType CheckoutDialogDelegate::GetDialogModalType() const {
  return ui::MODAL_TYPE_WINDOW;
}

base::string16 CheckoutDialogDelegate::GetDialogTitle() const {
  return base::string16();
}

GURL CheckoutDialogDelegate::GetDialogContentURL() const {
  return GURL(kBraveUICheckoutURL);
}

void CheckoutDialogDelegate::GetWebUIMessageHandlers(
    std::vector<WebUIMessageHandler*>* handlers) const {
  handlers->push_back(new CheckoutDialogHandler(request_));
}

void CheckoutDialogDelegate::GetDialogSize(gfx::Size* size) const {}

std::string CheckoutDialogDelegate::GetDialogArgs() const {
  std::string json;
  base::JSONWriter::Write(params_, &json);
  return json;
}

void CheckoutDialogDelegate::OnDialogClosed(const std::string& result) {
  int reason;
  DCHECK(base::StringToInt(result, &reason));
  switch (reason) {
    case DialogCloseReason::UserCancelled:
      request_->TerminateConnectionWithMessage(PaymentErrorReason::USER_CANCEL,
                                               errors::kTransactionCancelled);
      break;
    case DialogCloseReason::UnverifiedWallet:
      request_->TerminateConnectionWithMessage(
          PaymentErrorReason::NOT_SUPPORTED, errors::kUnverifiedUserWallet);
      break;
    case DialogCloseReason::InsufficientBalance:
      request_->TerminateConnectionWithMessage(
          PaymentErrorReason::NOT_SUPPORTED, errors::kInsufficientBalance);
      break;
  }
}

void CheckoutDialogDelegate::OnCloseContents(WebContents* source,
                                             bool* out_close_dialog) {
  *out_close_dialog = true;
}

bool CheckoutDialogDelegate::ShouldShowDialogTitle() const {
  return false;
}

CheckoutDialogHandler::CheckoutDialogHandler(
    base::WeakPtr<PaymentRequest> request)
    : request_(request),
      weak_factory_(this) {}

CheckoutDialogHandler::~CheckoutDialogHandler() = default;

RewardsService* CheckoutDialogHandler::GetRewardsService() {
  if (!rewards_service_) {
    Profile* profile = Profile::FromWebUI(web_ui());
    rewards_service_ = RewardsServiceFactory::GetForProfile(profile);
    rewards_service_->StartProcess(base::DoNothing());
  }
  return rewards_service_;
}

void CheckoutDialogHandler::RegisterMessages() {
  web_ui()->RegisterMessageCallback(
      "paymentRequestComplete",
      base::BindRepeating(&CheckoutDialogHandler::HandlePaymentCompletion,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "getWalletBalance",
      base::BindRepeating(&CheckoutDialogHandler::OnGetWalletBalance,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "getExternalWallet",
      base::BindRepeating(&CheckoutDialogHandler::GetExternalWallet,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "getRewardsParameters",
      base::BindRepeating(&CheckoutDialogHandler::GetRewardsParameters,
                          base::Unretained(this)));
}

void CheckoutDialogHandler::OnSKUProcessed(const ledger::type::Result result,
                                           const std::string& value) {
  if (result == ledger::type::Result::LEDGER_OK) {
    payments::mojom::PaymentDetailsPtr details =
        payments::mojom::PaymentDetails::New();
    details->id = value;
    request_->spec()->UpdateWith(std::move(details));
    request_->Pay();
    return;
  }
  request_->TerminateConnectionWithMessage(PaymentErrorReason::UNKNOWN,
                                           errors::kBatTransactionFailed);
}

void CheckoutDialogHandler::HandlePaymentCompletion(
    const base::ListValue* args) {
  auto spec = request_->spec();
  if (!request_->spec()->IsInitialized()) {
    return;
  }

  auto* rfh =
      content::RenderFrameHost::FromID(request_->initiator_frame_routing_id());
  if (!rfh) {
    request_->TerminateConnectionWithMessage(
        PaymentErrorReason::INVALID_DATA_FROM_RENDERER,
        errors::kInvalidRenderer);
    return;
  }

  auto* rewards_service = brave_rewards::RewardsServiceFactory::GetForProfile(
      Profile::FromBrowserContext(rfh->GetBrowserContext()));
  if (!rewards_service) {
    request_->TerminateConnectionWithMessage(
        PaymentErrorReason::INVALID_DATA_FROM_RENDERER,
        errors::kRewardsNotInitialized);
    return;
  }

  const auto& display_items =
      spec->GetDisplayItems(request_->state()->selected_app());
  for (size_t i = 0; i < display_items.size(); i++) {
    DCHECK((*display_items[i])->sku.has_value());
    auto item = ledger::type::SKUOrderItem::New();

    item->sku = (*display_items[i])->sku.value();
    item->quantity = 1;
    items_.push_back(std::move(item));
  }

  auto callback = base::BindOnce(&CheckoutDialogHandler::OnSKUProcessed,
                                 base::Unretained(this));

  rewards_service->ProcessSKU(
      std::move(items_), ledger::constant::kWalletUphold, std::move(callback));
}

void CheckoutDialogHandler::GetRewardsParameters(const base::ListValue* args) {
  if (auto* service = GetRewardsService()) {
    AllowJavascript();
    service->GetRewardsParameters(
        base::Bind(&CheckoutDialogHandler::GetRewardsParametersCallback,
                   weak_factory_.GetWeakPtr()));
  }
}

void CheckoutDialogHandler::GetRewardsParametersCallback(
    ledger::type::RewardsParametersPtr parameters) {
  if (!IsJavascriptAllowed()) {
    return;
  }

  base::Value data(base::Value::Type::DICTIONARY);
  if (parameters) {
    data.SetDoubleKey("rate", parameters->rate);
    data.SetDoubleKey("lastUpdated", base::Time::Now().ToJsTimeIgnoringNull());
  }
  FireWebUIListener("rewardsParametersUpdated", data);
}

void CheckoutDialogHandler::OnGetWalletBalance(const base::ListValue* args) {
  if (auto* service = GetRewardsService()) {
    AllowJavascript();
    service->FetchBalance(
        base::BindOnce(&CheckoutDialogHandler::FetchBalanceCallback,
                       weak_factory_.GetWeakPtr()));
  }
}

void CheckoutDialogHandler::FetchBalanceCallback(
    const ledger::type::Result result,
    ledger::type::BalancePtr balance) {
  if (!IsJavascriptAllowed()) {
    return;
  }

  base::Value data(base::Value::Type::DICTIONARY);
  data.SetDoubleKey("total", balance->total);

  FireWebUIListener("walletBalanceUpdated", data);
}

void CheckoutDialogHandler::GetExternalWallet(const base::ListValue* args) {
  if (auto* service = GetRewardsService()) {
    AllowJavascript();
    service->GetExternalWallet(
        base::BindOnce(&CheckoutDialogHandler::GetExternalWalletCallback,
                       weak_factory_.GetWeakPtr()));
  }
}

void CheckoutDialogHandler::GetExternalWalletCallback(
    const ledger::type::Result result,
    ledger::type::ExternalWalletPtr wallet) {
  if (!IsJavascriptAllowed()) {
    return;
  }

  base::Value data(base::Value::Type::DICTIONARY);

  if (wallet) {
    data.SetIntKey("status", static_cast<int>(wallet->status));
  }
  FireWebUIListener("externalWalletUpdated", data);
}

void ShowCheckoutDialog(base::WeakPtr<PaymentRequest> request) {
  double total;
  WebContents* initiator = request->web_contents();
  base::WeakPtr<payments::PaymentRequestSpec> spec = request->spec();
  if (!spec) {
    request->TerminateConnectionWithMessage(
        PaymentErrorReason::INVALID_DATA_FROM_RENDERER, errors::kInvalidData);
    return;
  }

  DCHECK(base::StringToDouble(spec->details().total->amount->value, &total));
  base::Value order_info(base::Value::Type::DICTIONARY);
  order_info.SetDoubleKey("total", total);

  base::Value params(base::Value::Type::DICTIONARY);
  params.SetKey("orderInfo", std::move(order_info));

  ShowConstrainedWebDialogWithAutoResize(
      initiator->GetBrowserContext(),
      std::make_unique<CheckoutDialogDelegate>(std::move(params), request),
      initiator, gfx::Size(kDialogWidth, kDialogMinHeight),
      gfx::Size(kDialogWidth, kDialogMaxHeight));
}

}  // namespace brave_rewards
