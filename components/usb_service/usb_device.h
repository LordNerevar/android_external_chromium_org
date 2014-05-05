// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_USB_SERVICE_USB_DEVICE_H_
#define COMPONENTS_USB_SERVICE_USB_DEVICE_H_

#include <vector>

#include "base/basictypes.h"
#include "base/callback.h"
#include "base/memory/ref_counted.h"
#include "base/threading/thread_checker.h"
#include "components/usb_service/usb_interface.h"
#include "components/usb_service/usb_service_export.h"

struct libusb_device;

namespace usb_service {

class UsbDeviceHandle;
class UsbContext;

typedef libusb_device* PlatformUsbDevice;

// A UsbDevice object represents a detected USB device, providing basic
// information about it. For further manipulation of the device, a
// UsbDeviceHandle must be created from Open() method.
class USB_SERVICE_EXPORT UsbDevice
    : public base::RefCountedThreadSafe<UsbDevice> {
 public:
  // Accessors to basic information.
  PlatformUsbDevice platform_device() const { return platform_device_; }
  uint16 vendor_id() const { return vendor_id_; }
  uint16 product_id() const { return product_id_; }
  uint32 unique_id() const { return unique_id_; }

#if defined(OS_CHROMEOS)
  // On ChromeOS, if an interface of a claimed device is not claimed, the
  // permission broker can change the owner of the device so that the unclaimed
  // interfaces can be used. If this argument is missing, permission broker will
  // not be used and this method fails if the device is claimed.
  virtual void RequestUsbAcess(
      int interface_id,
      const base::Callback<void(bool success)>& callback);
#endif  // OS_CHROMEOS

  // Creates a UsbDeviceHandle for further manipulation.
  // Blocking method. Must be called on FILE thread.
  virtual scoped_refptr<UsbDeviceHandle> Open();

  // Explicitly closes a device handle. This method will be automatically called
  // by the destructor of a UsbDeviceHandle as well.
  // Closing a closed handle is a safe
  // Blocking method. Must be called on FILE thread.
  virtual bool Close(scoped_refptr<UsbDeviceHandle> handle);

  // Lists the interfaces provided by the device and fills the given
  // UsbConfigDescriptor.
  // Blocking method. Must be called on FILE thread.
  virtual scoped_refptr<UsbConfigDescriptor> ListInterfaces();

 protected:
  friend class UsbServiceImpl;
  friend class base::RefCountedThreadSafe<UsbDevice>;

  // Called by UsbService only;
  UsbDevice(scoped_refptr<UsbContext> context,
            PlatformUsbDevice platform_device,
            uint16 vendor_id,
            uint16 product_id,
            uint32 unique_id);

  // Constructor called in test only.
  UsbDevice();
  virtual ~UsbDevice();

  // Called only be UsbService.
  virtual void OnDisconnect();

 private:
  PlatformUsbDevice platform_device_;
  uint16 vendor_id_;
  uint16 product_id_;
  uint32 unique_id_;

  // Retain the context so that it will not be released before UsbDevice.
  scoped_refptr<UsbContext> context_;

  // Opened handles.
  typedef std::vector<scoped_refptr<UsbDeviceHandle> > HandlesVector;
  HandlesVector handles_;

  base::ThreadChecker thread_checker_;

  DISALLOW_COPY_AND_ASSIGN(UsbDevice);
};

}  // namespace usb_service

#endif  // COMPONENTS_USB_SERVICE_USB_DEVICE_H_
