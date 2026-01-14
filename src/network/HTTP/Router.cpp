#include "network/HTTP/Router.h"
#include "network/WebHandler.h"

namespace Engine {
namespace Network {
namespace HTTP {

    Router::Router() {
        _sessions = std::make_shared<SessionStorage>(0);
    }
    Router::Router(const std::string sessionCacheID, const uint64_t sessionID) {
        _sessions = std::make_shared<SessionStorage>(sessionID);
        _sessions->LoadFromCache(sessionCacheID);
        _sessionStorageLocation = sessionCacheID;
    }
    Router::Router(std::shared_ptr<SessionStorage> sessions) {
        _sessions = sessions;
    }
    Router::~Router() {
        if(!_sessionStorageLocation.size()) return;
        _sessions->StoreToCache(_sessionStorageLocation);
    }

    void Router::Route(const std::string subpath, std::shared_ptr<Router> router, const bool useParentSessionStorage) {
        if(router == nullptr) _routes.erase(subpath);
        else {
            _routes[subpath] = router;
            if(useParentSessionStorage) router->_sessions = _sessions;
        }
    }
    void Router::Route(const std::string subpath, Router* router, const bool useParentSessionStorage) {
        if(router == nullptr) _routes.erase(subpath);
        else {
            _routes[subpath] = router;
            if(useParentSessionStorage) router->_sessions = _sessions;
        }
    }
    void Router::Route(std::shared_ptr<Router> router, const bool useParentSessionStorage) {
        _defaultTo = router;
        if(router && useParentSessionStorage) router->_sessions = _sessions;
    }
    void Router::Route(Router* router, const bool useParentSessionStorage) {
        _defaultTo = router;
        if(router && useParentSessionStorage) router->_sessions = _sessions;
    }

    bool Router::Get(const std::string path) {
        ASSERT(_currentRequest, "[Network::Router] Cannot use the Get function outside the virtual HandleRequest function")
        if(path != " " && _currentRequest->GetURL() != Util::RemoveLeading(path, '/')) return false;
        return _currentRequest->GetMethod() == HTTP::Method::Get;
    }
    bool Router::Head(const std::string path) {
        ASSERT(_currentRequest, "[Network::Router] Cannot use the Head function outside the virtual HandleRequest function")
        if(path != " " && _currentRequest->GetURL() != Util::RemoveLeading(path, '/')) return false;
        return _currentRequest->GetMethod() == HTTP::Method::Head;
    }
    bool Router::Put(const std::string path) {
        ASSERT(_currentRequest, "[Network::Router] Cannot use the Put function outside the virtual HandleRequest function")
        if(path != " " && _currentRequest->GetURL() != Util::RemoveLeading(path, '/')) return false;
        return _currentRequest->GetMethod() == HTTP::Method::Put;
    }
    bool Router::Post(const std::string path) {
        ASSERT(_currentRequest, "[Network::Router] Cannot use the Post function outside the virtual HandleRequest function")
        if(path != " " && _currentRequest->GetURL() != Util::RemoveLeading(path, '/')) return false;
        return _currentRequest->GetMethod() == HTTP::Method::Post;
    }
    bool Router::Delete(const std::string path) {
        ASSERT(_currentRequest, "[Network::Router] Cannot use the Delete function outside the virtual HandleRequest function")
        if(path != " " && _currentRequest->GetURL() != Util::RemoveLeading(path, '/')) return false;
        return _currentRequest->GetMethod() == HTTP::Method::Delete;
    }
    bool Router::Connect(const std::string path) {
        ASSERT(_currentRequest, "[Network::Router] Cannot use the Connect function outside the virtual HandleRequest function")
        if(path != " " && _currentRequest->GetURL() != Util::RemoveLeading(path, '/')) return false;
        return _currentRequest->GetMethod() == HTTP::Method::Connect;
    }
    bool Router::Options(const std::string path) {
        ASSERT(_currentRequest, "[Network::Router] Cannot use the Options function outside the virtual HandleRequest function")
        if(path != " " && _currentRequest->GetURL() != Util::RemoveLeading(path, '/')) return false;
        return _currentRequest->GetMethod() == HTTP::Method::Options;
    }
    bool Router::Trace(const std::string path) {
        ASSERT(_currentRequest, "[Network::Router] Cannot use the Trace function outside the virtual HandleRequest function")
        if(path != " " && _currentRequest->GetURL() != Util::RemoveLeading(path, '/')) return false;
        return _currentRequest->GetMethod() == HTTP::Method::Trace;
    }
    bool Router::Patch(const std::string path) {
        ASSERT(_currentRequest, "[Network::Router] Cannot use the Patch function outside the virtual HandleRequest function")
        if(path != " " && _currentRequest->GetURL() != Util::RemoveLeading(path, '/')) return false;
        return _currentRequest->GetMethod() == HTTP::Method::Patch;
    }
    bool Router::WebsocketUpgrade(const std::string path) {
        ASSERT(_currentRequest, "[Network::Router] Cannot use the WebsocketUpgrade function outside the virtual HandleRequest function")
        if(path != " " && _currentRequest->GetURL() != Util::RemoveLeading(path, '/')) return false;
        if(_currentRequest->GetHeader("Connection").find("Upgrade") == std::string::npos) return false;
        if(_currentRequest->GetHeader("Upgrade") != "websocket") return false;
        return _currentRequest->HasHeader("Sec-WebSocket-Key") && _currentRequest->HasHeader("Sec-WebSocket-Version");
    }

