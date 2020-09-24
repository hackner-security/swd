// Copyright 2020 Barger M., Knoll M., Kofler L.

#include "sip_client.hpp"

SIPClient::SIPClient(std::string username, std::string password, std::string uri, int port, int threadid) {
  this->context = nullptr;
  this->last_event = nullptr;
  this->reg_msg = nullptr;
  this->threadid = threadid;

  // Initialize context
  if ( (context = eXosip_malloc()) == nullptr ) {
    Logger::GetLogger()->Log("eXosip_malloc() failed.", LOG_LVL_FATAL, threadid);
    throw "Fatal error in SIPClient()";
  }

  if ( eXosip_init(context) != 0 ) {
    Logger::GetLogger()->Log("eXosip_init() failed", LOG_LVL_FATAL, threadid);
    throw "Fatal error in SIPClient()";
  }

  // Set data
  this->username = username;
  this->password = password;
  this->server_uri = uri;
  this->reg_id = -1;
  this->call_id = -1;
  this->dial_id = -1;
  this->registered = false;
  this->timeout = 15;

  // Set User Agent
  eXosip_set_user_agent(this->context, "swd");

  // Start listening socket for SIP
  if ( eXosip_listen_addr(context, IPPROTO_UDP, nullptr, port, AF_INET, 0) != 0 ) {
    Logger::GetLogger()->Log("Failed to open UDP socket", LOG_LVL_ERROR, threadid);
    throw "Fatal error in SIPClient()";
  }
}

SIPClient::~SIPClient() {
  // Terminate active call
  TerminateCall();

  // Deregister at the server
  if (registered) {
    eXosip_lock(context);
    if ( eXosip_register_build_register(context, reg_id, 0, &reg_msg) < 0 ) {
      eXosip_unlock(context);
      Logger::GetLogger()->Log("Failed to build UNREGISTER message.", LOG_LVL_ERROR, threadid);
      return;
    }

    // Send de-registration message
    int send_status = eXosip_register_send_register(context, reg_id, reg_msg);
    eXosip_unlock(context);
    if ( send_status != OSIP_SUCCESS ) {
      Logger::GetLogger()->Log(
          "Failed to send UNREGISTER message. Status: " + std::to_string(send_status), LOG_LVL_ERROR, threadid);
      return;
    }

    // Wait for the de-registration process to complete
    WaitForEvent(EXOSIP_REGISTRATION_SUCCESS, "Answer to DEREGISTER message was not received");

    Logger::GetLogger()->Log("Successfully closed SIP session.", LOG_LVL_STATUS, threadid);
  }

  // Free memory
  eXosip_event_free(last_event);
  eXosip_quit(context);
}

bool SIPClient::Register() {
  // Update Registration if there is an active Session
  if (registered) {
    eXosip_lock(context);
    int status = eXosip_register_build_register(context, reg_id, 300, &reg_msg);
    if (status < 0) {
      Logger::GetLogger()->Log("Failed to update Registration: " + std::to_string(status), LOG_LVL_ERROR);
      registered = false;
    }

  eXosip_register_send_register(context, reg_id, reg_msg);
    eXosip_unlock(context);
    return registered;
  }

  // Else make new Registration
  // Set authentication info if provided
  if (username != "" && password != "") {
    eXosip_lock(context);
    eXosip_add_authentication_info(context, username.c_str(), username.c_str(), password.c_str(), nullptr, nullptr);
    eXosip_unlock(context);
  }

  // Build REGISTER message
  eXosip_lock(context);
  std::string sip_from = "sip:" + username + "@" + server_uri;
  std::string sip_proxy = "sip:" + server_uri;
  reg_id = eXosip_register_build_initial_register(context, sip_from.c_str(), sip_proxy.c_str(), nullptr, 300,
                                                  &reg_msg);

  eXosip_unlock(context);

  if (reg_id < 0) {
    Logger::GetLogger()->Log("Failed to build REGISTER message. Status: " + std::to_string(reg_id),
                              LOG_LVL_ERROR, threadid);
    switch (reg_id) {
      case OSIP_BADPARAMETER: std::cout << "BAD OSIP_BADPARAMETER" << std::endl; break;
      case OSIP_WRONG_STATE: std::cout << "OSIP_WRONG_STATE" << std::endl; break;
      case OSIP_NOMEM: std::cout << "OSIP_NOMEM" << std::endl; break;
      case OSIP_SYNTAXERROR: std::cout << "OSIP_SYNTAXERROR" << std::endl; break;
      default: std::cout << "NONE" <<std::endl; break;
    }
    return false;
  }

  // Send REGISTER message
  eXosip_lock(context);
  int send_status = eXosip_register_send_register(context, reg_id, reg_msg);
  eXosip_unlock(context);
  if ( send_status != OSIP_SUCCESS ) {
    Logger::GetLogger()->Log("Failed to send REGISTER message. Status: " + std::to_string(send_status),
                              LOG_LVL_ERROR, threadid);
    return false;
  }

  // Wait for the registration process to complete
  registered = WaitForEvent(EXOSIP_REGISTRATION_SUCCESS, "Answer to REGISTER message was not received");
  char *msg = nullptr;
  size_t buf_size = 0;
  if (last_event != nullptr) {
    osip_message_to_str(last_event->response, &msg, &buf_size);
    registered = (std::string(msg).find("Authorization failure") == std::string::npos);
  }
  delete msg;

  if (registered) {
    Logger::GetLogger()->Log("Registration successfull.", LOG_LVL_STATUS, threadid);
  } else {
    Logger::GetLogger()->Log("Registration failed.", LOG_LVL_ERROR, threadid);
  }
  return registered;
}

