#ifndef ENGINE_NETWORK_WEBSOCKET_HANDLER_H
#define ENGINE_NETWORK_WEBSOCKET_HANDLER_H

#include "core/PCH.h"
#include "network/HTTP/SessionStorage.h"
#include "network/websocket/Connection.h"
#include "network/websocket/BasicHandler.h"
#include "network/websocket/Frame.h"

#include "util/serialization/Binary.h"
#include "util/serialization/ClassStructure.h"

namespace Engine {
namespace Network {

    #ifndef ENGINE_NETWORK_WEBSOCKET_CHECK_VALUE
    #define ENGINE_NETWORK_WEBSOCKET_CHECK_VALUE 133
    #endif

    #define ENGINE_NETWORK_WEBSOCKET_SERIALIZATION_FLAGS (static_cast<uint8_t>(Util::BinarySerializationOutputFlag::OutputBigEndian) \
                                                       | static_cast<uint8_t>(Util::BinarySerializationOutputFlag::ExcludeVersioning))
    #define ENGINE_NETWORK_WEBSOCKET_HANDLER_VERSION 1

    /**************************************************************************
     * Build in message types                                                 *
     *************************************************************************/

    /**
     * @brief Used every time a joystick is moved
     * 
     */
    struct JoystickEvent {
        /// The name of the joystick that moved
        std::string _name;
        /// The position of the joystick
        Util::Vec2F _pos;
    };
    
    /**************************************************************************
     * End of build in message types                                          *
     *************************************************************************/

    
    /**
     * @brief A websocket handler that implements the communication used in resources/engine/network/websocket.js
     * This class implements the server side part of the easy to use websocket protocol between the c++ code and the clients website
     * 
     * The structure of a message is the following:
     * WebsocketHandler::Packet (serialized with Util::BinarySerializer) {
     *      _type = A UUID of the message type;
     *      _data = AMessageType (serialized with util::BinarySerializer) {
     *          // The actual message
     *      }
     * }
     * 
     * You must put either of the following in the Derived class:
     * ```
     * using WebsocketHandler<Derived, MessageTypeID>::OnWebsocketMessage;
     * ```
     * Or put:
     * ```
     * template<class T>
     * OnWebsocketMessage(Websocket& connection, T& message) {
     *      // All the remaining types (not seperatly implemented) will go to this function
     *      THROW("Not implemented")
     * }
     * ```
     * 
     * The connecting client must specify it wants to use the 'gameengine-websocket-reflection-v1' websocket protocol (version may change, see ENGINE_NETWORK_WEBSOCKET_HANDLER_VERSION for accurate version)
     * 
     * To accept incoming websocket messages, include the following in your HTTP router:
     * ```
     *  std::shared_ptr<Network::HTTP::Response> HandleRequest(Network::HTTP::Request& request) override {
     *      // Not neccesary, but good to include (is for polling if the server is up after websocket disconnect)
     *      if(Get("IsWebsocketAlive")) return Text("true");
     *      // The actual important line (in this example this is a WebsocketHandler):
     *      if(WebsocketUpgrade()) return AcceptWebsocket(this);
     *      return NotHandled();
     *  }
     * ```
     * 
     * @tparam Derived The class that inherits from WebsocketHandler
     * @tparam MessageTypeID The format to identify message types with, with uin8_t a maximimum of UINT8_MAX types can be registered
     * @warning All the members of a message type must be accesible by Util::Serializer, Util::Deserializer and Util::ClassStructureSerializer
     * @warning You must implement OnWebsocketMessage in you Derived class
     */
    template<class Derived, Util::FundamentalType MessageTypeID=uint8_t>
    class WebsocketHandler : public Websocket::BasicHandler {
    public:
        typedef WebsocketHandler<Derived, MessageTypeID> WebsocketHandlerBase;

        class Connection {
        public:
            
            Connection() {}
            Connection(std::shared_ptr<WebsocketConnection> connection,
                std::shared_ptr<HTTP::Session> session,
                WebsocketHandler<Derived, MessageTypeID>* handler
            ) : _connection(connection), _session(session), _handler(handler) {}

            /**
             * @brief Send a message over the connection
             * 
             * @tparam T The message type (if it isn't registered at the WebsocketHandler, it will automatically be registered)
             * @param obj The object to send
             * @param deregisterAfter Deregisters the type T after sending the message
             */
            template<class T>
            void SendMessage(T& obj, const bool deregisterAfter = false) const;

            /**
             * @brief Returns the HTTP session associated with the websocket creation HTTP request
             * 
             * @return The session
             */
            std::shared_ptr<HTTP::Session> GetHTTPSession() const { return _session; }