    std::shared_ptr<Response> Router::DefaultResponse() {
        ASSERT(_currentRequest, "[Network::Router] Cannot use the DefaultResponse function outside the virtual HandleRequest function")
        std::shared_ptr<Response> response = std::make_shared<HTTP::Response>();
        response->SetResponseCode(HTTP::ResponseCode::OK);
        if(_currentRequest->GetHeader("Connection").find("keep-alive")!=std::string::npos) {
            response->SetHeader("Connection", "Keep-Alive");
            response->SetHeader("Keep-Alive", "timeout=5");
        }
        response->SetHeader("Cache-Control", "no-cache");
        return response;
    }
    std::shared_ptr<Response> Router::File(const std::string filepath, const bool dontHandleOnException) {
        ASSERT(_currentRequest, "[Network::Router] Cannot use the File function outside the virtual HandleRequest function")
        std::shared_ptr<Response> response = DefaultResponse();
        if(!response->SetBodyToFile(filepath)) {
            // Failed to find body
            if(dontHandleOnException) return NotHandled();
            else return NotFound();
        }
        return response;
    }
    std::shared_ptr<Response> Router::Text(const std::string text) {
        ASSERT(_currentRequest, "[Network::Router] Cannot use the Text function outside the virtual HandleRequest function")
        std::shared_ptr<Response> response = DefaultResponse();
        response->SetHeader("Content-Type", "text/html");
        response->SetBodyToString(text);
        return response;
    }
    std::shared_ptr<Response> Router::BadRequest(const std::string reason) {
        ASSERT(_currentRequest, "[Network::Router] Cannot use the BadRequest function outside the virtual HandleRequest function")
        std::shared_ptr<Response> response = DefaultResponse();
        response->SetResponseCode(HTTP::ResponseCode::BadRequest);
        if(reason.length()) {
            response->SetHeader("Content-Type", "text/html");
            response->SetBodyToString(reason);
        }
        return response;
    }
    std::shared_ptr<Response> Router::InternalServerError(const std::string reason) {
        ASSERT(_currentRequest, "[Network::Router] Cannot use the InternalServerError function outside the virtual HandleRequest function")
        std::shared_ptr<Response> response = DefaultResponse();
        response->SetResponseCode(HTTP::ResponseCode::InternalServerError);
        if(reason.length()) {
            response->SetHeader("Content-Type", "text/html");
            response->SetBodyToString(reason);
        }
        return response;
    }
    std::shared_ptr<Response> Router::NotFound(const std::string message) {
        ASSERT(_currentRequest, "[Network::Router] Cannot use the NotFound function outside the virtual HandleRequest function")
        std::shared_ptr<Response> response = DefaultResponse();
        response->SetResponseCode(HTTP::ResponseCode::NotFound);
        if(message.length()) {
            response->SetHeader("Content-Type", "text/html");
            response->SetBodyToString(message);
        }
        return response;
    }

