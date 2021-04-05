/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_IPFS_IPFS_LINK_IMPORT_WORKER_H_
#define BRAVE_COMPONENTS_IPFS_IPFS_LINK_IMPORT_WORKER_H_

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
#include "brave/components/ipfs/ipfs_import_worker_base.h"
#include "components/version_info/channel.h"
#include "url/gurl.h"

namespace ipfs {

class IpfsLinkImportWorker : public IpfsImportWorkerBase {
 public:
  IpfsLinkImportWorker(content::BrowserContext* context,
                       const GURL& endpoint,
                       ImportCompletedCallback callback,
                       const GURL& url);
  ~IpfsLinkImportWorker() override;

 private:
  void StartImportLink(const GURL& url);
  void OnImportDataAvailable(base::FilePath path);

  void CreateRequestWithFile(base::FilePath upload_file_path,
                             const std::string& mime_type,
                             int64_t file_size);
  void OnImportAddComplete(std::unique_ptr<std::string> response_body);

  GURL import_url_;
  std::unique_ptr<network::SimpleURLLoader> url_loader_;
  base::WeakPtrFactory<IpfsLinkImportWorker> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(IpfsLinkImportWorker);
};

}  // namespace ipfs

#endif  // BRAVE_COMPONENTS_IPFS_IPFS_LINK_IMPORT_WORKER_H_
