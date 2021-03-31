/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_shields/browser/custom_subscription_download_manager.h"

#include "base/bind.h"
#include "base/files/file_util.h"
#include "base/guid.h"
#include "base/metrics/histogram_functions.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/post_task.h"
#include "base/task/thread_pool.h"
#include "build/build_config.h"
//#include "chrome/browser/optimization_guide/prediction/prediction_model_download_observer.h"
#include "components/crx_file/crx_verifier.h"
#include "components/download/public/background_service/download_service.h"
//#include "components/optimization_guide/core/optimization_guide_enums.h"
//#include "components/optimization_guide/core/optimization_guide_features.h"
//#include "components/optimization_guide/core/optimization_guide_switches.h"
//#include "components/optimization_guide/core/optimization_guide_util.h"
#include "components/services/unzip/content/unzip_service.h"
#include "components/services/unzip/public/cpp/unzip.h"
#include "crypto/sha2.h"
#include "net/traffic_annotation/network_traffic_annotation.h"

namespace brave_shields {

namespace {

// Header for API key.
//constexpr char kGoogApiKey[] = "X-Goog-Api-Key";

// The SHA256 hash of the public key for the Optimization Guide Server that
// we require models to come from.
/*constexpr uint8_t kPublisherKeyHash[] = {
    0x66, 0xa1, 0xd9, 0x3e, 0x4e, 0x5a, 0x66, 0x8a, 0x0f, 0xd3, 0xfa,
    0xa3, 0x70, 0x71, 0x42, 0x16, 0x0d, 0x2d, 0x68, 0xb0, 0x53, 0x02,
    0x5c, 0x7f, 0xd0, 0x0c, 0xa1, 0x6e, 0xef, 0xdd, 0x63, 0x7a};*/
const net::NetworkTrafficAnnotationTag
    kBraveShieldsCustomSubscriptionTrafficAnnotation =
        net::DefineNetworkTrafficAnnotation("brave_shields_custom_subscription",
                                            R"(
        semantics {
          sender: "Brave Shields"
          description:
            "Brave periodically downloads updates to third-party filter lists "
            "added by users on brave://adblock."
          trigger:
            "After being registered in brave://adblock, any enabled filter "
            "list subscriptions will be updated in accordance with their "
            "`Expires` field if present, or daily otherwise. A manual refresh "
            "for a particular list can also be triggered in brave://adblock."
          data: "The URL endpoint provided by the user in brave://adblock to "
            "fetch list updates from. No user information is sent."
          destination: BRAVE_OWNED_SERVICE
        }
        policy {
          cookies_allowed: NO
          setting:
            "This request cannot be disabled in settings. However it will "
            "never be made if the corresponding entry is removed from the "
            "brave://adblock page's custom list subscription section."
          policy_exception_justification: "Not yet implemented."
        })");

const base::FilePath::CharType kModelInfoFileName[] =
    FILE_PATH_LITERAL("model-info.pb");
const base::FilePath::CharType kModelFileName[] =
    FILE_PATH_LITERAL("model.tflite");

bool IsRelevantFile(const base::FilePath& file_path) {
  base::FilePath::StringType base_name_value = file_path.BaseName().value();
  return base_name_value == kModelFileName ||
         base_name_value == kModelInfoFileName;
}

/*base::FilePath GetFilePathForModelInfo(const base::FilePath& dir,
                                       const proto::ModelInfo& model_info) {
  return dir.AppendASCII(base::StringPrintf(
      "%s_%s.tflite",
      proto::OptimizationTarget_Name(model_info.optimization_target()).c_str(),
      base::NumberToString(model_info.version()).c_str()));
}*/

/*void RecordCustomSubscriptionDownloadStatus(CustomSubscriptionDownloadStatus status) {
  base::UmaHistogramEnumeration(
      "BraveShields.CustomSubscriptionDownloadManager."
      "DownloadStatus",
      status);
}*/

}  // namespace

CustomSubscriptionDownloadManager::CustomSubscriptionDownloadManager(
    download::DownloadService* download_service,
    //const base::FilePath& models_dir,
    scoped_refptr<base::SequencedTaskRunner> background_task_runner)
    : download_service_(download_service),
      //models_dir_(models_dir),
      is_available_for_downloads_(true),
      //api_key_(features::GetOptimizationGuideServiceAPIKey()),
      background_task_runner_(background_task_runner) {}

CustomSubscriptionDownloadManager::~CustomSubscriptionDownloadManager() = default;

void CustomSubscriptionDownloadManager::StartDownload(const GURL& download_url) {
  download::DownloadParams download_params;
  download_params.client =
      download::DownloadClient::CUSTOM_LIST_SUBSCRIPTIONS;
  download_params.guid = base::GenerateGUID();
  download_params.callback =
      base::BindRepeating(&CustomSubscriptionDownloadManager::OnDownloadStarted,
                          ui_weak_ptr_factory_.GetWeakPtr());
  download_params.traffic_annotation = net::MutableNetworkTrafficAnnotationTag(
      kBraveShieldsCustomSubscriptionTrafficAnnotation);
  download_params.request_params.url = download_url;
  download_params.request_params.method = "GET";
  /*download_params.request_params.request_headers.SetHeader(kGoogApiKey,
                                                           api_key_);*/
  /*if (features::IsUnrestrictedModelDownloadingEnabled()) {
    // This feature param should really only be used for testing, so it is ok
    // to have this be a high priority download with no network restrictions.
    download_params.scheduling_params.priority =
        download::SchedulingParams::Priority::HIGH;
    download_params.scheduling_params.battery_requirements =
        download::SchedulingParams::BatteryRequirements::BATTERY_INSENSITIVE;
    download_params.scheduling_params.network_requirements =
        download::SchedulingParams::NetworkRequirements::NONE;
  } else {*/
    download_params.scheduling_params.priority =
        download::SchedulingParams::Priority::NORMAL;
    download_params.scheduling_params.battery_requirements =
        download::SchedulingParams::BatteryRequirements::BATTERY_INSENSITIVE;
    download_params.scheduling_params.network_requirements =
        download::SchedulingParams::NetworkRequirements::OPTIMISTIC;
  //}

  download_service_->StartDownload(download_params);
}

void CustomSubscriptionDownloadManager::CancelAllPendingDownloads() {
  for (const std::string& pending_download_guid : pending_download_guids_)
    download_service_->CancelDownload(pending_download_guid);
}

bool CustomSubscriptionDownloadManager::IsAvailableForDownloads() const {
  return is_available_for_downloads_;
}

/*void CustomSubscriptionDownloadManager::AddObserver(
    CustomSubscriptionDownloadObserver* observer) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  observers_.AddObserver(observer);
}

void CustomSubscriptionDownloadManager::RemoveObserver(
    CustomSubscriptionDownloadObserver* observer) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  observers_.RemoveObserver(observer);
}*/

void CustomSubscriptionDownloadManager::OnDownloadServiceReady(
    const std::set<std::string>& pending_download_guids,
    const std::map<std::string, base::FilePath>& successful_downloads) {
  for (const std::string& pending_download_guid : pending_download_guids) {
    DCHECK(false);  // TODO
    pending_download_guids_.insert(pending_download_guid);
  }

  // Successful downloads should already be notified via |onDownloadSucceeded|,
  // so we don't do anything with them here.
}

void CustomSubscriptionDownloadManager::OnDownloadServiceUnavailable() {
  is_available_for_downloads_ = false;
}

void CustomSubscriptionDownloadManager::OnDownloadStarted(
    const std::string& guid,
    download::DownloadParams::StartResult start_result) {
  if (start_result == download::DownloadParams::StartResult::ACCEPTED) {
    LOG(ERROR) << "download accepted with guid " << guid;
    pending_download_guids_.insert(guid);
  }
}

void CustomSubscriptionDownloadManager::OnDownloadSucceeded(
    const std::string& guid,
    const base::FilePath& file_path) {
  pending_download_guids_.erase(guid);

  base::UmaHistogramBoolean(
      "BraveShields.CustomSubscriptionDownloadManager.DownloadSucceeded",
      true);

  background_task_runner_->PostTaskAndReplyWithResult(
      FROM_HERE,
      base::BindOnce(&CustomSubscriptionDownloadManager::ProcessDownload,
                     base::Unretained(this), file_path),
      base::BindOnce(&CustomSubscriptionDownloadManager::StartUnzipping,
                     ui_weak_ptr_factory_.GetWeakPtr()));
}

void CustomSubscriptionDownloadManager::OnDownloadFailed(const std::string& guid) {
  pending_download_guids_.erase(guid);

  base::UmaHistogramBoolean(
      "BraveShields.CustomSubscriptionDownloadManager.DownloadSucceeded",
      false);
}

base::Optional<std::pair<base::FilePath, base::FilePath>>
CustomSubscriptionDownloadManager::ProcessDownload(
    const base::FilePath& file_path) {
  DCHECK(background_task_runner_->RunsTasksInCurrentSequence());

  LOG(ERROR) << "processed download";

  /*if (!switches::ShouldSkipModelDownloadVerificationForTesting()) {
    // Verify that the |file_path| contains a valid CRX file.
    std::string public_key;
    crx_file::VerifierResult verifier_result = crx_file::Verify(
        file_path, crx_file::VerifierFormat::CRX3,
        //required_key_hashes=//{},
        //required_file_hash=//{}, &public_key,
        //crx_id=//nullptr, //compressed_verified_contents=//nullptr);
    if (verifier_result != crx_file::VerifierResult::OK_FULL) {
      RecordCustomSubscriptionDownloadStatus(
          CustomSubscriptionDownloadStatus::kFailedCrxVerification);
      base::ThreadPool::PostTask(
          FROM_HERE, {base::TaskPriority::BEST_EFFORT, base::MayBlock()},
          base::BindOnce(base::GetDeleteFileCallback(), file_path));
      return base::nullopt;
    }

    // Verify that the CRX3 file is from a publisher we trust.
    std::vector<uint8_t> publisher_key_hash(std::begin(kPublisherKeyHash),
                                            std::end(kPublisherKeyHash));

    std::vector<uint8_t> public_key_hash(crypto::kSHA256Length);
    crypto::SHA256HashString(public_key, public_key_hash.data(),
                             public_key_hash.size());

    if (publisher_key_hash != public_key_hash) {
      RecordCustomSubscriptionDownloadStatus(
          CustomSubscriptionDownloadStatus::kFailedCrxInvalidPublisher);
      base::ThreadPool::PostTask(
          FROM_HERE, {base::TaskPriority::BEST_EFFORT, base::MayBlock()},
          base::BindOnce(base::GetDeleteFileCallback(), file_path));
      return base::nullopt;
    }
  }*/

  // Unzip download.
  base::FilePath temp_dir_path;
  if (!base::CreateNewTempDirectory(base::FilePath::StringType(),
                                    &temp_dir_path)) {
    /*RecordCustomSubscriptionDownloadStatus(
        CustomSubscriptionDownloadStatus::kFailedUnzipDirectoryCreation);*/
    base::ThreadPool::PostTask(
        FROM_HERE, {base::TaskPriority::BEST_EFFORT, base::MayBlock()},
        base::BindOnce(base::GetDeleteFileCallback(), file_path));
    return base::nullopt;
  }

  LOG(ERROR) << "Processing download: " << file_path << " " << temp_dir_path;

  return std::make_pair(file_path, temp_dir_path);
}

void CustomSubscriptionDownloadManager::StartUnzipping(
    const base::Optional<std::pair<base::FilePath, base::FilePath>>&
        unzip_paths) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!unzip_paths)
    return;

  unzip::UnzipWithFilter(
      unzip::LaunchUnzipper(), unzip_paths->first, unzip_paths->second,
      base::BindRepeating(&IsRelevantFile),
      base::BindOnce(&CustomSubscriptionDownloadManager::OnDownloadUnzipped,
                     ui_weak_ptr_factory_.GetWeakPtr(), unzip_paths->first,
                     unzip_paths->second));
}

