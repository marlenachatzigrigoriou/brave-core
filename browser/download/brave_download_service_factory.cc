/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/download/brave_download_service_factory.h"

#include <memory>
#include <utility>

#include "base/feature_list.h"
#include "base/files/file_path.h"
#include "base/memory/singleton.h"
#include "base/sequenced_task_runner.h"
#include "base/single_thread_task_runner.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool.h"
#include "build/chromeos_buildflags.h"
//#include "chrome/browser/ash/plugin_vm/plugin_vm_image_download_client.h"
//#include "chrome/browser/background_fetch/background_fetch_download_client.h"
#include "chrome/browser/download/deferred_client_wrapper.h"
#include "chrome/browser/download/download_manager_utils.h"
#include "chrome/browser/download/download_task_scheduler_impl.h"
#include "chrome/browser/download/simple_download_manager_coordinator_factory.h"
#include "chrome/browser/net/system_network_context_manager.h"
//#include "chrome/browser/optimization_guide/prediction/prediction_model_download_client.h"
#include "chrome/browser/profiles/incognito_helpers.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_key.h"
#include "chrome/browser/transition_manager/full_browser_transition_manager.h"
#include "chrome/common/chrome_constants.h"
#include "components/download/content/factory/download_service_factory_helper.h"
#include "components/download/content/factory/navigation_monitor_factory.h"
//#include "components/download/public/background_service/basic_task_scheduler.h"
#include "components/download/public/background_service/blob_context_getter_factory.h"

//#include "brave/components/download/public/background_service/clients.h"    // TODO move
#include "components/download/public/background_service/clients.h"    // TODO ??

#include "components/download/public/background_service/download_service.h"
#include "components/download/public/background_service/features.h"
#include "components/download/public/common/simple_download_manager_coordinator.h"
#include "components/keyed_service/core/simple_dependency_manager.h"
#include "components/leveldb_proto/public/proto_database_provider.h"
#include "components/offline_pages/buildflags/buildflags.h"
#include "components/optimization_guide/core/optimization_guide_features.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/download_manager.h"
#include "content/public/browser/network_service_instance.h"
#include "content/public/browser/storage_partition.h"

#include "brave/components/brave_shields/browser/custom_subscription_download_client.h"

/*#if defined(OS_ANDROID)
#include "chrome/browser/download/android/service/download_task_scheduler.h"
#endif*/

/*#if BUILDFLAG(ENABLE_OFFLINE_PAGES)
#include "chrome/browser/offline_pages/prefetch/offline_prefetch_download_client.h"
#endif*/

namespace {

/*std::unique_ptr<download::Client> CreateCustomSubscriptionDownloadClient(Profile* profile) {
  LOG(ERROR) << "Creating custom subscription download client";
  return std::make_unique<brave_shields::CustomSubscriptionDownloadClient>(profile);
}*/

/*std::unique_ptr<download::Client> CreateBackgroundFetchDownloadClient(
    Profile* profile) {
  return std::make_unique<BackgroundFetchDownloadClient>(profile);
}

#if BUILDFLAG(IS_CHROMEOS_ASH)
std::unique_ptr<download::Client> CreatePluginVmImageDownloadClient(
    Profile* profile) {
  return std::make_unique<plugin_vm::PluginVmImageDownloadClient>(profile);
}
#endif  // BUILDFLAG(IS_CHROMEOS_ASH)

std::unique_ptr<download::Client>
CreateOptimizationGuidePredictionModelDownloadClient(Profile* profile) {
  return std::make_unique<optimization_guide::PredictionModelDownloadClient>(
      profile);
}*/

// Called on profile created to retrieve the BlobStorageContextGetter.
void DownloadOnProfileCreated(download::BlobContextGetterCallback callback,
                              Profile* profile) {
  auto blob_context_getter =
      content::BrowserContext::GetBlobStorageContext(profile);
  DCHECK(callback);
  std::move(callback).Run(blob_context_getter);
}

// Provides BlobContextGetter from Chrome asynchronously.
class DownloadBlobContextGetterFactory
    : public download::BlobContextGetterFactory {
 public:
  explicit DownloadBlobContextGetterFactory(SimpleFactoryKey* key) : key_(key) {
    DCHECK(key_);
  }
  ~DownloadBlobContextGetterFactory() override = default;

 private:
  // download::BlobContextGetterFactory implementation.
  void RetrieveBlobContextGetter(
      download::BlobContextGetterCallback callback) override {
    FullBrowserTransitionManager::Get()->RegisterCallbackOnProfileCreation(
        key_, base::BindOnce(&DownloadOnProfileCreated, std::move(callback)));
  }

  SimpleFactoryKey* key_;
  DISALLOW_COPY_AND_ASSIGN(DownloadBlobContextGetterFactory);
};

}  // namespace

// static
BraveDownloadServiceFactory* BraveDownloadServiceFactory::GetInstance() {
  return base::Singleton<BraveDownloadServiceFactory>::get();
}

// static
download::DownloadService* BraveDownloadServiceFactory::GetForKey(
    SimpleFactoryKey* key) {
  return static_cast<download::DownloadService*>(
      GetInstance()->GetServiceForKey(key, true));
}

