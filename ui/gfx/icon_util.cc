// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/icon_util.h"

#include "base/file_util.h"
#include "base/files/important_file_writer.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/win/resource_util.h"
#include "base/win/scoped_gdi_object.h"
#include "base/win/scoped_handle.h"
#include "base/win/scoped_hdc.h"
#include "skia/ext/image_operations.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/gfx/gdi_util.h"
#include "ui/gfx/image/image.h"
#include "ui/gfx/image/image_family.h"
#include "ui/gfx/size.h"

namespace {

struct ScopedICONINFO : ICONINFO {
  ScopedICONINFO() {
    hbmColor = NULL;
    hbmMask = NULL;
  }

  ~ScopedICONINFO() {
    if (hbmColor)
      ::DeleteObject(hbmColor);
    if (hbmMask)
      ::DeleteObject(hbmMask);
  }
};

// Creates a new ImageFamily, |resized_image_family|, based on the images in
// |image_family|, but containing images of specific dimensions desirable for
// Windows icons. For each desired image dimension, it chooses the most
// appropriate image for that size, and resizes it to the desired size.
// Returns true on success, false on failure. Failure can occur if
// |image_family| is empty, all images in the family have size 0x0, or an image
// has no allocated pixel data.
// |resized_image_family| must be empty.
bool BuildResizedImageFamily(const gfx::ImageFamily& image_family,
                             gfx::ImageFamily* resized_image_family) {
  DCHECK(resized_image_family);
  DCHECK(resized_image_family->empty());

  for (size_t i = 0; i < IconUtil::kNumIconDimensions; ++i) {
    int dimension = IconUtil::kIconDimensions[i];
    gfx::Size size(dimension, dimension);
    const gfx::Image* best = image_family.GetBest(size);
    if (!best || best->IsEmpty()) {
      // Either |image_family| is empty, or all images have size 0x0.
      return false;
    }

    // Optimize for the "Large icons" view in Windows Vista+. This view displays
    // icons at full size if only if there is a 256x256 (kLargeIconSize) image
    // in the .ico file. Otherwise, it shrinks icons to 48x48 (kMediumIconSize).
    if (dimension > IconUtil::kMediumIconSize &&
        best->Width() <= IconUtil::kMediumIconSize &&
        best->Height() <= IconUtil::kMediumIconSize) {
      // There is no source icon larger than 48x48, so do not create any
      // images larger than 48x48. kIconDimensions is sorted in ascending
      // order, so it is safe to break here.
      break;
    }

    if (best->Size() == size) {
      resized_image_family->Add(*best);
    } else {
      // There is no |dimension|x|dimension| source image.
      // Resize this one to the desired size, and insert it.
      SkBitmap best_bitmap = best->AsBitmap();
      // If a gfx::Image was created from a SkBitmap with no allocated pixels,
      // AsBitmap will return a null bitmap instead. This bitmap will have no
      // config and a size of 0x0. Check this and fail early, to avoid having
      // 0x0-sized bitmaps in our resized image family.
      if (best_bitmap.config() == SkBitmap::kNo_Config)
        return false;
      SkBitmap resized_bitmap = skia::ImageOperations::Resize(
          best_bitmap, skia::ImageOperations::RESIZE_LANCZOS3,
          dimension, dimension);
      resized_image_family->Add(gfx::Image::CreateFrom1xBitmap(resized_bitmap));
    }
  }
  return true;
}

// Creates a set of bitmaps from an image family.
// All images smaller than 256x256 are converted to SkBitmaps, and inserted into
// |bitmaps| in order of aspect ratio (thinnest to widest), and then ascending
// size order. If an image of exactly 256x256 is specified, it is converted into
// PNG format and stored in |png_bytes|. Images with width or height larger than
// 256 are ignored.
// |bitmaps| must be an empty vector, and not NULL.
// Returns true on success, false on failure. This fails if any image in
// |image_family| is not a 32-bit ARGB image, or is otherwise invalid.
bool ConvertImageFamilyToBitmaps(
    const gfx::ImageFamily& image_family,
    std::vector<SkBitmap>* bitmaps,
    scoped_refptr<base::RefCountedMemory>* png_bytes) {
  DCHECK(bitmaps != NULL);
  DCHECK(bitmaps->empty());

  for (gfx::ImageFamily::const_iterator it = image_family.begin();
       it != image_family.end(); ++it) {
    const gfx::Image& image = *it;

    // All images should have one of the kIconDimensions sizes.
    DCHECK_GT(image.Width(), 0);
    DCHECK_LE(image.Width(), IconUtil::kLargeIconSize);
    DCHECK_GT(image.Height(), 0);
    DCHECK_LE(image.Height(), IconUtil::kLargeIconSize);

    SkBitmap bitmap = image.AsBitmap();

    // Only 32 bit ARGB bitmaps are supported. We also make sure the bitmap has
    // been properly initialized.
    SkAutoLockPixels bitmap_lock(bitmap);
    if ((bitmap.config() != SkBitmap::kARGB_8888_Config) ||
        (bitmap.getPixels() == NULL)) {
      return false;
    }

    // Special case: Icons exactly 256x256 are stored in PNG format.
    if (image.Width() == IconUtil::kLargeIconSize &&
        image.Height() == IconUtil::kLargeIconSize) {
      *png_bytes = image.As1xPNGBytes();
    } else {
      bitmaps->push_back(bitmap);
    }
  }

  return true;
}

}  // namespace