void CustomSubscriptionDownloadManager::OnDownloadUnzipped(
    const base::FilePath& original_file_path,
    const base::FilePath& unzipped_dir_path,
    bool success) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  LOG(ERROR) << "Unzipped: " << original_file_path << " " << unzipped_dir_path << " " << success;

  // Clean up original download file when this function finishes.
  background_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(base::GetDeleteFileCallback(), original_file_path));

  if (!success) {
    /*RecordCustomSubscriptionDownloadStatus(
        CustomSubscriptionDownloadStatus::kFailedCrxUnzip);*/
    return;
  }

  background_task_runner_->PostTaskAndReplyWithResult(
      FROM_HERE,
      base::BindOnce(&CustomSubscriptionDownloadManager::ProcessUnzippedContents,
                     base::Unretained(this), unzipped_dir_path),
      base::BindOnce(&CustomSubscriptionDownloadManager::NotifyModelReady,
                     ui_weak_ptr_factory_.GetWeakPtr()));
}

base::Optional</*proto::PredictionModel*/int>
CustomSubscriptionDownloadManager::ProcessUnzippedContents(
    const base::FilePath& unzipped_dir_path) {
  DCHECK(background_task_runner_->RunsTasksInCurrentSequence());

  return base::nullopt;
  /*
  // Clean up temp dir when this function finishes.
  base::SequencedTaskRunnerHandle::Get()->PostTask(
      FROM_HERE, base::BindOnce(base::GetDeletePathRecursivelyCallback(),
                                unzipped_dir_path));

  // Unpack and verify model info file.
  base::FilePath model_info_path = unzipped_dir_path.Append(kModelInfoFileName);
  std::string binary_model_info_pb;
  if (!base::ReadFileToString(model_info_path, &binary_model_info_pb)) {
    RecordCustomSubscriptionDownloadStatus(
        CustomSubscriptionDownloadStatus::kFailedModelInfoFileRead);
    return base::nullopt;
  }
  proto::ModelInfo model_info;
  if (!model_info.ParseFromString(binary_model_info_pb)) {
    RecordCustomSubscriptionDownloadStatus(
        CustomSubscriptionDownloadStatus::kFailedModelInfoParsing);
    return base::nullopt;
  }
  if (!model_info.has_version() || !model_info.has_optimization_target()) {
    RecordCustomSubscriptionDownloadStatus(
        CustomSubscriptionDownloadStatus::kFailedModelInfoInvalid);
    return base::nullopt;
  }

  // Move model file away from temp directory.
  base::FilePath temp_model_path = unzipped_dir_path.Append(kModelFileName);
  base::FilePath model_path = GetFilePathForModelInfo(models_dir_, model_info);
  base::File::Error file_error;
  if (!base::ReplaceFile(temp_model_path, model_path, &file_error)) {
    if (file_error == base::File::FILE_ERROR_NOT_FOUND) {
      RecordCustomSubscriptionDownloadStatus(
          CustomSubscriptionDownloadStatus::kFailedModelFileNotFound);
    } else {
      RecordCustomSubscriptionDownloadStatus(
          CustomSubscriptionDownloadStatus::kFailedModelFileOtherError);
    }
    return base::nullopt;
  }

  RecordCustomSubscriptionDownloadStatus(CustomSubscriptionDownloadStatus::kSuccess);

  proto::PredictionModel model;
  *model.mutable_model_info() = model_info;
  SetFilePathInPredictionModel(model_path, &model);
  return model;
  */
}

void CustomSubscriptionDownloadManager::NotifyModelReady(
    const base::Optional</*proto::PredictionModel*/int>& model) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!model)
    return;

  /*for (CustomSubscriptionDownloadObserver& observer : observers_)
    observer.OnModelReady(*model);*/
}

}  // namespace brave_shields
