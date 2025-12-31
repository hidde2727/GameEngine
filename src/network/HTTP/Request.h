#ifndef ENGINE_NETWORK_HTTP_REQUESTHEADER_H
#define ENGINE_NETWORK_HTTP_REQUESTHEADER_H

#include "core/PCH.h"
#include "network/HTTP/SessionStorage.h"
#include "util/TemplateConcepts.h"

namespace Engine {
namespace Network {
namespace HTTP {
    class Router;
    
    enum class Method {
        None,
		Get,
		Head,
		Post,
		Put,
		Delete,
		Connect,
		Options,
		Trace,
		Patch
	};

    Method StringToMethod(std::string);
    std::string MethodToString(Method);

    class Request {
    public:

        /// @warning Internal
        void ParseHeader(std::istream& stream, const size_t size);

        Method GetMethod() const { return _method; }
        std::string GetURL() const { return _url; }
        std::string GetVersion() const { return _version; }
        std::string GetHeader(const std::string name) { return _headers[name]; }
        template<Util::FundamentalType T>
        T GetHeaderAsNumber(const std::string name) {
            std::string& header = _headers[name];
            T result;
            auto [ptr, ec] = std::from_chars(header.data(), header.data() + header.size(), result);
            if (ec == std::errc::invalid_argument)
                THROW("[HTTP::Request] Trying to get header as a number, but header isn't a number")
            else if (ec == std::errc::result_out_of_range)
                THROW("[HTTP::Request] Trying to get header as a number, but header is a number that is too large")
            else if(ptr != NULL && ptr[0] != '\0')
                THROW("[HTTP::Request] Trying to get header as a number, but received a number followed by some string '" + std::string(ptr) + "'")
            return result;
        }
        bool HasHeader(const std::string name) const { return _headers.count(name) == 1; }
        
        std::string GetCookie(const std::string name) { return _cookies[name]; }
        bool HasCookie(const std::string& name) const { return _cookies.count(name) == 1; }
        template<Util::FundamentalType T>
        T GetCookieAsNumber(const std::string name) {
            std::string& cookie = _cookies[name];
            T result;
            auto [ptr, ec] = std::from_chars(cookie.data(), cookie.data() + cookie.size(), result);
            if (ec == std::errc::invalid_argument)
                THROW("[HTTP::Request] Trying to get cookie as a number, but cookie isn't a number")
            else if (ec == std::errc::result_out_of_range)
                THROW("[HTTP::Request] Trying to get cookie as a number, but cookie is a number that is too large")
            else if(ptr != NULL && ptr[0] != '\0')
                THROW("[HTTP::Request] Trying to get cookie as a number, but received a number followed by some string '" + std::string(ptr) + "'")
            return result;
        }
        std::string GetHead() const { return MethodToString(_method) + " " + _url + " " + _version; }

        std::vector<uint8_t>& GetBody() { return _body; }
        std::shared_ptr<Session> GetSession() { return _session; }

    private:
        friend class Network::HTTP::Router;

        Method _method;
        std::string _url;
        std::string _version;
        std::map<std::string, std::string> _headers;
        std::map<std::string, std::string> _cookies;

        std::vector<uint8_t> _body;
        std::shared_ptr<Session> _session;
    };

}
}
}

#endif