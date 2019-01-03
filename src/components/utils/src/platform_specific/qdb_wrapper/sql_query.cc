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

#include <algorithm>
#include <cassert>
#include <errno.h>
#include <string.h>

#include "utils/sql/platform_specific/qdb_wrapper/sql_handle.h"
#include "utils/sql/sql_query.h"
#include "utils/sql/sql_database.h"
#include "utils/logger.h"

namespace utils {
namespace dbms {

CREATE_LOGGERPTR_GLOBAL(logger_, "Utils")

class SQLQuery::PlatformSpecific {
 public:
  explicit PlatformSpecific(SQLDatabase* db);
  /**
   * The instantiation of database
   */
  SQLDatabase* db_;

  /**
   * The string of query
   */
  std::string query_;

  /**
   * The id of SQL statement in QDB
   */
  int statement_;

  /**
   * Containers for keeping bind data
   */
  std::vector<std::pair<int, int64_t> > int_binds_;
  std::vector<std::pair<int, double> > double_binds_;
  std::vector<std::pair<int, std::string> > string_binds_;
  std::vector<int> null_binds_;

  /**
   * The array for binging data to the prepare query
   */
  qdb_binding_t* bindings_;

  /**
   * Lock for guarding bindings
   */
  sync_primitives::Lock bindings_lock_;

  /**
   * The result of query
   */
  qdb_result_t* result_;

  /**
   * The current row in result for select
   */
  int current_row_;

  /**
   * The number of rows in a result
   */
  int rows_;

  /**
   * The last error that occurred with this query
   */
  Error error_;

  uint8_t SetBinds();
  bool Result();
};

SQLQuery::PlatformSpecific::PlatformSpecific(SQLDatabase* db)
    : db_(db)
    , query_("")
    , statement_(-1)
    , bindings_(NULL)
    , result_(NULL)
    , current_row_(0)
    , rows_(0)
    , error_(Error::OK) {}

class SetBindInteger {
 public:
  explicit SetBindInteger(qdb_binding_t* array) : array_(array) {}
  void operator()(const std::pair<int, int64_t>& x) {
    // In QDB the number of position for binding starts since 1.
    QDB_SETARRAYBIND_INT(array_, x.first + 1, x.second);
  }

 private:
  qdb_binding_t* array_;
};

class SetBindReal {
 public:
  explicit SetBindReal(qdb_binding_t* array) : array_(array) {}
  void operator()(const std::pair<int, double>& x) {
    // In QDB the number of position for binding starts since 1.
    QDB_SETARRAYBIND_REAL(array_, x.first + 1, x.second);
  }

 private:
  qdb_binding_t* array_;
};

class SetBindText {
 public:
  explicit SetBindText(qdb_binding_t* array) : array_(array) {}
  void operator()(const std::pair<int, std::string>& x) {
    // In QDB the number of position for binding starts since 1.
    QDB_SETARRAYBIND_TEXT(array_, x.first + 1, x.second.c_str());
  }

 private:
  qdb_binding_t* array_;
};

class SetBindNull {
 public:
  explicit SetBindNull(qdb_binding_t* array) : array_(array) {}
  void operator()(int x) {
    // In QDB the number of position for binding starts since 1.
    QDB_SETARRAYBIND_NULL(array_, x + 1);
  }

