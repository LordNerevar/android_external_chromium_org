// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBKIT_GLUE_PLUGINS_PEPPER_FONT_H_
#define WEBKIT_GLUE_PLUGINS_PEPPER_FONT_H_

#include <string>

#include "base/scoped_ptr.h"
#include "third_party/ppapi/c/ppb_font.h"
#include "webkit/glue/plugins/pepper_resource.h"

namespace WebKit {
class WebFont;
}

namespace pepper {

class PluginInstance;

class Font : public Resource {
 public:
  Font(PluginModule* module, const PP_FontDescription& desc);
  virtual ~Font();

  // Returns a pointer to the interface implementing PPB_Font that is exposed to
  // the plugin.
  static const PPB_Font* GetInterface();

  // Resource overrides.
  Font* AsFont() { return this; }

  // PPB_Font implementation.
  bool Describe(PP_FontDescription* description,
                PP_FontMetrics* metrics);
  bool DrawTextAt(PP_Resource image_data,
                  const PP_TextRun* text,
                  const PP_Point* position,
                  uint32_t color,
                  const PP_Rect* clip,
                  bool image_data_is_opaque);
  int32_t MeasureText(const PP_TextRun* text);
  uint32_t CharacterOffsetForPixel(const PP_TextRun* text,
                                   int32_t pixel_position);
  int32_t PixelOffsetForCharacter(const PP_TextRun* text,
                                  uint32_t char_offset);

 private:
  scoped_ptr<WebKit::WebFont> font_;
};

}  // namespace pepper

#endif  // WEBKIT_GLUE_PLUGINS_PEPPER_FONT_H_
