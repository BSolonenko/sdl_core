/*
 * Copyright (c) 2013, Ford Motor Company
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of the Ford Motor Company nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "utils/sql/platform_specific/sqlite_wrapper/sql_handle.h"
#include "utils/sql/sql_query.h"
#include <sqlite3.h>
#include <cassert>
#include "utils/sql/sql_database.h"

namespace utils {
namespace dbms {

class SQLQuery::PlatformSpecific {
 public:
  explicit PlatformSpecific(SQLDatabase* db);
  /**
   * The instantiation of database
   */
  SQLDatabase& db_;

  /**
   * The string of query
   */
  std::string query_;

  /**
   * The SQL statement in SQLite
   */
  sqlite3_stmt* statement_;

  /**
   * Lock for guarding statement
   */
  sync_primitives::Lock statement_lock_;

  /**
   * The last error that occurred with this query
   */
  int error_;
};

SQLQuery::PlatformSpecific::PlatformSpecific(SQLDatabase* db)
    : db_(*db), query_(""), statement_(NULL), error_(SQLITE_OK) {}

SQLQuery::SQLQuery(SQLDatabase* db)
    : specific_implementation_(new SQLQuery::PlatformSpecific(db)) {}

SQLQuery::~SQLQuery() {
  Finalize();
  delete specific_implementation_;
}

bool SQLQuery::Prepare(const std::string& query) {
  Finalize();
  sync_primitives::AutoLock auto_lock(
      specific_implementation_->statement_lock_);
  if (specific_implementation_->statement_)
    return false;
  specific_implementation_->error_ =
      sqlite3_prepare(specific_implementation_->db_.conn(),
                      query.c_str(),
                      query.length(),
                      &specific_implementation_->statement_,
                      NULL);
  specific_implementation_->query_ = query;
  return specific_implementation_->error_ == SQLITE_OK;
}

bool SQLQuery::Exec() {
  specific_implementation_->error_ =
      sqlite3_step(specific_implementation_->statement_);
  return specific_implementation_->error_ == SQLITE_ROW ||
         specific_implementation_->error_ == SQLITE_DONE;
}

bool SQLQuery::Next() {
  specific_implementation_->error_ =
      sqlite3_step(specific_implementation_->statement_);
  return specific_implementation_->error_ == SQLITE_ROW;
}

bool SQLQuery::Reset() {
  specific_implementation_->error_ =
      sqlite3_reset(specific_implementation_->statement_);
  return specific_implementation_->error_ == SQLITE_OK;
}

void SQLQuery::Finalize() {
  sync_primitives::AutoLock auto_lock(
      specific_implementation_->statement_lock_);
  specific_implementation_->error_ =
      sqlite3_finalize(specific_implementation_->statement_);
  if (specific_implementation_->error_ == SQLITE_OK) {
    specific_implementation_->statement_ = NULL;
  }
}

bool SQLQuery::Exec(const std::string& query) {
  specific_implementation_->query_ = query;
  specific_implementation_->error_ = sqlite3_exec(
      specific_implementation_->db_.conn(), query.c_str(), NULL, NULL, NULL);
  return specific_implementation_->error_ == SQLITE_OK;
}

void SQLQuery::Bind(int pos, int value) {
  // In SQLite the number of position for binding starts since 1.
  specific_implementation_->error_ =
      sqlite3_bind_int(specific_implementation_->statement_, pos + 1, value);
}

void SQLQuery::Bind(int pos, int64_t value) {
  // In SQLite the number of position for binding starts since 1.
  specific_implementation_->error_ =
      sqlite3_bind_int64(specific_implementation_->statement_, pos + 1, value);
}

void SQLQuery::Bind(int pos, double value) {
  // In SQLite the number of position for binding starts since 1.
  specific_implementation_->error_ =
      sqlite3_bind_double(specific_implementation_->statement_, pos + 1, value);
}

void SQLQuery::Bind(int pos, bool value) {
  Bind(pos, static_cast<int>(value));
}

void SQLQuery::Bind(int pos, const std::string& value) {
  // In SQLite the number of position for binding starts since 1.
  specific_implementation_->error_ =
      sqlite3_bind_text(specific_implementation_->statement_,
                        pos + 1,
                        value.c_str(),
                        value.length(),
                        SQLITE_TRANSIENT);
}

bool SQLQuery::GetBoolean(int pos) const {
  return static_cast<bool>(GetInteger(pos));
}

int SQLQuery::GetInteger(int pos) const {
  return sqlite3_column_int(specific_implementation_->statement_, pos);
}

uint32_t SQLQuery::GetUInteger(int pos) const {
  return static_cast<uint32_t>(
      sqlite3_column_int64(specific_implementation_->statement_, pos));
}

int64_t SQLQuery::GetLongInt(int pos) const {
  return static_cast<int64_t>(
      sqlite3_column_int64(specific_implementation_->statement_, pos));
}

double SQLQuery::GetDouble(int pos) const {
  return sqlite3_column_double(specific_implementation_->statement_, pos);
}

std::string SQLQuery::GetString(int pos) const {
  const unsigned char* str =
      sqlite3_column_text(specific_implementation_->statement_, pos);
  return str ? reinterpret_cast<const char*>(str) : "";
}

const std::string& SQLQuery::query() const {
  // TODO(KKolodiy): may return string query with value of arguments
  return specific_implementation_->query_;
}

bool SQLQuery::IsNull(int pos) const {
  return sqlite3_column_type(specific_implementation_->statement_, pos) ==
         SQLITE_NULL;
}

void SQLQuery::Bind(int pos) {
  // In SQLite the number of position for binding starts since 1.
  specific_implementation_->error_ =
      sqlite3_bind_null(specific_implementation_->statement_, pos + 1);
}

SQLError SQLQuery::LastError() const {
  return SQLError(Error(specific_implementation_->error_));
}

int64_t SQLQuery::LastInsertId() const {
  return sqlite3_last_insert_rowid(specific_implementation_->db_.conn());
}

}  // namespace dbms
}  // namespace utils
