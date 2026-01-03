#include "network/HTTP/Response.h"

namespace Engine {
namespace Network {
namespace HTTP {

    void Response::SetHTTPVersion(const std::string version) {
        _version = version;
    }
    void Response::SetResponseCode(const ResponseCode code) {
        if     (code==ResponseCode::Continue)                       _code="100 Continue";
        else if(code==ResponseCode::SwitchingProtocols)             _code="101 Switching Protocols";
        else if(code==ResponseCode::Processing)                     _code="102 Processing";
        else if(code==ResponseCode::EarlyHints)                     _code="103 Early Hints";
        else if(code==ResponseCode::OK)                             _code="200 OK";
        else if(code==ResponseCode::Created)                        _code="201 Created";
        else if(code==ResponseCode::Accepted)                       _code="202 Accepted";
        else if(code==ResponseCode::NonAuthoritativeInformation)    _code="203 Non-Authoritative Information";
        else if(code==ResponseCode::NoContent)                      _code="204 No Content";
        else if(code==ResponseCode::ResetContent)                   _code="205 Reset Content";
        else if(code==ResponseCode::PartialContent)                 _code="206 Partial Content";
        else if(code==ResponseCode::MultiStatus)                    _code="207 Multi-Status";
        else if(code==ResponseCode::AlreadyReported)                _code="208 Already Reported";
        else if(code==ResponseCode::IMUsed)                         _code="226 IM Used";
        else if(code==ResponseCode::MultipleChoices)                _code="300 Multiple Choices";
        else if(code==ResponseCode::MovedPermanently)               _code="301 Moved Permanently";
        else if(code==ResponseCode::Found)                          _code="302 Found";
        else if(code==ResponseCode::SeeOther)                       _code="303 See Other ";
        else if(code==ResponseCode::NotModified)                    _code="304 Not Modified";
        else if(code==ResponseCode::UseProxy)                       _code="305 Use Proxy";
        else if(code==ResponseCode::SwitchProxy)                    _code="306 Switch Proxy";
        else if(code==ResponseCode::TemporaryRedirect)              _code="307 Temporary Redirect";
        else if(code==ResponseCode::PermanentRedirect)              _code="308 Permanent Redirect";
        else if(code==ResponseCode::BadRequest)                     _code="400 Bad Request";
        else if(code==ResponseCode::Unauthorized)                   _code="401 Unauthorized";
        else if(code==ResponseCode::PaymentRequired)                _code="402 Payment Required";
        else if(code==ResponseCode::Forbidden)                      _code="403 Forbidden";
        else if(code==ResponseCode::NotFound)                       _code="404 Not Found";
        else if(code==ResponseCode::MethodNotAllowed)               _code="405 Method Not Allowed";
        else if(code==ResponseCode::NotAcceptable)                  _code="406 Not Acceptable";
        else if(code==ResponseCode::ProxyAuthenticationRequired)    _code="407 Proxy Authentication Required";
        else if(code==ResponseCode::RequestTimeout)                 _code="408 Request Timeout";
        else if(code==ResponseCode::Conflict)                       _code="409 Conflict";
        else if(code==ResponseCode::Gone)                           _code="410 Gone";
        else if(code==ResponseCode::LengthRequired)                 _code="411 Length Required";
        else if(code==ResponseCode::PreconditionFailed)             _code="412 Precondition Failed";
        else if(code==ResponseCode::PayloadTooLarge)                _code="413 Payload Too Large";
        else if(code==ResponseCode::URITooLong)                     _code="414 URI Too Long";
        else if(code==ResponseCode::UnsupportedMediaType)           _code="415 Unsupported Media Type";
        else if(code==ResponseCode::RangeNotSatisfiable)            _code="416 Range Not Satisfiable";
        else if(code==ResponseCode::ExpectationFailed)              _code="417 Expectation Failed";
        else if(code==ResponseCode::IamATeapot)                     _code="418 I'm a teapot";
        else if(code==ResponseCode::MisdirectedRequest)             _code="421 Misdirected Request";
        else if(code==ResponseCode::UnprocessableContent)           _code="422 Unprocessable Content";
        else if(code==ResponseCode::Locked)                         _code="423 Locked";
        else if(code==ResponseCode::FailedDependency)               _code="424 Failed Dependency";
        else if(code==ResponseCode::TooEarly)                       _code="425 Too Early";
        else if(code==ResponseCode::UpgradeRequired)                _code="426 Upgrade Required";
        else if(code==ResponseCode::PreconditionRequired)           _code="428 Precondition Required";
        else if(code==ResponseCode::TooManyRequests)                _code="429 Too Many Requests";
        else if(code==ResponseCode::RequestHeaderFieldsTooLarge)    _code="431 Request Header Fields Too Large";
        else if(code==ResponseCode::UnavailableForLegalReasons)     _code="451 Unavailable For Legal Reasons";
    }
    void Response::SetHeader(const std::string name, const std::string value) {
        _headers[name] = value;
    }
    void Response::SetCookie(const std::string name, const Cookie cookie) {
#ifdef __DEBUG__
        // Check the name (no control values, seperator and no ( ) < > @ , ; : \ " / [ ] ? = { })
        // https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Headers/Set-Cookie
        for(const char c : name) {
            if((c>=0 && c<=31) || c==127 || c==' ' || c=='\t'
                || c=='(' || c==')' || c=='<' || c=='>' || c=='@' || 
                c==',' || c==';' || c==':' || c=='\\' || c=='"' || 
                c=='/' || c=='[' || c==']' || c=='?' || c=='{' || c=='}') {
                THROW("[HTTP::Response] Trying to set a cookie name with illegal character (" + std::string(&c, 1) + ")")
            }
        }
        // Check the value (no control value, whitespaces and no " , ; \")
        // https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Headers/Set-Cookie
        for(const char c : cookie._value) {
            if((c>=0 && c<=31) || c==127 || c==' ' || c=='\t' || c=='\n'
                || c=='"' || c==',' || c==';' || c=='\\') {
                THROW("[HTTP::Response] Trying to set a cookie value with illegal characters (" + std::string(&c, 1) + ")")
            }
        }
        // Check the domain (can only contain a-z, A-z, 0-9 and -)
        // https://en.wikipedia.org/wiki/Hostname#Syntax
        int currentLabelLength = 0;
        for(const char c : cookie._domain) {
            if(!((c>='a' && c<='z')||(c>='A' && c<='Z')||(c>='0' && c<='9') || c=='-')) {
                THROW("[HTTP::Response] Trying to set a cookie value with illegal characters (" + std::string(&c, 1) + ")")
            }
            if(c == '.') currentLabelLength = 0;
            else currentLabelLength++;
            ASSERT(currentLabelLength <= 63, "[HTTP::Response] A domain cannot contain labels longer than 63 characters (sections between . cannot be longer than 63 characters)")
        }
        ASSERT(cookie._domain.size() < 255, "[HTTP::Response] A domain cannot be longer than 255 characters")
        // Check the path (no control values, ? and #)
        // https://developer.mozilla.org/en-US/docs/Web/URI/Reference/Path
        for(const char c : cookie._path) {
            if((c>=0 && c<=31) || c==127 || c=='?' || c=='#') {
                THROW("[HTTP::Response] Trying to set a cookie value with illegal characters (" + std::string(&c, 1) + ")")
            }
        }
#endif
        _cookies[name] = cookie;
    }
    void Response::SetCookie(const std::string name, const std::string value) {
        Cookie cookie{};
        cookie._value = value;
        SetCookie(name, cookie);
    }
    bool Response::SetBodyToFile(const std::string fileName, const bool ifNotFound404) {
        try {
            ASSERT(fileName.find("..")==std::string::npos, "[HTTP::Response::SetBodyToFile] Received a file with .., this is unsafe behavior")
            Util::FileManager::Get(fileName).Read(_body);
        }
        catch (std::exception ex) {
            if(!ifNotFound404) THROW("[HTTP::Response::SetBodyToFile] Failed to load file '" + fileName + "':\n" + std::string(ex.what()))
            SetResponseCode(ResponseCode::NotFound);
            _body.clear();
            return false;
        }
        // Set content type
        size_t lastDot = fileName.find_last_of('.');
		std::string extension = fileName.substr(lastDot, fileName.size() - lastDot);
		if      (extension.compare(".htm")  == 0) SetHeader("Content-Type", "text/html");
		else if (extension.compare(".html") == 0) SetHeader("Content-Type", "text/html");
		else if (extension.compare(".php")  == 0) SetHeader("Content-Type", "text/html");
		else if (extension.compare(".css")  == 0) SetHeader("Content-Type", "text/css");
		else if (extension.compare(".txt")  == 0) SetHeader("Content-Type", "text/plain");
		else if (extension.compare(".js")   == 0) SetHeader("Content-Type", "application/javascript");
		else if (extension.compare(".json") == 0) SetHeader("Content-Type", "application/json");
		else if (extension.compare(".xml")  == 0) SetHeader("Content-Type", "application/xml");
		else if (extension.compare(".pdf")  == 0) SetHeader("Content-Type", "application/pdf");
		else if (extension.compare(".zip")  == 0) SetHeader("Content-Type", "application/zip");
		else if (extension.compare(".png")  == 0) SetHeader("Content-Type", "image/png");
		else if (extension.compare(".jpe")  == 0) SetHeader("Content-Type", "image/jpeg");
		else if (extension.compare(".jpeg") == 0) SetHeader("Content-Type", "image/jpeg");
		else if (extension.compare(".jpg")  == 0) SetHeader("Content-Type", "image/jpeg");
		else if (extension.compare(".gif")  == 0) SetHeader("Content-Type", "image/gif");
		else if (extension.compare(".bmp")  == 0) SetHeader("Content-Type", "image/bmp");
		else if (extension.compare(".ico")  == 0) SetHeader("Content-Type", "image/vnd.microsoft.icon");
		else if (extension.compare(".tiff") == 0) SetHeader("Content-Type", "image/tiff");
		else if (extension.compare(".tif")  == 0) SetHeader("Content-Type", "image/tiff");
		else if (extension.compare(".svg")  == 0) SetHeader("Content-Type", "image/svg+xml");
		else if (extension.compare(".svgz") == 0) SetHeader("Content-Type", "image/svg+xml");
		else if (extension.compare(".mp4")  == 0) SetHeader("Content-Type", "video/mp4");
		else if (extension.compare(".mpeg") == 0) SetHeader("Content-Type", "video/mpeg");
		else if (extension.compare(".webm") == 0) SetHeader("Content-Type", "video/webm");
		else if (extension.compare(".wav")  == 0) SetHeader("Content-Type", "video/wav");
		else                                      SetHeader("Content-Type", "application/text");
        return true;
    }
    void Response::SetBodyToString(const std::string str) {
        _body.reserve(str.size());
        _body.resize(str.size());
        memcpy(_body.data(), str.data(), str.size());
    }
    
    
    void Response::SetUserData(Util::WeirdPointer<void> data) {
        _userData = data;
    }
    Util::WeirdPointer<void> Response::GetUserData() {
        return _userData;
    }

    
    bool Response::IsWebsocketUpgrade() {
        return _code == "101 Switching Protocols" && _headers["Connection"] == "Upgrade" && _headers["Upgrade"] == "websocket";
    }
    std::string Response::GetHeader(const std::string name) {
        return _headers[name];
    }

