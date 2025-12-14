#include "network/HTTPRouter.h"
#include "network/WebHandler.h"

namespace Engine {
namespace Network {

    HTTPRouter::HTTPRouter() {
        _sessions = std::make_shared<SessionStorage>();
    }
    HTTPRouter::HTTPRouter(const std::string sessionCacheID) {
        _sessions = std::make_shared<SessionStorage>();
        _sessions->LoadFromCache(sessionCacheID);
    }
    HTTPRouter::HTTPRouter(std::shared_ptr<SessionStorage> sessions) {
        _sessions = sessions;
    }

    void HTTPRouter::Route(const std::string subpath, std::shared_ptr<HTTPRouter> router) {
        if(router == nullptr) _routes.erase(subpath);
        else _routes[subpath] = router;
    }
    void HTTPRouter::Route(const std::string subpath, HTTPRouter* router) {
        if(router == nullptr) _routes.erase(subpath);
        else _routes[subpath] = router;
    }
    void HTTPRouter::Route(std::shared_ptr<HTTPRouter> router) {
        _defaultTo = router;
    }
    void HTTPRouter::Route(HTTPRouter* router) {
        _defaultTo = router;
    }

    bool HTTPRouter::Get(const std::string path) {
        ASSERT(_currentRequest, "[Network::HTTPRouter] Cannot use the Get function outside the virtual HandleRequest function")
        if(path != " " && _currentRequest->GetURL() != path) return false;
        return _currentRequest->GetMethod() == HTTP::Method::Get;
    }
    bool HTTPRouter::Head(const std::string path) {
        ASSERT(_currentRequest, "[Network::HTTPRouter] Cannot use the Head function outside the virtual HandleRequest function")
        if(path != " " && _currentRequest->GetURL() != path) return false;
        return _currentRequest->GetMethod() == HTTP::Method::Head;
    }
    bool HTTPRouter::Put(const std::string path) {
        ASSERT(_currentRequest, "[Network::HTTPRouter] Cannot use the Put function outside the virtual HandleRequest function")
        if(path != " " && _currentRequest->GetURL() != path) return false;
        return _currentRequest->GetMethod() == HTTP::Method::Put;
    }
    bool HTTPRouter::Post(const std::string path) {
        ASSERT(_currentRequest, "[Network::HTTPRouter] Cannot use the Post function outside the virtual HandleRequest function")
        if(path != " " && _currentRequest->GetURL() != path) return false;
        return _currentRequest->GetMethod() == HTTP::Method::Post;
    }
    bool HTTPRouter::Delete(const std::string path) {
        ASSERT(_currentRequest, "[Network::HTTPRouter] Cannot use the Delete function outside the virtual HandleRequest function")
        if(path != " " && _currentRequest->GetURL() != path) return false;
        return _currentRequest->GetMethod() == HTTP::Method::Delete;
    }
    bool HTTPRouter::Connect(const std::string path) {
        ASSERT(_currentRequest, "[Network::HTTPRouter] Cannot use the Connect function outside the virtual HandleRequest function")
        if(path != " " && _currentRequest->GetURL() != path) return false;
        return _currentRequest->GetMethod() == HTTP::Method::Connect;
    }
    bool HTTPRouter::Options(const std::string path) {
        ASSERT(_currentRequest, "[Network::HTTPRouter] Cannot use the Options function outside the virtual HandleRequest function")
        if(path != " " && _currentRequest->GetURL() != path) return false;
        return _currentRequest->GetMethod() == HTTP::Method::Options;
    }
    bool HTTPRouter::Trace(const std::string path) {
        ASSERT(_currentRequest, "[Network::HTTPRouter] Cannot use the Trace function outside the virtual HandleRequest function")
        if(path != " " && _currentRequest->GetURL() != path) return false;
        return _currentRequest->GetMethod() == HTTP::Method::Trace;
    }
    bool HTTPRouter::Patch(const std::string path) {
        ASSERT(_currentRequest, "[Network::HTTPRouter] Cannot use the Patch function outside the virtual HandleRequest function")
        if(path != " " && _currentRequest->GetURL() != path) return false;
        return _currentRequest->GetMethod() == HTTP::Method::Patch;
    }
    bool HTTPRouter::WebsocketUpgrade(const std::string path) {
        ASSERT(_currentRequest, "[Network::HTTPRouter] Cannot use the WebsocketUpgrade function outside the virtual HandleRequest function")
        if(path != " " && _currentRequest->GetURL() != path) return false;
        if(_currentRequest->GetHeader("Connection").find("Upgrade") == std::string::npos) return false;
        if(_currentRequest->GetHeader("Upgrade") != "websocket") return false;
        return _currentRequest->HasHeader("Sec-WebSocket-Key");
    }

