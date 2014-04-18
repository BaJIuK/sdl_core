/*
 Copyright (c) 2013, Ford Motor Company
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following
 disclaimer in the documentation and/or other materials provided with the
 distribution.

 Neither the name of the Ford Motor Company nor the names of its contributors
 may be used to endorse or promote products derived from this software
 without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SRC_COMPONENTS_APPLICATION_MANAGER_INCLUDE_APPLICATION_MANAGER_COMMANDS_COMMAND_IMPL_H_
#define SRC_COMPONENTS_APPLICATION_MANAGER_INCLUDE_APPLICATION_MANAGER_COMMANDS_COMMAND_IMPL_H_

#include "application_manager/commands/command.h"
#include "utils/logger.h"

namespace application_manager {
namespace commands {

/**
 * @brief Class is intended to encapsulate RPC as an object
 **/
class CommandImpl : public Command {
 public:
  /**
   * @brief CommandImpl class constructor
   *
   * @param message Incoming SmartObject message
   **/
  explicit CommandImpl(const MessageSharedPtr& message);

  /**
   * @brief CommandImpl class destructor
   *
   **/
  virtual ~CommandImpl();

  /**
   * @brief Init required by command resources
   **/
  virtual bool Init();

  /**
   * @brief Cleanup all resources used by command
   **/
  virtual bool CleanUp();

  /**
   * @brief Execute corresponding command by calling the action on reciever
   **/
  virtual void Run();

  // members
  static const int32_t hmi_protocol_type_;
  static const int32_t mobile_protocol_type_;
  static const int32_t protocol_version_;

 protected:
  MessageSharedPtr message_;

#ifdef ENABLE_LOG
static log4cxx::LoggerPtr logger_;
#endif // ENABLE_LOG

 private:
  DISALLOW_COPY_AND_ASSIGN(CommandImpl);
};

}  // namespace commands

}  // namespace application_manager

#endif  // SRC_COMPONENTS_APPLICATION_MANAGER_INCLUDE_APPLICATION_MANAGER_COMMANDS_COMMAND_IMPL_H_
