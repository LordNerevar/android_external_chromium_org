// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TOOLS_JSON_SCHEMA_COMPILER_UTIL_H__
#define TOOLS_JSON_SCHEMA_COMPILER_UTIL_H__

#include <string>
#include <vector>

#include "base/memory/linked_ptr.h"
#include "base/memory/scoped_ptr.h"
#include "base/values.h"

namespace json_schema_compiler {

namespace util {

// Creates a new item at |out| from |from|[|index|]. These are used by template
// specializations of |Get(Optional)ArrayFromList|.
bool GetItemFromList(const ListValue& from, int index, int* out);
bool GetItemFromList(const ListValue& from, int index, bool* out);
bool GetItemFromList(const ListValue& from, int index, double* out);
bool GetItemFromList(const ListValue& from, int index, std::string* out);
bool GetItemFromList(const ListValue& from,
                     int index,
                     linked_ptr<base::Value>* out);
bool GetItemFromList(const ListValue& from,
                     int index,
                     linked_ptr<base::DictionaryValue>* out);

// This template is used for types generated by tools/json_schema_compiler.
template<class T>
bool GetItemFromList(const ListValue& from, int index, linked_ptr<T>* out) {
  const DictionaryValue* dict;
  if (!from.GetDictionary(index, &dict))
    return false;
  scoped_ptr<T> obj(new T());
  if (!T::Populate(*dict, obj.get()))
    return false;
  *out = linked_ptr<T>(obj.release());
  return true;
}

// This is used for getting an enum out of a ListValue, which will happen if an
// array of enums is a parameter to a function.
template<class T>
bool GetItemFromList(const ListValue& from, int index, T* out) {
  int value;
  if (!from.GetInteger(index, &value))
    return false;
  *out = static_cast<T>(value);
  return true;
}

// Populates |out| with |list|. Returns false if there is no list at the
// specified key or if the list has anything other than |T|.
template <class T>
bool PopulateArrayFromList(
    const base::ListValue& list, std::vector<T>* out) {
  out->clear();
  T value;
  for (size_t i = 0; i < list.GetSize(); ++i) {
    if (!GetItemFromList(list, i, &value))
      return false;
    out->push_back(value);
  }

  return true;
}

// Populates |out| with |from|.|name|. Returns false if there is no list at
// the specified key or if the list has anything other than |T|.
template <class T>
bool PopulateArrayFromDictionary(
    const base::DictionaryValue& from,
    const std::string& name,
    std::vector<T>* out) {
  const base::ListValue* list = NULL;
  if (!from.GetListWithoutPathExpansion(name, &list))
    return false;

  return PopulateArrayFromList(*list, out);
}

// Creates a new vector containing |list| at |out|. Returns
// true on success or if there is nothing at the specified key. Returns false
// if anything other than a list of |T| is at the specified key.
template <class T>
bool PopulateOptionalArrayFromList(
    const base::ListValue& list,
    scoped_ptr<std::vector<T> >* out) {
  out->reset(new std::vector<T>());
  T value;
  for (size_t i = 0; i < list.GetSize(); ++i) {
    if (!GetItemFromList(list, i, &value)) {
      out->reset();
      return false;
    }
    (*out)->push_back(value);
  }

  return true;
}

// Creates a new vector containing |from|.|name| at |out|. Returns
// true on success or if there is nothing at the specified key. Returns false
// if anything other than a list of |T| is at the specified key.
template <class T>
bool PopulateOptionalArrayFromDictionary(
    const base::DictionaryValue& from,
    const std::string& name,
    scoped_ptr<std::vector<T> >* out) {
  const base::ListValue* list = NULL;
  {
    const base::Value* maybe_list = NULL;
    // Since |name| is optional, its absence is acceptable. However, anything
    // other than a ListValue is not.
    if (!from.GetWithoutPathExpansion(name, &maybe_list))
      return true;
    if (!maybe_list->IsType(base::Value::TYPE_LIST))
      return false;
    list = static_cast<const base::ListValue*>(maybe_list);
  }

  return PopulateOptionalArrayFromList(*list, out);
}

// Appends a Value newly created from |from| to |out|. These used by template
// specializations of |Set(Optional)ArrayToList|.
void AddItemToList(const int from, base::ListValue* out);
void AddItemToList(const bool from, base::ListValue* out);
void AddItemToList(const double from, base::ListValue* out);
void AddItemToList(const std::string& from, base::ListValue* out);
void AddItemToList(const linked_ptr<base::Value>& from,
                   base::ListValue* out);
void AddItemToList(const linked_ptr<base::DictionaryValue>& from,
                   base::ListValue* out);

// This template is used for types generated by tools/json_schema_compiler.
template<class T>
void AddItemToList(const linked_ptr<T>& from, ListValue* out) {
  out->Append(from->ToValue().release());
}

// Set |out| to the the contents of |from|. Requires GetItemFromList to be
// implemented for |T|.
template <class T>
void PopulateListFromArray(
    const std::vector<T>& from,
    base::ListValue* out) {
  out->Clear();
  for (typename std::vector<T>::const_iterator it = from.begin();
      it != from.end(); ++it) {
    AddItemToList(*it, out);
  }
}

// Set |out| to the the contents of |from| if |from| is non-NULL. Requires
// GetItemFromList to be implemented for |T|.
template <class T>
void PopulateListFromOptionalArray(
    const scoped_ptr<std::vector<T> >& from,
    base::ListValue* out) {
  if (from.get())
    PopulateListFromArray(*from, out);

}

template <class T>
scoped_ptr<Value> CreateValueFromArray(const std::vector<T>& from) {
  base::ListValue* list = new base::ListValue();
  PopulateListFromArray(from, list);
  return scoped_ptr<Value>(list);
}

template <class T>
scoped_ptr<Value> CreateValueFromOptionalArray(
    const scoped_ptr<std::vector<T> >& from) {
  if (from.get())
    return CreateValueFromArray(*from);
  return scoped_ptr<Value>();
}

}  // namespace util
}  // namespace json_schema_compiler

#endif // TOOLS_JSON_SCHEMA_COMPILER_UTIL_H__
