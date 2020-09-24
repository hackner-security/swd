// Copyright 2020 Barger M., Knoll M., Kofler L.

#ifndef INCLUDE_DB_CLIENT_HPP_
#define INCLUDE_DB_CLIENT_HPP_

#include <sqlite3.h>
#include <string>
#include "log.hpp"

class DBClient {
 public:
  // Constructor
  explicit DBClient(std::string path);

  // Destructor
  ~DBClient();

  // Insert a new set of data into the database
  //
  // id: Id of this call
  // number: Target number of this call
  // status: Current status in the processing chain
  // dev_type: Device type if fully analyzed
  //
  bool InsertData(std::string id, std::string number, std::string status, std::string dev_type);

  // Updates the status and/or device type of an entry in the table calls
  //
  // id: Id of the entry to update
  // status: new status, empty if it is not to be changed
  // dev_type: new device type, empty if it is not to be changed
  bool UpdateEntry(std::string id, std::string status, std::string dev_type);

  // Updates the call duration to the given integer
  //
  // id: Id of the entry to update
  // duration: The call duration in seconds
  bool UpdateDuration(std::string id, int duration);

 private:
  sqlite3 *db;         // Pointer to the open database
  std::string path;    // Path to the database
};

#endif  // INCLUDE_DB_CLIENT_HPP_
