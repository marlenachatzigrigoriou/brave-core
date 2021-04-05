/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_IPFS_IPFS_TEXT_IMPORT_WORKER_H_
#define BRAVE_COMPONENTS_IPFS_IPFS_TEXT_IMPORT_WORKER_H_

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

class IpfsTextImportWorker : public IpfsImportWorkerBase {
 public:
  IpfsTextImportWorker(content::BrowserContext* context,
                       const GURL& endpoint,
                       ImportCompletedCallback callback,
                       const std::string& text,
                       const std::string& host);
  ~IpfsTextImportWorker() override;

 private:
  void StartImportText(const std::string& text, const std::string& host);

  DISALLOW_COPY_AND_ASSIGN(IpfsTextImportWorker);
};

}  // namespace ipfs

#endif  // BRAVE_COMPONENTS_IPFS_IPFS_TEXT_IMPORT_WORKER_H_
