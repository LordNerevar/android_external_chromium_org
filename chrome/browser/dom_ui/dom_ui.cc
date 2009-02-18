// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/dom_ui/dom_ui.h"

#include "base/json_reader.h"
#include "base/json_writer.h"
#include "base/string_util.h"
#include "base/values.h"
#include "chrome/common/l10n_util.h"

///////////////////////////////////////////////////////////////////////////////
// DOMMessageHandler

DOMUI::DOMUI(DOMUIContents* contents) : contents_(contents) {
}

DOMUI::~DOMUI() {
  STLDeleteContainerPairSecondPointers(message_callbacks_.begin(),
                                       message_callbacks_.end());
  STLDeleteContainerPointers(handlers_.begin(), handlers_.end());
}

// DOMUI, public: -------------------------------------------------------------

void DOMUI::ProcessDOMUIMessage(const std::string& message,
                                const std::string& content) {
  // Look up the callback for this message.
  MessageCallbackMap::const_iterator callback = 
      message_callbacks_.find(message);
  if (callback == message_callbacks_.end())
    return;

  // Convert the content JSON into a Value.
  scoped_ptr<Value> value;
  if (!content.empty()) {
    value.reset(JSONReader::Read(content, false));
    if (!value.get()) {
      // The page sent us something that we didn't understand.
      // This probably indicates a programming error.
      NOTREACHED();
      return;
    }
  }

  // Forward this message and content on.
  callback->second->Run(value.get());
}

void DOMUI::CallJavascriptFunction(const std::wstring& function_name,
                                   const Value& arg) {
  std::string json;
  JSONWriter::Write(&arg, false, &json);
  std::wstring javascript = function_name + L"(" + UTF8ToWide(json) + L");";

  ExecuteJavascript(javascript);
}

void DOMUI::CallJavascriptFunction(
    const std::wstring& function_name,
    const Value& arg1, const Value& arg2) {
  std::string json;
  JSONWriter::Write(&arg1, false, &json);
  std::wstring javascript = function_name + L"(" + UTF8ToWide(json);
  JSONWriter::Write(&arg2, false, &json);
  javascript += L"," + UTF8ToWide(json) + L");";

  ExecuteJavascript(javascript);
}

void DOMUI::RegisterMessageCallback(const std::string &message, 
                                    MessageCallback *callback) {
  message_callbacks_.insert(std::make_pair(message, callback));
}

// DOMUI, protected: ----------------------------------------------------------

void DOMUI::AddMessageHandler(DOMMessageHandler* handler) {
  handlers_.push_back(handler);
}

// DOMUI, private: ------------------------------------------------------------

void DOMUI::ExecuteJavascript(const std::wstring& javascript) {
  DCHECK(contents_);
  contents_->render_view_host()->ExecuteJavascriptInWebFrame(std::wstring(), 
                                                             javascript);
}

///////////////////////////////////////////////////////////////////////////////
// DOMMessageHandler

DOMMessageHandler::DOMMessageHandler(DOMUI *dom_ui) : dom_ui_(dom_ui) {
}

// DOMMessageHandler, protected: ----------------------------------------------

void DOMMessageHandler::SetURLAndTitle(DictionaryValue* dictionary, 
                                       std::wstring title,
                                       const GURL& gurl) {
  std::wstring wstring_url = UTF8ToWide(gurl.spec());
  dictionary->SetString(L"url", wstring_url);

  bool using_url_as_the_title = false;
  if (title.empty()) {
    using_url_as_the_title = true;
    title = wstring_url;
  }

  // Since the title can contain BiDi text, we need to mark the text as either
  // RTL or LTR, depending on the characters in the string. If we use the URL
  // as the title, we mark the title as LTR since URLs are always treated as
  // left to right strings.
  std::wstring title_to_set(title);
  if (l10n_util::GetTextDirection() == l10n_util::RIGHT_TO_LEFT) {
    if (using_url_as_the_title) {
      l10n_util::WrapStringWithLTRFormatting(&title_to_set);
    } else {
      bool success =
          l10n_util::AdjustStringForLocaleDirection(title, &title_to_set);
      DCHECK(success ? (title != title_to_set) : (title == title_to_set));
    }
  }
  dictionary->SetString(L"title", title_to_set);
}