bool SIPClient::Invite(std::string tel_nr, double max_call_duration, bool save_data, int *call_duration) {
  if (!registered) {
    return false;
  }
  // Build INVITE message
  osip_message_t *invite_msg;
  std::string sip_to = "<sip:" + tel_nr + "@" + server_uri + ">";
  std::string sip_from = "<sip:" + username + "@" + server_uri + ">";
  int build_status = eXosip_call_build_initial_invite(context, &invite_msg, sip_to.c_str(), sip_from.c_str(),
                                                      nullptr, "SIP Call");

  if (build_status != 0) {
    Logger::GetLogger()->Log("Failed to build INVITE message. Status: " +
                      std::to_string(build_status), LOG_LVL_ERROR, threadid, tel_nr);
    return false;
  }

  // Add SDP body to message
  osip_message_set_supported(invite_msg, "100rel");
  int rtp_local_port = -1;
  int rtp_remote_port = 4242;   // This is a dummy to allow random allocation
  char local_ip[128];
  eXosip_guess_localip(context, AF_INET, local_ip, 128);

  // Start RTP Client
  std::string filename = save_data ? "rtp_dump_" + tel_nr : "";
  RTPClient rtp = RTPClient(server_uri, &rtp_local_port, 8, filename);

  std::string sdp_body =
    "v=0\r\n"
    "o=swd 0 0 IN IP4 " + std::string(local_ip) + "\r\n"
    "s=SIP Call\r\n"
    "c=IN IP4 " + std::string(local_ip) + "\r\n"
    "t=0 0\r\n"
    "m=audio " + std::to_string(rtp_local_port) + " RTP/AVP 8\r\n";

  osip_message_set_body(invite_msg, sdp_body.c_str(), sdp_body.length());
  osip_message_set_content_type(invite_msg, "application/sdp");

  // Send INVITE message
  Logger::GetLogger()->Log("Starting call", LOG_LVL_STATUS, threadid, tel_nr);
  eXosip_t *call_context = nullptr;
  eXosip_lock(context);
  call_id = eXosip_call_send_initial_invite(context, invite_msg);
  if (call_id <= 0) {
    eXosip_unlock(context);
    return false;
  }

  eXosip_call_set_reference(context, call_id, call_context);
  eXosip_unlock(context);

  // Get Call data by waiting for the first ring or a call request failure (auth)

  // Received no auth, this can happen but is weird
  if ( WaitForEvent(EXOSIP_CALL_REQUESTFAILURE, "Received no AUTH for INVITE") == false ) {
    if (last_event != nullptr) {
      dial_id = last_event->did;
    }
    if ( WaitForEvent(EXOSIP_CALL_PROCEEDING, "Called number can not be invited") == false ) {
      return false;
    }
  }
  dial_id = last_event->did;

  // Wait for the opposite site to pick up
  if ( WaitForEvent(EXOSIP_CALL_ANSWERED, "") == true ) {
    call_id = last_event->cid;
    dial_id = last_event->did;
    Logger::GetLogger()->Log("Invite has been accepted.", LOG_LVL_STATUS, threadid, tel_nr);

    rtp_remote_port = ParseSDPResponse(last_event->response);
    if (rtp_remote_port == -1) {
      Logger::GetLogger()->Log("Did not receive valid RTP target port or payload type.", LOG_LVL_WARN,
                                threadid, tel_nr);
      return false;
    }
  } else {
    Logger::GetLogger()->Log("Answer to INVITE message was not received", LOG_LVL_STATUS, threadid, tel_nr);
    // Call has not been answered
    TerminateCall();
    return false;
  }

  // Call Handling and Data Retrieval
  rtp.Init(rtp_remote_port);

  // Wait for the given amount of milliseconds or until call is closed
  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
  std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - begin;

  while (elapsed.count() < max_call_duration) {
    if (WaitForEvent(EXOSIP_CALL_CLOSED, "", 0, 500)) {
      Logger::GetLogger()->Log("Call was closed by remote side.", LOG_LVL_STATUS, threadid, tel_nr);
      call_id = -1;
      dial_id = -1;
      break;
    }

    rtp.ReceiveAll();
    elapsed = std::chrono::steady_clock::now() - begin;
  }
  *call_duration = static_cast<int> (elapsed.count() / 1000.f);
  rtp.ReceiveAll();
  call_data = rtp.GetRawData();

  Logger::GetLogger()->Log("Terminating call", LOG_LVL_STATUS, threadid, tel_nr);
  TerminateCall();
  return true;
}

