/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_IPFS_IPFS_IMPORT_WORKER_BASE_H_
#define BRAVE_COMPONENTS_IPFS_IPFS_IMPORT_WORKER_BASE_H_

#include <list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/callback.h"
#include "base/containers/queue.h"
#include "base/files/file_util.h"
#include "base/memory/scoped_refptr.h"
#include "brave/components/ipfs/imported_data.h"
#include "components/version_info/channel.h"
#include "storage/browser/blob/blob_data_builder.h"
#include "url/gurl.h"

namespace content {
class BrowserContext;
class ChromeBlobStorageContext;
}  // namespace content

namespace network {
class SharedURLLoaderFactory;
class SimpleURLLoader;
struct ResourceRequest;
}  // namespace network


namespace ipfs {

class IpfsImportWorkerBase {
 public:
  IpfsImportWorkerBase(content::BrowserContext* context,
                       version_info::Channel channel,
                       ImportCompletedCallback callback);
  virtual ~IpfsImportWorkerBase();

  using BlobBuilderCallback =
      base::OnceCallback<std::unique_ptr<storage::BlobDataBuilder>()>;

 protected:
  void StartImport(BlobBuilderCallback blob_builder_callback,
                   const std::string& content_type);
  std::unique_ptr<network::SimpleURLLoader> CreateURLLoader(const GURL& gurl,
      const std::string& method);
  scoped_refptr<network::SharedURLLoaderFactory> GetUrlLoaderFactory();

 private:
  std::unique_ptr<network::ResourceRequest> CreateResourceRequest(
      BlobBuilderCallback blob_builder_callback,
      const std::string& content_type,
      scoped_refptr<content::ChromeBlobStorageContext> ctx);
  // Uploading blob functions
  void UploadDataUI(std::unique_ptr<network::ResourceRequest> request);
  std::unique_ptr<network::ResourceRequest>  PrepareBlobBodyIO(
              scoped_refptr<content::ChromeBlobStorageContext> ctx,
                         base::FilePath upload_file_path,
                         int64_t file_size);
  void OnImportAddComplete(std::unique_ptr<std::string> response_body);

  void CreateBraveDirectory();
  void OnImportDirectoryCreated(const std::string& directory,
                                std::unique_ptr<std::string> response_body);
  void CopyFilesToBraveDirectory();
  void OnImportFilesMoved(std::unique_ptr<std::string> response_body);

  void NotifyImportCompleted();

  ImportCompletedCallback callback_;
  std::unique_ptr<ipfs::ImportedData> data_;

  scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory_;
  std::unique_ptr<network::SimpleURLLoader> url_loader_;
  GURL server_endpoint_;
  content::BrowserContext* browser_context_ = nullptr;
  scoped_refptr<base::SequencedTaskRunner> io_task_runner_;
  base::WeakPtrFactory<IpfsImportWorkerBase> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(IpfsImportWorkerBase);
};

}  // namespace ipfs

#endif  // BRAVE_COMPONENTS_IPFS_IPFS_IMPORT_WORKER_BASE_H_