// The icon images appear in the icon file in same order in which their
// corresponding dimensions appear in this array, so it is important to keep
// this array sorted. Also note that the maximum icon image size we can handle
// is 256 by 256. See:
// http://msdn.microsoft.com/en-us/library/windows/desktop/aa511280.aspx#size
const int IconUtil::kIconDimensions[] = {
  8,    // Recommended by the MSDN as a nice to have icon size.
  10,   // Used by the Shell (e.g. for shortcuts).
  14,   // Recommended by the MSDN as a nice to have icon size.
  16,   // Toolbar, Application and Shell icon sizes.
  22,   // Recommended by the MSDN as a nice to have icon size.
  24,   // Used by the Shell (e.g. for shortcuts).
  32,   // Toolbar, Dialog and Wizard icon size.
  40,   // Quick Launch.
  48,   // Alt+Tab icon size.
  64,   // Recommended by the MSDN as a nice to have icon size.
  96,   // Recommended by the MSDN as a nice to have icon size.
  128,  // Used by the Shell (e.g. for shortcuts).
  256   // Used by Vista onwards for large icons.
};

const size_t IconUtil::kNumIconDimensions = arraysize(kIconDimensions);
const size_t IconUtil::kNumIconDimensionsUpToMediumSize = 9;

HICON IconUtil::CreateHICONFromSkBitmap(const SkBitmap& bitmap) {
  // Only 32 bit ARGB bitmaps are supported. We also try to perform as many
  // validations as we can on the bitmap.
  SkAutoLockPixels bitmap_lock(bitmap);
  if ((bitmap.config() != SkBitmap::kARGB_8888_Config) ||
      (bitmap.width() <= 0) || (bitmap.height() <= 0) ||
      (bitmap.getPixels() == NULL))
    return NULL;

  // We start by creating a DIB which we'll use later on in order to create
  // the HICON. We use BITMAPV5HEADER since the bitmap we are about to convert
  // may contain an alpha channel and the V5 header allows us to specify the
  // alpha mask for the DIB.
  BITMAPV5HEADER bitmap_header;
  InitializeBitmapHeader(&bitmap_header, bitmap.width(), bitmap.height());
  void* bits;
  HDC hdc = ::GetDC(NULL);
  HBITMAP dib;
  dib = ::CreateDIBSection(hdc, reinterpret_cast<BITMAPINFO*>(&bitmap_header),
                           DIB_RGB_COLORS, &bits, NULL, 0);
  DCHECK(dib);
  ::ReleaseDC(NULL, hdc);
  memcpy(bits, bitmap.getPixels(), bitmap.width() * bitmap.height() * 4);

  // Icons are generally created using an AND and XOR masks where the AND
  // specifies boolean transparency (the pixel is either opaque or
  // transparent) and the XOR mask contains the actual image pixels. If the XOR
  // mask bitmap has an alpha channel, the AND monochrome bitmap won't
  // actually be used for computing the pixel transparency. Even though all our
  // bitmap has an alpha channel, Windows might not agree when all alpha values
  // are zero. So the monochrome bitmap is created with all pixels transparent
  // for this case. Otherwise, it is created with all pixels opaque.
  bool bitmap_has_alpha_channel = PixelsHaveAlpha(
      static_cast<const uint32*>(bitmap.getPixels()),
      bitmap.width() * bitmap.height());

  scoped_ptr<uint8[]> mask_bits;
  if (!bitmap_has_alpha_channel) {
    // Bytes per line with paddings to make it word alignment.
    size_t bytes_per_line = (bitmap.width() + 0xF) / 16 * 2;
    size_t mask_bits_size = bytes_per_line * bitmap.height();

    mask_bits.reset(new uint8[mask_bits_size]);
    DCHECK(mask_bits.get());

    // Make all pixels transparent.
    memset(mask_bits.get(), 0xFF, mask_bits_size);
  }

  HBITMAP mono_bitmap = ::CreateBitmap(bitmap.width(), bitmap.height(), 1, 1,
      reinterpret_cast<LPVOID>(mask_bits.get()));
  DCHECK(mono_bitmap);

  ICONINFO icon_info;
  icon_info.fIcon = TRUE;
  icon_info.xHotspot = 0;
  icon_info.yHotspot = 0;
  icon_info.hbmMask = mono_bitmap;
  icon_info.hbmColor = dib;
  HICON icon = ::CreateIconIndirect(&icon_info);
  ::DeleteObject(dib);
  ::DeleteObject(mono_bitmap);
  return icon;
}