void SIPClient::TerminateCall() {
  if (call_id != -1) {
    eXosip_lock(context);
    int status = eXosip_call_terminate(context, call_id, dial_id);
    if (status < 0 && status != -3) {
      Logger::GetLogger()->Log("Failed to build BYE message: " + std::to_string(status), LOG_LVL_WARN);
    }
    eXosip_unlock(context);

    call_id = -1;
    dial_id = -1;
  }
}

void SIPClient::SetTimeout(int timeout) {
  this->timeout = timeout;
}

std::vector<int8_t> SIPClient::GetCallData() {
  return call_data;
}

bool SIPClient::WaitForEvent(eXosip_event_type_t event_type, std::string err_msg, int timeout_s, int timeout_ms) {
  const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point curr_time = std::chrono::steady_clock::now();
  const double timeout = timeout_s * 1000 + timeout_ms;

  do {
    // Check if the timeout has been reached
    curr_time = std::chrono::steady_clock::now();
    if ( (std::chrono::duration<double, std::milli>(curr_time - begin)).count() >= timeout ) {
    if (err_msg != "") {
        Logger::GetLogger()->Log(err_msg, LOG_LVL_WARN, threadid);
      }
      return false;
    }

    // Receive the current event
    last_event = eXosip_event_wait(context, timeout_s, timeout_ms);
    if (last_event == nullptr) {
      if (err_msg != "") {
        Logger::GetLogger()->Log(err_msg, LOG_LVL_WARN, threadid);
      }
      return false;
    }

    // Process the event with the default action
    eXosip_lock(context);
    eXosip_automatic_action(context);
    eXosip_unlock(context);
  } while (last_event != nullptr && last_event->type != event_type);

  return true;
}

bool SIPClient::WaitForEvent(eXosip_event_type_t event_type, std::string err_msg) {
  return WaitForEvent(event_type, err_msg, timeout, 0);
}

int SIPClient::ParseSDPResponse(osip_message_t *response) {
  // Convert the response to a string
  char *buffer = nullptr;
  size_t buf_size = 0;
  osip_message_to_str(response, &buffer, &buf_size);
  std::istringstream msg_stream = std::istringstream(std::string(buffer));
  delete buffer;

  // Iterate over each line and look for m=audio
  std::string curr_line = "";
  while (std::getline(msg_stream, curr_line)) {
    if ( curr_line.find(std::string("m=audio")) != std::string::npos ) {
      std::vector<std::string> token;
      boost::split(token, curr_line, boost::is_any_of(" "));

      // Check if payload type is PCMA (8)
      if (std::stoi(token.at(3)) == 8) {
        return std::stoi(token.at(1));
      }
    }
  }

  // If PCMA was not found return -1
  return -1;
}

