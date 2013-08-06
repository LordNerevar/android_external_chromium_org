// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_CORE_BROWSER_AUTOFILL_TYPE_H_
#define COMPONENTS_AUTOFILL_CORE_BROWSER_AUTOFILL_TYPE_H_

#include <string>

#include "components/autofill/core/browser/field_types.h"

namespace autofill {

// The high-level description of Autofill types, used to categorize form fields
// and for associating form fields with form values in the Web Database.
class AutofillType {
 public:
  explicit AutofillType(AutofillFieldType field_type);
  AutofillType(const AutofillType& autofill_type);
  AutofillType& operator=(const AutofillType& autofill_type);

  AutofillFieldType field_type() const;
  FieldTypeGroup group() const;

  // Maps |field_type| to a field type that can be directly stored in a profile
  // (in the sense that it makes sense to call |AutofillProfile::SetInfo()| with
  // the returned field type as the first parameter).
  static AutofillFieldType GetEquivalentFieldType(AutofillFieldType field_type);

  // Maps |field_type| to a field type from ADDRESS_BILLING FieldTypeGroup if
  // field type is an Address type.
  static AutofillFieldType GetEquivalentBillingFieldType(
      AutofillFieldType field_type);

  // Utilities for serializing and deserializing an |AutofillFieldType|.
  static std::string FieldTypeToString(AutofillFieldType field_type);
  static AutofillFieldType StringToFieldType(const std::string& str);

 private:
  AutofillFieldType field_type_;
};

}  // namespace autofill

#endif  // COMPONENTS_AUTOFILL_CORE_BROWSER_AUTOFILL_TYPE_H_
