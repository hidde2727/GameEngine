#ifndef ENGINE_NETWORK_HTTP_ROUTER_H
#define ENGINE_NETWORK_HTTP_ROUTER_H

#include "core/PCH.h"
#include "network/HTTP/Request.h"
#include "network/HTTP/Response.h"
#include "network/HTTP/SessionStorage.h"
#include "network/websocket/BasicHandler.h"

#include "util/TemplateConcepts.h"
#include "util/WeirdPointer.h"
#include "util/Strings.h"

namespace Engine {
namespace Network {
namespace HTTP {

    class Router {
    public:

        /**
         * @brief Create with an empty session storage
         * 
         */
        Router();
        /**
         * @brief Create from a cache file
         * 
         * @param sessionCacheID The cacheID to retrieve the stored sessions (see FileManager::GetCacheFile for cacheID's)
         * @param sessionID The id to use to identify the cookies of different session storages (0 is in use by the Game class)
         */
        Router(const std::string sessionCacheID, const uint64_t sessionID);
        /**
         * @brief Create with a session storage
         * 
         * @param sessions The session storage to use (must have a unique sessionID and not use 0 as id)
         * @warning The SessionStorage id must be unique and not 0 (0 is in use by the Game class) 
         */
        Router(std::shared_ptr<SessionStorage> sessions);

        ~Router();

        /** 
         * Override this method to implement custom HTTP responses.
         * 
         * Example usage:
         * ```
         * std::shared_ptr<Response> HandleRequest(HTTPRequest request) override {
         *     if(Get("/index.html")) {
         *         return File("index.html");
         *     }
         *     return NotHandled();// Indicates 'not handled'
         * }
         * ```
         * 
         * @param request The incoming request
         * @return NotHandled()/nullptr if not handled, else a std::shared_ptr<Response> created with one of the helper functions
         */
        virtual std::shared_ptr<Response> HandleRequest(Request& request) { return nullptr; }

        /**
         * Use this method to, if the current handeled requests path starts with subpath, let the router paramter handle the request.
         * The router will receive the same request, but with the subpath parameter removed from the start of the path.
         * Routing "/" will result in all requests being forwarded to the router parameter.
         * 
         * @param subpath The path this router will forward to the router parameter.
         * @param router The router to forward the requests to subpath to. (use nullptr to remove the route)
         * @param useParentSessionStorage Will set the session storage of router to the session storage of this
         */
        void Route(const std::string subpath, std::shared_ptr<Router> router, const bool useParentSessionStorage=true);
        /**
         * Use this method to, if the current handeled requests path starts with subpath, let the router paramter handle the request.
         * The router will receive the same request, but with the subpath parameter removed from the start of the path.
         * Routing "/" will result in all requests being forwarded to the router parameter.
         * 
         * @param subpath The path this router will forward to the router parameter.
         * @param router The router to forward the requests to subpath to. (use nullptr to remove the route)
         * @param useParentSessionStorage Will set the session storage of router to the session storage of this
         * @warning The user must keep the pointer until removed from ```this```
         */
        void Route(const std::string subpath, Router* router, const bool useParentSessionStorage=true);

        /**
         * @brief Removes a route
         * Is equivalent to calling:
         * ```
         * Route(subpath, nullptr);
         * ```
         * @param subpath The path to remove
         */
        void RemoveRouter(const std::string subpath) {
            Route(subpath, (Router*)nullptr, false);
        }

        /**
         * Use this method to, if the current handeled request isn't handled by this, let the router paramter handle the request.
         * 
         * @param router The router to forward the requests to subpath to. (use nullptr to remove the route)
         * @param useParentSessionStorage Will set the session storage of router to the session storage of this
         */
        void Route(std::shared_ptr<Router> router, const bool useParentSessionStorage=true);
        /**
         * Use this method to, if the current handeled request isn't handled by this, let the router paramter handle the request.
         * 
         * @param router The router to forward the requests to subpath to. (use nullptr to remove the route)
         * @param useParentSessionStorage Will set the session storage of router to the session storage of this
         * @warning The user must keep the pointer until removed from ```this```
         */
        void Route(Router* router, const bool useParentSessionStorage=true);

        /** 
         * These methods check if the current handled request is of the method(name of function) and path(parameter).
         * If the path parameter is omitted it will just check the request method.
         * The path parameter can only contain characters valid inside a HTTP URI.
         * 
         * Example usage:
         * ```
         * if(!Get()) return BadRequest();
         * // Checks if the current request is a GET request for index.html
         * if(Get("index.html")) {// Corresponding URL: https://localhost/index.html
         *     return File("index.html");
         * }
         * if(Get("")) {// Corresponding URL: https://localhost/
         *     return File("index.html");
         * }
         * ```
         * Note that the default param of " " is used to indicate no path checking should be done (a space is illegal in a url)
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
        std::shared_ptr<Response> DefaultResponse();
        /**
         * Returns a response with a body set to a file.
         * If the file is not found && dontHandleOnException==false, returns NotFound().
         * If the file is not found && dontHandleOnException==true, returns NotHandled().
         */
        std::shared_ptr<Response> File(const std::string filepath, const bool dontHandleOnException = true);
        std::shared_ptr<Response> Text(const std::string text);
        std::shared_ptr<Response> BadRequest(const std::string reason = "");
        std::shared_ptr<Response> InternalServerError(const std::string reason = "");
        std::shared_ptr<Response> NotFound(const std::string message = "");
        /// @warning Only use if the request is a websocket upgrade request
        std::shared_ptr<Response> AcceptWebsocket(std::shared_ptr<Websocket::BasicHandler> handler);
        /// @warning Only use if the request is a websocket upgrade request
        /// @warning The handler must stay alive until the websocket connection closes
        std::shared_ptr<Response> AcceptWebsocket(Websocket::BasicHandler* handler);
        /// @warning Only use if the request is a websocket upgrade request
        std::shared_ptr<Response> DenyWebsocket();
        /// The same as returning nullptr
        std::shared_ptr<Response> NotHandled();
        ///@}

        /// Create a std::shared_ptr<Response> from a HTTPRequest
        /// @warning cannot be used inside the HandlerRequest() function
        std::shared_ptr<Response> NotFound(std::shared_ptr<Request> request, const std::string message = " ");

    protected:
        std::shared_ptr<Response> HandleRequestInternal(std::shared_ptr<Request> request);
    private:
        std::shared_ptr<Response> AcceptWebsocket(Util::WeirdPointer<Websocket::BasicHandler> handler);

        std::map<std::string, Util::WeirdPointer<Router>> _routes;
        Util::WeirdPointer<Router> _defaultTo = nullptr;

        // State while handeling a request
        std::shared_ptr<Request> _currentRequest = nullptr;

        std::shared_ptr<SessionStorage> _sessions = nullptr;
        std::string _sessionStorageLocation;
    };

}
}
}

#endif