        private:

            friend class WebsocketHandler<Derived, MessageTypeID>;
            std::shared_ptr<WebsocketConnection> _connection;
            std::shared_ptr<HTTP::Session> _session;
            WebsocketHandler<Derived, MessageTypeID>* _handler = nullptr;
        };

        WebsocketHandler() {
            RegisterPacketType<RegisterTypeMessage>(true);// id=0
            RegisterPacketType<UnregisterTypeMessage>(true);// id=1
            RegisterDefaultPacketTypes();
        }
        WebsocketHandler(WebsocketHandler<Derived, MessageTypeID>&) = delete;

        /**
         * @brief Called just after the HTTP accept message is send.
         * 
         * @param connection The connection object on which the new connection started.
         * @param request The HTTP upgrade request.
         */
        virtual void OnWebsocketStart(Connection& connection, HTTP::Request& request) {}
        /**
         * Is when a connection is stopped
         * 
         * @param connection The connection object of which the connection stopped.
         */
        virtual void OnWebsocketStop(Connection& connection) {}
        /**
         * @brief Is called when a recognized message is received
         * To receive certain message types, do the following:
         * ```
         * class YourImplementation : public WebsocketHandler<YourImplementation> {
         *      OnWebsocketMessage(Websocket& connection, SomeType& message) {
         *          // Do something with it
         *      }
         *      // Use either:
         *      using WebsocketHandler<Derived, MessageType>::OnWebsocketMessage;// All non implemented type will be handled with this statement
         *      // Or use:
         *      template<class T>
         *      OnWebsocketMessage(Websocket& connection, T& message) {
         *          // All the remaining types (not seperatly implemented) will go to this function
         *          THROW("Not implemented")
         *      }
         * }
         * 
         * @param connection The connection object on which a new message was received.
         * @param message The websocket message.
         * @tparam T A type registered with RegisterPacketType that was received
         */
        template<class T>
        void OnWebsocketMessage(Connection& connection, T& message) {
            WARNING("[Network::WebsocketHandler] OnWebsocketConnection was called, but isn't implemented for '" + Util::PrettyNameOf<T>() + "'")
        }
        /**
         * @brief Called when a message is received where the protocol is not recognized
         * 
         * @param connection The connection object on which a new message was received.
         * @param message The websocket message.
         */
        virtual void OnWebsocketMessage(Connection& connection, Websocket::Frame& message) {}

        /**
         * @brief Adds a class type to the types that can be send and received
         * 
         * @tparam T The type that can after this call be used in communication
         */
        template<class T>
        void RegisterPacketType();
        /**
         * @brief Registers the package types of all the ui components in resources/engine/network/ui.js
         * Registers the following classes:
         * - JoystickEvent
         */
        void RegisterDefaultPacketTypes();

        /**
         * @brief Removes a class type from the types that can be send and received
         * 
         * @tparam T The type to remove
         */
        template<class T>
        void UnregisterPacketType();

        /**
         * @brief Unregisters the package types of all the ui components in resources/engine/network/ui.js
         * Unregisters the following classes:
         * - JoystickEvent
         */
        void UnregisterDefaultPacketTypes();

        /**
         * @brief Check if T is registered
         * 
         * @tparam T The type to check
         * @return True -> registered, False-> not registered
         */
        template<class T>
        bool IsPacketTypeRegistered() const;
        
        /**
         * @brief Send a message to all the active websocket connections
         * 
         * @tparam T The message type to send (will be registered if not already registered with RegisterPacketType)
         * @param message The message to send
         */
        template<class T>
        void Broadcast(T& message);

        void OnWebsocketStart(WebsocketConnection& connection, HTTP::Request& request) override;
        void OnWebsocketStop(WebsocketConnection& connection) override;
        void OnWebsocketMessage(WebsocketConnection& connection, Websocket::Frame& message) override;
        bool SupportsProtocol(const std::string& protocol) override {
            return protocol == "gameengine-websocket-reflection-v" + std::to_string(ENGINE_NETWORK_WEBSOCKET_HANDLER_VERSION);
        }

        void operator=(WebsocketHandler<Derived, MessageTypeID>&) = delete;// I don't want to implement the copying of the connections
    private:
        /**
         * @brief Internal version of RegisterPacketType()
         * 
         * @tparam T The type that can after this call be used in communication
         * @param internalUsage If true will not send a registration message to all connections (will assign an id)
         */
        template<class T>
        void RegisterPacketType(const bool internalUsage);

        std::map<size_t, Connection> _websockets;

        /// Message to the client to register a packet type
        struct RegisterTypeMessage {
            MessageTypeID _id;
            std::vector<uint8_t> _typeData;
        };
        /// Message to the client to unregister a packet type
        struct UnregisterTypeMessage {
            MessageTypeID _id;
        };

