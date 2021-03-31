/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_CUSTOM_SUBSCRIPTION_DOWNLOAD_CLIENT_H_
#define BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_CUSTOM_SUBSCRIPTION_DOWNLOAD_CLIENT_H_

#include "components/download/public/background_service/client.h"

class Profile;

namespace download {
struct CompletionInfo;
struct DownloadMetaData;
}  // namespace download

namespace brave_shields {

class CustomSubscriptionDownloadManager;

class CustomSubscriptionDownloadClient : public download::Client {
 public:
  explicit CustomSubscriptionDownloadClient(/*Profile* profile*/);
  ~CustomSubscriptionDownloadClient() override;
  CustomSubscriptionDownloadClient(const CustomSubscriptionDownloadClient&) = delete;
  CustomSubscriptionDownloadClient& operator=(
      const CustomSubscriptionDownloadClient&) = delete;

  // download::Client:
  void OnServiceInitialized(
      bool state_lost,
      const std::vector<download::DownloadMetaData>& downloads) override;
  void OnServiceUnavailable() override;
  void OnDownloadStarted(
      const std::string& guid,
      const std::vector<GURL>& url_chain,
      const scoped_refptr<const net::HttpResponseHeaders>& headers) override;
  void OnDownloadFailed(const std::string& guid,
                        const download::CompletionInfo& completion_info,
                        download::Client::FailureReason reason) override;
  void OnDownloadSucceeded(
      const std::string& guid,
      const download::CompletionInfo& completion_info) override;
  bool CanServiceRemoveDownloadedFile(const std::string& guid,
                                      bool force_delete) override;
  void GetUploadData(const std::string& guid,
                     download::GetUploadDataCallback callback) override;

 private:
  // Returns the CustomSubscriptionDownloadManager for the profile.
  CustomSubscriptionDownloadManager* GetCustomSubscriptionDownloadManager();

  //Profile* profile_;
};

}  // namespace brave_shields

#endif  // BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_CUSTOM_SUBSCRIPTION_DOWNLOAD_CLIENT_H_
