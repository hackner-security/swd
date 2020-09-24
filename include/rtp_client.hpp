// Copyright 2020 Barger M., Knoll M. Kofler L.
#ifndef INCLUDE_RTP_CLIENT_HPP_
#define INCLUDE_RTP_CLIENT_HPP_

#include <signal.h>
#include <stdlib.h>

#include <ortp/ortp.h>
#include <string>
#include <vector>
#include "log.hpp"


class RTPClient {
 public:
  // Constructor
  //
  // server_uri: The SIP trunk uri of your provider
  // local_port: The local port which has been randomly chosen
  // payload_type: The Type of the payload as specified by SDP
  // file_name: The name of the output file. If none is given the data won't be saved
  RTPClient(std::string server_uri, int *local_port, int payload_type, std::string file_name);

  // Destructor
  ~RTPClient();

  // Initialize the RTP session and open ports
  //
  // remote_port: remote port which is to be used for RTP
  void Init(int remote_port);

  // Send Data (NOT IMPLEMENTED)
  void SendData();

  // Receive and save all RTP packets in the queue
  void ReceiveAll();

  // Get raw data vector
  //
  // returns a vector containing the received raw data
  std::vector<int8_t> GetRawData();

 private:
  RtpSession *session;          // RTP Session

  std::string server_uri;       // URI of the remote server
  std::string file_name;        // Output file name
  std::ofstream file;           // Output file
  int local_port;               // Local RTP port
  int remote_port;              // Remote RTP port
  int payload_type;             // Payload type
  int recv_ts;                  // Timestamp of the next packet to receive
  bool active;                  // States if the RTP client is active and listening
  bool save_data;               // Indicates if data is to be saved
  const int pdu_size = 160;     // Size of the data of a single PCMA encoded PDU
  std::vector<int8_t> raw_data;   // A vector containing the raw data
};

#endif  // INCLUDE_RTP_CLIENT_HPP_
