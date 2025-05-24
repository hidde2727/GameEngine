#ifndef ENGINE_NETWORK_WEBSOCKET_FRAME_H
#define ENGINE_NETWORK_WEBSOCKET_FRAME_H

#include "core/PCH.h"
#include "util/Endianess.h"

namespace Engine {
namespace Network {
namespace Websocket {

    #ifndef ENGINE_NETWORK_MAXIMUM_WEBSOCKET_BODY
    #define ENGINE_NETWORK_MAXIMUM_WEBSOCKET_BODY 2048
    #endif;

    // See https://en.wikipedia.org/wiki/WebSocket#Opcodes
    enum class Opcode {
        Continuation=0,
        Text=1,
        Binary=2,
        Close=8,
        Ping=9,
        Pong=10
    };
    // See https://en.wikipedia.org/wiki/WebSocket#Status_codes
    enum class StatusCode {
        Close=1000,
        GoingAway=1001,
 	    ProtocolError=1002,
        UnsupportedData=1003,
        NoCodeReceived=1005,
        ConnectionClosedAbnormally=1006,
 	    InvalidPayloadData=1007,
        PolicyViolated=1008,
        MessageTooBig=1009,
        UnsupportedExtension=1010,
        InternalServerError=1011,
        TLSHandshakeFailure=1015
    };

    // Should only be used for either input and output, not both at the same time
    class Frame {
    public:

        // Network -> data
        asio::mutable_buffer GetHeaderBuffer();
        void OnHeaderRead();
        asio::mutable_buffer GetSecondHeaderBuffer();
        void OnSecondHeaderRead();
        asio::mutable_buffer GetBodyBuffer();
        void OnBodyRead();
        bool HasMandatoryResponse();
        bool ShouldCloseAfterReturningMessage();
        bool ShouldCloseAfterNextReceive();

        // Data -> Network
        std::vector<uint8_t> _body;// To create a request to be send to the network, just set _body to your data (and set it to be the correct size). The data needs to be in big endian
        std::vector<asio::const_buffer> GetWriteBuffers();
        void SetMandatoryResponse();// Will repurpose this frame to be a data->network frame to react to te incoming frame
        void SetClosingHandshake();// Will set the frame to contain data for stopping the connecction

        // General
        Opcode GetOpcode() { return _code; }

    private:
        std::vector<uint8_t> _buffer;
        Opcode _code;
        bool _errorOccured = false;
        StatusCode _errorCode;
        bool _masked;
        uint32_t _bodySize;
        uint8_t _mask[4];

        void CloseConnection(const StatusCode code);
    };

}
}
}

#endif