SkBitmap* IconUtil::CreateSkBitmapFromHICON(HICON icon, const gfx::Size& s) {
  // We start with validating parameters.
  if (!icon || s.IsEmpty())
    return NULL;
  ScopedICONINFO icon_info;
  if (!::GetIconInfo(icon, &icon_info))
    return NULL;
  if (!icon_info.fIcon)
    return NULL;
  return new SkBitmap(CreateSkBitmapFromHICONHelper(icon, s));
}

scoped_ptr<SkBitmap> IconUtil::CreateSkBitmapFromIconResource(HMODULE module,
                                                              int resource_id,
                                                              int size) {
  DCHECK_LE(size, kLargeIconSize);

  // For everything except the Vista+ 256x256 icons, use |LoadImage()|.
  if (size != kLargeIconSize) {
    HICON icon_handle =
        static_cast<HICON>(LoadImage(module, MAKEINTRESOURCE(resource_id),
                                     IMAGE_ICON, size, size,
                                     LR_DEFAULTCOLOR | LR_DEFAULTSIZE));
    scoped_ptr<SkBitmap> bitmap(IconUtil::CreateSkBitmapFromHICON(icon_handle));
    DestroyIcon(icon_handle);
    return bitmap.Pass();
  }

  // For Vista+ 256x256 PNG icons, read the resource directly and find
  // the corresponding icon entry to get its PNG bytes.
  void* icon_dir_data = NULL;
  size_t icon_dir_size = 0;
  if (!base::win::GetResourceFromModule(module, resource_id, RT_GROUP_ICON,
                                        &icon_dir_data, &icon_dir_size)) {
    return scoped_ptr<SkBitmap>();
  }
  DCHECK(icon_dir_data);
  DCHECK_GE(icon_dir_size, sizeof(GRPICONDIR));

  const GRPICONDIR* icon_dir =
      reinterpret_cast<const GRPICONDIR*>(icon_dir_data);
  const GRPICONDIRENTRY* large_icon_entry = NULL;
  for (size_t i = 0; i < icon_dir->idCount; ++i) {
    const GRPICONDIRENTRY* entry = &icon_dir->idEntries[i];
    // 256x256 icons are stored with width and height set to 0.
    // See: http://en.wikipedia.org/wiki/ICO_(file_format)
    if (entry->bWidth == 0 && entry->bHeight == 0) {
      large_icon_entry = entry;
      break;
    }
  }
  if (!large_icon_entry)
    return scoped_ptr<SkBitmap>();

  void* png_data = NULL;
  size_t png_size = 0;
  if (!base::win::GetResourceFromModule(module, large_icon_entry->nID, RT_ICON,
                                        &png_data, &png_size)) {
    return scoped_ptr<SkBitmap>();
  }
  DCHECK(png_data);
  DCHECK_EQ(png_size, large_icon_entry->dwBytesInRes);

  const unsigned char* png_bytes =
      reinterpret_cast<const unsigned char*>(png_data);
  gfx::Image image = gfx::Image::CreateFrom1xPNGBytes(png_bytes, png_size);
  return scoped_ptr<SkBitmap>(new SkBitmap(image.AsBitmap()));
}