    std::vector<asio::const_buffer> Response::ToBuffers() {
        ConstructHead();
		std::vector<asio::const_buffer> buffers;
		buffers.push_back(asio::buffer(_constructedHead));
		if (_body.size()) buffers.push_back(asio::buffer(_body));
		return buffers;
    }    
    void Response::ConstructHead() {
        if(_body.size() > 0) SetHeader("Content-Length", std::to_string(_body.size()));
        
        _constructedHead = _version + " " + _code + "\r\n";
		for (auto const& [name, value] : _headers){
			_constructedHead += name + ": " + value + "\r\n";
		}
		for (auto const& [name, cookie] : _cookies) {
			_constructedHead += "Set-Cookie: " + name + "=" + cookie._value;

            if(cookie._domain.size()) _constructedHead += "; Domain=" + cookie._domain;
            if(cookie._path.size()) _constructedHead += "; Path=" + cookie._path;
            if(cookie._maxAge != UINT32_MAX) _constructedHead += "; Max-Age=" + std::to_string(cookie._maxAge);
            if(cookie._sameSite == SameSite::None) _constructedHead += "; SameSite=None";
            else if(cookie._sameSite == SameSite::Lax) _constructedHead += "; SameSite=Lax";
            else if(cookie._sameSite == SameSite::Strict) _constructedHead += "; SameSite=Strict";
            if(cookie._secure) _constructedHead += "; Secure";
            if(cookie._httpOnly) _constructedHead += "; HttpOnly";
            if(cookie._secure && cookie._partitioned) _constructedHead += "; Partitioned";
            
            _constructedHead += "\r\n";
		}
		_constructedHead += "\r\n";
    }

}
}
}