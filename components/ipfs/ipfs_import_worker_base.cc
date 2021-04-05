/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/ipfs/ipfs_import_worker_base.h"

#include <utility>

#include "base/command_line.h"
#include "base/files/file_util.h"
#include "base/guid.h"
#include "base/strings/strcat.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/task/post_task.h"
#include "base/task_runner_util.h"
#include "base/threading/scoped_blocking_call.h"
#include "base/time/time.h"
#include "brave/components/ipfs/ipfs_constants.h"
#include "brave/components/ipfs/ipfs_json_parser.h"
#include "brave/components/ipfs/ipfs_utils.h"
#include "brave/components/ipfs/pref_names.h"
#include "brave/components/ipfs/service_sandbox_type.h"
#include "components/grit/brave_components_strings.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "components/user_prefs/user_prefs.h"
#include "content/browser/blob_storage/chrome_blob_storage_context.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/service_process_host.h"
#include "content/public/browser/storage_partition.h"
#include "net/base/mime_util.h"
#include "net/base/url_util.h"
#include "net/http/http_request_headers.h"
#include "net/http/http_response_headers.h"
#include "net/http/http_status_code.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "storage/browser/blob/blob_data_builder.h"
#include "storage/browser/blob/blob_data_handle.h"
#include "storage/browser/blob/blob_impl.h"
#include "storage/browser/blob/blob_storage_context.h"
#include "third_party/blink/public/mojom/blob/serialized_blob.mojom.h"
#include "url/gurl.h"

namespace {

net::NetworkTrafficAnnotationTag GetNetworkTrafficAnnotationTag() {
  return net::DefineNetworkTrafficAnnotation("ipfs_import_worker", R"(
        semantics {
          sender: "IPFS import worker"
          description:
            "This worker is used to communicate with IPFS daemon "
          trigger:
            "Triggered by user actions"
          data:
            "Options of the commands."
          destination: WEBSITE
        }
        policy {
          cookies_allowed: NO
          setting:
            "You can enable or disable this feature in brave://settings."
          policy_exception_justification:
            "Not implemented."
        }
      )");
}

// Return a date string formatted as "YYYY-MM-DD".
std::string TimeFormatDate(const base::Time& time) {
  base::Time::Exploded exploded_time;
  time.UTCExplode(&exploded_time);
  return base::StringPrintf("%04d-%02d-%02d", exploded_time.year,
                            exploded_time.month, exploded_time.day_of_month);
}

}  // namespace

namespace ipfs {

IpfsImportWorkerBase::IpfsImportWorkerBase(content::BrowserContext* context,
                                           const GURL& endpoint,
                                           ImportCompletedCallback callback)
    : callback_(std::move(callback)),
      server_endpoint_(endpoint),
      browser_context_(context),
      io_task_runner_(base::CreateSequencedTaskRunner(
          {base::MayBlock(), content::BrowserThread::IO,
           base::TaskPriority::BEST_EFFORT,
           base::TaskShutdownBehavior::BLOCK_SHUTDOWN})),
      weak_factory_(this) {
  url_loader_factory_ =
      content::BrowserContext::GetDefaultStoragePartition(context)
          ->GetURLLoaderFactoryForBrowserProcess();
  data_.reset(new ipfs::ImportedData());
}

IpfsImportWorkerBase::~IpfsImportWorkerBase() = default;

void IpfsImportWorkerBase::StartImport(
    BlobBuilderCallback blob_builder_callback,
    const std::string& content_type) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  auto* context = content::ChromeBlobStorageContext::GetFor(browser_context_);
  base::PostTaskAndReplyWithResult(
      io_task_runner_.get(), FROM_HERE,
      base::BindOnce(&IpfsImportWorkerBase::CreateResourceRequest,
                     base::Unretained(this), std::move(blob_builder_callback),
                     content_type, base::WrapRefCounted(context)),
      base::BindOnce(&IpfsImportWorkerBase::UploadDataUI,
                     weak_factory_.GetWeakPtr()));
}

