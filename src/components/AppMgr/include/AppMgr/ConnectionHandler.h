//
// Copyright (c) 2013, Ford Motor Company
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following
// disclaimer in the documentation and/or other materials provided with the
// distribution.
//
// Neither the name of the Ford Motor Company nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

#ifndef APPCONNECTIONHANDLER_H
#define APPCONNECTIONHANDLER_H

#include <string>
#include "utils/logger.h"


namespace connection_handler {
class DevicesDiscoveryStarter;
typedef std::string DeviceHandle;
}

namespace NsAppManager {
/**
 * \brief ConnectionHandler acts as wrapper for connection handler
 */
class ConnectionHandler {
  public:
    /**
     * \brief Returning class instance
     * \return class instance
     */
    static ConnectionHandler& getInstance();

    /**
     * \brief Sets connection handler instance
     * \param handler connection handler
     */
    void setConnectionHandler(connection_handler::DevicesDiscoveryStarter* handler);

    /**
     * \brief Gets connection handler instance
     * \return connection handler
     */
    connection_handler::DevicesDiscoveryStarter* getConnectionHandler() const;

    /**
     * \brief Start device discovery
     */
    void StartDevicesDiscovery();

    /**
     * \brief Connect to device specified in params
     * \param deviceHandle device handle
     */
    void ConnectToDevice(connection_handler::DeviceHandle deviceHandle);

    void StartTransportManager();

  private:

    /**
     * \brief Default class constructor
     */
    ConnectionHandler();

    /**
     * \brief Copy constructor
     */
    ConnectionHandler(const ConnectionHandler&);

    connection_handler::DevicesDiscoveryStarter* mConnectionHandler;
    static log4cxx::LoggerPtr logger_;
};

}

#endif // APPCONNECTIONHANDLER_H
