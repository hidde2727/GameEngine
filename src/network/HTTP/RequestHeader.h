#ifndef ENGINE_NETWORK_HTTP_REQUESTHEADER_H
#define ENGINE_NETWORK_HTTP_REQUESTHEADER_H

#include "core/PCH.h"

namespace Engine {
namespace Network {
namespace HTTP {
    
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

    class RequestHeader {
    public:
        void Parse(std::istream& stream, const size_t size);

        Method GetMethod() const { return _method; }
        std::string GetURL() const { return _url; }
        std::string GetVersion() const { return _version; }
        std::string GetHeader(const std::string name) { return _headers[name]; }
        std::string GetCookie(const std::string name) { return _cookies[name]; }
        std::string GetHead() const { return MethodToString(_method) + " " + _url + " " + _version; }

    private:
        Method _method;
        std::string _url;
        std::string _version;
        std::map<std::string, std::string> _headers;
        std::map<std::string, std::string> _cookies;
    };

}
}
}

#endif