std::unique_ptr<network::ResourceRequest>
IpfsImportWorkerBase::CreateResourceRequest(
    BlobBuilderCallback blob_builder_callback,
    const std::string& content_type,
    scoped_refptr<content::ChromeBlobStorageContext> ctx) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
  std::unique_ptr<storage::BlobDataBuilder> blob_builder =
      std::move(blob_builder_callback).Run();

  auto* storage_context = ctx->context();
  std::unique_ptr<storage::BlobDataHandle> blob_handle =
      storage_context->AddFinishedBlob(std::move(blob_builder));

  auto blob = blink::mojom::SerializedBlob::New();
  blob->uuid = blob_handle->uuid();
  blob->size = blob_handle->size();
  storage::BlobImpl::Create(
      std::make_unique<storage::BlobDataHandle>(*blob_handle),
      blob->blob.InitWithNewPipeAndPassReceiver());
  // Use a Data Pipe to transfer the blob.
  mojo::PendingRemote<network::mojom::DataPipeGetter> data_pipe_getter_remote;
  mojo::Remote<blink::mojom::Blob> blob_remote(std::move(blob->blob));
  blob_remote->AsDataPipeGetter(
      data_pipe_getter_remote.InitWithNewPipeAndPassReceiver());

  auto request = std::make_unique<network::ResourceRequest>();
  request->request_body = new network::ResourceRequestBody();
  request->request_body->AppendDataPipe(std::move(data_pipe_getter_remote));
  request->headers.SetHeader(net::HttpRequestHeaders::kContentType,
                             content_type);
  return request;
}

void IpfsImportWorkerBase::UploadDataUI(
    std::unique_ptr<network::ResourceRequest> request) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  if (!request)
    return NotifyImportCompleted();

  GURL url = net::AppendQueryParameter(server_endpoint_.Resolve(kImportAddPath),
                                       "stream-channels", "true");
  url = net::AppendQueryParameter(url, "wrap-with-directory", "true");
  url = net::AppendQueryParameter(url, "pin", "false");
  url = net::AppendQueryParameter(url, "progress", "false");

  request->url = url;
  request->method = "POST";

  // Remove trailing "/".
  std::string origin = server_endpoint_.spec();
  if (base::EndsWith(origin, "/", base::CompareCase::INSENSITIVE_ASCII)) {
    origin.pop_back();
  }
  request->headers.SetHeader(net::HttpRequestHeaders::kOrigin, origin);
  DCHECK(!url_loader_);
  url_loader_ = network::SimpleURLLoader::Create(
      std::move(request), GetNetworkTrafficAnnotationTag());

  url_loader_->DownloadToStringOfUnboundedSizeUntilCrashAndDie(
      url_loader_factory_.get(),
      base::BindOnce(&IpfsImportWorkerBase::OnImportAddComplete,
                     weak_factory_.GetWeakPtr()));
}

void IpfsImportWorkerBase::OnImportAddComplete(
    std::unique_ptr<std::string> response_body) {
  int error_code = url_loader_->NetError();
  int response_code = -1;
  if (url_loader_->ResponseInfo() && url_loader_->ResponseInfo()->headers)
    response_code = url_loader_->ResponseInfo()->headers->response_code();

  if (response_body)
    DLOG(INFO) << " response_body:" << *response_body;
  DLOG(INFO) << "error_code:" << error_code
             << " response_code:" << response_code;
  bool success = (error_code == net::OK && response_code == net::HTTP_OK);

  if (success) {
    std::vector<std::string> parts = base::SplitString(
        *response_body, "\n", base::KEEP_WHITESPACE, base::SPLIT_WANT_ALL);
    auto content = *response_body;
    if (parts.size()) {
      auto front = parts.front();
      if (front.front() == '{' && front.back() == '}')
        content = front;
    }
    success = IPFSJSONParser::GetImportResponseFromJSON(
                                  content, data_.get());
  }
  url_loader_.reset();
  if (success && !data_->hash.empty()) {
    CreateBraveDirectory();
    return;
  }
  NotifyImportCompleted();
}

