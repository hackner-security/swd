// Copyright 2020 Barger M., Knoll M., Kofler L.
#ifndef INCLUDE_SIP_CLIENT_HPP_
#define INCLUDE_SIP_CLIENT_HPP_

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <osip2/osip.h>
#include <osipparser2/osip_parser.h>
#include <eXosip2/eXosip.h>

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <chrono> // NOLINT
#include <cstring>
#include <boost/algorithm/string.hpp>

#include "log.hpp"
#include "rtp_client.hpp"

class SIPClient {
 public:
  // Constructor
  //
  // username: SIP username
  // password: SIP password
  // uri: The SIP Trunk uri of your provider
  // port: The local SIP port
  SIPClient(std::string username, std::string password, std::string uri, int port, int threadid);

  // Destructor
  ~SIPClient();

  // Register a new SIP Session with the set SIP provider
  // If Authentication is requred the credentials will be used in a second step
  //
  // Returns true if successfull, else false
  bool Register();

  // Invite a SIP client to a call after a successfull registration
  //
  // tel_nr: Telephone number which is to be called
  // max_call_duration: Maximum call duration in milliseconds
  // save_data: Specifies if call data is to be saved to the disk
  // call_duration: Pointer to an int where the call duration is to be saved
  //
  // Returns true if successfull, else false
  bool Invite(std::string tel_nr, double max_call_duration, bool save_data, int *call_duration);

  // Set a new default timeout value for events
  //
  // timeout: new value in seconds
  void SetTimeout(int timeout);

  // Get raw PCMA encoded Data from call
  //
  // Returns a vector containing the call data
  std::vector<int8_t> GetCallData();

 private:
  // Terminates the call if it is marked as active
  void TerminateCall();

  // Wait for a specific event to hapen. This method immediately returns control
  // back to the callee when the specified event occurs and stores the event in
  // this->last_event.
  //
  // event_type: Event to wait for
  // err_msg: Error message which is to displayed if the event does not occur
  // timeout_s: Timeout value in seconds which only applies to this method
  // timeout_ms: Timeout value in milliseconds which only applies to this method
  //
  // Returns true if the event has occured, false if not
  bool WaitForEvent(eXosip_event_type_t event_type, std::string err_msg, int timeout_s, int timeout_ms);

  // See WaitForEvent(eXpsip_event_type_t event_type, std::string err_msg)
  // The difference is, that this method uses the set timeout value
  bool WaitForEvent(eXosip_event_type_t event_type, std::string err_msg);

  // Parse a SDP response to an SIP Invite.
  // This response contains the port used for the rtp connection
  //
  // response: a pointer to the SDP response
  //
  // Returns the remote port used for the rtp connection.
  int ParseSDPResponse(osip_message_t *response);

  // event: The event id
  //
  // Returns a string representation for the given event.
  std::string EventToString(eXosip_event_type event);

  eXosip_t *context;            // eXosip context
  eXosip_event_t *last_event;   // Last event which has been processed

  std::string server_uri;       // URI of the SIP provider
  std::string username;         // SIP Username
  std::string password;         // SIP Password
  osip_message_t *reg_msg;      // The original registration message
  int reg_id;                   // Registration ID of the SIP session
  int call_id;                  // Call ID of the active call
  int dial_id;                  // Dial ID of the active call
  bool registered;              // True if registered with a SIP provider
  std::vector<int8_t> call_data;        // Raw pcma encoded data of the last call

  int timeout;                  // Default timeout value for events in seconds
  int threadid;
};

#endif  // INCLUDE_SIP_CLIENT_HPP_
