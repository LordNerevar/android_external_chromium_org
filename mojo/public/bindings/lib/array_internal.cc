// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/public/bindings/lib/array_internal.h"

namespace mojo {
namespace internal {

ArrayDataTraits<bool>::BitRef::~BitRef() {
}

ArrayDataTraits<bool>::BitRef::BitRef(uint8_t* storage, uint8_t mask)
    : storage_(storage),
      mask_(mask) {
}

ArrayDataTraits<bool>::BitRef&
ArrayDataTraits<bool>::BitRef::operator=(bool value) {
  if (value) {
    *storage_ |= mask_;
  } else {
    *storage_ &= ~mask_;
  }
  return *this;
}

ArrayDataTraits<bool>::BitRef&
ArrayDataTraits<bool>::BitRef::operator=(const BitRef& value) {
  return (*this) = static_cast<bool>(value);
}

ArrayDataTraits<bool>::BitRef::operator bool() const {
  return (*storage_ & mask_) != 0;
}

}  // namespace internal
}  // namespace mojo
