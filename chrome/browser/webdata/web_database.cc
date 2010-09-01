// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/webdata/web_database.h"

#include <algorithm>
#include <limits>
#include <set>
#include <string>

#include "app/l10n_util.h"
#include "app/sql/statement.h"
#include "app/sql/transaction.h"
#include "base/tuple.h"
#include "base/utf_string_conversions.h"
#include "chrome/browser/autofill/autofill_profile.h"
#include "chrome/browser/autofill/autofill_type.h"
#include "chrome/browser/autofill/credit_card.h"
#include "chrome/browser/diagnostics/sqlite_diagnostics.h"
#include "chrome/browser/history/history_database.h"
#include "chrome/browser/search_engines/template_url.h"
#include "chrome/browser/webdata/autofill_change.h"
#include "chrome/common/notification_service.h"
#include "gfx/codec/png_codec.h"
#include "webkit/glue/form_field.h"
#include "webkit/glue/password_form.h"
#include "third_party/skia/include/core/SkBitmap.h"

// Encryptor is now in place for Windows and Mac.  The Linux implementation
// currently obfuscates only.  Mac Encryptor implementation can block the
// active thread while presenting UI to the user.  See |encryptor_mac.mm| for
// details.
// For details on the Linux work see:
//   http://crbug.com/25404
#include "chrome/browser/password_manager/encryptor.h"

using webkit_glue::FormField;
using webkit_glue::PasswordForm;

////////////////////////////////////////////////////////////////////////////////
//
// Schema
//
// keywords                 Most of the columns mirror that of a field in
//                          TemplateURL. See TemplateURL for more details.
//   id
//   short_name
//   keyword
//   favicon_url
//   url
//   show_in_default_list
//   safe_for_autoreplace
//   originating_url
//   date_created           This column was added after we allowed keywords.
//                          Keywords created before we started tracking
//                          creation date have a value of 0 for this.
//   usage_count
//   input_encodings        Semicolon separated list of supported input
//                          encodings, may be empty.
//   suggest_url
//   prepopulate_id         See TemplateURL::prepopulate_id.
//   autogenerate_keyword
//   logo_id                See TemplateURL::logo_id
//
// logins
//   origin_url
//   action_url
//   username_element
//   username_value
//   password_element
//   password_value
//   submit_element
//   signon_realm        The authority (scheme, host, port).
//   ssl_valid           SSL status of page containing the form at first
//                       impression.
//   preferred           MRU bit.
//   date_created        This column was added after logins support. "Legacy"
//                       entries have a value of 0.
//   blacklisted_by_user Tracks whether or not the user opted to 'never
//                       remember'
//                       passwords for this site.
//
// autofill
//   name                The name of the input as specified in the html.
//   value               The literal contents of the text field.
//   value_lower         The contents of the text field made lower_case.
//   pair_id             An ID number unique to the row in the table.
//   count               How many times the user has entered the string |value|
//                       in a field of name |name|.
//
// autofill_dates        This table associates a row to each separate time the
//                       user submits a form containing a certain name/value
//                       pair.  The |pair_id| should match the |pair_id| field
//                       in the appropriate row of the autofill table.
//   pair_id
//   date_created
//
// autofill_profiles    This table contains AutoFill profile data added by the
//                      user with the AutoFill dialog.  Most of the columns are
//                      standard entries in a contact information form.
//
//   label              The label of the profile.  Presented to the user when
//                      selecting profiles.
//   unique_id          The unique ID of this profile.
//   first_name
//   middle_name
//   last_name
//   email
//   company_name
//   address_line_1
//   address_line_2
//   city
//   state
//   zipcode
//   country
//   phone
//   fax
//
// credit_cards         This table contains credit card data added by the user
//                      with the AutoFill dialog.  Most of the columns are
//                      standard entries in a credit card form.
//
//   label              The label of the credit card.  Presented to the user
//                      when selecting credit cards.
//   unique_id          The unique ID of this credit card.
//   name_on_card
//   type
//   card_number        Before version 23 stores credit card number, 23 and
//                      after stores empty string.
//   expiration_month
//   expiration_year
//   verification_code  Before version 23 stores the CVC/CVV/CVV2 card security
//                      code. After that stores the empty string.
//   billing_address    A foreign key into the autofill_profiles table.
//   shipping_address   A foreign key into the autofill_profiles table.
//     For the following two fields encryption is used. Currently it uses
//     Encryptor, that does encryption on windows only. As on the other
//     systems this file is readable by owner only, it is good for now.
//     For potentially going over the wire other encryption is used, see
//     chrome/browser/sync/protocol/autofill_specifics.proto
//   card_number_encrypted Stores encrypted credit card number.
//   verification_code_encrypted  The CVC/CVV/CVV2 card security code.
//
// web_app_icons
//   url         URL of the web app.
//   width       Width of the image.
//   height      Height of the image.
//   image       PNG encoded image data.
//
// web_apps
//   url                 URL of the web app.
//   has_all_images      Do we have all the images?
//
////////////////////////////////////////////////////////////////////////////////

using base::Time;

