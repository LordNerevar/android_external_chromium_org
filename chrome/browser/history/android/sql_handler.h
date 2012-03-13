// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_HISTORY_ANDROID_SQL_HANDLER_H_
#define CHROME_BROWSER_HISTORY_ANDROID_SQL_HANDLER_H_

#include "chrome/browser/history/android/android_history_types.h"

namespace history {

// This is a wrapper of information needed for Insert/Update/Delete
// method in SQLHandler. Refer to SQLHandler's comment below for how
// it is used.
struct TableIDRow {
  TableIDRow();
  ~TableIDRow();

  URLID url_id;
  GURL url;
};

typedef std::vector<TableIDRow> TableIDRows;

// This base class is used by AndroidProviderBackend to manipulate the indvidual
// table or BookmarkModel in its Insert/Update/Delete method.
//
// The implementation needs to provides an array of columns. Once the columns
// are inserted or updated, the corresponding Insert() or Update() method will
// be invoked. The Delete() method is called to delete rows.
//
// The BookmarkRow given in Insert() or Update() provide the data for insert
// or update. No all the data in BookmarkRow maybe valid, using
// BookmarkRow::is_value_set_explicitly() method to see if the data need be
// inserted or updated.
class SQLHandler {
 public:
  // |columns| is the implementation's columns.
  // |column_count| is the number of column in |columns|.
  SQLHandler(const BookmarkRow::BookmarkColumnID columns[], int column_count);
  virtual ~SQLHandler();

  // Updates the rows whose URLID or URL is in the given |ids_set| with new
  // value stored in |row|. Return true if the update succeeds.
  virtual bool Update(const BookmarkRow& row,
                      const TableIDRows& ids_set) = 0;

  // Inserts the given |row|, return true on success; The id of insertted row
  // should be set in |row|, so other implemnetations could use it to complete
  // the insert.
  virtual bool Insert(BookmarkRow* row) = 0;

  // Deletes the rows whose id is in |ids_set|, returns false if any deletion
  // failed, otherwise return true even all/some of rows are not found.
  virtual bool Delete(const TableIDRows& ids_set) = 0;

  // Return true if |row| has a value explicitly set for at least one of the
  // columns in |row| that are known to this class.
  bool HasColumnIn(const BookmarkRow& row);

  // Returns true if |id| is one of the columns known to this class.
  bool HasColumn(BookmarkRow::BookmarkColumnID id);

 private:
  // The columns of this handler.
  const std::set<BookmarkRow::BookmarkColumnID> columns_;

  DISALLOW_COPY_AND_ASSIGN(SQLHandler);
};

}  // namespace history.

#endif  // CHROME_BROWSER_HISTORY_ANDROID_SQL_HANDLER_H_
