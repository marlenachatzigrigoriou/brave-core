/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_shields/browser/custom_subscription_download_client.h"

#include "base/bind.h"
//#include "base/metrics/histogram_macros_local.h"
#include "base/threading/sequenced_task_runner_handle.h"
/*#include "chrome/browser/optimization_guide/optimization_guide_keyed_service.h"
#include "chrome/browser/optimization_guide/optimization_guide_keyed_service_factory.h"
#include "chrome/browser/optimization_guide/prediction/prediction_manager.h"
#include "chrome/browser/optimization_guide/prediction/prediction_model_download_manager.h"*/
#include "components/download/public/background_service/download_metadata.h"
#include "services/network/public/cpp/resource_request_body.h"

#include "brave/components/brave_shields/browser/ad_block_service.h"
#include "brave/components/brave_shields/browser/ad_block_subscription_service_manager.h"
#include "brave/components/brave_shields/browser/custom_subscription_download_manager.h"
#include "brave/browser/brave_browser_process_impl.h"

namespace brave_shields {

CustomSubscriptionDownloadClient::CustomSubscriptionDownloadClient(/*Profile* profile*//*SimpleFactoryKey* key*/)
    /*: profile_(profile)*/ {}

CustomSubscriptionDownloadClient::~CustomSubscriptionDownloadClient() = default;

CustomSubscriptionDownloadManager*
CustomSubscriptionDownloadClient::GetCustomSubscriptionDownloadManager() {
  AdBlockService* ad_block_service = g_brave_browser_process->ad_block_service();
  AdBlockSubscriptionServiceManager* subscription_service_manager = ad_block_service->subscription_service_manager();
  return subscription_service_manager->download_manager();
}

void CustomSubscriptionDownloadClient::OnServiceInitialized(
    bool state_lost,
    const std::vector<download::DownloadMetaData>& downloads) {
  CustomSubscriptionDownloadManager* download_manager =
      GetCustomSubscriptionDownloadManager();
  if (!download_manager)
    return;

  std::set<std::string> outstanding_download_guids;
  std::map<std::string, base::FilePath> successful_downloads;
  for (const auto& download : downloads) {
    if (!download.completion_info) {
      outstanding_download_guids.emplace(download.guid);
      continue;
    }

    successful_downloads.emplace(download.guid, download.completion_info->path);
  }

  download_manager->OnDownloadServiceReady(outstanding_download_guids,
                                           successful_downloads);
}

void CustomSubscriptionDownloadClient::OnServiceUnavailable() {
  CustomSubscriptionDownloadManager* download_manager =
      GetCustomSubscriptionDownloadManager();
  if (download_manager)
    download_manager->OnDownloadServiceUnavailable();
}

void CustomSubscriptionDownloadClient::OnDownloadStarted(
    const std::string& guid,
    const std::vector<GURL>& url_chain,
    const scoped_refptr<const net::HttpResponseHeaders>& headers) {
  // Do not remove. This is a hook used by integration tests that test
  // client-server interaction.
  /*LOCAL_HISTOGRAM_BOOLEAN(
      "OptimizationGuide.PredictionModelDownloadClient.DownloadStarted", true);*/
}

void CustomSubscriptionDownloadClient::OnDownloadFailed(
    const std::string& guid,
    const download::CompletionInfo& completion_info,
    download::Client::FailureReason reason) {
  CustomSubscriptionDownloadManager* download_manager =
      GetCustomSubscriptionDownloadManager();
  if (download_manager)
    download_manager->OnDownloadFailed(guid);
}

void CustomSubscriptionDownloadClient::OnDownloadSucceeded(
    const std::string& guid,
    const download::CompletionInfo& completion_info) {
  CustomSubscriptionDownloadManager* download_manager =
      GetCustomSubscriptionDownloadManager();
  if (download_manager)
    download_manager->OnDownloadSucceeded(guid, completion_info.path);
}

bool CustomSubscriptionDownloadClient::CanServiceRemoveDownloadedFile(
    const std::string& guid,
    bool force_delete) {
  // Always return true. We immediately postprocess successful downloads and the
  // file downloaded by the Download Service should already be deleted and this
  // hypothetically should never be called with anything that matters.
  return true;
}

void CustomSubscriptionDownloadClient::GetUploadData(
    const std::string& guid,
    download::GetUploadDataCallback callback) {
  base::SequencedTaskRunnerHandle::Get()->PostTask(
      FROM_HERE, base::BindOnce(std::move(callback), nullptr));
}

}  // namespace brave_shields