SkBitmap* IconUtil::CreateSkBitmapFromHICON(HICON icon) {
  // We start with validating parameters.
  if (!icon)
    return NULL;

  ScopedICONINFO icon_info;
  BITMAP bitmap_info = { 0 };

  if (!::GetIconInfo(icon, &icon_info))
    return NULL;

  if (!::GetObject(icon_info.hbmMask, sizeof(bitmap_info), &bitmap_info))
    return NULL;

  gfx::Size icon_size(bitmap_info.bmWidth, bitmap_info.bmHeight);
  return new SkBitmap(CreateSkBitmapFromHICONHelper(icon, icon_size));
}

HICON IconUtil::CreateCursorFromDIB(const gfx::Size& icon_size,
                                    const gfx::Point& hotspot,
                                    const void* dib_bits,
                                    size_t dib_size) {
  BITMAPINFO icon_bitmap_info = {0};
  gfx::CreateBitmapHeader(
      icon_size.width(),
      icon_size.height(),
      reinterpret_cast<BITMAPINFOHEADER*>(&icon_bitmap_info));

  base::win::ScopedGetDC dc(NULL);
  base::win::ScopedCreateDC working_dc(CreateCompatibleDC(dc));
  base::win::ScopedGDIObject<HBITMAP> bitmap_handle(
      CreateDIBSection(dc,
                       &icon_bitmap_info,
                       DIB_RGB_COLORS,
                       0,
                       0,
                       0));
  if (dib_size > 0) {
    SetDIBits(0,
              bitmap_handle,
              0,
              icon_size.height(),
              dib_bits,
              &icon_bitmap_info,
              DIB_RGB_COLORS);
  }

  HBITMAP old_bitmap = reinterpret_cast<HBITMAP>(
      SelectObject(working_dc, bitmap_handle));
  SetBkMode(working_dc, TRANSPARENT);
  SelectObject(working_dc, old_bitmap);

  base::win::ScopedGDIObject<HBITMAP> mask(
      CreateBitmap(icon_size.width(),
                   icon_size.height(),
                   1,
                   1,
                   NULL));
  ICONINFO ii = {0};
  ii.fIcon = FALSE;
  ii.xHotspot = hotspot.x();
  ii.yHotspot = hotspot.y();
  ii.hbmMask = mask;
  ii.hbmColor = bitmap_handle;

  return CreateIconIndirect(&ii);
}