namespace {

typedef std::vector<Tuple3<int64, string16, string16> > AutofillElementList;

// Current version number.  Note: when changing the current version number,
// corresponding changes must happen in the unit tests, and new migration test
// added.  See |WebDatabaseMigrationTest::kCurrentTestedVersionNumber|.
const int kCurrentVersionNumber = 25;
const int kCompatibleVersionNumber = 25;
const int kUrlIdPosition = 14;

// Keys used in the meta table.
const char* kDefaultSearchProviderKey = "Default Search Provider ID";
const char* kBuiltinKeywordVersion = "Builtin Keyword Version";

// The maximum length allowed for form data.
const size_t kMaxDataLength = 1024;

std::string JoinStrings(const std::string& separator,
                        const std::vector<std::string>& strings) {
  if (strings.empty())
    return std::string();
  std::vector<std::string>::const_iterator i(strings.begin());
  std::string result(*i);
  while (++i != strings.end())
    result += separator + *i;
  return result;
}

void BindURLToStatement(const TemplateURL& url, sql::Statement* s) {
  s->BindString(0, WideToUTF8(url.short_name()));
  s->BindString(1, WideToUTF8(url.keyword()));
  GURL favicon_url = url.GetFavIconURL();
  if (!favicon_url.is_valid()) {
    s->BindString(2, std::string());
  } else {
    s->BindString(2, history::HistoryDatabase::GURLToDatabaseURL(
                       url.GetFavIconURL()));
  }
  if (url.url())
    s->BindString(3, url.url()->url());
  else
    s->BindString(3, std::string());
  s->BindInt(4, url.safe_for_autoreplace() ? 1 : 0);
  if (!url.originating_url().is_valid()) {
    s->BindString(5, std::string());
  } else {
    s->BindString(5, history::HistoryDatabase::GURLToDatabaseURL(
        url.originating_url()));
  }
  s->BindInt64(6, url.date_created().ToTimeT());
  s->BindInt(7, url.usage_count());
  s->BindString(8, JoinStrings(";", url.input_encodings()));
  s->BindInt(9, url.show_in_default_list() ? 1 : 0);
  if (url.suggestions_url())
    s->BindString(10, url.suggestions_url()->url());
  else
    s->BindString(10, std::string());
  s->BindInt(11, url.prepopulate_id());
  s->BindInt(12, url.autogenerate_keyword() ? 1 : 0);
  s->BindInt(13, url.logo_id());
}

void InitPasswordFormFromStatement(PasswordForm* form, sql::Statement* s) {
  std::string tmp;
  string16 decrypted_password;
  tmp = s->ColumnString(0);
  form->origin = GURL(tmp);
  tmp = s->ColumnString(1);
  form->action = GURL(tmp);
  form->username_element = s->ColumnString16(2);
  form->username_value = s->ColumnString16(3);
  form->password_element = s->ColumnString16(4);

  int encrypted_password_len = s->ColumnByteLength(5);
  std::string encrypted_password;
  if (encrypted_password_len) {
    encrypted_password.resize(encrypted_password_len);
    memcpy(&encrypted_password[0], s->ColumnBlob(5), encrypted_password_len);
    Encryptor::DecryptString16(encrypted_password, &decrypted_password);
  }

  form->password_value = decrypted_password;
  form->submit_element = s->ColumnString16(6);
  tmp = s->ColumnString(7);
  form->signon_realm = tmp;
  form->ssl_valid = (s->ColumnInt(8) > 0);
  form->preferred = (s->ColumnInt(9) > 0);
  form->date_created = Time::FromTimeT(s->ColumnInt64(10));
  form->blacklisted_by_user = (s->ColumnInt(11) > 0);
  int scheme_int = s->ColumnInt(12);
  DCHECK((scheme_int >= 0) && (scheme_int <= PasswordForm::SCHEME_OTHER));
  form->scheme = static_cast<PasswordForm::Scheme>(scheme_int);
}

// TODO(jhawkins): This is a temporary stop-gap measure designed to prevent
// a malicious site from DOS'ing the browser with extremely large profile
// data.  The correct solution is to parse this data asynchronously.
// See http://crbug.com/49332.
string16 LimitDataSize(const string16& data) {
  if (data.size() > kMaxDataLength)
    return data.substr(0, kMaxDataLength);

  return data;
}

void BindAutoFillProfileToStatement(const AutoFillProfile& profile,
                                    sql::Statement* s) {
  s->BindString16(0, profile.Label());
  s->BindInt(1, profile.unique_id());

  string16 text = profile.GetFieldText(AutoFillType(NAME_FIRST));
  s->BindString16(2, LimitDataSize(text));
  text = profile.GetFieldText(AutoFillType(NAME_MIDDLE));
  s->BindString16(3, LimitDataSize(text));
  text = profile.GetFieldText(AutoFillType(NAME_LAST));
  s->BindString16(4, LimitDataSize(text));
  text = profile.GetFieldText(AutoFillType(EMAIL_ADDRESS));
  s->BindString16(5, LimitDataSize(text));
  text = profile.GetFieldText(AutoFillType(COMPANY_NAME));
  s->BindString16(6, LimitDataSize(text));
  text = profile.GetFieldText(AutoFillType(ADDRESS_HOME_LINE1));
  s->BindString16(7, LimitDataSize(text));
  text = profile.GetFieldText(AutoFillType(ADDRESS_HOME_LINE2));
  s->BindString16(8, LimitDataSize(text));
  text = profile.GetFieldText(AutoFillType(ADDRESS_HOME_CITY));
  s->BindString16(9, LimitDataSize(text));
  text = profile.GetFieldText(AutoFillType(ADDRESS_HOME_STATE));
  s->BindString16(10, LimitDataSize(text));
  text = profile.GetFieldText(AutoFillType(ADDRESS_HOME_ZIP));
  s->BindString16(11, LimitDataSize(text));
  text = profile.GetFieldText(AutoFillType(ADDRESS_HOME_COUNTRY));
  s->BindString16(12, LimitDataSize(text));
  text = profile.GetFieldText(AutoFillType(PHONE_HOME_WHOLE_NUMBER));
  s->BindString16(13, LimitDataSize(text));
  text = profile.GetFieldText(AutoFillType(PHONE_FAX_WHOLE_NUMBER));
  s->BindString16(14, LimitDataSize(text));
}

AutoFillProfile* AutoFillProfileFromStatement(const sql::Statement& s) {
  AutoFillProfile* profile = new AutoFillProfile(
      s.ColumnString16(0), s.ColumnInt(1));
  profile->SetInfo(AutoFillType(NAME_FIRST),
                   s.ColumnString16(2));
  profile->SetInfo(AutoFillType(NAME_MIDDLE),
                   s.ColumnString16(3));
  profile->SetInfo(AutoFillType(NAME_LAST),
                   s.ColumnString16(4));
  profile->SetInfo(AutoFillType(EMAIL_ADDRESS),
                   s.ColumnString16(5));
  profile->SetInfo(AutoFillType(COMPANY_NAME),
                   s.ColumnString16(6));
  profile->SetInfo(AutoFillType(ADDRESS_HOME_LINE1),
                   s.ColumnString16(7));
  profile->SetInfo(AutoFillType(ADDRESS_HOME_LINE2),
                   s.ColumnString16(8));
  profile->SetInfo(AutoFillType(ADDRESS_HOME_CITY),
                   s.ColumnString16(9));
  profile->SetInfo(AutoFillType(ADDRESS_HOME_STATE),
                   s.ColumnString16(10));
  profile->SetInfo(AutoFillType(ADDRESS_HOME_ZIP),
                   s.ColumnString16(11));
  profile->SetInfo(AutoFillType(ADDRESS_HOME_COUNTRY),
                   s.ColumnString16(12));
  profile->SetInfo(AutoFillType(PHONE_HOME_WHOLE_NUMBER),
                   s.ColumnString16(13));
  profile->SetInfo(AutoFillType(PHONE_FAX_WHOLE_NUMBER),
                   s.ColumnString16(14));

  return profile;
}

void BindCreditCardToStatement(const CreditCard& credit_card,
                               sql::Statement* s) {
  s->BindString16(0, credit_card.Label());
  s->BindInt(1, credit_card.unique_id());

  string16 text = credit_card.GetFieldText(AutoFillType(CREDIT_CARD_NAME));
  s->BindString16(2, LimitDataSize(text));
  text = credit_card.GetFieldText(AutoFillType(CREDIT_CARD_TYPE));
  s->BindString16(3, LimitDataSize(text));
  text.clear();  // No unencrypted cc info.
  s->BindString16(4, LimitDataSize(text));
  text = credit_card.GetFieldText(AutoFillType(CREDIT_CARD_EXP_MONTH));
  s->BindString16(5, LimitDataSize(text));
  text = credit_card.GetFieldText(AutoFillType(CREDIT_CARD_EXP_4_DIGIT_YEAR));
  s->BindString16(6, LimitDataSize(text));
  text.clear();
  s->BindString16(7, LimitDataSize(text));
  s->BindString16(8, credit_card.billing_address());
  // We don't store the shipping address anymore.
  text.clear();
  s->BindString16(9, LimitDataSize(text));
  text = credit_card.GetFieldText(AutoFillType(CREDIT_CARD_NUMBER));
  std::string encrypted_data;
  Encryptor::EncryptString16(text, &encrypted_data);
  s->BindBlob(10, encrypted_data.data(),
              static_cast<int>(encrypted_data.length()));
  // We don't store the CVV anymore.
  text.clear();
  s->BindBlob(11, text.data(), static_cast<int>(text.length()));
}

CreditCard* CreditCardFromStatement(const sql::Statement& s) {
  CreditCard* credit_card = new CreditCard(
      s.ColumnString16(0), s.ColumnInt(1));
  credit_card->SetInfo(AutoFillType(CREDIT_CARD_NAME),
                       s.ColumnString16(2));
  credit_card->SetInfo(AutoFillType(CREDIT_CARD_TYPE),
                       s.ColumnString16(3));
  string16 credit_card_number = s.ColumnString16(4);
  // It could be non-empty prior to version 23. After that it encrypted in
  // the column 10.
  if (credit_card_number.empty()) {
    int encrypted_cc_len = s.ColumnByteLength(10);
    std::string encrypted_cc;
    if (encrypted_cc_len) {
      encrypted_cc.resize(encrypted_cc_len);
      memcpy(&encrypted_cc[0], s.ColumnBlob(10), encrypted_cc_len);
      Encryptor::DecryptString16(encrypted_cc, &credit_card_number);
    }
  }
  credit_card->SetInfo(AutoFillType(CREDIT_CARD_NUMBER), credit_card_number);
  credit_card->SetInfo(AutoFillType(CREDIT_CARD_EXP_MONTH),
                       s.ColumnString16(5));
  credit_card->SetInfo(AutoFillType(CREDIT_CARD_EXP_4_DIGIT_YEAR),
                       s.ColumnString16(6));

  string16 credit_card_verification_code = s.ColumnString16(7);
  // We don't store the CVV anymore.
  credit_card->set_billing_address(s.ColumnString16(8));
  // We don't store the shipping address anymore.
  // Column 10 is processed above.
  // Column 11 is processed above.

  return credit_card;
}

}  // namespace

WebDatabase::WebDatabase() {
}

WebDatabase::~WebDatabase() {
}

void WebDatabase::BeginTransaction() {
  db_.BeginTransaction();
}

void WebDatabase::CommitTransaction() {
  db_.CommitTransaction();
}

