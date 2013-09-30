// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_POLICY_CLOUD_EXTERNAL_DATA_MANAGER_BASE_H_
#define CHROME_BROWSER_CHROMEOS_POLICY_CLOUD_EXTERNAL_DATA_MANAGER_BASE_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/threading/non_thread_safe.h"
#include "chrome/browser/policy/cloud/cloud_external_data_manager.h"

namespace base {
class SequencedTaskRunner;
}

namespace policy {

class CloudExternalDataStore;
class ExternalPolicyDataFetcherBackend;
struct PolicyDefinitionList;

// Downloads, verifies, caches and retrieves external data referenced by
// policies.
// This is a common base class used by specializations for regular users and
// device-local accounts.
class CloudExternalDataManagerBase : public CloudExternalDataManager,
                                     public base::NonThreadSafe {
 public:
  // The |policy_definitions| are used to determine the maximum size that the
  // data referenced by each policy can have. Download scheduling, verification,
  // caching and retrieval tasks are done via the |backend_task_runner|, which
  // must support file I/O. Network I/O is done via the |io_task_runner|.
  CloudExternalDataManagerBase(
      const PolicyDefinitionList* policy_definitions,
      scoped_refptr<base::SequencedTaskRunner> backend_task_runner,
      scoped_refptr<base::SequencedTaskRunner> io_task_runner);
  virtual ~CloudExternalDataManagerBase();

  // Allows downloaded external data to be cached in |external_data_store|.
  // Ownership of the store is taken. The store can be destroyed by calling
  // SetExternalDataStore(scoped_ptr<CloudExternalDataStore>()). Accesses to the
  // store are made via |backend_task_runner_| only.
  void SetExternalDataStore(
      scoped_ptr<CloudExternalDataStore> external_data_store);

  // CloudExternalDataManager:
  virtual void SetPolicyStore(CloudPolicyStore* policy_store) OVERRIDE;
  virtual void OnPolicyStoreLoaded() OVERRIDE;
  virtual void Connect(
      scoped_refptr<net::URLRequestContextGetter> request_context) OVERRIDE;
  virtual void Disconnect() OVERRIDE;
  virtual void Fetch(
      const std::string& policy,
      const ExternalDataFetcher::FetchCallback& callback) OVERRIDE;

  // Allows policies to reference |max_size| bytes of external data even if no
  // |max_size| was specified in policy_templates.json.
  // TODO(bartfab): This override is only needed because there are no policies
  // that reference external data and have a |max_size| yet. Once the first such
  // policy is added, use that policy in tests and remove the override.
  static void SetMaxExternalDataSizeForTesting(int max_size);

 protected:
  friend class CloudExternalDataManagerBaseTest;

  // Try to download and cache all external data referenced by policies in
  // |policy_store_|.
  void FetchAll();

  scoped_refptr<base::SequencedTaskRunner> backend_task_runner_;
  scoped_refptr<base::SequencedTaskRunner> io_task_runner_;

 private:
  // The |external_policy_data_fetcher_backend_| handles network I/O for the
  // |backend_| because URLRequestContextGetter and URLFetchers cannot be
  // referenced from background threads. It is instantiated on the thread |this|
  // runs on but after that, must only be accessed and eventually destroyed via
  // the |io_task_runner_|.
  scoped_ptr<ExternalPolicyDataFetcherBackend>
      external_policy_data_fetcher_backend_;

  // The |backend_| handles all data download scheduling, verification, caching
  // and retrieval. It is instantiated on the thread |this| runs on but after
  // that, must only be accessed and eventually destroyed via the
  // |backend_task_runner_|.
  class Backend;
  scoped_ptr<Backend> backend_;

  DISALLOW_COPY_AND_ASSIGN(CloudExternalDataManagerBase);
};

}  // namespace policy

#endif  // CHROME_BROWSER_CHROMEOS_POLICY_CLOUD_EXTERNAL_DATA_MANAGER_BASE_H_