SkBitmap IconUtil::CreateSkBitmapFromHICONHelper(HICON icon,
                                                 const gfx::Size& s) {
  DCHECK(icon);
  DCHECK(!s.IsEmpty());

  // Allocating memory for the SkBitmap object. We are going to create an ARGB
  // bitmap so we should set the configuration appropriately.
  SkBitmap bitmap;
  bitmap.setConfig(SkBitmap::kARGB_8888_Config, s.width(), s.height());
  bitmap.allocPixels();
  bitmap.eraseARGB(0, 0, 0, 0);
  SkAutoLockPixels bitmap_lock(bitmap);

  // Now we should create a DIB so that we can use ::DrawIconEx in order to
  // obtain the icon's image.
  BITMAPV5HEADER h;
  InitializeBitmapHeader(&h, s.width(), s.height());
  HDC hdc = ::GetDC(NULL);
  uint32* bits;
  HBITMAP dib = ::CreateDIBSection(hdc, reinterpret_cast<BITMAPINFO*>(&h),
      DIB_RGB_COLORS, reinterpret_cast<void**>(&bits), NULL, 0);
  DCHECK(dib);
  HDC dib_dc = CreateCompatibleDC(hdc);
  ::ReleaseDC(NULL, hdc);
  DCHECK(dib_dc);
  HGDIOBJ old_obj = ::SelectObject(dib_dc, dib);

  // Windows icons are defined using two different masks. The XOR mask, which
  // represents the icon image and an AND mask which is a monochrome bitmap
  // which indicates the transparency of each pixel.
  //
  // To make things more complex, the icon image itself can be an ARGB bitmap
  // and therefore contain an alpha channel which specifies the transparency
  // for each pixel. Unfortunately, there is no easy way to determine whether
  // or not a bitmap has an alpha channel and therefore constructing the bitmap
  // for the icon is nothing but straightforward.
  //
  // The idea is to read the AND mask but use it only if we know for sure that
  // the icon image does not have an alpha channel. The only way to tell if the
  // bitmap has an alpha channel is by looking through the pixels and checking
  // whether there are non-zero alpha bytes.
  //
  // We start by drawing the AND mask into our DIB.
  size_t num_pixels = s.GetArea();
  memset(bits, 0, num_pixels * 4);
  ::DrawIconEx(dib_dc, 0, 0, icon, s.width(), s.height(), 0, NULL, DI_MASK);

  // Capture boolean opacity. We may not use it if we find out the bitmap has
  // an alpha channel.
  scoped_ptr<bool[]> opaque(new bool[num_pixels]);
  for (size_t i = 0; i < num_pixels; ++i)
    opaque[i] = !bits[i];

  // Then draw the image itself which is really the XOR mask.
  memset(bits, 0, num_pixels * 4);
  ::DrawIconEx(dib_dc, 0, 0, icon, s.width(), s.height(), 0, NULL, DI_NORMAL);
  memcpy(bitmap.getPixels(), static_cast<void*>(bits), num_pixels * 4);

  // Finding out whether the bitmap has an alpha channel.
  bool bitmap_has_alpha_channel = PixelsHaveAlpha(
      static_cast<const uint32*>(bitmap.getPixels()), num_pixels);

  // If the bitmap does not have an alpha channel, we need to build it using
  // the previously captured AND mask. Otherwise, we are done.
  if (!bitmap_has_alpha_channel) {
    uint32* p = static_cast<uint32*>(bitmap.getPixels());
    for (size_t i = 0; i < num_pixels; ++p, ++i) {
      DCHECK_EQ((*p & 0xff000000), 0u);
      if (opaque[i])
        *p |= 0xff000000;
      else
        *p &= 0x00ffffff;
    }
  }

  ::SelectObject(dib_dc, old_obj);
  ::DeleteObject(dib);
  ::DeleteDC(dib_dc);

  return bitmap;
}

