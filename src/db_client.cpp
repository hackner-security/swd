// Copyright 2020 Barger M., Knoll M, Kofler L.

#include "db_client.hpp"

DBClient::DBClient(std::string path) {
  this->db = nullptr;
  this->path = path;
  sqlite3_stmt *stmt = nullptr;

  int ret = sqlite3_open(this->path.c_str(), &db);
  if ( ret != SQLITE_OK ) {
    Logger::GetLogger()->Log("Failed to open db file", LOG_LVL_ERROR);
    return;
  }

  // Create table if it does not exist
  std::string cmd =
    "CREATE TABLE IF NOT EXISTS calls("
    "  id text primary key not null,"
    "  number text not null,"
    "  start_time text not null,"
    "  duration integer,"
    "  status text not null,"
    "  dev_type text"
    ");";

  ret = sqlite3_prepare_v3(db, cmd.c_str(), -1, 0, &stmt, nullptr);
  if (ret != SQLITE_OK) {
    Logger::GetLogger()->Log("Failed to create sqlite create statement.", LOG_LVL_ERROR);
    return;
  }

  ret = sqlite3_step(stmt);

  if (ret != SQLITE_DONE) {
    switch (ret) {
      case SQLITE_BUSY:
        Logger::GetLogger()->Log("Failed to create table: DB is locked.", LOG_LVL_ERROR);
      break;
      case SQLITE_ROW:
        Logger::GetLogger()->Log("Failed to create table: Statement returned data when it should not.", LOG_LVL_ERROR);
      break;
      case SQLITE_MISUSE:
        Logger::GetLogger()->Log("Failed to create table: Misuse detected.", LOG_LVL_ERROR);
      break;
      default:
        Logger::GetLogger()->Log("Failed to create table: Error code " + std::to_string(ret), LOG_LVL_ERROR);
    }
    Logger::GetLogger()->Log(std::string(sqlite3_errmsg(db)), LOG_LVL_ERROR);
  }

  sqlite3_finalize(stmt);
}

DBClient::~DBClient() {
  if (db != nullptr) {
    sqlite3_close(db);
  }
}

bool DBClient::InsertData(std::string id, std::string number, std::string status, std::string dev_type) {
  sqlite3_stmt *stmt = nullptr;
  std::string cmd =
    "insert into calls (id, number, status, dev_type, start_time) "
    "values(@id, @number, @status, @dev_type, datetime());";

  int ret = sqlite3_prepare_v3(db, cmd.c_str(), -1, 0, &stmt, nullptr);
  if (ret != SQLITE_OK) {
    Logger::GetLogger()->Log("Failed to create sqlite insert statement.", LOG_LVL_ERROR);
    return false;
  }

  sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, "@id"), id.c_str(), -1, 0);
  sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, "@number"), number.c_str(), -1, 0);
  sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, "@status"), status.c_str(), -1, 0);
  sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, "@dev_type"), dev_type.c_str(), -1, 0);

  ret = sqlite3_step(stmt);
  if ( ret != SQLITE_DONE ) {
    switch (ret) {
      case SQLITE_BUSY:
        Logger::GetLogger()->Log("Failed to insert entry into table: DB is locked.", LOG_LVL_ERROR);
      break;
      case SQLITE_ROW:
        Logger::GetLogger()->Log("Failed to insert entry into table: Statement returned data when it should not.",
          LOG_LVL_ERROR);
      break;
      case SQLITE_MISUSE:
        Logger::GetLogger()->Log("Failed to insert entry into table: Misuse detected.", LOG_LVL_ERROR);
      break;
      default:
        Logger::GetLogger()->Log("Failed to insert entry into table: Error code " + std::to_string(ret),
          LOG_LVL_ERROR);
    }
    Logger::GetLogger()->Log(std::string(sqlite3_errmsg(db)), LOG_LVL_ERROR);
  }

  sqlite3_finalize(stmt);
  return true;
}