    HTTPResponse HTTPRouter::DefaultResponse() {
        ASSERT(_currentRequest, "[Network::HTTPRouter] Cannot use the DefaultResponse function outside the virtual HandleRequest function")
        HTTPResponse response = std::make_shared<HTTP::Response>();
        response->SetResponseCode(HTTP::ResponseCode::OK);
        if(_currentRequest->GetHeader("Connection").find("close")!=std::string::npos) {
            response->SetHeader("Connection", "keep-alive");
            response->SetHeader("Keep-Alive", "timeout=5");
        }
        return response;
    }
    HTTPResponse HTTPRouter::File(const std::string filepath, const bool dontHandleOnException) {
        ASSERT(_currentRequest, "[Network::HTTPRouter] Cannot use the File function outside the virtual HandleRequest function")
        HTTPResponse response = DefaultResponse();
        if(!response->SetBodyToFile(filepath)) {
            // Failed to find body
            if(dontHandleOnException) return NotHandled();
            else return NotFound();
        }
        return response;
    }
    HTTPResponse HTTPRouter::Text(const std::string text) {
        ASSERT(_currentRequest, "[Network::HTTPRouter] Cannot use the Text function outside the virtual HandleRequest function")
        HTTPResponse response = DefaultResponse();
        response->SetHeader("Content-Type", "text/html");
        response->SetBodyToString(text);
        return response;
    }
    HTTPResponse HTTPRouter::BadRequest(const std::string reason) {
        ASSERT(_currentRequest, "[Network::HTTPRouter] Cannot use the BadRequest function outside the virtual HandleRequest function")
        HTTPResponse response = DefaultResponse();
        response->SetResponseCode(HTTP::ResponseCode::BadRequest);
        if(reason.length()) {
            response->SetHeader("Content-Type", "text/html");
            response->SetBodyToString(reason);
        }
        return response;
    }
    HTTPResponse HTTPRouter::InternalServerError(const std::string reason) {
        ASSERT(_currentRequest, "[Network::HTTPRouter] Cannot use the InternalServerError function outside the virtual HandleRequest function")
        HTTPResponse response = DefaultResponse();
        response->SetResponseCode(HTTP::ResponseCode::InternalServerError);
        if(reason.length()) {
            response->SetHeader("Content-Type", "text/html");
            response->SetBodyToString(reason);
        }
        return response;
    }
    HTTPResponse HTTPRouter::NotFound(const std::string message) {
        ASSERT(_currentRequest, "[Network::HTTPRouter] Cannot use the NotFound function outside the virtual HandleRequest function")
        HTTPResponse response = DefaultResponse();
        response->SetResponseCode(HTTP::ResponseCode::NotFound);
        if(message.length()) {
            response->SetHeader("Content-Type", "text/html");
            response->SetBodyToString(message);
        }
        return response;
    }
    HTTPResponse HTTPRouter::AcceptWebsocket(std::shared_ptr<WebsocketHandler> handler) {
        ASSERT(_currentRequest, "[Network::HTTPRouter] Cannot use the AcceptWebsocket function outside the virtual HandleRequest function")
        HTTPResponse response = DefaultResponse();
        response->SetResponseCode(HTTP::ResponseCode::SwitchingProtocols);
        response->SetHeader("Connection", "Upgrade");
        response->SetHeader("Upgrade", "websocket");
        response->SetHeader("Sec-WebSocket-Accept",
            Util::Base64Encode(Util::SHA1(_currentRequest->GetHeader("Sec-WebSocket-Key") + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"))
        );
        response->SetUserData(Util::WeirdPointer<WebsocketHandler>(handler));
        return response;
    }
    HTTPResponse HTTPRouter::AcceptWebsocket(WebsocketHandler* handler) {
        ASSERT(_currentRequest, "[Network::HTTPRouter] Cannot use the AcceptWebsocket function outside the virtual HandleRequest function")
        HTTPResponse response = DefaultResponse();
        response->SetResponseCode(HTTP::ResponseCode::SwitchingProtocols);
        response->SetHeader("Connection", "Upgrade");
        response->SetHeader("Upgrade", "websocket");
        response->SetHeader("Sec-WebSocket-Accept",
            Util::Base64Encode(Util::SHA1(_currentRequest->GetHeader("Sec-WebSocket-Key") + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"))
        );
        response->SetUserData(Util::WeirdPointer<WebsocketHandler>(handler));
        return response;
    }
    HTTPResponse HTTPRouter::DenyWebsocket() {
        ASSERT(_currentRequest, "[Network::HTTPRouter] Cannot use the DenyWebsocket function outside the virtual HandleRequest function")
        HTTPResponse response = DefaultResponse();
        response->SetResponseCode(HTTP::ResponseCode::Forbidden);
        return response;
    }
    HTTPResponse HTTPRouter::NotHandled() {
        return nullptr;
    }

    HTTPResponse HTTPRouter::NotFound(std::shared_ptr<HTTPRequest> request, const std::string message) {
        ASSERT(!_currentRequest, "[Network::HTTPRouter] Cannot use the NotFound (with request parameter) function inside the virtual HandleRequest function")
        _currentRequest = request;
        HTTPResponse response = NotFound(message);
        _currentRequest = nullptr;
        return response;
    }

    HTTPResponse HTTPRouter::HandleRequestInternal(std::shared_ptr<HTTPRequest> request) {
        ASSERT(!_currentRequest, "[Network::HTTPRouter] Detected routing loop or usage of HTTPRouter from multiple threads")
        _currentRequest = request;

        for(const auto&[path, router] : _routes) {
            if(path.starts_with(path)) {
                request->_url = request->_url.substr(path.size()-1);
                HTTPResponse response = router->HandleRequestInternal(request);
                _currentRequest = nullptr;
                return response;
            }
        }

        HTTPResponse response = HandleRequest(*request);
        if(response) {
            _currentRequest = nullptr;
            return response;
        }
        if(_defaultTo) {
            HTTPResponse response = _defaultTo->HandleRequestInternal(request);
            _currentRequest = nullptr;
            return response;
        }
        else {
            _currentRequest = nullptr;
            return nullptr;
        }
    }
}
}