// static
bool IconUtil::CreateIconFileFromImageFamily(
    const gfx::ImageFamily& image_family,
    const base::FilePath& icon_path) {
  // Creating a set of bitmaps corresponding to the icon images we'll end up
  // storing in the icon file. Each bitmap is created by resizing the most
  // appropriate image from |image_family| to the desired size.
  gfx::ImageFamily resized_image_family;
  if (!BuildResizedImageFamily(image_family, &resized_image_family))
    return false;

  std::vector<SkBitmap> bitmaps;
  scoped_refptr<base::RefCountedMemory> png_bytes;
  if (!ConvertImageFamilyToBitmaps(resized_image_family, &bitmaps, &png_bytes))
    return false;

  // Guaranteed true because BuildResizedImageFamily will provide at least one
  // image < 256x256.
  DCHECK(!bitmaps.empty());
  size_t bitmap_count = bitmaps.size();  // Not including PNG image.
  // Including PNG image, if any.
  size_t image_count = bitmap_count + (png_bytes.get() ? 1 : 0);

  // Computing the total size of the buffer we need in order to store the
  // images in the desired icon format.
  size_t buffer_size = ComputeIconFileBufferSize(bitmaps);
  // Account for the bytes needed for the PNG entry.
  if (png_bytes.get())
    buffer_size += sizeof(ICONDIRENTRY) + png_bytes->size();

  // Setting the information in the structures residing within the buffer.
  // First, we set the information which doesn't require iterating through the
  // bitmap set and then we set the bitmap specific structures. In the latter
  // step we also copy the actual bits.
  std::vector<uint8> buffer(buffer_size);
  ICONDIR* icon_dir = reinterpret_cast<ICONDIR*>(&buffer[0]);
  icon_dir->idType = kResourceTypeIcon;
  icon_dir->idCount = static_cast<WORD>(image_count);
  // - 1 because there is already one ICONDIRENTRY in ICONDIR.
  size_t icon_dir_count = image_count - 1;

  size_t offset = sizeof(ICONDIR) + (sizeof(ICONDIRENTRY) * icon_dir_count);
  for (size_t i = 0; i < bitmap_count; i++) {
    ICONIMAGE* image = reinterpret_cast<ICONIMAGE*>(&buffer[offset]);
    DCHECK_LT(offset, buffer_size);
    size_t icon_image_size = 0;
    SetSingleIconImageInformation(bitmaps[i], i, icon_dir, image, offset,
                                  &icon_image_size);
    DCHECK_GT(icon_image_size, 0U);
    offset += icon_image_size;
  }

  // Add the PNG entry, if necessary.
  if (png_bytes.get()) {
    ICONDIRENTRY* entry = &icon_dir->idEntries[bitmap_count];
    entry->bWidth = 0;
    entry->bHeight = 0;
    entry->wPlanes = 1;
    entry->wBitCount = 32;
    entry->dwBytesInRes = static_cast<DWORD>(png_bytes->size());
    entry->dwImageOffset = static_cast<DWORD>(offset);
    memcpy(&buffer[offset], png_bytes->front(), png_bytes->size());
    offset += png_bytes->size();
  }

  DCHECK_EQ(offset, buffer_size);

  std::string data(buffer.begin(), buffer.end());
  return base::ImportantFileWriter::WriteFileAtomically(icon_path, data);
}

bool IconUtil::PixelsHaveAlpha(const uint32* pixels, size_t num_pixels) {
  for (const uint32* end = pixels + num_pixels; pixels != end; ++pixels) {
    if ((*pixels & 0xff000000) != 0)
      return true;
  }

  return false;
}