    std::shared_ptr<Response> Router::AcceptWebsocket(std::shared_ptr<Websocket::BasicHandler> handler) {
        return AcceptWebsocket(Util::WeirdPointer<Websocket::BasicHandler>(handler));
    }
    std::shared_ptr<Response> Router::AcceptWebsocket(Websocket::BasicHandler* handler) {
        return AcceptWebsocket(Util::WeirdPointer<Websocket::BasicHandler>(handler));
    }
    std::shared_ptr<Response> Router::AcceptWebsocket(Util::WeirdPointer<Websocket::BasicHandler> handler) {
        ASSERT(_currentRequest, "[Network::Router] Cannot use the AcceptWebsocket function outside the virtual HandleRequest function")
        std::shared_ptr<Response> response = DefaultResponse();
        // Websocket version
        if(_currentRequest->GetHeaderAsNumber<uint32_t>("Sec-WebSocket-Version") > 13) {
            WARNING("[Network::Router] Received a request to upgrade to a websocket version greater than implemented in this GameEngine (" + _currentRequest->GetHeader("Sec-WebSocket-Version") + ")")
            response = DenyWebsocket();
            response->SetHeader("Sec-WebSocket-Version", "13");
            return response;
        }
        // Websocket protocols
        if(_currentRequest->HasHeader("Sec-WebSocket-Protocol")) {
            // Check if the handler supports any of the protocols
            std::stringstream ss(_currentRequest->GetHeader("Sec-WebSocket-Protocol"));
            std::string selectedProtocol = "";
            while(!ss.eof()) {
                std::string protocol;
                ss >> std::ws;
                std::getline(ss, protocol, ',');
                if(handler->SupportsProtocol(protocol)) {
                    selectedProtocol = protocol;
                    break;
                }
            }

            if(!selectedProtocol.size()) {
                WARNING("[Network::Router] Received a request to upgrade to a websocket with a certain protocol, but the handler does not support it (" + _currentRequest->GetHeader("Sec-WebSocket-Protocol") + ")")
                response = DenyWebsocket();
                return response;
            }

            response->SetHeader("Sec-WebSocket-Protocol", selectedProtocol);
        }
        // Accept
        response->SetResponseCode(HTTP::ResponseCode::SwitchingProtocols);
        response->SetHeader("Connection", "Upgrade");
        response->SetHeader("Upgrade", "websocket");
        response->SetHeader("Sec-WebSocket-Accept",
            Util::Base64Encode(Util::SHA1(_currentRequest->GetHeader("Sec-WebSocket-Key") + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"))
        );
        response->SetUserData(handler);
        return response;
    }

    std::shared_ptr<Response> Router::DenyWebsocket() {
        ASSERT(_currentRequest, "[Network::Router] Cannot use the DenyWebsocket function outside the virtual HandleRequest function")
        std::shared_ptr<Response> response = DefaultResponse();
        response->SetResponseCode(HTTP::ResponseCode::Forbidden);
        return response;
    }
    std::shared_ptr<Response> Router::NotHandled() {
        return nullptr;
    }

    std::shared_ptr<Response> Router::NotFound(std::shared_ptr<HTTP::Request> request, const std::string message) {
        ASSERT(!_currentRequest, "[Network::Router] Cannot use the NotFound (with request parameter) function inside the virtual HandleRequest function")
        _currentRequest = request;
        std::shared_ptr<Response> response = NotFound(message);
        _currentRequest = nullptr;
        return response;
    }

    std::shared_ptr<Response> Router::HandleRequestInternal(std::shared_ptr<HTTP::Request> request) {
        ASSERT(!_currentRequest, "[Network::Router] Detected routing loop or usage of HTTP::Router from multiple threads")
        _currentRequest = request;
        // Remove leading /
        _currentRequest->_url = Util::RemoveLeading(_currentRequest->_url, '/');

        // Check all registered routes if they want to respond
        for(const auto&[path, router] : _routes) {
            if(path.starts_with(path)) {
                request->_url = request->_url.substr(path.size()-1);
                std::shared_ptr<Response> response = router->HandleRequestInternal(request);
                _currentRequest = nullptr;
                return response;
            }
        }
        
        // Retrieve the session
        uint64_t sessionID;
        bool createdNewSession = false;
        std::string sessionName = _sessions->GetCookieName();
        if(request->HasCookie(sessionName)) {
            try {
                sessionID = request->GetCookieAsNumber<uint64_t>(sessionName);
                if(!_sessions->HasSession(sessionID)) {
                    sessionID = _sessions->CreateSession();
                    createdNewSession = true;
                }
            } catch(...) {
                WARNING("[HTTP::Router] Found an illegal session ID on a request");
                sessionID = _sessions->CreateSession();
                createdNewSession = true;
            }
        } else {
            sessionID = _sessions->CreateSession();
            createdNewSession = true;
        }
        request->_session = _sessions->GetSession(sessionID);
        ASSERT(request->_session, "[HTTP::Router] Received a newly generated session as nullptr")

        // Handle request
        std::shared_ptr<Response> response;
        try {
            response = HandleRequest(*request);// Returns nullptr if not handled
        } catch(...) {
            return InternalServerError();
        }
        if(!response && _defaultTo) {
            try {
                response = _defaultTo->HandleRequestInternal(request);// Returns nullptr if not handled
            } catch(...) {
                return InternalServerError();
            }
        }
        if(response && createdNewSession) {
            Cookie cookie;
            cookie._value = std::to_string(sessionID);
            cookie._path = "/";
            cookie._httpOnly = true;
            cookie._sameSite = SameSite::Strict;
            cookie._maxAge = std::chrono::duration_cast<std::chrono::seconds>(ENGINE_NETWORK_SESSION_EXPIRATION).count();
            response->SetCookie(sessionName, cookie);
        }
        _currentRequest = nullptr;
        return response;
    }

}
}
}