sql::InitStatus WebDatabase::Init(const FilePath& db_name) {
  // When running in unit tests, there is already a NotificationService object.
  // Since only one can exist at a time per thread, check first.
  if (!NotificationService::current())
    notification_service_.reset(new NotificationService);

  // Set the exceptional sqlite error handler.
  db_.set_error_delegate(GetErrorHandlerForWebDb());

  // We don't store that much data in the tables so use a small page size.
  // This provides a large benefit for empty tables (which is very likely with
  // the tables we create).
  db_.set_page_size(2048);

  // We shouldn't have much data and what access we currently have is quite
  // infrequent. So we go with a small cache size.
  db_.set_cache_size(32);

  // Run the database in exclusive mode. Nobody else should be accessing the
  // database while we're running, and this will give somewhat improved perf.
  db_.set_exclusive_locking();

  if (!db_.Open(db_name))
    return sql::INIT_FAILURE;

  // Initialize various tables
  sql::Transaction transaction(&db_);
  if (!transaction.Begin())
    return sql::INIT_FAILURE;

  // Version check.
  if (!meta_table_.Init(&db_, kCurrentVersionNumber, kCompatibleVersionNumber))
    return sql::INIT_FAILURE;
  if (meta_table_.GetCompatibleVersionNumber() > kCurrentVersionNumber) {
    LOG(WARNING) << "Web database is too new.";
    return sql::INIT_TOO_NEW;
  }

  // Initialize the tables.
  if (!InitKeywordsTable() || !InitLoginsTable() || !InitWebAppIconsTable() ||
      !InitWebAppsTable() || !InitAutofillTable() ||
      !InitAutofillDatesTable() || !InitAutoFillProfilesTable() ||
      !InitCreditCardsTable() || !InitTokenServiceTable()) {
    LOG(WARNING) << "Unable to initialize the web database.";
    return sql::INIT_FAILURE;
  }

  // If the file on disk is an older database version, bring it up to date.
  MigrateOldVersionsAsNeeded();

  return transaction.Commit() ? sql::INIT_OK : sql::INIT_FAILURE;
}

bool WebDatabase::SetWebAppImage(const GURL& url, const SkBitmap& image) {
  // Don't bother with a cached statement since this will be a relatively
  // infrequent operation.
  sql::Statement s(db_.GetUniqueStatement(
      "INSERT OR REPLACE INTO web_app_icons "
      "(url, width, height, image) VALUES (?, ?, ?, ?)"));
  if (!s)
    return false;

  std::vector<unsigned char> image_data;
  gfx::PNGCodec::EncodeBGRASkBitmap(image, false, &image_data);

  s.BindString(0, history::HistoryDatabase::GURLToDatabaseURL(url));
  s.BindInt(1, image.width());
  s.BindInt(2, image.height());
  s.BindBlob(3, &image_data.front(), static_cast<int>(image_data.size()));
  return s.Run();
}