        /// The packet that is send containing the actual message, type info and message type
        struct Packet {
            uint8_t _check = ENGINE_NETWORK_WEBSOCKET_CHECK_VALUE;
            MessageTypeID _type;
            std::vector<uint8_t> _data;
        };

        struct MessageType {
            std::function<void(Connection&, std::vector<uint8_t>&, const size_t)> _onReceive;
            std::function<void(std::shared_ptr<WebsocketConnection>, void*)> _send;
            MessageTypeID _id;
            std::vector<uint8_t> _typeInfo;
        };
        std::map<std::type_index, std::shared_ptr<MessageType>> _messageTypes;
        std::map<size_t, std::shared_ptr<MessageType>> _messageTypesByID;
        MessageTypeID _nextTypeID = 0;// First ID (0) and second (1) are reserved for type (de)registering
    };






    /*****************************************************************************
     * Connection                                                                *
     *****************************************************************************/
    template<class Derived, Util::FundamentalType MessageTypeID>
    template<class T>
    void WebsocketHandler<Derived, MessageTypeID>::Connection::SendMessage(T& obj, const bool deregisterAfter) const {
        if(!_handler->IsPacketTypeRegistered<T>()) _handler->template RegisterPacketType<T>();
        _handler->_messageTypes[std::type_index(typeid(T))]->_send(_connection, &obj);
        if(deregisterAfter) _handler->template UnregisterPacketType<T>();
    }

    /*****************************************************************************
     * Packet type registration                                                  *
     *****************************************************************************/
    template<class Derived, Util::FundamentalType MessageTypeID>
    template<class T>
    void WebsocketHandler<Derived, MessageTypeID>::RegisterPacketType() {
        RegisterPacketType<T>(false);
    }
    template<class Derived, Util::FundamentalType MessageTypeID>
    template<class T>
    void WebsocketHandler<Derived, MessageTypeID>::RegisterPacketType(const bool internalUsage) {
        ASSERT(!IsPacketTypeRegistered<T>(), "[Network::WebsocketHandler] Canot register a packet type multiple times")
        // Register
        std::shared_ptr<MessageType> type = std::make_shared<MessageType>();
        type->_id = _nextTypeID++;
        type->_onReceive = [this](Connection& websocket, std::vector<uint8_t>& message, const size_t startingOffset) {
            T decoded;
            Util::BinaryDeserializer deserializer;
            ASSERT(message[startingOffset] == ENGINE_NETWORK_WEBSOCKET_SERIALIZATION_FLAGS, "[Network::WebsocketHandler] Serialized data doesn't have the correct flags set")
            deserializer.Deserialize(decoded, message, startingOffset);
            try {
                static_cast<Derived*>(this)->OnWebsocketMessage(websocket, decoded);
            } catch(std::runtime_error err) {
                WARNING("[Network::WebsocketHandler] OnWebsocketMessage failed with the following error: " + std::string(err.what()))
            } catch(...) {
                WARNING("[Network::WebsocketHandler] OnWebsocketMessage failed")
            }
        };
        type->_send = [this](std::shared_ptr<WebsocketConnection> connection, void* data) {
            T& toEncode = *static_cast<T*>(data);

            std::shared_ptr<Websocket::Frame> frame = std::make_shared<Websocket::Frame>();
            frame->_body.resize(1+sizeof(MessageTypeID));
            frame->_body[0] = ENGINE_NETWORK_WEBSOCKET_CHECK_VALUE;
            *reinterpret_cast<MessageTypeID*>(&frame->_body[1]) = _messageTypes[std::type_index(typeid(T))]->_id;

            Util::BinarySerializer serializer;
            serializer.Serialize(toEncode, frame->_body, ENGINE_NETWORK_WEBSOCKET_SERIALIZATION_FLAGS);
            connection->SendData(frame);
        };
        {
        Util::ClassStructureSerializer serializer;
        serializer.Serialize<T>(type->_typeInfo, ENGINE_NETWORK_WEBSOCKET_SERIALIZATION_FLAGS);
        }

        _messageTypes[std::type_index(typeid(T))] = type;
        _messageTypesByID[type->_id] = type;
        if(internalUsage) return;

        // Send the type info to all the active connections
        RegisterTypeMessage message{};
        message._id = type->_id;
        message._typeData.resize(type->_typeInfo.size());
        memcpy(message._typeData.data(), type->_typeInfo.data(), type->_typeInfo.size());
        Broadcast(message);
    }

