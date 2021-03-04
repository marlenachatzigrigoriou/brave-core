/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_shields/browser/ad_block_subscription_service.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "base/base_paths.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/thread_restrictions.h"
#include "brave/browser/brave_browser_process_impl.h"
#include "brave/common/pref_names.h"
//#include "brave/components/brave_shields/browser/ad_block_regional_service_manager.h"
#include "brave/components/brave_shields/browser/ad_block_service.h"
#include "brave/components/brave_shields/browser/ad_block_service_helper.h"
//#include "brave/vendor/adblock_rust_ffi/src/wrapper.h"
#include "components/prefs/pref_service.h"

namespace brave_shields {

std::string AdBlockSubscriptionService::g_ad_block_regional_component_id_;  // NOLINT
std::string
    AdBlockSubscriptionService::g_ad_block_regional_component_base64_public_key_;  // NOLINT

// Constructor for a new subscription
AdBlockSubscriptionService::AdBlockSubscriptionService(
      const GURL& list_url,
      brave_component_updater::BraveComponent::Delegate* delegate) :
  AdBlockBaseService(delegate),
  list_url_(list_url),
  enabled_(true) { // TODO only enable after successful update?
}

// Constructor from cached information
AdBlockSubscriptionService::AdBlockSubscriptionService(
      const FilterListSubscriptionInfo& cached_info,
      brave_component_updater::BraveComponent::Delegate* delegate) :
  AdBlockBaseService(delegate),
  title_(cached_info.title),
  list_url_(cached_info.list_url),
  enabled_(cached_info.enabled),
  last_update_attempt_(cached_info.last_update_attempt),
  last_update_was_successful_(cached_info.last_update_was_successful) {
}

AdBlockSubscriptionService::~AdBlockSubscriptionService() {
}

FilterListSubscriptionInfo AdBlockSubscriptionService::GetInfo() const {
  return FilterListSubscriptionInfo {
    .title = title_,
    .list_url = list_url_,
    .last_update_attempt = last_update_attempt_,
    .last_update_was_successful = last_update_was_successful_,
    .enabled = enabled_,
  };
}

void AdBlockSubscriptionService::SetEnabled(bool enabled) {
  enabled_ = enabled;
}

void AdBlockSubscriptionService::RefreshSubscription() {
  // TODO
  last_update_attempt_ = base::Time::Now();
  last_update_was_successful_ = true;
}

bool AdBlockSubscriptionService::Init() {
  AdBlockBaseService::Init();

  Register(title_,
           !g_ad_block_regional_component_id_.empty()
               ? g_ad_block_regional_component_id_
               : component_id_,
           !g_ad_block_regional_component_base64_public_key_.empty()
               ? g_ad_block_regional_component_base64_public_key_
               : base64_public_key_);

  return true;
}

void AdBlockSubscriptionService::OnComponentReady(
    const std::string& component_id,
    const base::FilePath& install_dir,
    const std::string& manifest) {
  /*base::FilePath dat_file_path =
      install_dir.AppendASCII(std::string("rs-") + uuid_)
          .AddExtension(FILE_PATH_LITERAL(".dat"));
  GetDATFileData(dat_file_path);
  base::FilePath resources_file_path =
      install_dir.AppendASCII(kAdBlockResourcesFilename);*/

  /*base::PostTaskAndReplyWithResult(
      GetTaskRunner().get(), FROM_HERE,
      base::BindOnce(&brave_component_updater::GetDATFileAsString,
                     resources_file_path),
      base::BindOnce(&AdBlockSubscriptionService::OnResourcesFileDataReady,
                     weak_factory_.GetWeakPtr()));*/
}

/*void AdBlockSubscriptionService::OnResourcesFileDataReady(
    const std::string& resources) {
  g_brave_browser_process->ad_block_regional_service_manager()->AddResources(
      resources);
}*/

// static
void AdBlockSubscriptionService::SetComponentIdAndBase64PublicKeyForTest(
    const std::string& component_id,
    const std::string& component_base64_public_key) {
  g_ad_block_regional_component_id_ = component_id;
  g_ad_block_regional_component_base64_public_key_ =
      component_base64_public_key;
}

///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<AdBlockSubscriptionService> AdBlockSubscriptionServiceFactory(
    const GURL& list_url,
    brave_component_updater::BraveComponent::Delegate* delegate) {
  return std::make_unique<AdBlockSubscriptionService>(list_url, delegate);
}

std::unique_ptr<AdBlockSubscriptionService> AdBlockSubscriptionServiceFactory(
    const FilterListSubscriptionInfo& cached_info,
    brave_component_updater::BraveComponent::Delegate* delegate) {
  auto service = std::make_unique<AdBlockSubscriptionService>(cached_info.list_url, delegate);
  // TODO add argument for DAT and initialize the service with it
  return service;
}

}  // namespace brave_shields
