/*
 * Copyright (c) 2018, Ford Motor Company
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

#ifndef SRC_COMPONENTS_UTILS_INCLUDE_UTILS_QDB_WRAPPER_SQL_DATABASE_H_
#define SRC_COMPONENTS_UTILS_INCLUDE_UTILS_QDB_WRAPPER_SQL_DATABASE_H_

#include <string>
#include "utils/sql/sql_error.h"
#include "utils/lock.h"

namespace utils {
namespace dbms {

class SQLQuery;
class SQLHandle;

/**
 * Represents a connection to a database.
 */
class SQLDatabase {
 public:
  SQLDatabase();
  explicit SQLDatabase(const std::string& db_name);
  ~SQLDatabase();

  /**
   * Opens connection to the temporary in-memory database
   * @return true if successfully
   */
  bool Open();

  /**
   * Closes connection to the database
   */
  void Close();

  /**
   * Begins a transaction on the database
   * @return true if successfully
   */
  bool BeginTransaction();

  /**
   * Commits a transaction to the database
   * @return true if successfully
   */
  bool CommitTransaction();

  /**
   * Rolls back a transaction on the database
   * @return true if successfully
   */
  bool RollbackTransaction();

  /**
   * Gets information about the last error that occurred on the database
   * @return last error
   */
  SQLError LastError() const;

  /**
   * Checks if database is read/write
   * @return true if database is read/write
   */
  bool IsReadWrite();

  /**
   * Call backup for opened DB
   */
  bool Backup();

  /**
   * Sets path to database
   * After setting the path need to reopen the database
   */
  void set_path(const std::string& path);

  /**
  * @brief get_path database location path.
  * @return the path to the database location
  */
  std::string get_path() const;

 protected:
  /**
   * Gets connection to the SQLite database
   * @return pointer to connection
   */
  SQLHandle conn() const;

 private:
  class PlatformSpecific;

  PlatformSpecific* specific_implementation_;

  friend class SQLQuery;
};

}  // namespace dbms
}  // namespace utils

#endif  // SRC_COMPONENTS_UTILS_INCLUDE_UTILS_QDB_WRAPPER_SQL_DATABASE_H_
