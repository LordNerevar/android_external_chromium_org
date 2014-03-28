// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/policy/core/browser/cloud/message_util.h"

#include "base/logging.h"
#include "grit/component_strings.h"
#include "ui/base/l10n/l10n_util.h"

namespace policy {

namespace {

int GetIDSForDMStatus(DeviceManagementStatus status) {
  switch (status) {
    case DM_STATUS_SUCCESS:
      return IDS_POLICY_DM_STATUS_SUCCESS;
    case DM_STATUS_REQUEST_INVALID:
      return IDS_POLICY_DM_STATUS_REQUEST_INVALID;
    case DM_STATUS_REQUEST_FAILED:
      return IDS_POLICY_DM_STATUS_REQUEST_FAILED;
    case DM_STATUS_TEMPORARY_UNAVAILABLE:
      return IDS_POLICY_DM_STATUS_TEMPORARY_UNAVAILABLE;
    case DM_STATUS_HTTP_STATUS_ERROR:
      return IDS_POLICY_DM_STATUS_HTTP_STATUS_ERROR;
    case DM_STATUS_RESPONSE_DECODING_ERROR:
      return IDS_POLICY_DM_STATUS_RESPONSE_DECODING_ERROR;
    case DM_STATUS_SERVICE_MANAGEMENT_NOT_SUPPORTED:
      return IDS_POLICY_DM_STATUS_SERVICE_MANAGEMENT_NOT_SUPPORTED;
    case DM_STATUS_SERVICE_DEVICE_NOT_FOUND:
      return IDS_POLICY_DM_STATUS_SERVICE_DEVICE_NOT_FOUND;
    case DM_STATUS_SERVICE_MANAGEMENT_TOKEN_INVALID:
      return IDS_POLICY_DM_STATUS_SERVICE_MANAGEMENT_TOKEN_INVALID;
    case DM_STATUS_SERVICE_ACTIVATION_PENDING:
      return IDS_POLICY_DM_STATUS_SERVICE_ACTIVATION_PENDING;
    case DM_STATUS_SERVICE_INVALID_SERIAL_NUMBER:
      return IDS_POLICY_DM_STATUS_SERVICE_INVALID_SERIAL_NUMBER;
    case DM_STATUS_SERVICE_DEVICE_ID_CONFLICT:
      return IDS_POLICY_DM_STATUS_SERVICE_DEVICE_ID_CONFLICT;
    case DM_STATUS_SERVICE_MISSING_LICENSES:
      return IDS_POLICY_DM_STATUS_SERVICE_MISSING_LICENSES;
    case DM_STATUS_SERVICE_DEPROVISIONED:
      return IDS_POLICY_DM_STATUS_SERVICE_DEPROVISIONED;
    case DM_STATUS_SERVICE_DOMAIN_MISMATCH:
      return IDS_POLICY_DM_STATUS_SERVICE_DOMAIN_MISMATCH;
    case DM_STATUS_SERVICE_POLICY_NOT_FOUND:
      return IDS_POLICY_DM_STATUS_SERVICE_POLICY_NOT_FOUND;
  }
  NOTREACHED() << "Unhandled DM status " << status;
  return IDS_POLICY_DM_STATUS_UNKNOWN_ERROR;
}

int GetIDSForValidationStatus(CloudPolicyValidatorBase::Status status) {
  switch (status) {
    case CloudPolicyValidatorBase::VALIDATION_OK:
      return IDS_POLICY_VALIDATION_OK;
    case CloudPolicyValidatorBase::VALIDATION_BAD_INITIAL_SIGNATURE:
      return IDS_POLICY_VALIDATION_BAD_INITIAL_SIGNATURE;
    case CloudPolicyValidatorBase::VALIDATION_BAD_SIGNATURE:
      return IDS_POLICY_VALIDATION_BAD_SIGNATURE;
    case CloudPolicyValidatorBase::VALIDATION_ERROR_CODE_PRESENT:
      return IDS_POLICY_VALIDATION_ERROR_CODE_PRESENT;
    case CloudPolicyValidatorBase::VALIDATION_PAYLOAD_PARSE_ERROR:
      return IDS_POLICY_VALIDATION_PAYLOAD_PARSE_ERROR;
    case CloudPolicyValidatorBase::VALIDATION_WRONG_POLICY_TYPE:
      return IDS_POLICY_VALIDATION_WRONG_POLICY_TYPE;
    case CloudPolicyValidatorBase::VALIDATION_WRONG_SETTINGS_ENTITY_ID:
      return IDS_POLICY_VALIDATION_WRONG_SETTINGS_ENTITY_ID;
    case CloudPolicyValidatorBase::VALIDATION_BAD_TIMESTAMP:
      return IDS_POLICY_VALIDATION_BAD_TIMESTAMP;
    case CloudPolicyValidatorBase::VALIDATION_WRONG_TOKEN:
      return IDS_POLICY_VALIDATION_WRONG_TOKEN;
    case CloudPolicyValidatorBase::VALIDATION_BAD_USERNAME:
      return IDS_POLICY_VALIDATION_BAD_USERNAME;
    case CloudPolicyValidatorBase::VALIDATION_POLICY_PARSE_ERROR:
      return IDS_POLICY_VALIDATION_POLICY_PARSE_ERROR;
    case CloudPolicyValidatorBase::VALIDATION_BAD_KEY_VERIFICATION_SIGNATURE:
      return IDS_POLICY_VALIDATION_BAD_KEY_VERIFICATION_SIGNATURE;
    case CloudPolicyValidatorBase::VALIDATION_STATUS_SIZE:
      NOTREACHED();
  }
  NOTREACHED() << "Unhandled validation status " << status;
  return IDS_POLICY_VALIDATION_UNKNOWN_ERROR;
}

int GetIDSForStoreStatus(CloudPolicyStore::Status status) {
  switch (status) {
    case CloudPolicyStore::STATUS_OK:
      return IDS_POLICY_STORE_STATUS_OK;
    case CloudPolicyStore::STATUS_LOAD_ERROR:
      return IDS_POLICY_STORE_STATUS_LOAD_ERROR;
    case CloudPolicyStore::STATUS_STORE_ERROR:
      return IDS_POLICY_STORE_STATUS_STORE_ERROR;
    case CloudPolicyStore::STATUS_PARSE_ERROR:
      return IDS_POLICY_STORE_STATUS_PARSE_ERROR;
    case CloudPolicyStore::STATUS_SERIALIZE_ERROR:
      return IDS_POLICY_STORE_STATUS_SERIALIZE_ERROR;
    case CloudPolicyStore::STATUS_VALIDATION_ERROR:
      // This is handled separately below to include the validation error.
      break;
    case CloudPolicyStore::STATUS_BAD_STATE:
      return IDS_POLICY_STORE_STATUS_BAD_STATE;
  }
  NOTREACHED() << "Unhandled store status " << status;
  return IDS_POLICY_STORE_STATUS_UNKNOWN_ERROR;
}

}  // namespace

base::string16 FormatDeviceManagementStatus(DeviceManagementStatus status) {
  return l10n_util::GetStringUTF16(GetIDSForDMStatus(status));
}

base::string16 FormatValidationStatus(
    CloudPolicyValidatorBase::Status validation_status) {
  return l10n_util::GetStringUTF16(
      GetIDSForValidationStatus(validation_status));
}

base::string16 FormatStoreStatus(
    CloudPolicyStore::Status store_status,
    CloudPolicyValidatorBase::Status validation_status) {
  if (store_status == CloudPolicyStore::STATUS_VALIDATION_ERROR) {
    return l10n_util::GetStringFUTF16(
        IDS_POLICY_STORE_STATUS_VALIDATION_ERROR,
        l10n_util::GetStringUTF16(
            GetIDSForValidationStatus(validation_status)));
  }

  return l10n_util::GetStringUTF16(GetIDSForStoreStatus(store_status));
}

}  // namespace policy