BraveDownloadServiceFactory::BraveDownloadServiceFactory()
    : SimpleKeyedServiceFactory("download::DownloadService",
                                SimpleDependencyManager::GetInstance()) {
  DependsOn(SimpleDownloadManagerCoordinatorFactory::GetInstance());
  DependsOn(download::NavigationMonitorFactory::GetInstance());
}

BraveDownloadServiceFactory::~BraveDownloadServiceFactory() = default;

std::unique_ptr<KeyedService> BraveDownloadServiceFactory::BuildServiceInstanceFor(
    SimpleFactoryKey* key) const {
  //auto clients = std::make_unique<download::BraveDownloadClientMap>();
  auto clients = std::make_unique<download::DownloadClientMap>();
  ProfileKey* profile_key = ProfileKey::FromSimpleFactoryKey(key);

  clients->insert(std::make_pair(
      //download::BraveDownloadClient::CUSTOM_LIST_SUBSCRIPTIONS,
      download::DownloadClient::CUSTOM_LIST_SUBSCRIPTIONS,
      std::make_unique<brave_shields::CustomSubscriptionDownloadClient>(/*key*/)));
      /*std::make_unique<download::DeferredClientWrapper>(
          base::BindOnce(&CreateCustomSubscriptionDownloadClient), key)));*/

/*#if BUILDFLAG(ENABLE_OFFLINE_PAGES)
  // Offline prefetch doesn't support incognito.
  if (!key->IsOffTheRecord()) {
    clients->insert(std::make_pair(
        download::DownloadClient::OFFLINE_PAGE_PREFETCH,
        std::make_unique<offline_pages::OfflinePrefetchDownloadClient>(key)));
  }
#endif  // BUILDFLAG(ENABLE_OFFLINE_PAGES)

  clients->insert(std::make_pair(
      download::DownloadClient::BACKGROUND_FETCH,
      std::make_unique<download::DeferredClientWrapper>(
          base::BindOnce(&CreateBackgroundFetchDownloadClient), key)));

#if BUILDFLAG(IS_CHROMEOS_ASH)
  if (!key->IsOffTheRecord()) {
    clients->insert(std::make_pair(
        download::DownloadClient::PLUGIN_VM_IMAGE,
        std::make_unique<download::DeferredClientWrapper>(
            base::BindOnce(&CreatePluginVmImageDownloadClient), key)));
  }
#endif  // BUILDFLAG(IS_CHROMEOS_ASH)

  if (optimization_guide::features::IsModelDownloadingEnabled() &&
      !key->IsOffTheRecord()) {
    clients->insert(std::make_pair(
        download::DownloadClient::OPTIMIZATION_GUIDE_PREDICTION_MODELS,
        std::make_unique<download::DeferredClientWrapper>(
            base::BindOnce(
                &CreateOptimizationGuidePredictionModelDownloadClient),
            key)));
  }*/

  // Build in memory download service for incognito profile.
  if (key->IsOffTheRecord() &&
      base::FeatureList::IsEnabled(download::kDownloadServiceIncognito)) {
    auto blob_context_getter_factory =
        std::make_unique<DownloadBlobContextGetterFactory>(key);
    scoped_refptr<base::SingleThreadTaskRunner> io_task_runner =
        content::GetIOThreadTaskRunner({});
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory =
        SystemNetworkContextManager::GetInstance()->GetSharedURLLoaderFactory();

    return download::BuildInMemoryDownloadService(
        key, std::move(clients), content::GetNetworkConnectionTracker(),
        base::FilePath(), std::move(blob_context_getter_factory),
        io_task_runner, url_loader_factory);
  } else {
    // Build download service for normal profile.
    base::FilePath storage_dir;
    if (!key->IsOffTheRecord() && !key->GetPath().empty()) {
      storage_dir =
          key->GetPath().Append(chrome::kDownloadServiceStorageDirname);
    }
    scoped_refptr<base::SequencedTaskRunner> background_task_runner =
        base::ThreadPool::CreateSequencedTaskRunner(
            {base::MayBlock(), base::TaskPriority::BEST_EFFORT});

    std::unique_ptr<download::TaskScheduler> task_scheduler;
#if defined(OS_ANDROID)
    task_scheduler =
        std::make_unique<download::android::DownloadTaskScheduler>();
#else
    task_scheduler = std::make_unique<DownloadTaskSchedulerImpl>(key);
#endif
    // Some tests doesn't initialize DownloadManager when profile is created,
    // and cause the download service to fail. Call
    // InitializeSimpleDownloadManager() to initialize the DownloadManager
    // whenever profile becomes available.
    DownloadManagerUtils::InitializeSimpleDownloadManager(profile_key);
    leveldb_proto::ProtoDatabaseProvider* proto_db_provider =
        profile_key->GetProtoDatabaseProvider();
    return download::BuildDownloadService(
        key, std::move(clients), content::GetNetworkConnectionTracker(),
        storage_dir, SimpleDownloadManagerCoordinatorFactory::GetForKey(key),
        proto_db_provider, background_task_runner, std::move(task_scheduler));
  }
}

SimpleFactoryKey* BraveDownloadServiceFactory::GetKeyToUse(
    SimpleFactoryKey* key) const {
  return key;
}