std::string SIPClient::EventToString(eXosip_event_type event) {
  switch (event) {
    case EXOSIP_REGISTRATION_SUCCESS: return "Registration Success";
    case EXOSIP_REGISTRATION_FAILURE: return "Registration Failure";
    case EXOSIP_CALL_INVITE: return "Call Invite";
    case EXOSIP_CALL_REINVITE: return "Call Reinvite";
    case EXOSIP_CALL_NOANSWER: return "Call No Answer";
    case EXOSIP_CALL_PROCEEDING: return "Call Proceeding";
    case EXOSIP_CALL_RINGING: return "Call Ringing";
    case EXOSIP_CALL_ANSWERED: return "Call Answered";
    case EXOSIP_CALL_REDIRECTED: return "Call Redirected";
    case EXOSIP_CALL_REQUESTFAILURE: return "Call Request Failure";
    case EXOSIP_CALL_SERVERFAILURE: return "Call Server Failure";
    case EXOSIP_CALL_GLOBALFAILURE: return "Call Global Failure";
    case EXOSIP_CALL_ACK: return "Call Acknowledgement";
    case EXOSIP_CALL_CANCELLED: return "Call Answered";
    case EXOSIP_CALL_MESSAGE_NEW: return "Call Message New";
    case EXOSIP_CALL_MESSAGE_PROCEEDING: return "Call Message Proceeding";
    case EXOSIP_CALL_MESSAGE_ANSWERED: return "Call Message Answered";
    case EXOSIP_CALL_MESSAGE_REDIRECTED: return "Call Message Redirected";
    case EXOSIP_CALL_MESSAGE_REQUESTFAILURE: return "Call Message Request Failure";
    case EXOSIP_CALL_MESSAGE_SERVERFAILURE: return "Call Message Server Failure";
    case EXOSIP_CALL_MESSAGE_GLOBALFAILURE: return "Call Message Global Failure";
    case EXOSIP_CALL_CLOSED: return "Call Closed";
    case EXOSIP_CALL_RELEASED: return "Call Released";
    case EXOSIP_MESSAGE_NEW: return "Message New";
    case EXOSIP_MESSAGE_PROCEEDING: return "Message Proceeding";
    case EXOSIP_MESSAGE_ANSWERED: return "Message Answered";
    case EXOSIP_MESSAGE_REDIRECTED: return "Message Redirected";
    case EXOSIP_MESSAGE_REQUESTFAILURE: return "Message Request Failure";
    case EXOSIP_MESSAGE_SERVERFAILURE: return "Message Server Failure";
    case EXOSIP_MESSAGE_GLOBALFAILURE: return "Message Global Failure";
    case EXOSIP_SUBSCRIPTION_NOANSWER: return "Suscription No Answer";
    case EXOSIP_SUBSCRIPTION_PROCEEDING: return "Subscription Proceeding";
    case EXOSIP_SUBSCRIPTION_ANSWERED: return "Subscription Answered";
    case EXOSIP_SUBSCRIPTION_REDIRECTED: return "Subscription Redirected";
    case EXOSIP_SUBSCRIPTION_REQUESTFAILURE: return "Subscription Request Failure";
    case EXOSIP_SUBSCRIPTION_SERVERFAILURE: return "Subscription Server Failure";
    case EXOSIP_SUBSCRIPTION_GLOBALFAILURE: return "Subscription Global Failure";
    case EXOSIP_SUBSCRIPTION_NOTIFY: return "Subscription Notify";
    case EXOSIP_IN_SUBSCRIPTION_NEW: return "New Subscriber/Referrer";
    case EXOSIP_NOTIFICATION_NOANSWER: return "Notification No Answer";
    case EXOSIP_NOTIFICATION_PROCEEDING: return "Notification Proceeding";
    case EXOSIP_NOTIFICATION_ANSWERED: return "Notification Answered";
    case EXOSIP_NOTIFICATION_REDIRECTED: return "Notification Redirected";
    case EXOSIP_NOTIFICATION_REQUESTFAILURE: return "Notification Request Failure";
    case EXOSIP_NOTIFICATION_SERVERFAILURE: return "Notification Server Failure";
    case EXOSIP_NOTIFICATION_GLOBALFAILURE: return "Notification Global Failure";
    default: return "No known event with id " + std::to_string(event);
  }
}
