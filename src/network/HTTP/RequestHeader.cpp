#include "network/HTTP/RequestHeader.h"

namespace Engine {
namespace Network {
namespace HTTP {

    Method StringToMethod(const std::string method) {
		if		(method == "GET")		{ return Method::Get; }
		else if (method == "HEAD")		{ return Method::Head; }
		else if (method == "POST")		{ return Method::Post; }
		else if (method == "PUT")		{ return Method::Put; }
		else if (method == "DELETE")	{ return Method::Delete; }
		else if (method == "CONNECT")	{ return Method::Connect; }
		else if (method == "OPTIONS")	{ return Method::Options; }
		else if (method == "TRACE")		{ return Method::Trace; }
		else if (method == "PATCH")		{ return Method::Patch; }
        return Method::None;
	}

    void RequestHeader::Parse(std::istream& stream, const size_t size) {
        {// Head
            std::string head; std::getline(stream, head);
            const size_t firstSpace = head.find_first_of(' ');
            _method = StringToMethod(head.substr(0, firstSpace));
            ASSERT(_method==Method::None, "Received a HTTP request with an illegal method")
            const size_t secondSpace = head.find_first_of(' ', firstSpace+1);
            _url = head.substr(firstSpace+1, secondSpace-firstSpace-1);
            _version = head.substr(secondSpace+1, head.size()-secondSpace-1);
        }
        // Headers
        std::string header; 
        while(stream.peek() != '\r' && !stream.eof()) {
            std::getline(stream, header);
            const size_t spacerPos = header.find(": ");
            _headers[header.substr(0, spacerPos)] = header.substr(spacerPos+2, header.size()-spacerPos-3);
        }
        // Cookies
        if(_headers["Cookie"].size() == 0) return;
        std::stringstream ss(_headers["Cookie"]);
        std::string cookieName;
        std::string cookieVal;
        while(!ss.eof()) {
            std::getline(ss, cookieName, '=');
            std::getline(ss, cookieVal, ';');
            _cookies[cookieName] = cookieVal;
        }
    }

}
}
}