bool WebDatabase::GetWebAppImages(const GURL& url,
                                  std::vector<SkBitmap>* images) {
  sql::Statement s(db_.GetUniqueStatement(
      "SELECT image FROM web_app_icons WHERE url=?"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }
  s.BindString(0, history::HistoryDatabase::GURLToDatabaseURL(url));
  while (s.Step()) {
    SkBitmap image;
    int col_bytes = s.ColumnByteLength(0);
    if (col_bytes > 0) {
      if (gfx::PNGCodec::Decode(
              reinterpret_cast<const unsigned char*>(s.ColumnBlob(0)),
              col_bytes, &image)) {
        images->push_back(image);
      } else {
        // Should only have valid image data in the db.
        NOTREACHED();
      }
    }
  }
  return true;
}

bool WebDatabase::SetWebAppHasAllImages(const GURL& url,
                                        bool has_all_images) {
  sql::Statement s(db_.GetUniqueStatement(
      "INSERT OR REPLACE INTO web_apps (url, has_all_images) VALUES (?, ?)"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }
  s.BindString(0, history::HistoryDatabase::GURLToDatabaseURL(url));
  s.BindInt(1, has_all_images ? 1 : 0);
  return s.Run();
}

bool WebDatabase::GetWebAppHasAllImages(const GURL& url) {
  sql::Statement s(db_.GetUniqueStatement(
      "SELECT has_all_images FROM web_apps WHERE url=?"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }
  s.BindString(0, history::HistoryDatabase::GURLToDatabaseURL(url));
  return (s.Step() && s.ColumnInt(0) == 1);
}

bool WebDatabase::RemoveWebApp(const GURL& url) {
  sql::Statement delete_s(db_.GetUniqueStatement(
      "DELETE FROM web_app_icons WHERE url = ?"));
  if (!delete_s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }
  delete_s.BindString(0, history::HistoryDatabase::GURLToDatabaseURL(url));
  if (!delete_s.Run())
    return false;

  sql::Statement delete_s2(db_.GetUniqueStatement(
      "DELETE FROM web_apps WHERE url = ?"));
  if (!delete_s2) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }
  delete_s2.BindString(0, history::HistoryDatabase::GURLToDatabaseURL(url));
  return delete_s2.Run();
}

bool WebDatabase::RemoveAllTokens() {
  sql::Statement s(db_.GetUniqueStatement(
      "DELETE FROM token_service"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  return s.Run();
}

bool WebDatabase::SetTokenForService(const std::string& service,
                                     const std::string& token) {
  // Don't bother with a cached statement since this will be a relatively
  // infrequent operation.
  sql::Statement s(db_.GetUniqueStatement(
      "INSERT OR REPLACE INTO token_service "
      "(service, encrypted_token) VALUES (?, ?)"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  std::string encrypted_token;

  bool encrypted = Encryptor::EncryptString(token, &encrypted_token);
  if (!encrypted) {
    return false;
  }

  s.BindString(0, service);
  s.BindBlob(1, encrypted_token.data(),
             static_cast<int>(encrypted_token.length()));
  return s.Run();
}

bool WebDatabase::GetAllTokens(std::map<std::string, std::string>* tokens) {
  sql::Statement s(db_.GetUniqueStatement(
      "SELECT service, encrypted_token FROM token_service"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  while (s.Step()) {
    std::string encrypted_token;
    std::string decrypted_token;
    std::string service;
    service = s.ColumnString(0);
    bool entry_ok = !service.empty() &&
                    s.ColumnBlobAsString(1, &encrypted_token);
    if (entry_ok) {
      Encryptor::DecryptString(encrypted_token, &decrypted_token);
      (*tokens)[service] = decrypted_token;
    } else {
      NOTREACHED();
      return false;
    }
  }
  return true;
}

bool WebDatabase::InitKeywordsTable() {
  if (!db_.DoesTableExist("keywords")) {
    if (!db_.Execute("CREATE TABLE keywords ("
                     "id INTEGER PRIMARY KEY,"
                     "short_name VARCHAR NOT NULL,"
                     "keyword VARCHAR NOT NULL,"
                     "favicon_url VARCHAR NOT NULL,"
                     "url VARCHAR NOT NULL,"
                     "show_in_default_list INTEGER,"
                     "safe_for_autoreplace INTEGER,"
                     "originating_url VARCHAR,"
                     "date_created INTEGER DEFAULT 0,"
                     "usage_count INTEGER DEFAULT 0,"
                     "input_encodings VARCHAR,"
                     "suggest_url VARCHAR,"
                     "prepopulate_id INTEGER DEFAULT 0,"
                     "autogenerate_keyword INTEGER DEFAULT 0,"
                     "logo_id INTEGER DEFAULT 0)")) {
      NOTREACHED();
      return false;
    }
  }
  return true;
}

bool WebDatabase::InitLoginsTable() {
  if (!db_.DoesTableExist("logins")) {
    if (!db_.Execute("CREATE TABLE logins ("
                     "origin_url VARCHAR NOT NULL, "
                     "action_url VARCHAR, "
                     "username_element VARCHAR, "
                     "username_value VARCHAR, "
                     "password_element VARCHAR, "
                     "password_value BLOB, "
                     "submit_element VARCHAR, "
                     "signon_realm VARCHAR NOT NULL,"
                     "ssl_valid INTEGER NOT NULL,"
                     "preferred INTEGER NOT NULL,"
                     "date_created INTEGER NOT NULL,"
                     "blacklisted_by_user INTEGER NOT NULL,"
                     "scheme INTEGER NOT NULL,"
                     "UNIQUE "
                     "(origin_url, username_element, "
                     "username_value, password_element, "
                     "submit_element, signon_realm))")) {
      NOTREACHED();
      return false;
    }
    if (!db_.Execute("CREATE INDEX logins_signon ON logins (signon_realm)")) {
      NOTREACHED();
      return false;
    }
  }

#if defined(OS_WIN)
  if (!db_.DoesTableExist("ie7_logins")) {
    if (!db_.Execute("CREATE TABLE ie7_logins ("
                     "url_hash VARCHAR NOT NULL, "
                     "password_value BLOB, "
                     "date_created INTEGER NOT NULL,"
                     "UNIQUE "
                     "(url_hash))")) {
      NOTREACHED();
      return false;
    }
    if (!db_.Execute("CREATE INDEX ie7_logins_hash ON "
                     "ie7_logins (url_hash)")) {
      NOTREACHED();
      return false;
    }
  }
#endif

  return true;
}

bool WebDatabase::InitAutofillTable() {
  if (!db_.DoesTableExist("autofill")) {
    if (!db_.Execute("CREATE TABLE autofill ("
                     "name VARCHAR, "
                     "value VARCHAR, "
                     "value_lower VARCHAR, "
                     "pair_id INTEGER PRIMARY KEY, "
                     "count INTEGER DEFAULT 1)")) {
      NOTREACHED();
      return false;
    }
    if (!db_.Execute("CREATE INDEX autofill_name ON autofill (name)")) {
       NOTREACHED();
       return false;
    }
    if (!db_.Execute("CREATE INDEX autofill_name_value_lower ON "
                     "autofill (name, value_lower)")) {
       NOTREACHED();
       return false;
    }
  }
  return true;
}

bool WebDatabase::InitAutofillDatesTable() {
  if (!db_.DoesTableExist("autofill_dates")) {
    if (!db_.Execute("CREATE TABLE autofill_dates ( "
                     "pair_id INTEGER DEFAULT 0, "
                     "date_created INTEGER DEFAULT 0)")) {
      NOTREACHED();
      return false;
    }
    if (!db_.Execute("CREATE INDEX autofill_dates_pair_id ON "
                     "autofill_dates (pair_id)")) {
      NOTREACHED();
      return false;
    }
  }
  return true;
}

bool WebDatabase::InitAutoFillProfilesTable() {
  if (!db_.DoesTableExist("autofill_profiles")) {
    if (!db_.Execute("CREATE TABLE autofill_profiles ( "
                     "label VARCHAR, "
                     "unique_id INTEGER PRIMARY KEY, "
                     "first_name VARCHAR, "
                     "middle_name VARCHAR, "
                     "last_name VARCHAR, "
                     "email VARCHAR, "
                     "company_name VARCHAR, "
                     "address_line_1 VARCHAR, "
                     "address_line_2 VARCHAR, "
                     "city VARCHAR, "
                     "state VARCHAR, "
                     "zipcode VARCHAR, "
                     "country VARCHAR, "
                     "phone VARCHAR, "
                     "fax VARCHAR)")) {
      NOTREACHED();
      return false;
    }
    if (!db_.Execute("CREATE INDEX autofill_profiles_label_index "
                     "ON autofill_profiles (label)")) {
      NOTREACHED();
      return false;
    }
  }
  return true;
}

bool WebDatabase::InitCreditCardsTable() {
  if (!db_.DoesTableExist("credit_cards")) {
    if (!db_.Execute("CREATE TABLE credit_cards ( "
                     "label VARCHAR, "
                     "unique_id INTEGER PRIMARY KEY, "
                     "name_on_card VARCHAR, "
                     "type VARCHAR, "
                     "card_number VARCHAR, "
                     "expiration_month INTEGER, "
                     "expiration_year INTEGER, "
                     "verification_code VARCHAR, "
                     "billing_address VARCHAR, "
                     "shipping_address VARCHAR, "
                     "card_number_encrypted BLOB, "
                     "verification_code_encrypted BLOB)")) {
      NOTREACHED();
      return false;
    }
    if (!db_.Execute("CREATE INDEX credit_cards_label_index "
                     "ON credit_cards (label)")) {
      NOTREACHED();
      return false;
    }
  }
  return true;
}

bool WebDatabase::InitWebAppIconsTable() {
  if (!db_.DoesTableExist("web_app_icons")) {
    if (!db_.Execute("CREATE TABLE web_app_icons ("
                     "url LONGVARCHAR,"
                     "width int,"
                     "height int,"
                     "image BLOB, UNIQUE (url, width, height))")) {
      NOTREACHED();
      return false;
    }
  }
  return true;
}

bool WebDatabase::InitWebAppsTable() {
  if (!db_.DoesTableExist("web_apps")) {
    if (!db_.Execute("CREATE TABLE web_apps ("
                     "url LONGVARCHAR UNIQUE,"
                     "has_all_images INTEGER NOT NULL)")) {
      NOTREACHED();
      return false;
    }
    if (!db_.Execute("CREATE INDEX web_apps_url_index ON web_apps (url)")) {
      NOTREACHED();
      return false;
    }
  }
  return true;
}

bool WebDatabase::InitTokenServiceTable() {
  if (!db_.DoesTableExist("token_service")) {
    if (!db_.Execute("CREATE TABLE token_service ("
                     "service VARCHAR PRIMARY KEY NOT NULL,"
                     "encrypted_token BLOB)")) {
      NOTREACHED();
      return false;
    }
  }
  return true;
}

bool WebDatabase::AddKeyword(const TemplateURL& url) {
  DCHECK(url.id());
  sql::Statement s(db_.GetCachedStatement(SQL_FROM_HERE,
      "INSERT INTO keywords "
      "(short_name, keyword, favicon_url, url, safe_for_autoreplace, "
      "originating_url, date_created, usage_count, input_encodings, "
      "show_in_default_list, suggest_url, prepopulate_id, "
      "autogenerate_keyword, logo_id, id) VALUES "
      "(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }
  BindURLToStatement(url, &s);
  s.BindInt64(kUrlIdPosition, url.id());
  if (!s.Run()) {
    NOTREACHED();
    return false;
  }
  return true;
}

bool WebDatabase::RemoveKeyword(TemplateURLID id) {
  DCHECK(id);
  sql::Statement s(db_.GetUniqueStatement("DELETE FROM keywords WHERE id = ?"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }
  s.BindInt64(0, id);
  return s.Run();
}

bool WebDatabase::GetKeywords(std::vector<TemplateURL*>* urls) {
  sql::Statement s(db_.GetUniqueStatement(
      "SELECT id, short_name, keyword, favicon_url, url, "
      "safe_for_autoreplace, originating_url, date_created, "
      "usage_count, input_encodings, show_in_default_list, "
      "suggest_url, prepopulate_id, autogenerate_keyword, logo_id "
      "FROM keywords ORDER BY id ASC"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }
  while (s.Step()) {
    TemplateURL* template_url = new TemplateURL();
    template_url->set_id(s.ColumnInt64(0));

    std::string tmp;
    tmp = s.ColumnString(1);
    DCHECK(!tmp.empty());
    template_url->set_short_name(UTF8ToWide(tmp));

    tmp = s.ColumnString(2);
    template_url->set_keyword(UTF8ToWide(tmp));

    tmp = s.ColumnString(3);
    if (!tmp.empty())
      template_url->SetFavIconURL(GURL(tmp));

    tmp = s.ColumnString(4);
    template_url->SetURL(tmp, 0, 0);

    template_url->set_safe_for_autoreplace(s.ColumnInt(5) == 1);

    tmp = s.ColumnString(6);
    if (!tmp.empty())
      template_url->set_originating_url(GURL(tmp));

    template_url->set_date_created(Time::FromTimeT(s.ColumnInt64(7)));

    template_url->set_usage_count(s.ColumnInt(8));

    std::vector<std::string> encodings;
    SplitString(s.ColumnString(9), ';', &encodings);
    template_url->set_input_encodings(encodings);

    template_url->set_show_in_default_list(s.ColumnInt(10) == 1);

    tmp = s.ColumnString(11);
    template_url->SetSuggestionsURL(tmp, 0, 0);

    template_url->set_prepopulate_id(s.ColumnInt(12));

    template_url->set_autogenerate_keyword(s.ColumnInt(13) == 1);

    template_url->set_logo_id(s.ColumnInt(14));

    urls->push_back(template_url);
  }
  return s.Succeeded();
}

bool WebDatabase::UpdateKeyword(const TemplateURL& url) {
  DCHECK(url.id());
  sql::Statement s(db_.GetUniqueStatement(
      "UPDATE keywords "
      "SET short_name=?, keyword=?, favicon_url=?, url=?, "
      "safe_for_autoreplace=?, originating_url=?, date_created=?, "
      "usage_count=?, input_encodings=?, show_in_default_list=?, "
      "suggest_url=?, prepopulate_id=?, autogenerate_keyword=?, "
      "logo_id=? WHERE id=?"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }
  BindURLToStatement(url, &s);
  s.BindInt64(kUrlIdPosition, url.id());
  return s.Run();
}

bool WebDatabase::SetDefaultSearchProviderID(int64 id) {
  return meta_table_.SetValue(kDefaultSearchProviderKey, id);
}

int64 WebDatabase::GetDefaulSearchProviderID() {
  int64 value = 0;
  meta_table_.GetValue(kDefaultSearchProviderKey, &value);
  return value;
}

bool WebDatabase::SetBuitinKeywordVersion(int version) {
  return meta_table_.SetValue(kBuiltinKeywordVersion, version);
}

int WebDatabase::GetBuitinKeywordVersion() {
  int version = 0;
  meta_table_.GetValue(kBuiltinKeywordVersion, &version);
  return version;
}

bool WebDatabase::AddLogin(const PasswordForm& form) {
  sql::Statement s(db_.GetUniqueStatement(
      "INSERT OR REPLACE INTO logins "
      "(origin_url, action_url, username_element, username_value, "
      " password_element, password_value, submit_element, "
      " signon_realm, ssl_valid, preferred, date_created, "
      " blacklisted_by_user, scheme) "
      "VALUES "
      "(?,?,?,?,?,?,?,?,?,?,?,?,?)"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  std::string encrypted_password;
  s.BindString(0, form.origin.spec());
  s.BindString(1, form.action.spec());
  s.BindString16(2, form.username_element);
  s.BindString16(3, form.username_value);
  s.BindString16(4, form.password_element);
  Encryptor::EncryptString16(form.password_value, &encrypted_password);
  s.BindBlob(5, encrypted_password.data(),
             static_cast<int>(encrypted_password.length()));
  s.BindString16(6, form.submit_element);
  s.BindString(7, form.signon_realm);
  s.BindInt(8, form.ssl_valid);
  s.BindInt(9, form.preferred);
  s.BindInt64(10, form.date_created.ToTimeT());
  s.BindInt(11, form.blacklisted_by_user);
  s.BindInt(12, form.scheme);
  if (!s.Run()) {
    NOTREACHED();
    return false;
  }
  return true;
}

bool WebDatabase::UpdateLogin(const PasswordForm& form) {
  sql::Statement s(db_.GetUniqueStatement(
      "UPDATE logins SET "
      "action_url = ?, "
      "password_value = ?, "
      "ssl_valid = ?, "
      "preferred = ? "
      "WHERE origin_url = ? AND "
      "username_element = ? AND "
      "username_value = ? AND "
      "password_element = ? AND "
      "signon_realm = ?"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  s.BindString(0, form.action.spec());
  std::string encrypted_password;
  Encryptor::EncryptString16(form.password_value, &encrypted_password);
  s.BindBlob(1, encrypted_password.data(),
             static_cast<int>(encrypted_password.length()));
  s.BindInt(2, form.ssl_valid);
  s.BindInt(3, form.preferred);
  s.BindString(4, form.origin.spec());
  s.BindString16(5, form.username_element);
  s.BindString16(6, form.username_value);
  s.BindString16(7, form.password_element);
  s.BindString(8, form.signon_realm);

  if (!s.Run()) {
    NOTREACHED();
    return false;
  }
  return true;
}

bool WebDatabase::RemoveLogin(const PasswordForm& form) {
  // Remove a login by UNIQUE-constrained fields.
  sql::Statement s(db_.GetUniqueStatement(
      "DELETE FROM logins WHERE "
      "origin_url = ? AND "
      "username_element = ? AND "
      "username_value = ? AND "
      "password_element = ? AND "
      "submit_element = ? AND "
      "signon_realm = ?"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }
  s.BindString(0, form.origin.spec());
  s.BindString16(1, form.username_element);
  s.BindString16(2, form.username_value);
  s.BindString16(3, form.password_element);
  s.BindString16(4, form.submit_element);
  s.BindString(5, form.signon_realm);

  if (!s.Run()) {
    NOTREACHED();
    return false;
  }
  return true;
}

bool WebDatabase::RemoveLoginsCreatedBetween(base::Time delete_begin,
                                             base::Time delete_end) {
  sql::Statement s1(db_.GetUniqueStatement(
      "DELETE FROM logins WHERE "
      "date_created >= ? AND date_created < ?"));
  if (!s1) {
    NOTREACHED() << "Statement 1 prepare failed";
    return false;
  }
  s1.BindInt64(0, delete_begin.ToTimeT());
  s1.BindInt64(1,
               delete_end.is_null() ?
                   std::numeric_limits<int64>::max() :
                   delete_end.ToTimeT());
  bool success = s1.Run();

#if defined(OS_WIN)
  sql::Statement s2(db_.GetUniqueStatement(
      "DELETE FROM ie7_logins WHERE date_created >= ? AND date_created < ?"));
  if (!s2) {
    NOTREACHED() << "Statement 2 prepare failed";
    return false;
  }
  s2.BindInt64(0, delete_begin.ToTimeT());
  s2.BindInt64(1,
               delete_end.is_null() ?
                   std::numeric_limits<int64>::max() :
                   delete_end.ToTimeT());
  success = success && s2.Run();
#endif

  return success;
}

bool WebDatabase::GetLogins(const PasswordForm& form,
                            std::vector<PasswordForm*>* forms) {
  DCHECK(forms);
  sql::Statement s(db_.GetUniqueStatement(
                "SELECT origin_url, action_url, "
                "username_element, username_value, "
                "password_element, password_value, "
                "submit_element, signon_realm, "
                "ssl_valid, preferred, "
                "date_created, blacklisted_by_user, scheme FROM logins "
                "WHERE signon_realm == ?"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  s.BindString(0, form.signon_realm);

  while (s.Step()) {
    PasswordForm* new_form = new PasswordForm();
    InitPasswordFormFromStatement(new_form, &s);

    forms->push_back(new_form);
  }
  return s.Succeeded();
}

bool WebDatabase::GetAllLogins(std::vector<PasswordForm*>* forms,
                               bool include_blacklisted) {
  DCHECK(forms);
  std::string stmt = "SELECT origin_url, action_url, "
                     "username_element, username_value, "
                     "password_element, password_value, "
                     "submit_element, signon_realm, ssl_valid, preferred, "
                     "date_created, blacklisted_by_user, scheme FROM logins ";
  if (!include_blacklisted)
    stmt.append("WHERE blacklisted_by_user == 0 ");
  stmt.append("ORDER BY origin_url");

  sql::Statement s(db_.GetUniqueStatement(stmt.c_str()));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  while (s.Step()) {
    PasswordForm* new_form = new PasswordForm();
    InitPasswordFormFromStatement(new_form, &s);

    forms->push_back(new_form);
  }
  return s.Succeeded();
}

bool WebDatabase::AddFormFieldValues(const std::vector<FormField>& elements,
                                     std::vector<AutofillChange>* changes) {
  return AddFormFieldValuesTime(elements, changes, Time::Now());
}

bool WebDatabase::AddFormFieldValuesTime(const std::vector<FormField>& elements,
                                         std::vector<AutofillChange>* changes,
                                         base::Time time) {
  // Only add one new entry for each unique element name.  Use |seen_names| to
  // track this.  Add up to |kMaximumUniqueNames| unique entries per form.
  const size_t kMaximumUniqueNames = 256;
  std::set<string16> seen_names;
  bool result = true;
  for (std::vector<FormField>::const_iterator
       itr = elements.begin();
       itr != elements.end();
       itr++) {
    if (seen_names.size() >= kMaximumUniqueNames)
      break;
    if (seen_names.find(itr->name()) != seen_names.end())
      continue;
    result = result && AddFormFieldValueTime(*itr, changes, time);
    seen_names.insert(itr->name());
  }
  return result;
}

bool WebDatabase::ClearAutofillEmptyValueElements() {
  sql::Statement s(db_.GetUniqueStatement(
      "SELECT pair_id FROM autofill WHERE TRIM(value)= \"\""));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  std::set<int64> ids;
  while (s.Step())
    ids.insert(s.ColumnInt64(0));

  bool success = true;
  for (std::set<int64>::const_iterator iter = ids.begin(); iter != ids.end();
       ++iter) {
    if (!RemoveFormElementForID(*iter))
      success = false;
  }

  return success;
}

bool WebDatabase::GetIDAndCountOfFormElement(
    const FormField& element,
    int64* pair_id,
    int* count) {
  sql::Statement s(db_.GetUniqueStatement(
      "SELECT pair_id, count FROM autofill "
      "WHERE name = ? AND value = ?"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  s.BindString16(0, element.name());
  s.BindString16(1, element.value());

  *count = 0;

  if (s.Step()) {
    *pair_id = s.ColumnInt64(0);
    *count = s.ColumnInt(1);
  }

  return true;
}

bool WebDatabase::GetCountOfFormElement(int64 pair_id, int* count) {
  sql::Statement s(db_.GetUniqueStatement(
      "SELECT count FROM autofill WHERE pair_id = ?"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  s.BindInt64(0, pair_id);

  if (s.Step()) {
    *count = s.ColumnInt(0);
    return true;
  }
  return false;
}

bool WebDatabase::GetAllAutofillEntries(std::vector<AutofillEntry>* entries) {
  DCHECK(entries);
  sql::Statement s(db_.GetUniqueStatement(
      "SELECT name, value, date_created FROM autofill a JOIN "
      "autofill_dates ad ON a.pair_id=ad.pair_id"));

  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  bool first_entry = true;
  AutofillKey* current_key_ptr = NULL;
  std::vector<base::Time>* timestamps_ptr = NULL;
  string16 name, value;
  base::Time time;
  while (s.Step()) {
    name = s.ColumnString16(0);
    value = s.ColumnString16(1);
    time = Time::FromTimeT(s.ColumnInt64(2));

    if (first_entry) {
      current_key_ptr = new AutofillKey(name, value);

      timestamps_ptr = new std::vector<base::Time>;
      timestamps_ptr->push_back(time);

      first_entry = false;
    } else {
      // we've encountered the next entry
      if (current_key_ptr->name().compare(name) != 0 ||
          current_key_ptr->value().compare(value) != 0) {
        AutofillEntry entry(*current_key_ptr, *timestamps_ptr);
        entries->push_back(entry);

        delete current_key_ptr;
        delete timestamps_ptr;

        current_key_ptr = new AutofillKey(name, value);
        timestamps_ptr = new std::vector<base::Time>;
      }
      timestamps_ptr->push_back(time);
    }
  }
  // If there is at least one result returned, first_entry will be false.
  // For this case we need to do a final cleanup step.
  if (!first_entry) {
    AutofillEntry entry(*current_key_ptr, *timestamps_ptr);
    entries->push_back(entry);
    delete current_key_ptr;
    delete timestamps_ptr;
  }

  return s.Succeeded();
}

bool WebDatabase::GetAutofillTimestamps(const string16& name,
                                        const string16& value,
                                        std::vector<base::Time>* timestamps) {
  DCHECK(timestamps);
  sql::Statement s(db_.GetUniqueStatement(
      "SELECT date_created FROM autofill a JOIN "
      "autofill_dates ad ON a.pair_id=ad.pair_id "
      "WHERE a.name = ? AND a.value = ?"));

  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  s.BindString16(0, name);
  s.BindString16(1, value);
  while (s.Step()) {
    timestamps->push_back(Time::FromTimeT(s.ColumnInt64(0)));
  }

  return s.Succeeded();
}

bool WebDatabase::UpdateAutofillEntries(
    const std::vector<AutofillEntry>& entries) {
  if (!entries.size())
    return true;

  // Remove all existing entries.
  for (size_t i = 0; i < entries.size(); i++) {
    std::string sql = "SELECT pair_id FROM autofill "
                      "WHERE name = ? AND value = ?";
    sql::Statement s(db_.GetUniqueStatement(sql.c_str()));
    if (!s.is_valid()) {
      NOTREACHED() << "Statement prepare failed";
      return false;
    }

    s.BindString16(0, entries[i].key().name());
    s.BindString16(1, entries[i].key().value());
    if (s.Step()) {
      if (!RemoveFormElementForID(s.ColumnInt64(0)))
        return false;
    }
  }

  // Insert all the supplied autofill entries.
  for (size_t i = 0; i < entries.size(); i++) {
    if (!InsertAutofillEntry(entries[i]))
      return false;
  }

  return true;
}

bool WebDatabase::InsertAutofillEntry(const AutofillEntry& entry) {
  std::string sql = "INSERT INTO autofill (name, value, value_lower, count) "
                    "VALUES (?, ?, ?, ?)";
  sql::Statement s(db_.GetUniqueStatement(sql.c_str()));
  if (!s.is_valid()) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  s.BindString16(0, entry.key().name());
  s.BindString16(1, entry.key().value());
  s.BindString16(2, l10n_util::ToLower(entry.key().value()));
  s.BindInt(3, entry.timestamps().size());

  if (!s.Run()) {
    NOTREACHED();
    return false;
  }

  int64 pair_id = db_.GetLastInsertRowId();
  for (size_t i = 0; i < entry.timestamps().size(); i++) {
    if (!InsertPairIDAndDate(pair_id, entry.timestamps()[i]))
      return false;
  }

  return true;
}

bool WebDatabase::InsertFormElement(const FormField& element,
                                    int64* pair_id) {
  sql::Statement s(db_.GetUniqueStatement(
      "INSERT INTO autofill (name, value, value_lower) VALUES (?,?,?)"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  s.BindString16(0, element.name());
  s.BindString16(1, element.value());
  s.BindString16(2, l10n_util::ToLower(element.value()));

  if (!s.Run()) {
    NOTREACHED();
    return false;
  }

  *pair_id = db_.GetLastInsertRowId();
  return true;
}

bool WebDatabase::InsertPairIDAndDate(int64 pair_id,
                                      base::Time date_created) {
  sql::Statement s(db_.GetUniqueStatement(
      "INSERT INTO autofill_dates "
      "(pair_id, date_created) VALUES (?, ?)"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  s.BindInt64(0, pair_id);
  s.BindInt64(1, date_created.ToTimeT());

  if (!s.Run()) {
    NOTREACHED();
    return false;
  }

  return true;
}

bool WebDatabase::SetCountOfFormElement(int64 pair_id, int count) {
  sql::Statement s(db_.GetUniqueStatement(
      "UPDATE autofill SET count = ? WHERE pair_id = ?"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  s.BindInt(0, count);
  s.BindInt64(1, pair_id);
  if (!s.Run()) {
    NOTREACHED();
    return false;
  }

  return true;
}

bool WebDatabase::AddFormFieldValue(const FormField& element,
                                    std::vector<AutofillChange>* changes) {
  return AddFormFieldValueTime(element, changes, base::Time::Now());
}

bool WebDatabase::AddFormFieldValueTime(const FormField& element,
                                        std::vector<AutofillChange>* changes,
                                        base::Time time) {
  int count = 0;
  int64 pair_id;

  if (!GetIDAndCountOfFormElement(element, &pair_id, &count))
    return false;

  if (count == 0 && !InsertFormElement(element, &pair_id))
    return false;

  if (!SetCountOfFormElement(pair_id, count + 1))
    return false;

  if (!InsertPairIDAndDate(pair_id, time))
    return false;

  AutofillChange::Type change_type =
      count == 0 ? AutofillChange::ADD : AutofillChange::UPDATE;
  changes->push_back(
      AutofillChange(change_type,
                     AutofillKey(element.name(), element.value())));
  return true;
}

bool WebDatabase::GetFormValuesForElementName(const string16& name,
                                              const string16& prefix,
                                              std::vector<string16>* values,
                                              int limit) {
  DCHECK(values);
  sql::Statement s;

  if (prefix.empty()) {
    s.Assign(db_.GetUniqueStatement(
        "SELECT value FROM autofill "
        "WHERE name = ? "
        "ORDER BY count DESC "
        "LIMIT ?"));
    if (!s) {
      NOTREACHED() << "Statement prepare failed";
      return false;
    }

    s.BindString16(0, name);
    s.BindInt(1, limit);
  } else {
    string16 prefix_lower = l10n_util::ToLower(prefix);
    string16 next_prefix = prefix_lower;
    next_prefix[next_prefix.length() - 1]++;

    s.Assign(db_.GetUniqueStatement(
        "SELECT value FROM autofill "
        "WHERE name = ? AND "
        "value_lower >= ? AND "
        "value_lower < ? "
        "ORDER BY count DESC "
        "LIMIT ?"));
    if (!s) {
      NOTREACHED() << "Statement prepare failed";
      return false;
    }

    s.BindString16(0, name);
    s.BindString16(1, prefix_lower);
    s.BindString16(2, next_prefix);
    s.BindInt(3, limit);
  }

  values->clear();
  while (s.Step())
    values->push_back(s.ColumnString16(0));
  return s.Succeeded();
}

bool WebDatabase::RemoveFormElementsAddedBetween(
    base::Time delete_begin,
    base::Time delete_end,
    std::vector<AutofillChange>* changes) {
  DCHECK(changes);
  // Query for the pair_id, name, and value of all form elements that
  // were used between the given times.
  sql::Statement s(db_.GetUniqueStatement(
      "SELECT DISTINCT a.pair_id, a.name, a.value "
      "FROM autofill_dates ad JOIN autofill a ON ad.pair_id = a.pair_id "
      "WHERE ad.date_created >= ? AND ad.date_created < ?"));
  if (!s) {
    NOTREACHED() << "Statement 1 prepare failed";
    return false;
  }
  s.BindInt64(0, delete_begin.ToTimeT());
  s.BindInt64(1,
              delete_end.is_null() ?
                  std::numeric_limits<int64>::max() :
                  delete_end.ToTimeT());

  AutofillElementList elements;
  while (s.Step()) {
    elements.push_back(MakeTuple(s.ColumnInt64(0),
                                 s.ColumnString16(1),
                                 s.ColumnString16(2)));
  }

  if (!s.Succeeded()) {
    NOTREACHED();
    return false;
  }

  for (AutofillElementList::iterator itr = elements.begin();
       itr != elements.end(); itr++) {
    int how_many = 0;
    if (!RemoveFormElementForTimeRange(itr->a, delete_begin, delete_end,
                                       &how_many)) {
      return false;
    }
    bool was_removed = false;
    if (!AddToCountOfFormElement(itr->a, -how_many, &was_removed))
      return false;
    AutofillChange::Type change_type =
        was_removed ? AutofillChange::REMOVE : AutofillChange::UPDATE;
    changes->push_back(AutofillChange(change_type,
                                      AutofillKey(itr->b, itr->c)));
  }

  return true;
}

bool WebDatabase::RemoveFormElementForTimeRange(int64 pair_id,
                                                const Time delete_begin,
                                                const Time delete_end,
                                                int* how_many) {
  sql::Statement s(db_.GetUniqueStatement(
      "DELETE FROM autofill_dates WHERE pair_id = ? AND "
      "date_created >= ? AND date_created < ?"));
  if (!s) {
    NOTREACHED() << "Statement 1 prepare failed";
    return false;
  }
  s.BindInt64(0, pair_id);
  s.BindInt64(1, delete_begin.is_null() ? 0 : delete_begin.ToTimeT());
  s.BindInt64(2, delete_end.is_null() ? std::numeric_limits<int64>::max() :
                                        delete_end.ToTimeT());

  bool result = s.Run();
  if (how_many)
    *how_many = db_.GetLastChangeCount();

  return result;
}

bool WebDatabase::RemoveFormElement(const string16& name,
                                    const string16& value) {
  // Find the id for that pair.
  sql::Statement s(db_.GetUniqueStatement(
      "SELECT pair_id FROM autofill WHERE  name = ? AND value= ?"));
  if (!s) {
    NOTREACHED() << "Statement 1 prepare failed";
    return false;
  }
  s.BindString16(0, name);
  s.BindString16(1, value);

  if (s.Step())
    return RemoveFormElementForID(s.ColumnInt64(0));
  return false;
}

bool WebDatabase::AddAutoFillProfile(const AutoFillProfile& profile) {
  sql::Statement s(db_.GetUniqueStatement(
      "INSERT INTO autofill_profiles"
      "(label, unique_id, first_name, middle_name, last_name, email,"
      " company_name, address_line_1, address_line_2, city, state, zipcode,"
      " country, phone, fax)"
      "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  BindAutoFillProfileToStatement(profile, &s);

  if (!s.Run()) {
    NOTREACHED();
    return false;
  }

  return s.Succeeded();
}

bool WebDatabase::GetAutoFillProfileForLabel(const string16& label,
                                             AutoFillProfile** profile) {
  DCHECK(profile);
  sql::Statement s(db_.GetUniqueStatement(
      "SELECT * FROM autofill_profiles "
      "WHERE label = ?"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  s.BindString16(0, label);
  if (!s.Step())
    return false;

  *profile = AutoFillProfileFromStatement(s);

  return s.Succeeded();
}

bool WebDatabase::GetAutoFillProfiles(
    std::vector<AutoFillProfile*>* profiles) {
  DCHECK(profiles);
  profiles->clear();

  sql::Statement s(db_.GetUniqueStatement("SELECT * FROM autofill_profiles"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  while (s.Step())
    profiles->push_back(AutoFillProfileFromStatement(s));

  return s.Succeeded();
}

bool WebDatabase::UpdateAutoFillProfile(const AutoFillProfile& profile) {
  DCHECK(profile.unique_id());
  sql::Statement s(db_.GetUniqueStatement(
      "UPDATE autofill_profiles "
      "SET label=?, unique_id=?, first_name=?, middle_name=?, last_name=?, "
      "    email=?, company_name=?, address_line_1=?, address_line_2=?, "
      "    city=?, state=?, zipcode=?, country=?, phone=?, fax=? "
      "WHERE unique_id=?"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  BindAutoFillProfileToStatement(profile, &s);
  s.BindInt(15, profile.unique_id());
  bool result = s.Run();
  DCHECK_GT(db_.GetLastChangeCount(), 0);
  return result;
}

bool WebDatabase::RemoveAutoFillProfile(int profile_id) {
  DCHECK_NE(0, profile_id);
  sql::Statement s(db_.GetUniqueStatement(
      "DELETE FROM autofill_profiles WHERE unique_id = ?"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  s.BindInt(0, profile_id);
  return s.Run();
}

bool WebDatabase::GetAutoFillProfileForID(int profile_id,
    AutoFillProfile** profile) {
  sql::Statement s(db_.GetUniqueStatement(
      "SELECT * FROM autofill_profiles "
      "WHERE unique_id = ?"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  s.BindInt(0, profile_id);
  if (s.Step())
    *profile = AutoFillProfileFromStatement(s);

  return s.Succeeded();
}

bool WebDatabase::AddCreditCard(const CreditCard& credit_card) {
  sql::Statement s(db_.GetUniqueStatement(
      "INSERT INTO credit_cards"
      "(label, unique_id, name_on_card, type, card_number,"
      " expiration_month, expiration_year, verification_code, billing_address,"
      " shipping_address, card_number_encrypted, verification_code_encrypted)"
      "VALUES (?,?,?,?,?,?,?,?,?,?,?,?)"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  BindCreditCardToStatement(credit_card, &s);

  if (!s.Run()) {
    NOTREACHED();
    return false;
  }

  DCHECK_GT(db_.GetLastChangeCount(), 0);
  return s.Succeeded();
}

bool WebDatabase::GetCreditCardForLabel(const string16& label,
                                        CreditCard** credit_card) {
  DCHECK(credit_card);
  sql::Statement s(db_.GetUniqueStatement(
      "SELECT * FROM credit_cards "
      "WHERE label = ?"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  s.BindString16(0, label);
  if (!s.Step())
    return false;

  *credit_card = CreditCardFromStatement(s);

  return s.Succeeded();
}

bool WebDatabase::GetCreditCardForID(int credit_card_id,
                                     CreditCard** credit_card) {
  sql::Statement s(db_.GetUniqueStatement(
      "SELECT * FROM credit_cards "
      "WHERE unique_id = ?"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  s.BindInt(0, credit_card_id);
  if (!s.Step())
    return false;

  *credit_card = CreditCardFromStatement(s);

  return s.Succeeded();
}

bool WebDatabase::GetCreditCards(
    std::vector<CreditCard*>* credit_cards) {
  DCHECK(credit_cards);
  credit_cards->clear();

  sql::Statement s(db_.GetUniqueStatement("SELECT * FROM credit_cards"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  while (s.Step())
    credit_cards->push_back(CreditCardFromStatement(s));

  return s.Succeeded();
}

bool WebDatabase::UpdateCreditCard(const CreditCard& credit_card) {
  DCHECK(credit_card.unique_id());
  sql::Statement s(db_.GetUniqueStatement(
      "UPDATE credit_cards "
      "SET label=?, unique_id=?, name_on_card=?, type=?, card_number=?, "
      "    expiration_month=?, expiration_year=?, verification_code=?, "
      "    billing_address=?, shipping_address=?, card_number_encrypted=?, "
      "    verification_code_encrypted=? "
      "WHERE unique_id=?"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  BindCreditCardToStatement(credit_card, &s);
  s.BindInt(12, credit_card.unique_id());
  bool result = s.Run();
  DCHECK_GT(db_.GetLastChangeCount(), 0);
  return result;
}

bool WebDatabase::RemoveCreditCard(int credit_card_id) {
  DCHECK_NE(0, credit_card_id);
  sql::Statement s(db_.GetUniqueStatement(
      "DELETE FROM credit_cards WHERE unique_id = ?"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }

  s.BindInt(0, credit_card_id);
  return s.Run();
}

bool WebDatabase::AddToCountOfFormElement(int64 pair_id,
                                          int delta,
                                          bool* was_removed) {
  DCHECK(was_removed);
  int count = 0;
  *was_removed = false;

  if (!GetCountOfFormElement(pair_id, &count))
    return false;

  if (count + delta == 0) {
    if (!RemoveFormElementForID(pair_id))
      return false;
    *was_removed = true;
  } else {
    if (!SetCountOfFormElement(pair_id, count + delta))
      return false;
  }
  return true;
}

bool WebDatabase::RemoveFormElementForID(int64 pair_id) {
  sql::Statement s(db_.GetUniqueStatement(
      "DELETE FROM autofill WHERE pair_id = ?"));
  if (!s) {
    NOTREACHED() << "Statement prepare failed";
    return false;
  }
  s.BindInt64(0, pair_id);
  if (s.Run()) {
    return RemoveFormElementForTimeRange(pair_id, base::Time(), base::Time(),
                                         NULL);
  }
  return false;
}

void WebDatabase::MigrateOldVersionsAsNeeded(){
  // Migrate if necessary.
  int current_version = meta_table_.GetVersionNumber();
  switch (current_version) {
    // Versions 1 - 19 are unhandled.  Version numbers greater than
    // kCurrentVersionNumber should have already been weeded out by the caller.
    default:
      // When the version is too old, we just try to continue anyway.  There
      // should not be a released product that makes a database too old for us
      // to handle.
      LOG(WARNING) << "Web database version " << current_version <<
          " is too old to handle.";
      return;

    case 20:
      // Add the autogenerate_keyword column.
      if (!db_.Execute("ALTER TABLE keywords ADD COLUMN autogenerate_keyword "
                       "INTEGER DEFAULT 0")) {
        NOTREACHED();
        LOG(WARNING) << "Unable to update web database to version 21.";
        return;
      }
      meta_table_.SetVersionNumber(21);
      meta_table_.SetCompatibleVersionNumber(
          std::min(21, kCompatibleVersionNumber));
      // FALL THROUGH

    case 21:
      if (!ClearAutofillEmptyValueElements()) {
        NOTREACHED() << "Failed to clean-up autofill DB.";
      }
      meta_table_.SetVersionNumber(22);
      // No change in the compatibility version number.

      // FALL THROUGH

    case 22:
      // Add the card_number_encrypted column if credit card table was not
      // created in this build (otherwise the column already exists).
      // WARNING: Do not change the order of the execution of the SQL
      // statements in this case!  Profile corruption and data migration
      // issues WILL OCCUR. (see http://crbug.com/10913)
      //
      // The problem is that if a user has a profile which was created before
      // r37036, when the credit_cards table was added, and then failed to
      // update this profile between the credit card addition and the addition
      // of the "encrypted" columns (44963), the next data migration will put
      // the user's profile in an incoherent state: The user will update from
      // a data profile set to be earlier than 22, and therefore pass through
      // this update case.  But because the user did not have a credit_cards
      // table before starting Chrome, it will have just been initialized
      // above, and so already have these columns -- and thus this data
      // update step will have failed.
      //
      // The false assumption in this case is that at this step in the
      // migration, the user has a credit card table, and that this
      // table does not include encrypted columns!
      // Because this case does not roll back the complete set of SQL
      // transactions properly in case of failure (that is, it does not
      // roll back the table initialization done above), the incoherent
      // profile will now see itself as being at version 22 -- but include a
      // fully initialized credit_cards table.  Every time Chrome runs, it
      // will try to update the web database and fail at this step, unless
      // we allow for the faulty assumption described above by checking for
      // the existence of the columns only AFTER we've executed the commands
      // to add them.
      if (!db_.Execute("ALTER TABLE credit_cards ADD COLUMN "
                       "card_number_encrypted BLOB DEFAULT NULL")) {
        LOG(WARNING) << "Could not add card_number_encrypted to "
                        "credit_cards table.";
      }
      if (!db_.Execute("ALTER TABLE credit_cards ADD COLUMN "
                       "verification_code_encrypted BLOB DEFAULT NULL")) {
        LOG(WARNING) << "Could not add verification_code_encrypted to "
                        "credit_cards table.";
      }
      if (!db_.Execute(
              "SELECT card_number_encrypted FROM credit_cards limit 1") ||
          !db_.Execute(
              "SELECT verification_code_encrypted FROM credit_cards limit 1")) {
        NOTREACHED();
        LOG(WARNING) << "Unable to update web database to version 23.";
        return;
      }
      meta_table_.SetVersionNumber(23);
      // FALL THROUGH

    case 23: {
      // One-time cleanup for Chromium bug 38364.  In the presence of
      // multi-byte UTF-8 characters, that bug could cause AutoFill strings
      // to grow larger and more corrupt with each save.  The cleanup removes
      // any row with a string field larger than a reasonable size.  The string
      // fields examined here are precisely the ones that were subject to
      // corruption by the original bug.
      const std::string autofill_is_too_big =
          "max(length(name), length(value)) > 500";

      const std::string credit_cards_is_too_big =
          "max(length(label), length(name_on_card), length(type), "
          "    length(expiration_month), length(expiration_year), "
          "    length(billing_address), length(shipping_address) "
          ") > 500";

      const std::string autofill_profiles_is_too_big =
          "max(length(label), length(first_name), "
          "    length(middle_name), length(last_name), length(email), "
          "    length(company_name), length(address_line_1), "
          "    length(address_line_2), length(city), length(state), "
          "    length(zipcode), length(country), length(phone), "
          "    length(fax)) > 500";

      std::string query = "DELETE FROM autofill_dates WHERE pair_id IN ("
          "SELECT pair_id FROM autofill WHERE " + autofill_is_too_big + ")";
      if (!db_.Execute(query.c_str())) {
        NOTREACHED();
        LOG(WARNING) << "Unable to update web database to version 24.";
        return;
      }
      query = "DELETE FROM autofill WHERE " + autofill_is_too_big;
      if (!db_.Execute(query.c_str())) {
        NOTREACHED();
        LOG(WARNING) << "Unable to update web database to version 24.";
        return;
      }
      query = "DELETE FROM credit_cards WHERE (" + credit_cards_is_too_big +
          ") OR label IN (SELECT label FROM autofill_profiles WHERE " +
          autofill_profiles_is_too_big + ")";
      if (!db_.Execute(query.c_str())) {
        NOTREACHED();
        LOG(WARNING) << "Unable to update web database to version 24.";
        return;
      }
      query = "DELETE FROM autofill_profiles WHERE " +
          autofill_profiles_is_too_big;
      if (!db_.Execute(query.c_str())) {
        NOTREACHED();
        LOG(WARNING) << "Unable to update web database to version 24.";
        return;
      }

      meta_table_.SetVersionNumber(24);

      // FALL THROUGH
    }

    case 24:
      // Add the logo_id column if keyword table was not created in this build.
      if (!db_.Execute("ALTER TABLE keywords ADD COLUMN logo_id "
                       "INTEGER DEFAULT 0")) {
        NOTREACHED();
        LOG(WARNING) << "Unable to update web database to version 25.";
        return;
      }
      meta_table_.SetVersionNumber(25);
      meta_table_.SetCompatibleVersionNumber(
          std::min(25, kCompatibleVersionNumber));
      // FALL THROUGH

    // Add successive versions here.  Each should set the version number and
    // compatible version number as appropriate, then fall through to the next
    // case.

    case kCurrentVersionNumber:
      // No migration needed.
      return;
  }
}
