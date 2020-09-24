// Copyright 2020 Barger M., Knoll M., Kofler L.

#include "rtp_client.hpp"

RTPClient::RTPClient(std::string server_uri, int *local_port, int payload_type, std::string file_name) {
  this->server_uri = server_uri;
  this->remote_port = 4242;
  this->payload_type = payload_type;
  this->file_name = file_name;

  this->recv_ts = 0;
  this->active = false;
  this->save_data = file_name != "" ? true : false;

  if (this->save_data) {
    file.open(file_name, std::ios::out | std::ios::binary);
  }
  ortp_init();
  ortp_scheduler_init();
  ortp_set_log_level_mask(nullptr, 0);

  session = rtp_session_new(RTP_SESSION_SENDRECV);
  rtp_session_set_scheduling_mode(session, 1);
  rtp_session_set_blocking_mode(session, 1);
  rtp_session_set_connected_mode(session, true);
  rtp_session_set_symmetric_rtp(session, true);
  rtp_session_set_payload_type(session, payload_type);

  rtp_session_set_remote_addr(session, server_uri.c_str(), remote_port);
  *local_port = this->local_port = rtp_session_get_local_port(session);
}

RTPClient::~RTPClient() {
  rtp_session_destroy(session);
  ortp_exit();

  if (file.is_open()) {
    file.close();
  }

  // If no data has been received delete the file
  if (raw_data.size() == 0) {
    std::remove(file_name.c_str());
  }
}

void RTPClient::Init(int remote_port) {
  // Send dummy data to initialize RTP transfer
  this->remote_port = remote_port;
  rtp_session_set_remote_addr(session, server_uri.c_str(), this->remote_port);
  rtp_session_send_with_ts(session, nullptr, 0, 0);
  this->active = true;
}

void RTPClient::SendData() {
  if (this->active == false) {
    Logger::GetLogger()->Log("Trying to send on an inactive RTP Session", LOG_LVL_ERROR);
    return;
  }
}

void RTPClient::ReceiveAll() {
  if (this->active == false) {
    Logger::GetLogger()->Log("Trying to receive on an inactive RTP Session", LOG_LVL_ERROR);
    return;
  }

  uint8_t *buffer = new uint8_t[pdu_size];
  int bytes_rcvd = 0;
  int bytes_left = 0;

  // Get payload data from queue
  do {
    bytes_rcvd = rtp_session_recv_with_ts(session, buffer, pdu_size, recv_ts, &bytes_left);
    recv_ts += pdu_size;

    if (bytes_rcvd != 0) {
      // Write data to file
      if (save_data) {
        file.write(reinterpret_cast<char *>(buffer), pdu_size);
        // file.flush();
      }

      // Write data to vector
      for (int i = 0; i < pdu_size; i++) {
        raw_data.push_back(buffer[i]);
      }
    }
  } while (bytes_rcvd != 0);
  free(buffer);
}

std::vector<int8_t> RTPClient::GetRawData() {
  return raw_data;
}
