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
#include "utils/sql/sql_database.h"
#include <sqlite3.h>

namespace utils {
namespace dbms {

class SQLDatabase::PlatformSpecific {
 public:
  PlatformSpecific();
  explicit PlatformSpecific(const std::string& db_name);

  /**
   * The connection to the SQLite database
   */
  SQLHandle conn_;

  /**
   * Lock for guarding connection to database
   */
  sync_primitives::Lock conn_lock_;

  /**
   * The file path of database
   */
  std::string path_;

  /**
   * The filename of database
   */
  std::string databasename_;

  /**
   * The last error that occurred on the database
   */
  int error_;

  /**
   *  The temporary in-memory database
   *  @see SQLite manual
   */
  static const std::string kInMemory;

  /**
   * The extension of filename of database
   */
  static const std::string kExtension;

  /**
   * Execs query for internal using in this class
   * @param query sql query without return results
   * @return true if query was executed successfully
   */
  inline bool Exec(const std::string& query);
};

const std::string SQLDatabase::PlatformSpecific::kInMemory = ":memory:";
const std::string SQLDatabase::PlatformSpecific::kExtension = ".sqlite";

SQLDatabase::PlatformSpecific::PlatformSpecific()
    : conn_(NULL), databasename_(kInMemory), error_(SQLITE_OK) {}

SQLDatabase::PlatformSpecific::PlatformSpecific(const std::string& db_name)
    : conn_(NULL), databasename_(db_name + kExtension), error_(SQLITE_OK) {}

bool SQLDatabase::PlatformSpecific::Exec(const std::string& query) {
  sync_primitives::AutoLock auto_lock(conn_lock_);
  error_ = sqlite3_exec(conn_, query.c_str(), NULL, NULL, NULL);
  return error_ == SQLITE_OK;
}

SQLDatabase::SQLDatabase()
    : specific_implementation_(new SQLDatabase::PlatformSpecific()) {}

SQLDatabase::SQLDatabase(const std::string& db_name)
    : specific_implementation_(new SQLDatabase::PlatformSpecific(db_name)) {}

SQLDatabase::~SQLDatabase() {
  Close();
  delete specific_implementation_;
}

bool SQLDatabase::Open() {
  sync_primitives::AutoLock auto_lock(specific_implementation_->conn_lock_);
  if (specific_implementation_->conn_)
    return true;
  specific_implementation_->error_ =
      sqlite3_open(get_path().c_str(), specific_implementation_->conn_);
  if (specific_implementation_->error_ != SQLITE_OK) {
    specific_implementation_->conn_ = NULL;
  }
  return specific_implementation_->error_ == SQLITE_OK;
}

bool SQLDatabase::IsReadWrite() {
  const char* schema = "main";
  return sqlite3_db_readonly(specific_implementation_->conn_, schema) == 0;
}

void SQLDatabase::Close() {
  if (!specific_implementation_->conn_) {
    return;
  }

  sync_primitives::AutoLock auto_lock(specific_implementation_->conn_lock_);
  specific_implementation_->error_ =
      sqlite3_close(specific_implementation_->conn_);
  if (specific_implementation_->error_ == SQLITE_OK) {
    specific_implementation_->conn_ = NULL;
  }
}

bool SQLDatabase::BeginTransaction() {
  return specific_implementation_->Exec("BEGIN TRANSACTION");
}

bool SQLDatabase::CommitTransaction() {
  return specific_implementation_->Exec("COMMIT TRANSACTION");
}

bool SQLDatabase::RollbackTransaction() {
  return specific_implementation_->Exec("ROLLBACK TRANSACTION");
}

SQLError SQLDatabase::LastError() const {
  return SQLError(Error(specific_implementation_->error_));
}

SQLHandle SQLDatabase::conn() const {
  return specific_implementation_->conn_;
}

void SQLDatabase::set_path(const std::string& path) {
  specific_implementation_->path_ = path;
}

std::string SQLDatabase::get_path() const {
  return specific_implementation_->path_ +
         specific_implementation_->databasename_;
}

bool SQLDatabase::Backup() {
  return true;
}
}  // namespace dbms
}  // namespace utils
