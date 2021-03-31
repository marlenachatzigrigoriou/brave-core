/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_CUSTOM_SUBSCRIPTION_DOWNLOAD_MANAGER_H_
#define BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_CUSTOM_SUBSCRIPTION_DOWNLOAD_MANAGER_H_

#include <map>
#include <set>
#include <string>

#include "base/memory/weak_ptr.h"
//#include "base/observer_list.h"
#include "components/download/public/background_service/download_params.h"

namespace download {
class DownloadService;
}  // namespace download

namespace brave_shields {

class CustomSubscriptionDownloadClient;
//class CustomSubscriptionDownloadObserver;

namespace proto {
class PredictionModel;
}  // namespace proto

// Manages the downloads of prediction models.
class CustomSubscriptionDownloadManager {
 public:
  CustomSubscriptionDownloadManager(
      download::DownloadService* download_service,
      //const base::FilePath& models_dir,
      scoped_refptr<base::SequencedTaskRunner> background_task_runner);
  virtual ~CustomSubscriptionDownloadManager();
  CustomSubscriptionDownloadManager(const CustomSubscriptionDownloadManager&) =
      delete;
  CustomSubscriptionDownloadManager& operator=(
      const CustomSubscriptionDownloadManager&) = delete;

  // Starts a download for |download_url|.
  virtual void StartDownload(const GURL& download_url);

  // Cancels all pending downloads.
  virtual void CancelAllPendingDownloads();

  // Returns whether the downloader can download models.
  virtual bool IsAvailableForDownloads() const;

  // Adds and removes observers.
  //
  // All methods called on observers will be invoked on the UI thread.
  //virtual void AddObserver(CustomSubscriptionDownloadObserver* observer);
  //virtual void RemoveObserver(CustomSubscriptionDownloadObserver* observer);

 private:
  friend class CustomSubscriptionDownloadClient;
  friend class CustomSubscriptionDownloadManagerTest;

  // Invoked when the Download Service is ready.
  //
  // |pending_download_guids| is the set of GUIDs that were previously scheduled
  // to be downloaded and have still not been downloaded yet.
  // |successful_downloads| is the map from GUID to the file path that it was
  // successfully downloaded to.
  void OnDownloadServiceReady(
      const std::set<std::string>& pending_download_guids,
      const std::map<std::string, base::FilePath>& successful_downloads);

  // Invoked when the Download Service fails to initialize and should not be
  // used for the session.
  void OnDownloadServiceUnavailable();

  // Invoked when the download has been accepted and persisted by the
  // DownloadService.
  void OnDownloadStarted(const std::string& guid,
                         download::DownloadParams::StartResult start_result);

  // Invoked when the download as specified by |downloaded_guid| succeeded.
  void OnDownloadSucceeded(const std::string& downloaded_guid,
                           const base::FilePath& file_path);

  // Invoked when the download as specified by |failed_download_guid| failed.
  void OnDownloadFailed(const std::string& failed_download_guid);

  // Verifies the download came from a trusted source and process the downloaded
  // contents. Returns a pair of file paths of the form (src, dst) if
  // |file_path| is successfully verified.
  //
  // Must be called on the background thread, as it performs file I/O.
  base::Optional<std::pair<base::FilePath, base::FilePath>> ProcessDownload(
      const base::FilePath& file_path);

  // Starts unzipping the contents of |unzip_paths|, if present. |unzip_paths|
  // is a pair of the form (src, dst), if present.
  void StartUnzipping(
      const base::Optional<std::pair<base::FilePath, base::FilePath>>&
          unzip_paths);

  // Invoked when the contents of |original_file_path| have been unzipped to
  // |unzipped_dir_path|.
  void OnDownloadUnzipped(const base::FilePath& original_file_path,
                          const base::FilePath& unzipped_dir_path,
                          bool success);

  // Processes the contents in |unzipped_dir_path|.
  //
  // Must be called on the background thread, as it performs file I/O.
  base::Optional</*proto::PredictionModel*/int> ProcessUnzippedContents(
      const base::FilePath& unzipped_dir_path);

  // Notifies |observers_| that a model is ready.
  //
  // Must be invoked on the UI thread.
  void NotifyModelReady(const base::Optional</*proto::PredictionModel*/int>& model);

  // The set of GUIDs that are still pending download.
  std::set<std::string> pending_download_guids_;

  // The Download Service to schedule model downloads with.
  //
  // Guaranteed to outlive |this|.
  download::DownloadService* download_service_;

  // The directory to store verified models in.
  base::FilePath models_dir_;

  // Whether the download service is available.
  bool is_available_for_downloads_;

  // The API key to attach to download requests.
  std::string api_key_;

  // The set of observers to be notified of completed downloads.
  //base::ObserverList<CustomSubscriptionDownloadObserver> observers_;

  // Whether the download should be verified. Should only be false for testing.
  bool should_verify_download_ = true;

  // Background thread where download file processing should be performed.
  scoped_refptr<base::SequencedTaskRunner> background_task_runner_;

  // Sequence checker used to verify all public API methods are called on the
  // UI thread.
  SEQUENCE_CHECKER(sequence_checker_);

  // Used to get weak ptr to self on the UI thread.
  base::WeakPtrFactory<CustomSubscriptionDownloadManager> ui_weak_ptr_factory_{
      this};
};

}  // namespace brave_shields

#endif  // BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_CUSTOM_SUBSCRIPTION_DOWNLOAD_MANAGER_H_