    template<class Derived, Util::FundamentalType MessageTypeID>
    void WebsocketHandler<Derived, MessageTypeID>::RegisterDefaultPacketTypes() {
        RegisterPacketType<JoystickEvent>();
    }

    template<class Derived, Util::FundamentalType MessageTypeID>
    template<class T>
    void WebsocketHandler<Derived, MessageTypeID>::UnregisterPacketType() {
        std::shared_ptr<MessageType> type = _messageTypes[std::type_index(typeid(T))];
        _messageTypes.erase(std::type_index(typeid(T)));
        _messageTypesByID.erase(type->_id);

        // Notify all the active connections
        UnregisterTypeMessage message{};
        message._id = type->_id;
        Broadcast(message);
    }

    template<class Derived, Util::FundamentalType MessageTypeID>
    void WebsocketHandler<Derived, MessageTypeID>::UnregisterDefaultPacketTypes() {
        UnregisterPacketType<JoystickEvent>();
    }

    template<class Derived, Util::FundamentalType MessageTypeID>
    template<class T>
    bool WebsocketHandler<Derived, MessageTypeID>::IsPacketTypeRegistered() const {
        return _messageTypes.count(std::type_index(typeid(T)));
    }
    /*****************************************************************************
     * Broadcasting                                                              *
     *****************************************************************************/
    template<class Derived, Util::FundamentalType MessageTypeID>
    template<class T>
    void WebsocketHandler<Derived, MessageTypeID>::Broadcast(T& message) {
        for(const auto& [id, websocket] : _websockets) {
            websocket.template SendMessage<T>(message);
        }
    }

    /*****************************************************************************
     * BasicHandler overrides                                                    *
     *****************************************************************************/
    template<class Derived, Util::FundamentalType MessageTypeID>
    void WebsocketHandler<Derived, MessageTypeID>::OnWebsocketStart(WebsocketConnection& connection, HTTP::Request& request) {
        _websockets[connection.GetUUID()] = Connection(connection.GetShared(), request.GetSession(), this);
        // Send the first message (versioning data and size of MessageTypeID)
        std::shared_ptr<Websocket::Frame> frame = std::make_shared<Websocket::Frame>();
        frame->_body.resize(4);
        frame->_body[0] = ENGINE_NETWORK_WEBSOCKET_CHECK_VALUE;
        frame->_body[1] = ENGINE_UTIL_BINARY_SERIALIZATION_VERSION;
        frame->_body[2] = ENGINE_UTIL_CLASS_STRUCTURE_SERIALIZATION_VERSION;
        frame->_body[3] = sizeof(MessageTypeID);
        connection.SendData(frame);
        // Send the type infos
        for(const auto& [id, typeInfo] : _messageTypes) {
            if(id == std::type_index(typeid(RegisterTypeMessage))) continue;// Internal message type
            if(id == std::type_index(typeid(UnregisterTypeMessage))) continue;// Internal message type

            RegisterTypeMessage message{};
            message._id = typeInfo->_id;
            message._typeData.resize(typeInfo->_typeInfo.size());
            memcpy(message._typeData.data(), typeInfo->_typeInfo.data(), typeInfo->_typeInfo.size());
            Broadcast(message);
        }
        // Notify Derived
        OnWebsocketStart(_websockets[connection.GetUUID()], request);
    }
    template<class Derived, Util::FundamentalType MessageTypeID>
    void WebsocketHandler<Derived, MessageTypeID>::OnWebsocketStop(WebsocketConnection& connection) {
        OnWebsocketStop(_websockets[connection.GetUUID()]);
        _websockets.erase(connection.GetUUID()); 
    }
    template<class Derived, Util::FundamentalType MessageTypeID>
    void WebsocketHandler<Derived, MessageTypeID>::OnWebsocketMessage(WebsocketConnection& connection, Websocket::Frame& message) {
        try {
            MessageTypeID type;
            ASSERT(message._body.size() > 1 + sizeof(MessageTypeID), "[Network::WebsocketHandler] Received a message that is too small")
            ASSERT(message._body[0] == ENGINE_NETWORK_WEBSOCKET_CHECK_VALUE, "[Network::WebsocketHandler] Received a message with the wrong check value")
            type = *(reinterpret_cast<MessageTypeID*>(&message._body[1]));
            type = Util::FromBigEndian(type);
            _messageTypesByID[type]->_onReceive(_websockets[connection.GetUUID()], message._body, 1+sizeof(MessageTypeID));
        } catch(...) {
            // Apparently it is not in the correct format
            LOG("[Network::WebsocketHandler] Received a message in an incorrect format")
            OnWebsocketMessage(_websockets[connection.GetUUID()], message);
            return;
        }
    }

}
}

#endif