 private:
  qdb_binding_t* array_;
};

SQLQuery::SQLQuery(SQLDatabase* db)
    : specific_implementation_(new SQLQuery::PlatformSpecific(db)) {}

SQLQuery::~SQLQuery() {
  Finalize();
  delete specific_implementation_;
}

bool SQLQuery::Prepare(const std::string& query) {
  specific_implementation_->query_ = query;
  specific_implementation_->statement_ = qdb_stmt_init(
      specific_implementation_->db_->conn(), query.c_str(), query.length() + 1);
  if (specific_implementation_->statement_ == -1) {
    LOG4CXX_DEBUG(logger_, "Prepare error: " << strerror(errno));
    specific_implementation_->error_ = Error::ERROR;
    return false;
  }
  return true;
}

uint8_t SQLQuery::PlatformSpecific::SetBinds() {
  uint8_t binding_count = int_binds_.size() + double_binds_.size() +
                          string_binds_.size() + null_binds_.size();

  bindings_ = new qdb_binding_t[binding_count];

  std::for_each(
      int_binds_.begin(), int_binds_.end(), SetBindInteger(bindings_));
  std::for_each(
      double_binds_.begin(), double_binds_.end(), SetBindReal(bindings_));
  std::for_each(
      string_binds_.begin(), string_binds_.end(), SetBindText(bindings_));
  std::for_each(null_binds_.begin(), null_binds_.end(), SetBindNull(bindings_));

  return binding_count;
}

bool SQLQuery::PlatformSpecific::Result() {
  result_ = qdb_getresult(db_->conn());
  if (!result_) {
    error_ = Error::ERROR;
    return false;
  }
  rows_ = qdb_rows(result_);
  if (rows_ == -1) {
    rows_ = 0;
    error_ = Error::ERROR;
    return false;
  }
  return true;
}

bool SQLQuery::Exec() {
  sync_primitives::AutoLock auto_lock(specific_implementation_->bindings_lock_);
  if (specific_implementation_->result_)
    return true;

  specific_implementation_->current_row_ = 0;
  uint8_t binding_count = specific_implementation_->SetBinds();
  if (qdb_stmt_exec(specific_implementation_->db_->conn(),
                    specific_implementation_->statement_,
                    specific_implementation_->bindings_,
                    binding_count) == -1) {
    specific_implementation_->error_ = Error::ERROR;
    return false;
  }
  return specific_implementation_->Result();
}

bool SQLQuery::Next() {
  ++specific_implementation_->current_row_;
  return Exec() &&
         specific_implementation_->current_row_ <
             specific_implementation_->rows_;
}

bool SQLQuery::Reset() {
  sync_primitives::AutoLock auto_lock(specific_implementation_->bindings_lock_);
  specific_implementation_->int_binds_.clear();
  specific_implementation_->double_binds_.clear();
  specific_implementation_->string_binds_.clear();
  specific_implementation_->null_binds_.clear();
  delete[] specific_implementation_->bindings_;
  specific_implementation_->bindings_ = NULL;
  specific_implementation_->rows_ = 0;
  specific_implementation_->current_row_ = 0;
  if (specific_implementation_->result_ &&
      qdb_freeresult(specific_implementation_->result_) == -1) {
    specific_implementation_->error_ = Error::ERROR;
    return false;
  }
  specific_implementation_->result_ = NULL;
  return true;
}

void SQLQuery::Finalize() {
  if (Reset() &&
      qdb_stmt_free(specific_implementation_->db_->conn(),
                    specific_implementation_->statement_) != -1) {
    specific_implementation_->statement_ = 0;
  } else {
    specific_implementation_->error_ = Error::ERROR;
  }
}

bool SQLQuery::Exec(const std::string& query) {
  specific_implementation_->query_ = query;
  if (qdb_statement(specific_implementation_->db_->conn(), query.c_str()) ==
      -1) {
    specific_implementation_->error_ = Error::ERROR;
    return false;
  }
  return true;
}

void SQLQuery::Bind(int pos, int value) {
  specific_implementation_->int_binds_.push_back(std::make_pair(pos, value));
}

void SQLQuery::Bind(int pos, int64_t value) {
  specific_implementation_->int_binds_.push_back(std::make_pair(pos, value));
}

void SQLQuery::Bind(int pos, double value) {
  specific_implementation_->double_binds_.push_back(std::make_pair(pos, value));
}

void SQLQuery::Bind(int pos, bool value) {
  Bind(pos, static_cast<int>(value));
}

void SQLQuery::Bind(int pos, const std::string& value) {
  specific_implementation_->string_binds_.push_back(std::make_pair(pos, value));
}

void SQLQuery::Bind(int pos) {
  specific_implementation_->null_binds_.push_back(pos);
}

bool SQLQuery::GetBoolean(int pos) const {
  return static_cast<bool>(GetInteger(pos));
}

int SQLQuery::GetInteger(int pos) const {
  void* ret = qdb_cell(specific_implementation_->result_,
                       specific_implementation_->current_row_,
                       pos);
  if (specific_implementation_->rows_ != 0 && ret) {
    return *static_cast<int*>(ret);
  }
  return 0;
}

uint32_t SQLQuery::GetUInteger(int pos) const {
  void* ret = qdb_cell(specific_implementation_->result_,
                       specific_implementation_->current_row_,
                       pos);
  if (specific_implementation_->rows_ != 0 && ret) {
    return *static_cast<uint32_t*>(ret);
  }
  return 0;
}

int64_t SQLQuery::GetLongInt(int pos) const {
  void* ret = qdb_cell(specific_implementation_->result_,
                       specific_implementation_->current_row_,
                       pos);
  if (specific_implementation_->rows_ != 0 && ret) {
    return *static_cast<int64_t*>(ret);
  }
  return 0;
}

double SQLQuery::GetDouble(int pos) const {
  void* ret = qdb_cell(specific_implementation_->result_,
                       specific_implementation_->current_row_,
                       pos);
  if (specific_implementation_->rows_ != 0 && ret) {
    return *static_cast<double*>(ret);
  }
  return 0;
}

std::string SQLQuery::GetString(int pos) const {
  void* ret = qdb_cell(specific_implementation_->result_,
                       specific_implementation_->current_row_,
                       pos);
  if (specific_implementation_->rows_ != 0 && ret) {
    return static_cast<const char*>(ret);
  }
  return "";
}

bool SQLQuery::IsNull(int pos) const {
  return specific_implementation_->rows_ == 0 ||
         qdb_cell_type(specific_implementation_->result_,
                       specific_implementation_->current_row_,
                       pos) == QDB_NULL;
}

const std::string& SQLQuery::query() const {
  // TODO(KKolodiy): may return string query with value of arguments
  return specific_implementation_->query_;
}

SQLError SQLQuery::LastError() const {
  return SQLError(specific_implementation_->error_,
                  qdb_geterrmsg(specific_implementation_->db_->conn()));
}

int64_t SQLQuery::LastInsertId() const {
  return qdb_last_insert_rowid(specific_implementation_->db_->conn(),
                               specific_implementation_->result_);
}

}  // namespace dbms
}  // namespace utils