bool DBClient::UpdateEntry(std::string id, std::string status, std::string dev_type) {
  sqlite3_stmt *stmt = nullptr;
  int ret = 0;

  if (status == "" && dev_type == "") {
    return true;
  }
  std::string cmd =
    "update calls\n"
    "set ";

  if (status != "") {
    cmd += "status = @status\n";
  }
  if (dev_type != "") {
    cmd += status == "" ? "" : ",";
    cmd += "dev_type = @dev_type\n";
  }
  cmd += "where id = @id;";

  ret = sqlite3_prepare_v3(db, cmd.c_str(), -1, 0, &stmt, nullptr);
  if (ret != SQLITE_OK) {
    switch (ret) {
      case SQLITE_BUSY:
        Logger::GetLogger()->Log("Failed to update entry in table: DB is locked.", LOG_LVL_ERROR);
      break;
      case SQLITE_ROW:
        Logger::GetLogger()->Log("Failed to update entry in table: Statement returned data when it should not.",
          LOG_LVL_ERROR);
      break;
      case SQLITE_MISUSE:
        Logger::GetLogger()->Log("Failed to update entry in table: Misuse detected.", LOG_LVL_ERROR);
      break;
      default:
        Logger::GetLogger()->Log("Failed to update entry in table: Error code " + std::to_string(ret),
          LOG_LVL_ERROR);
    }
    Logger::GetLogger()->Log(std::string(sqlite3_errmsg(db)), LOG_LVL_ERROR);
    return false;
  }

  sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, "@status"), status.c_str(), -1, 0);
  sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, "@dev_type"), dev_type.c_str(), -1, 0);
  sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, "@id"), id.c_str(), -1, 0);

  ret = sqlite3_step(stmt);
  if (ret != SQLITE_DONE) {
    switch (ret) {
      case SQLITE_BUSY:
        Logger::GetLogger()->Log("Failed to update entry in table: DB is locked.", LOG_LVL_ERROR);
      break;
      case SQLITE_ROW:
        Logger::GetLogger()->Log("Failed to update entry in table: Statement returned data when it should not.",
          LOG_LVL_ERROR);
      break;
      case SQLITE_MISUSE:
        Logger::GetLogger()->Log("Failed to update entry in table: Misuse detected.", LOG_LVL_ERROR);
      break;
      default:
        Logger::GetLogger()->Log("Failed to update entry in table: Error code " + std::to_string(ret),
          LOG_LVL_ERROR);
    }
    Logger::GetLogger()->Log(std::string(sqlite3_errmsg(db)), LOG_LVL_ERROR);
  }

  sqlite3_finalize(stmt);
  return true;
}

bool DBClient::UpdateDuration(std::string id, int duration) {
  sqlite3_stmt *stmt = nullptr;
  int ret = 0;

  std::string cmd =
    "update calls\n"
    "set duration = @duration\n"
    "where id = @id;";

  ret = sqlite3_prepare_v3(db, cmd.c_str(), -1, 0, &stmt, nullptr);
  if (ret != SQLITE_OK) {
    switch (ret) {
      case SQLITE_BUSY:
        Logger::GetLogger()->Log("Failed to update entry in table: DB is locked.", LOG_LVL_ERROR);
      break;
      case SQLITE_ROW:
        Logger::GetLogger()->Log("Failed to update entry in table: Statement returned data when it should not.",
          LOG_LVL_ERROR);
      break;
      case SQLITE_MISUSE:
        Logger::GetLogger()->Log("Failed to update entry in table: Misuse detected.", LOG_LVL_ERROR);
      break;
      default:
        Logger::GetLogger()->Log("Failed to update entry in table: Error code " + std::to_string(ret),
          LOG_LVL_ERROR);
    }
    Logger::GetLogger()->Log(std::string(sqlite3_errmsg(db)), LOG_LVL_ERROR);
    return false;
  }

  sqlite3_bind_int(stmt, sqlite3_bind_parameter_index(stmt, "@duration"), duration);
  sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, "@id"), id.c_str(), -1, 0);

  ret = sqlite3_step(stmt);
  if (ret != SQLITE_DONE) {
    switch (ret) {
      case SQLITE_BUSY:
        Logger::GetLogger()->Log("Failed to update entry in table: DB is locked.", LOG_LVL_ERROR);
      break;
      case SQLITE_ROW:
        Logger::GetLogger()->Log("Failed to update entry in table: Statement returned data when it should not.",
          LOG_LVL_ERROR);
      break;
      case SQLITE_MISUSE:
        Logger::GetLogger()->Log("Failed to update entry in table: Misuse detected.", LOG_LVL_ERROR);
      break;
      default:
        Logger::GetLogger()->Log("Failed to update entry in table: Error code " + std::to_string(ret),
          LOG_LVL_ERROR);
    }
    Logger::GetLogger()->Log(std::string(sqlite3_errmsg(db)), LOG_LVL_ERROR);
  }

  sqlite3_finalize(stmt);
  return true;
}