void IconUtil::InitializeBitmapHeader(BITMAPV5HEADER* header, int width,
                                      int height) {
  DCHECK(header);
  memset(header, 0, sizeof(BITMAPV5HEADER));
  header->bV5Size = sizeof(BITMAPV5HEADER);

  // Note that icons are created using top-down DIBs so we must negate the
  // value used for the icon's height.
  header->bV5Width = width;
  header->bV5Height = -height;
  header->bV5Planes = 1;
  header->bV5Compression = BI_RGB;

  // Initializing the bitmap format to 32 bit ARGB.
  header->bV5BitCount = 32;
  header->bV5RedMask = 0x00FF0000;
  header->bV5GreenMask = 0x0000FF00;
  header->bV5BlueMask = 0x000000FF;
  header->bV5AlphaMask = 0xFF000000;

  // Use the system color space.  The default value is LCS_CALIBRATED_RGB, which
  // causes us to crash if we don't specify the approprite gammas, etc.  See
  // <http://msdn.microsoft.com/en-us/library/ms536531(VS.85).aspx> and
  // <http://b/1283121>.
  header->bV5CSType = LCS_WINDOWS_COLOR_SPACE;

  // Use a valid value for bV5Intent as 0 is not a valid one.
  // <http://msdn.microsoft.com/en-us/library/dd183381(VS.85).aspx>
  header->bV5Intent = LCS_GM_IMAGES;
}

void IconUtil::SetSingleIconImageInformation(const SkBitmap& bitmap,
                                             size_t index,
                                             ICONDIR* icon_dir,
                                             ICONIMAGE* icon_image,
                                             size_t image_offset,
                                             size_t* image_byte_count) {
  DCHECK(icon_dir != NULL);
  DCHECK(icon_image != NULL);
  DCHECK_GT(image_offset, 0U);
  DCHECK(image_byte_count != NULL);
  DCHECK_LT(bitmap.width(), kLargeIconSize);
  DCHECK_LT(bitmap.height(), kLargeIconSize);

  // We start by computing certain image values we'll use later on.
  size_t xor_mask_size, bytes_in_resource;
  ComputeBitmapSizeComponents(bitmap,
                              &xor_mask_size,
                              &bytes_in_resource);

  icon_dir->idEntries[index].bWidth = static_cast<BYTE>(bitmap.width());
  icon_dir->idEntries[index].bHeight = static_cast<BYTE>(bitmap.height());
  icon_dir->idEntries[index].wPlanes = 1;
  icon_dir->idEntries[index].wBitCount = 32;
  icon_dir->idEntries[index].dwBytesInRes = bytes_in_resource;
  icon_dir->idEntries[index].dwImageOffset = image_offset;
  icon_image->icHeader.biSize = sizeof(BITMAPINFOHEADER);

  // The width field in the BITMAPINFOHEADER structure accounts for the height
  // of both the AND mask and the XOR mask so we need to multiply the bitmap's
  // height by 2. The same does NOT apply to the width field.
  icon_image->icHeader.biHeight = bitmap.height() * 2;
  icon_image->icHeader.biWidth = bitmap.width();
  icon_image->icHeader.biPlanes = 1;
  icon_image->icHeader.biBitCount = 32;

  // We use a helper function for copying to actual bits from the SkBitmap
  // object into the appropriate space in the buffer. We use a helper function
  // (rather than just copying the bits) because there is no way to specify the
  // orientation (bottom-up vs. top-down) of a bitmap residing in a .ico file.
  // Thus, if we just copy the bits, we'll end up with a bottom up bitmap in
  // the .ico file which will result in the icon being displayed upside down.
  // The helper function copies the image into the buffer one scanline at a
  // time.
  //
  // Note that we don't need to initialize the AND mask since the memory
  // allocated for the icon data buffer was initialized to zero. The icon we
  // create will therefore use an AND mask containing only zeros, which is OK
  // because the underlying image has an alpha channel. An AND mask containing
  // only zeros essentially means we'll initially treat all the pixels as
  // opaque.
  unsigned char* image_addr = reinterpret_cast<unsigned char*>(icon_image);
  unsigned char* xor_mask_addr = image_addr + sizeof(BITMAPINFOHEADER);
  CopySkBitmapBitsIntoIconBuffer(bitmap, xor_mask_addr, xor_mask_size);
  *image_byte_count = bytes_in_resource;
}