void IpfsImportWorkerBase::CreateBraveDirectory() {
  DCHECK(!url_loader_);
  GURL url = net::AppendQueryParameter(
      server_endpoint_.Resolve(kImportMakeDirectoryPath), "parents", "true");
  std::string directory = kImportDirectory;
  directory += TimeFormatDate(base::Time::Now());
  directory += "/";
  url = net::AppendQueryParameter(url, "arg", directory);

  url_loader_ = CreateURLLoader(url, "POST");
  url_loader_->DownloadToStringOfUnboundedSizeUntilCrashAndDie(
      url_loader_factory_.get(),
      base::BindOnce(&IpfsImportWorkerBase::OnImportDirectoryCreated,
                     base::Unretained(this), directory));
}

void IpfsImportWorkerBase::OnImportDirectoryCreated(
    const std::string& directory,
    std::unique_ptr<std::string> response_body) {
  int error_code = url_loader_->NetError();
  int response_code = -1;
  if (url_loader_->ResponseInfo() && url_loader_->ResponseInfo()->headers)
    response_code = url_loader_->ResponseInfo()->headers->response_code();
  url_loader_.reset();
  if (response_body)
    DLOG(INFO) << " response_body:" << *response_body;
  DLOG(INFO) << "error_code:" << error_code
             << " response_code:" << response_code;
  bool success = (error_code == net::OK && response_code == net::HTTP_OK);
  if (success) {
    data_->directory = directory;
    CopyFilesToBraveDirectory();
    return;
  }
  NotifyImportCompleted();
}

void IpfsImportWorkerBase::CopyFilesToBraveDirectory() {
  DCHECK(!url_loader_);
  std::string from = "/ipfs/" + data_->hash;
  GURL url = net::AppendQueryParameter(
      server_endpoint_.Resolve(kImportCopyPath), "arg", from);
  std::string to = data_->directory + "/" + data_->name;
  url = net::AppendQueryParameter(url, "arg", to);

  url_loader_ = CreateURLLoader(url, "POST");
  url_loader_->DownloadToStringOfUnboundedSizeUntilCrashAndDie(
      url_loader_factory_.get(),
      base::BindOnce(&IpfsImportWorkerBase::OnImportFilesMoved,
                     base::Unretained(this)));
}

void IpfsImportWorkerBase::OnImportFilesMoved(
    std::unique_ptr<std::string> response_body) {
  int error_code = url_loader_->NetError();
  int response_code = -1;
  if (url_loader_->ResponseInfo() && url_loader_->ResponseInfo()->headers)
    response_code = url_loader_->ResponseInfo()->headers->response_code();
  url_loader_.reset();
  bool success = (error_code == net::OK && response_code == net::HTTP_OK);
  if (!success) {
    VLOG(1) << "error_code:" << error_code << " response_code:" << response_code
            << " response_body:" << *response_body;
  }
  
  NotifyImportCompleted();
}

void IpfsImportWorkerBase::NotifyImportCompleted() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  if (callback_)
    std::move(callback_).Run(*data_.get());
}

std::unique_ptr<network::SimpleURLLoader> IpfsImportWorkerBase::CreateURLLoader(
    const GURL& gurl,
    const std::string& method) {
  auto request = std::make_unique<network::ResourceRequest>();
  request->url = gurl;
  request->method = method;
  // Remove trailing "/".
  std::string origin = server_endpoint_.spec();
  if (base::EndsWith(origin, "/", base::CompareCase::INSENSITIVE_ASCII)) {
    origin.pop_back();
  }
  request->headers.SetHeader(net::HttpRequestHeaders::kOrigin, origin);
  auto url_loader = network::SimpleURLLoader::Create(
      std::move(request), GetNetworkTrafficAnnotationTag());
  return url_loader;
}

scoped_refptr<network::SharedURLLoaderFactory>
IpfsImportWorkerBase::GetUrlLoaderFactory() {
  return url_loader_factory_;
}
}  // namespace ipfs
