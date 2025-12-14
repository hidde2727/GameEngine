#ifndef ENGINE_NETWORK_HTTPROUTER_H
#define ENGINE_NETWORK_HTTPROUTER_H

#include "core/PCH.h"
#include "network/HTTP/Request.h"
#include "network/HTTP/Response.h"
#include "network/WebsocketHandler.h"
#include "network/SessionStorage.h"

#include "util/TemplateConcepts.h"
#include "util/WeirdPointer.h"

namespace Engine {
namespace Network {

    typedef std::shared_ptr<HTTP::Response> HTTPResponse;
    typedef HTTP::Request HTTPRequest;

    class HTTPRouter {
    public:

        /// @brief Create with an empty session storage
        HTTPRouter();
        /// @brief Create from a cache file
        HTTPRouter(const std::string sessionCacheID);
        /// @brief Create with a session storage
        HTTPRouter(std::shared_ptr<SessionStorage> sessions);

        /** 
         * Override this method to implement custom HTTP responses.
         * 
         * Example usage:
         * ```
         * HTTPResponse HandleRequest(HTTPRequest request) override {
         *     if(Get("/index.html")) {
         *         return File("index.html");
         *     }
         *     return NotHandled();// Indicates 'not handled'
         * }
         * ```
         * 
         * @param request The incoming request
         * @return NotHandled()/nullptr if not handled, else a HTTPResponse created with one of the helper functions
         */
        virtual HTTPResponse HandleRequest(HTTPRequest& request) { return nullptr; }

        /**
         * Use this method to, if the current handeled requests path starts with subpath, let the router paramter handle the request.
         * The router will receive the same request, but with the subpath parameter removed from the start of the path.
         * Routing "/" will result in all requests being forwarded to the router parameter.
         * 
         * @param subpath The path this router will forward to the router parameter.
         * @param router The router to forward the requests to subpath to. (use nullptr to remove the route)
         */
        void Route(const std::string subpath, std::shared_ptr<HTTPRouter> router);
        /**
         * Use this method to, if the current handeled requests path starts with subpath, let the router paramter handle the request.
         * The router will receive the same request, but with the subpath parameter removed from the start of the path.
         * Routing "/" will result in all requests being forwarded to the router parameter.
         * 
         * @param subpath The path this router will forward to the router parameter.
         * @param router The router to forward the requests to subpath to. (use nullptr to remove the route)
         * @warning The user must keep the pointer until removed from ```this```
         */
        void Route(const std::string subpath, HTTPRouter* router);

        /**
         * Use this method to, if the current handeled request isn't handled by this, let the router paramter handle the request.
         * 
         * @param router The router to forward the requests to subpath to. (use nullptr to remove the route)
         */
        void Route(std::shared_ptr<HTTPRouter> router);
        /**
         * Use this method to, if the current handeled request isn't handled by this, let the router paramter handle the request.
         * 
         * @param router The router to forward the requests to subpath to. (use nullptr to remove the route)
         * @warning The user must keep the pointer until removed from ```this```
         */
        void Route(HTTPRouter* router);

        /** 
         * These methods check if the current handled request is of the method(name of function) and path(parameter).
         * If the path parameter is omitted it will just check the request method.
         * The path parameter can only contain characters valid inside a HTTP URI.
         * 
         * Example usage:
         * ```
         * if(!Get()) return BadRequest();
         * // Checks if the current request is a GET request for index.html
         * if(Get("/index.html")) {
         *     return File("index.html");
         * }
         * ```
         * 
         * @name RequestMethods
         * @warning Only use these functions when handeling a request inside the HandleRequest function
         * @see HandleRequest()
         */
        ///@{
        bool Get(const std::string path = " ");
        bool Head(const std::string path = " ");
        bool Put(const std::string path = " ");
        bool Post(const std::string path = " ");
        bool Delete(const std::string path = " ");
        bool Connect(const std::string path = " ");
        bool Options(const std::string path = " ");
        bool Trace(const std::string path = " ");
        bool Patch(const std::string path = " ");
        /// Checks the path and websocket requirements for a request (does not check the method)
        bool WebsocketUpgrade(const std::string path = " ");
        ///@}

        /** 
         * These methods provide an easy way to return a response inside the HandleRequest function.
         *  
         * Example usage:
         * ```
         * // Checks if the current request is a GET request for index.html
         * if(Get("/index.html")) {
         *     return File("index.html");
         * }
         * ```
         * 
         * @name ResponseGenerators
         * @warning Only use these functions when handeling a request inside the HandleRequest function
         * @see HandleRequest()
         */
        ///@{
        HTTPResponse DefaultResponse();
        /**
         * Returns a response with a body set to a file.
         * If the file is not found && dontHandleOnException==false, returns NotFound().
         * If the file is not found && dontHandleOnException==true, returns NotHandled().
         */
        HTTPResponse File(const std::string filepath, const bool dontHandleOnException = true);
        HTTPResponse Text(const std::string text);
        HTTPResponse BadRequest(const std::string reason = "");
        HTTPResponse InternalServerError(const std::string reason = "");
        HTTPResponse NotFound(const std::string message = "");
        /// @warning Only use if the request is a websocket upgrade request
        HTTPResponse AcceptWebsocket(std::shared_ptr<WebsocketHandler> handler);
        /// @warning Only use if the request is a websocket upgrade request
        /// @warning The handler must stay alive until the websocket connection closes
        HTTPResponse AcceptWebsocket(WebsocketHandler* handler);
        /// @warning Only use if the request is a websocket upgrade request
        HTTPResponse DenyWebsocket();
        /// The same as returning nullptr
        HTTPResponse NotHandled();
        ///@}

        /// Create a HTTPResponse from a HTTPRequest
        /// @warning cannot be used inside the HandlerRequest() function
        HTTPResponse NotFound(std::shared_ptr<HTTPRequest> request, const std::string message = " ");

    protected:
        HTTPResponse HandleRequestInternal(std::shared_ptr<HTTPRequest> request);
    private:
        std::map<std::string, Util::WeirdPointer<HTTPRouter>> _routes;
        Util::WeirdPointer<HTTPRouter> _defaultTo = nullptr;

        // State while handeling a request
        std::shared_ptr<HTTPRequest> _currentRequest = nullptr;

        std::shared_ptr<SessionStorage> _sessions = nullptr;
    };

}
}

#endif