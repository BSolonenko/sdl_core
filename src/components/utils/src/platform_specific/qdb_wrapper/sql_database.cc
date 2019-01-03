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

#include <cstring>

#include "utils/sql/platform_specific/qdb_wrapper/sql_handle.h"
#include "utils/sql/sql_database.h"
#include "utils/logger.h"

namespace utils {
namespace dbms {

CREATE_LOGGERPTR_GLOBAL(logger_, "Utils")

class SQLDatabase::PlatformSpecific {
 public:
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
   * The database name
   */
  std::string db_name_;

  /**
   * The last error that occurred on the database
   */
  Error error_;

  /**
   * Execs query for internal using in this class
   * @param query sql query without return results
   * @return true if query was executed successfully
   */
  inline bool Exec(const std::string& query);
};
SQLDatabase::PlatformSpecific::PlatformSpecific(const std::string& db_name)
    : conn_(NULL), db_name_(db_name), error_(Error::OK) {}

bool SQLDatabase::PlatformSpecific::Exec(const std::string& query) {
  sync_primitives::AutoLock auto_lock(conn_lock_);
  if (qdb_statement(conn_, query.c_str()) == -1) {
    error_ = Error::ERROR;
    return false;
  }
  return true;
}

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
  specific_implementation_->conn_ =
      qdb_connect(specific_implementation_->db_name_.c_str(), 0);
  if (specific_implementation_->conn_ == NULL) {
    specific_implementation_->error_ = Error::ERROR;
    return false;
  }
  return true;
}

void SQLDatabase::Close() {
  sync_primitives::AutoLock auto_lock(specific_implementation_->conn_lock_);
  if (specific_implementation_->conn_) {
    if (qdb_disconnect(specific_implementation_->conn_) != -1) {
      specific_implementation_->conn_ = NULL;
    } else {
      specific_implementation_->error_ = Error::ERROR;
    }
  }
}

bool SQLDatabase::IsReadWrite() {
  return true;
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
  return SQLError(specific_implementation_->error_,
                  qdb_geterrmsg(specific_implementation_->conn_));
}

SQLHandle SQLDatabase::conn() const {
  return specific_implementation_->conn_;
}

bool SQLDatabase::Backup() {
  if (qdb_backup(specific_implementation_->conn_, QDB_ATTACH_DEFAULT) == -1) {
    specific_implementation_->error_ = Error::ERROR;
    LOG4CXX_ERROR(logger_, "Backup returned error: " << std::strerror(errno));
    return false;
  }
  LOG4CXX_INFO(logger_, "Backup was successful.");
  return true;
}

void SQLDatabase::set_path(const std::string& path) {
  specific_implementation_->path_ = path;
}

std::string SQLDatabase::get_path() const {
  return specific_implementation_->path_ + specific_implementation_->db_name_;
}

}  // namespace dbms
}  // namespace utils