void IconUtil::CopySkBitmapBitsIntoIconBuffer(const SkBitmap& bitmap,
                                              unsigned char* buffer,
                                              size_t buffer_size) {
  SkAutoLockPixels bitmap_lock(bitmap);
  unsigned char* bitmap_ptr = static_cast<unsigned char*>(bitmap.getPixels());
  size_t bitmap_size = bitmap.height() * bitmap.width() * 4;
  DCHECK_EQ(buffer_size, bitmap_size);
  for (size_t i = 0; i < bitmap_size; i += bitmap.width() * 4) {
    memcpy(buffer + bitmap_size - bitmap.width() * 4 - i,
           bitmap_ptr + i,
           bitmap.width() * 4);
  }
}

size_t IconUtil::ComputeIconFileBufferSize(const std::vector<SkBitmap>& set) {
  DCHECK(!set.empty());

  // We start by counting the bytes for the structures that don't depend on the
  // number of icon images. Note that sizeof(ICONDIR) already accounts for a
  // single ICONDIRENTRY structure, which is why we subtract one from the
  // number of bitmaps.
  size_t total_buffer_size = sizeof(ICONDIR);
  size_t bitmap_count = set.size();
  total_buffer_size += sizeof(ICONDIRENTRY) * (bitmap_count - 1);
  // May not have all icon sizes, but must have at least up to medium icon size.
  DCHECK_GE(bitmap_count, kNumIconDimensionsUpToMediumSize);

  // Add the bitmap specific structure sizes.
  for (size_t i = 0; i < bitmap_count; i++) {
    size_t xor_mask_size, bytes_in_resource;
    ComputeBitmapSizeComponents(set[i],
                                &xor_mask_size,
                                &bytes_in_resource);
    total_buffer_size += bytes_in_resource;
  }
  return total_buffer_size;
}

void IconUtil::ComputeBitmapSizeComponents(const SkBitmap& bitmap,
                                           size_t* xor_mask_size,
                                           size_t* bytes_in_resource) {
  // The XOR mask size is easy to calculate since we only deal with 32bpp
  // images.
  *xor_mask_size = bitmap.width() * bitmap.height() * 4;

  // Computing the AND mask is a little trickier since it is a monochrome
  // bitmap (regardless of the number of bits per pixels used in the XOR mask).
  // There are two things we must make sure we do when computing the AND mask
  // size:
  //
  // 1. Make sure the right number of bytes is allocated for each AND mask
  //    scan line in case the number of pixels in the image is not divisible by
  //    8. For example, in a 15X15 image, 15 / 8 is one byte short of
  //    containing the number of bits we need in order to describe a single
  //    image scan line so we need to add a byte. Thus, we need 2 bytes instead
  //    of 1 for each scan line.
  //
  // 2. Make sure each scan line in the AND mask is 4 byte aligned (so that the
  //    total icon image has a 4 byte alignment). In the 15X15 image example
  //    above, we can not use 2 bytes so we increase it to the next multiple of
  //    4 which is 4.
  //
  // Once we compute the size for a singe AND mask scan line, we multiply that
  // number by the image height in order to get the total number of bytes for
  // the AND mask. Thus, for a 15X15 image, we need 15 * 4 which is 60 bytes
  // for the monochrome bitmap representing the AND mask.
  size_t and_line_length = (bitmap.width() + 7) >> 3;
  and_line_length = (and_line_length + 3) & ~3;
  size_t and_mask_size = and_line_length * bitmap.height();
  size_t masks_size = *xor_mask_size + and_mask_size;
  *bytes_in_resource = masks_size + sizeof(BITMAPINFOHEADER);
}
