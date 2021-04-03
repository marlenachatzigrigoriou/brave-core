/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_shields/browser/ad_block_subscription_service_manager.h"

#include <memory>
#include <utility>
#include <vector>

#include "base/strings/string_util.h"
#include "base/task/post_task.h"
#include "base/values.h"
#include "brave/browser/brave_browser_process_impl.h"
#include "brave/common/pref_names.h"
#include "brave/components/adblock_rust_ffi/src/wrapper.h"
#include "brave/components/brave_shields/browser/ad_block_subscription_service.h"
#include "brave/components/brave_shields/browser/ad_block_service.h"
#include "brave/components/brave_shields/browser/ad_block_service_helper.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"

namespace brave_shields {

AdBlockSubscriptionServiceManager::AdBlockSubscriptionServiceManager(
    brave_component_updater::BraveComponent::Delegate* delegate)
    : delegate_(delegate),
      initialized_(false) {
}

AdBlockSubscriptionServiceManager::~AdBlockSubscriptionServiceManager() {
}

void AdBlockSubscriptionServiceManager::CreateSubscription(const GURL list_url) {
  auto subscription_service = AdBlockSubscriptionServiceFactory(list_url, delegate_);
  subscription_service->Start();
  subscription_services_.insert(std::make_pair(list_url, std::move(subscription_service)));
}

std::vector<FilterListSubscriptionInfo> AdBlockSubscriptionServiceManager::GetSubscriptions() const {
  auto infos = std::vector<FilterListSubscriptionInfo>();

  for (const auto& subscription_service : subscription_services_) {
    infos.push_back(subscription_service.second->GetInfo());
  }

  return infos;
}

void AdBlockSubscriptionServiceManager::EnableSubscription(const SubscriptionIdentifier& id, bool enabled) {
  auto it = subscription_services_.find(id);
  DCHECK(it != subscription_services_.end());
  it->second->SetEnabled(enabled);

  base::PostTask(
      FROM_HERE, {content::BrowserThread::UI},
      base::BindOnce(&AdBlockSubscriptionServiceManager::UpdateFilterListPrefs,
                     base::Unretained(this), id, it->second->GetInfo()));
}

void AdBlockSubscriptionServiceManager::DeleteSubscription(const SubscriptionIdentifier& id) {
  auto it = subscription_services_.find(id);
  DCHECK(it != subscription_services_.end());
  it->second->Unregister();
  subscription_services_.erase(it);
}

void AdBlockSubscriptionServiceManager::RefreshSubscription(const SubscriptionIdentifier& id) {
  auto it = subscription_services_.find(id);
  DCHECK(it != subscription_services_.end());
  it->second->RefreshSubscription();
}

void AdBlockSubscriptionServiceManager::RefreshAllSubscriptions() {
  for (const auto& subscription_service : subscription_services_) {
    subscription_service.second->RefreshSubscription();
  }
}

void AdBlockSubscriptionServiceManager::StartSubscriptionServices() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  PrefService* local_state = g_browser_process->local_state();
  if (!local_state)
    return;

  initialized_ = true;
}

// Updates preferences to reflect a new state for the specified filter list.
// Creates the entry if it does not yet exist.
void AdBlockSubscriptionServiceManager::UpdateFilterListPrefs(
    const SubscriptionIdentifier& id,
    const FilterListSubscriptionInfo& info) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
}

bool AdBlockSubscriptionServiceManager::IsInitialized() const {
  return initialized_;
}

bool AdBlockSubscriptionServiceManager::Start() {
  //base::AutoLock lock(regional_services_lock_);
  for (const auto& subscription_service : subscription_services_) {
    subscription_service.second->Start();
  }

  return true;
}

void AdBlockSubscriptionServiceManager::ShouldStartRequest(
    const GURL& url,
    blink::mojom::ResourceType resource_type,
    const std::string& tab_host,
    bool* did_match_rule,
    bool* did_match_exception,
    bool* did_match_important,
    std::string* mock_data_url) {
  //base::AutoLock lock(regional_services_lock_);
  for (const auto& subscription_service : subscription_services_) {
    if (subscription_service.second->GetInfo().enabled) {
      subscription_service.second->ShouldStartRequest(
          url, resource_type, tab_host, did_match_rule, did_match_exception,
          did_match_important, mock_data_url);
      if (did_match_important && *did_match_important) {
        return;
      }
    }
  }
}

void AdBlockSubscriptionServiceManager::EnableTag(const std::string& tag,
                                              bool enabled) {
  //base::AutoLock lock(regional_services_lock_);
  for (const auto& subscription_service : subscription_services_) {
    subscription_service.second->EnableTag(tag, enabled);
  }
}

void AdBlockSubscriptionServiceManager::AddResources(
    const std::string& resources) {
  //base::AutoLock lock(regional_services_lock_);
  for (const auto& subscription_service : subscription_services_) {
    subscription_service.second->AddResources(resources);
  }
}

base::Optional<base::Value>
AdBlockSubscriptionServiceManager::UrlCosmeticResources(
        const std::string& url) {
  //base::AutoLock lock(regional_services_lock_);
  auto it = subscription_services_.begin();
  if (it == subscription_services_.end()) {
    return base::Optional<base::Value>();
  }
  base::Optional<base::Value> first_value =
      it->second->UrlCosmeticResources(url);

  for ( ; it != subscription_services_.end(); it++) {
    base::Optional<base::Value> next_value =
        it->second->UrlCosmeticResources(url);
    if (first_value) {
      if (next_value) {
        MergeResourcesInto(std::move(*next_value), &*first_value, false);
      }
    } else {
      first_value = std::move(next_value);
    }
  }

  return first_value;
}

base::Optional<base::Value>
AdBlockSubscriptionServiceManager::HiddenClassIdSelectors(
        const std::vector<std::string>& classes,
        const std::vector<std::string>& ids,
        const std::vector<std::string>& exceptions) {
  //base::AutoLock lock(regional_services_lock_);
  auto it = subscription_services_.begin();
  if (it == subscription_services_.end()) {
    return base::Optional<base::Value>();
  }
  base::Optional<base::Value> first_value =
      it->second->HiddenClassIdSelectors(classes, ids, exceptions);

  for ( ; it != subscription_services_.end(); it++) {
    base::Optional<base::Value> next_value =
        it->second->HiddenClassIdSelectors(classes, ids, exceptions);
    if (first_value && first_value->is_list()) {
      if (next_value && next_value->is_list()) {
        for (auto i = next_value->GetList().begin();
                i < next_value->GetList().end();
                i++) {
          first_value->Append(std::move(*i));
        }
      }
    } else {
      first_value = std::move(next_value);
    }
  }

  return first_value;
}

///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<AdBlockSubscriptionServiceManager>
AdBlockSubscriptionServiceManagerFactory(BraveComponent::Delegate* delegate) {
  return std::make_unique<AdBlockSubscriptionServiceManager>(delegate);
}

}  // namespace brave_shields
