/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_IPFS_IMPORTED_DATA_H_
#define BRAVE_COMPONENTS_IPFS_IMPORTED_DATA_H_

#include <string>
#include <vector>

#include "base/callback.h"
#include "url/gurl.h"

namespace ipfs {

struct ImportedData {
  ImportedData();
  ~ImportedData();

  std::string name;
  std::string hash;
  int64_t size = -1;
  std::string directory;
};

using ImportCompletedCallback =
  base::OnceCallback<void(const ipfs::ImportedData&)>;

}  // namespace ipfs

#endif  // BRAVE_COMPONENTS_IPFS_IMPORTED_DATA_H_
