#include "network/websocket/Frame.h"

namespace Engine {
namespace Network {
namespace Websocket {

    asio::mutable_buffer Frame::GetHeaderBuffer() {
        _buffer.resize(2);
        return asio::mutable_buffer(_buffer.data(), 2);
    }
    void Frame::OnHeaderRead() {
        if((_buffer[0]&0b10000000) == 0) { CloseConnection(StatusCode::PolicyViolated); return WARNING("[Websocket::Frame] Parser is not capable of handeling split message frames") }
        if((_buffer[0]&0b01110000) != 0) { CloseConnection(StatusCode::UnsupportedExtension); return WARNING("[Websocket::Frame] Parser received a message frame with a websocket extensions it does not support") }
        _code = static_cast<Opcode>(_buffer[0]&0b00001111);
        _masked = _buffer[1]>>7;
        _bodySize = _buffer[1]&0b01111111;
    }
    asio::mutable_buffer Frame::GetSecondHeaderBuffer() {
        int remainingHeaderSize = 0;
        if(_bodySize == 126) remainingHeaderSize += 2;
        if(_bodySize == 127) remainingHeaderSize += 4;
        if(_masked) remainingHeaderSize += 4;
        _buffer.resize(remainingHeaderSize);
        return asio::mutable_buffer(_buffer.data(), remainingHeaderSize);
    }
    void Frame::OnSecondHeaderRead() {
        int offset = 0;
        if(_bodySize == 126) {
            uint16_t size = *reinterpret_cast<uint16_t*>(&_buffer[0]);
            _bodySize = (uint32_t)Util::FromBigEndian(size);
            offset+=2;
        }
        else if(_bodySize == 127) {
            uint32_t size = *reinterpret_cast<uint32_t*>(&_buffer[0]);
            _bodySize = Util::FromBigEndian(size);
            offset+=4;
        }
        if(_masked) {
            _mask[0] = _buffer[offset];
            _mask[1] = _buffer[offset+1];
            _mask[2] = _buffer[offset+2];
            _mask[3] = _buffer[offset+3];
        }
        if(_bodySize > ENGINE_NETWORK_MAXIMUM_WEBSOCKET_BODY) { CloseConnection(StatusCode::MessageTooBig); WARNING("[Websocket::Frame] Parser received data that was bigger then ENGINE_NETWORK_MAXIMUM_WEBSOCKET_BODY") }
    }
    asio::mutable_buffer Frame::GetBodyBuffer() {
        _body.resize(_bodySize);
        return asio::mutable_buffer(_body.data(), _bodySize);
    }
    void Frame::OnBodyRead() {
        if(_masked) {
            for(uint32_t i = 0; i < _bodySize; i++) {
                _body[i] = _body[i] ^ _mask[i%4];
            }
        }
    }
    bool Frame::HasMandatoryResponse() {
        return (static_cast<uint8_t>(_code) > 7 && _code!=Opcode::Pong) || _errorOccured;
    }
    bool Frame::ShouldCloseAfterReturningMessage() {
        return _code == Opcode::Close;
    }
    bool Frame::ShouldCloseAfterNextReceive() {
        return _errorOccured;
    }

    std::vector<asio::const_buffer> Frame::GetWriteBuffers() {
        _buffer.clear();
        _buffer.push_back(0b10000010);// Binary frame
        if(_body.size() < 126) {
            _buffer.push_back((uint8_t)_body.size());
        } else if(_body.size() <= 0xFFFF) {
            uint16_t size = (uint16_t)_body.size();
            size = Util::ToBigEndian(size);
            uint8_t* sizePtr = reinterpret_cast<uint8_t*>(&size);
            _buffer.push_back(126);
            _buffer.push_back(*sizePtr);
            _buffer.push_back(*(sizePtr+1));
        } else if(_body.size() <= 0xFFFFFFFF) {
            uint32_t size = (uint32_t)_body.size();
            size = Util::ToBigEndian(size);
            uint8_t* sizePtr = reinterpret_cast<uint8_t*>(&size);
            _buffer.push_back(127);
            _buffer.push_back(*sizePtr);
            _buffer.push_back(*(sizePtr+1));
            _buffer.push_back(*(sizePtr+2));
            _buffer.push_back(*(sizePtr+3));
        } else {
            THROW("[Websocket::Frame] Received a message with a size too big to send in a websocket message (split messages are not supported)")
        }
        std::vector<asio::const_buffer> buffers;
        buffers.push_back(asio::const_buffer(_buffer.data(), _buffer.size()));
        buffers.push_back(asio::const_buffer(_body.data(), _body.size()));
        return buffers;
    }
    void Frame::SetMandatoryResponse() {
        if(_code == Opcode::Close || _errorOccured) {
            _code = Opcode::Close;
            _body.resize(2);
            uint16_t errorCode = Util::ToBigEndian(static_cast<uint16_t>(_errorCode));
            uint8_t* errorCodePtr = reinterpret_cast<uint8_t*>(&errorCode);
            _body[0] = *errorCodePtr;
            _body[1] = *(errorCodePtr+1);
        } else if(_code == Opcode::Ping) {
            _code = Opcode::Pong;
        }
    }
    void Frame::SetClosingHandshake() {
        CloseConnection(StatusCode::Close);
        SetMandatoryResponse();
    }

    void Frame::CloseConnection(const StatusCode code) {
        _errorOccured = true;
        _errorCode = code;
    }

}
}
}