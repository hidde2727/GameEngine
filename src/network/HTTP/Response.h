#ifndef ENGINE_NETWORK_HTTP_RESPONSE_H
#define ENGINE_NETWORK_HTTP_RESPONSE_H

#include "core/PCH.h"

namespace Engine {
namespace Network {
namespace HTTP {

    enum class ResponseCode {
        Continue= 100,
        SwitchingProtocols= 101,
        Processing= 102,
        EarlyHints= 103,
        OK= 200,
        Created= 201,
        Accepted= 202,
        NonAuthoritativeInformation= 203,
        NoContent= 204,
        ResetContent= 205,
        PartialContent= 206,
        MultiStatus= 207,
        AlreadyReported= 208,
        IMUsed= 226,
        MultipleChoices= 300,
        MovedPermanently= 301,
        Found= 302,
        SeeOther= 303,
        NotModified= 304,
        UseProxy= 305,
        SwitchProxy= 306,
        TemporaryRedirect= 307,
        PermanentRedirect= 308,
        BadRequest= 400,
        Unauthorized= 401,
        PaymentRequired= 402,
        Forbidden= 403,
        NotFound= 404,
        MethodNotAllowed= 405,
        NotAcceptable= 406,
        ProxyAuthenticationRequired= 407,
        RequestTimeout= 408,
        Conflict= 409,
        Gone= 410,
        LengthRequired= 411,
        PreconditionFailed= 412,
        PayloadTooLarge= 413,
        URITooLong= 414,
        UnsupportedMediaType= 415,
        RangeNotSatisfiable= 416,
        ExpectationFailed= 417,
        IamATeapot= 418,
        MisdirectedRequest= 421,
        UnprocessableContent= 422,
        Locked= 423,
        FailedDependency= 424,
        TooEarly= 425,
        UpgradeRequired= 426,
        PreconditionRequired= 428,
        TooManyRequests= 429,
        RequestHeaderFieldsTooLarge= 431,
        UnavailableForLegalReasons= 451,
    };

    class Response {
    public:

        void SetHTTPVersion(const std::string version);
        void SetResponseCode(const ResponseCode code);
        void SetHeader(const std::string name, const std::string value);
        void SetCookie(const std::string name, const std::string value);

        // Also sets the Content-Type header
        // Sets the response code to 404 if the file is not found (only when ifNotFound404==true, if it equals false it will throw an exception)
        bool SetBodyToFile(const std::string fileName, const bool ifNotFound404=true);
        void SetBodyToString(const std::string str);

        // Keep this object alive until you are done using the buffers
		std::vector<asio::const_buffer> ToBuffers();

    private:
        std::string _version = "HTTP/1.1";
        std::string _code = "200 OK";
        std::map<std::string, std::string> _headers;
        std::map<std::string, std::string> _cookies;
        std::vector<char> _body;

        std::string _constructedHead;

        void ConstructHead();
    };

}
}
}

#endif