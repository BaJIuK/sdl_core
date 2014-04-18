/*
 * Copyright (c) 2014, Ford Motor Company
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

#include "security_manager/security_manager.h"
#include "security_manager/crypto_manager_impl.h"
#include "protocol_handler/protocol_packet.h"
#include "utils/logger.h"
#include "utils/byte_order.h"
#include "json/json.h"

namespace security_manager {

CREATE_LOGGER(logger_, "SecurityManager")

static const char* err_id ="id";
static const char* err_text ="text";

SecurityManager::SecurityManager():
  security_messages_("SecurityManager::security_messages_", this),
  session_observer_(NULL), crypto_manager_(NULL), protocol_handler_(NULL) {
}

void SecurityManager::OnMessageReceived(
    const protocol_handler::RawMessagePtr message) {
  LOG4CXX_INFO(logger_, "OnMessageReceived");
  if (message->service_type() != protocol_handler::kControl) {
    LOG4CXX_INFO(logger_, "Skipping message; not the under SM handling");
    return;
  }

  SecurityMessage securityMessagePtr(new SecurityQuery());
  const bool result = securityMessagePtr->ParseQuery(
        message->data(), message->data_size());
  if (!result) {
    // result will be false only if data less then query header
    const std::string error_text("Incorrect message received");
    LOG4CXX_ERROR(logger_, error_text);
    SendInternalError(message->connection_key(),
                      SecurityQuery::ERROR_INVALID_QUERY_SIZE, error_text);
    return;
  }
  securityMessagePtr->set_connection_key(message->connection_key());

  // Post message to message query for next processing in thread
  security_messages_.PostMessage(securityMessagePtr);
}

void SecurityManager::OnMobileMessageSent(
    const protocol_handler::RawMessagePtr ) {
}

void SecurityManager::set_session_observer(
    protocol_handler::SessionObserver *observer) {
  if (!observer) {
    LOG4CXX_ERROR(logger_, "Invalid (NULL) pointer to SessionObserver.");
    return;
  }
  session_observer_ = observer;
}

void SecurityManager::set_protocol_handler(
    protocol_handler::ProtocolHandler *handler) {
  if (!handler) {
    LOG4CXX_ERROR(logger_, "Invalid (NULL) pointer to ProtocolHandler.");
    return;
  }
  protocol_handler_ = handler;
}

void SecurityManager::set_crypto_manager(CryptoManager *crypto_manager) {
  if (!crypto_manager) {
    LOG4CXX_ERROR(logger_, "Invalid (NULL) pointer to CryptoManager.");
    return;
  }
  crypto_manager_ = crypto_manager;
}

void SecurityManager::Handle(const SecurityMessage &message) {
  DCHECK(message);
  LOG4CXX_INFO(logger_, "Received Security message from Mobile side");
  if (!crypto_manager_)  {
    const std::string error_text("Invalid (NULL) CryptoManager.");
    LOG4CXX_ERROR(logger_, error_text);
    SendInternalError(message->get_connection_key(),
                      SecurityQuery::ERROR_NOT_SUPPORTED, error_text);
    return;
  }
  switch (message->get_header().query_id) {
    case SecurityQuery::SEND_HANDSHAKE_DATA:
      if (!ProccessHandshakeData(message)) {
        LOG4CXX_ERROR(logger_, "Proccess HandshakeData failed");
      }
      break;
    case SecurityQuery::SEND_INTERNAL_ERROR:
      if (!ProccessInternalError(message)) {
        LOG4CXX_ERROR(logger_, "Processing income InternalError failed");
      }
      break;
    default: {
      // SecurityQuery::InvalidQuery
      const std::string error_text("Unknown query identifier.");
      LOG4CXX_ERROR(logger_, error_text);
      SendInternalError(message->get_connection_key(),
                        SecurityQuery::ERROR_INVALID_QUERY_ID, error_text,
                        message->get_header().seq_number);
      }
      break;
    }
}

bool SecurityManager::ProtectConnection(const uint32_t& connection_key) {
  LOG4CXX_INFO(logger_, "ProtectService processing");
  DCHECK(session_observer_);
  DCHECK(crypto_manager_);

  if (session_observer_->GetSSLContext(connection_key,
                                      protocol_handler::kControl)) {
    const std::string error_text("Connection is already protected");
    LOG4CXX_WARN(logger_, error_text << ", key " << connection_key);
    SendInternalError(connection_key,
                      SecurityQuery::ERROR_SERVICE_ALREADY_PROTECTED, error_text);
    NotifyListenersOnHandshakeDone(connection_key, false);
    return false;
  }

  security_manager::SSLContext * newSSLContext = crypto_manager_->CreateSSLContext();
  if (!newSSLContext) {
    const std::string error_text("CryptoManager could not create SSL context.");
    LOG4CXX_ERROR(logger_, error_text);
    // Generate response query and post to security_messages_
    SendInternalError(connection_key, SecurityQuery::ERROR_CREATE_SLL, error_text);
    NotifyListenersOnHandshakeDone(connection_key, false);
    return false;
  }

  const int result = session_observer_->SetSSLContext(connection_key, newSSLContext);
  if (SecurityQuery::ERROR_SUCCESS != result) {
    // delete SSLContex on any error
    crypto_manager_->ReleaseSSLContext(newSSLContext);
    SendInternalError(connection_key, result, "");
    NotifyListenersOnHandshakeDone(connection_key, false);
    return false;
  }
  DCHECK(session_observer_->GetSSLContext(connection_key,
                                          protocol_handler::kControl));
  LOG4CXX_DEBUG(logger_, "Set SSL context to connection_key " << connection_key);
  return true;
}

void SecurityManager::StartHandshake(uint32_t connection_key) {
  DCHECK(session_observer_);
  LOG4CXX_INFO(logger_, "StartHandshake: connection_key " << connection_key);
  security_manager::SSLContext* ssl_context =
      session_observer_->GetSSLContext(connection_key,
                                       protocol_handler::kControl);
  if (!ssl_context) {
    const std::string error_text("StartHandshake failed, connection is not protected");
    LOG4CXX_ERROR(logger_, error_text);
    SendInternalError(connection_key, SecurityQuery::ERROR_INTERNAL, error_text);
    NotifyListenersOnHandshakeDone(connection_key, false);
    return;
  }

  if (!ssl_context->IsInitCompleted()) {
    size_t data_size;
    const uint8_t *data;
    security_manager::SSLContext::HandshakeResult result =
        ssl_context->StartHandshake(&data, &data_size);
    DCHECK(result == security_manager::SSLContext::Handshake_Result_Success);
    SendHandshakeBinData(connection_key, data, data_size);
  } else {
    NotifyListenersOnHandshakeDone(connection_key, true);
  }
}
void SecurityManager::AddListener(SecurityManagerListener * const listener) {
  if (!listener) {
    LOG4CXX_ERROR(logger_, "Invalid (NULL) pointer to SecurityManagerListener.");
    return;
  }
  listeners_.push_back(listener);
}
void SecurityManager::RemoveListener(SecurityManagerListener * const listener) {
  if (!listener) {
    LOG4CXX_ERROR(logger_, "Invalid (NULL) pointer to SecurityManagerListener.");
    return;
  }
  listeners_.remove(listener);
}
void SecurityManager::NotifyListenersOnHandshakeDone(const uint32_t &connection_key,
                                                     const bool success) {
  std::list<SecurityManagerListener*>::iterator it = listeners_.begin();
  while (it != listeners_.end()) {
    if ((*it)->OnHandshakeDone(connection_key, success)) {
      // On get notification remove listener
      it = listeners_.erase(it);
    } else {
      ++it;
      }
  }
}

bool SecurityManager::ProccessHandshakeData(const SecurityMessage &inMessage) {
  LOG4CXX_INFO(logger_, "SendHandshakeData processing");
  DCHECK(inMessage);
  DCHECK(inMessage->get_header().query_id == SecurityQuery::SEND_HANDSHAKE_DATA);
  const uint32_t seqNumber = inMessage->get_header().seq_number;
  const uint32_t connection_key = inMessage->get_connection_key();

  LOG4CXX_DEBUG(logger_, "Received " << inMessage->get_data_size()
                << " bytes handshake data ")

  if (!inMessage->get_data_size()) {
    const std::string error_text("SendHandshakeData: null arguments size.");
    LOG4CXX_ERROR(logger_, error_text);
    SendInternalError(connection_key,
                      SecurityQuery::ERROR_INVALID_QUERY_SIZE, error_text, seqNumber);
    return false;
  }
  DCHECK(session_observer_);
  SSLContext * sslContext =
      session_observer_->GetSSLContext(connection_key,
                                       protocol_handler::kControl);
  if (!sslContext) {
    const std::string error_text("SendHandshakeData: No ssl context.");
    LOG4CXX_ERROR(logger_, error_text);
    SendInternalError(connection_key,
                      SecurityQuery::ERROR_SERVICE_NOT_PROTECTED, error_text, seqNumber);
    NotifyListenersOnHandshakeDone(connection_key, false);
    return false;
  }
  size_t out_data_size;
  const uint8_t * out_data;
  const SSLContext::HandshakeResult handshake_result =
      sslContext->DoHandshakeStep(inMessage->get_data(), inMessage->get_data_size(),
                                  &out_data, &out_data_size);
  if (handshake_result == SSLContext::Handshake_Result_AbnormalFail) {
    // Do not return handshake data on AbnormalFail or null returned values
    const std::string erorr_text(LastError());
    LOG4CXX_ERROR(logger_, "SendHandshakeData: Handshake failed: " << erorr_text);
    SendInternalError(connection_key,
                      SecurityQuery::ERROR_SSL_INVALID_DATA, erorr_text, seqNumber);
    NotifyListenersOnHandshakeDone(connection_key, false);
    // no handshake data to send
    return false;
  }
  if (sslContext->IsInitCompleted()) {
    // On handshake success
    LOG4CXX_DEBUG(logger_, "SSL initialization finished success.");
    NotifyListenersOnHandshakeDone(connection_key, true);
  } else if (handshake_result == SSLContext::Handshake_Result_Fail) {
    // On handshake fail
    LOG4CXX_WARN(logger_, "SSL initialization finished with fail.");
    NotifyListenersOnHandshakeDone(connection_key, false);
  }

  if (out_data && out_data_size) {
    // answer with the same seqNumber as income message
    SendHandshakeBinData(connection_key, out_data, out_data_size,
                         seqNumber);
  }
  return true;
}

bool SecurityManager::ProccessInternalError(const SecurityMessage &inMessage) {
  LOG4CXX_INFO(logger_, "Recieved InternalError with Json message"
                << inMessage->get_json_message());
  Json::Value root;
  Json::Reader reader;
  const bool parsingSuccessful =
      reader.parse(inMessage->get_json_message(), root);
  if (!parsingSuccessful)
    return false;
  LOG4CXX_DEBUG(logger_, "Recieved InternalError id " << root[err_id].asString()
                << ", text: " << root[err_text].asString());
  return true;
}

void SecurityManager::SendHandshakeBinData(
    const uint32_t connection_key, const uint8_t * const data,
    const size_t data_size, const uint32_t seq_number) {
  const SecurityQuery::QueryHeader header(
        SecurityQuery::NOTIFICATION,
        SecurityQuery::SEND_HANDSHAKE_DATA, seq_number);
  SendData(connection_key, header, data, data_size);
  LOG4CXX_DEBUG(logger_, "Sent " << data_size << " bytes handshake data ")
}

void SecurityManager::SendInternalError(const uint32_t connection_key,
                                        const uint8_t &error_id,
                                        const std::string& erorr_text,
                                        const uint32_t seq_number) {
  Json::Value value;
  value[err_id]   = error_id;
  value[err_text] = erorr_text;
  const std::string error_str = value.toStyledString();
  SecurityQuery::QueryHeader header(SecurityQuery::NOTIFICATION,
                                    SecurityQuery::SEND_INTERNAL_ERROR,
                                    seq_number, error_str.size());
  std::vector<uint8_t> data_sending(error_str.size() + 1);
  memcpy(&data_sending[0], error_str.c_str(), error_str.size());
  data_sending[data_sending.size()-1] = error_id;

  SendData(connection_key, header, &data_sending[0], data_sending.size());
  LOG4CXX_DEBUG(logger_, "Sent Internal error id " << error_id << " : "
                << erorr_text << ".");
}

void SecurityManager::SendData(
    const uint32_t connection_key,
    SecurityQuery::QueryHeader header,
    const uint8_t * const data, const size_t data_size) {
  // FIXME(EZ): move to SecurityQuery class
  header.query_id  = LE_TO_BE32(header.query_id << 8);
  header.json_size = LE_TO_BE32(header.json_size);

  const size_t header_size = sizeof(header);
  std::vector<uint8_t> data_sending(header_size + data_size);
  memcpy(&data_sending[0], &header, header_size);
  // TODO(EZamakhov): Fix invalid read (by Valgrind)
  memcpy(&data_sending[header_size], data, data_size);

  SendBinaryData(connection_key, &data_sending[0], data_sending.size());
}

void SecurityManager::SendBinaryData(const uint32_t connection_key,
                                     const uint8_t * const data,
                                     size_t data_size) {
  DCHECK(protocol_handler_);
  const protocol_handler::RawMessagePtr rawMessagePtr(
        new protocol_handler::RawMessage(connection_key,
                                         protocol_handler::PROTOCOL_VERSION_2,
                                         data, data_size,
                                         protocol_handler::kControl));
  // Add RawMessage to ProtocolHandler message query
  protocol_handler_->SendMessageToMobileApp(rawMessagePtr, false);
}

const char *SecurityManager::ConfigSection() {
  return "Security Manager";
}
}  // namespace security_manager
