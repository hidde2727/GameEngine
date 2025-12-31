#include "network/WebHandler.h"

namespace Engine {
namespace Network {
    
    WebHandler::WebHandler(Private) : _acceptor(_context), Router("/engine/CookieCache.bin", 0) {
    }

    void WebHandler::Start() {
        try {
            _localAddress = GetLocalAdress();
            LOG("[Network::WebHandler] Opening for web requests on 'http://" + _localAddress + ":8000'")
        } catch(std::exception exc) {
            WARNING("[Network::WebHandler] Failed to get the local adress, are you connected to the internet?")
            return;
        }
        asio::ip::tcp::resolver resolver(_context);
        asio::ip::tcp::endpoint endpoint(asio::ip::address_v4::any(), 8000);
        _acceptor.open(endpoint.protocol());
        _acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
        _acceptor.bind(endpoint);
        _acceptor.listen();

        _running = true;

        AwaitConnection();
        _thread = std::thread([&] { try { _context.run(); } catch (std::exception err) { THROW("[Network::Webhandler] Experienced an error:\n\t\t" + std::string(err.what())); return 1;} return 0; });
    }
    void WebHandler::Stop() {
        _running = false;
        
        for(const auto[uuid, connection] : _httpConnections) {
            connection->Stop();
        }
        _httpConnections.clear();
        for(const auto[uuid, connection] : _websocketConnections) {
            connection->Stop();
        }
        _websocketConnections.clear();
        
        _context.stop();
        if(_thread.joinable()) _thread.join();
    }
    
    void WebHandler::Update() {
        auto count = _requestHandler.poll();
        if(_requestHandler.stopped()) _requestHandler.restart();
    }

    void WebHandler::AwaitConnection() {
        if(ENGINE_NETWORK_VERBOSE_HTTP) LOG("[Network::WebHandler] Awaiting connections")
		_acceptor.async_accept(
            asio::make_strand(_context),
            [this](const std::error_code& ec, asio::ip::tcp::socket socket)
            {
                AwaitConnection();
                if (!ec) {
                    std::shared_ptr<HTTPConnection>newConnection = std::make_shared<HTTPConnection>(shared_from_this(), std::move(socket));
                    size_t uuid = reinterpret_cast<size_t>(newConnection.get());// Using the memory adress as the uuid (lifetime of the uuid and the unique_ptr are the same, thus the uuid is unique)
                    if(ENGINE_NETWORK_VERBOSE_HTTP) LOG("[Network::WebHandler] Starting a connection with id: '" + std::to_string(uuid) + "'")
                    newConnection->Start(uuid);
                    _httpConnections[uuid] = std::move(newConnection);
                    if(ENGINE_NETWORK_VERBOSE_HTTP) LOG("[Network::WebHandler] Started a connection with id: '" + std::to_string(uuid) + "'")
                }
            }
        );
        asio::detail::event event;
    }

    void WebHandler::StopHTTPConnection(const size_t uuid) {
        if(!_running) return;
        _httpConnections.erase(uuid);
    }
    void WebHandler::StopWebsocketConnection(const size_t uuid) {
        if(!_running) return;
        _websocketConnections.erase(uuid);
    }
    void WebHandler::UpgradeHTTPConnection(const size_t olduuid, asio::ip::tcp::socket&& socket, std::shared_ptr<HTTP::Request> request, std::shared_ptr<HTTP::Response> response) {
        _httpConnections.erase(olduuid);
        Util::WeirdPointer<Websocket::BasicHandler> handler = response->GetUserData();
        std::shared_ptr<WebsocketConnection> newConnection = std::make_shared<WebsocketConnection>(shared_from_this(), std::move(socket), handler);

        size_t uuid = reinterpret_cast<size_t>(newConnection.get());// Using the memory adress as the uuid (lifetime of the uuid and the unique_ptr are the same, thus the uuid is unique)
        newConnection->Start(uuid);
        _websocketConnections[uuid] = newConnection;
        if(ENGINE_NETWORK_VERBOSE_HTTP_WEBSOCKET) LOG("[Network::WebHandler] Upgraded connection (" + std::to_string(uuid) + ")")
        // Post work for the main thread
        asio::post(_requestHandler, [this, newConnection, request, handler]() {
            handler->OnWebsocketStart(*newConnection.get(), *request);
        });
    }

    std::string WebHandler::GetLocalAdress() {
        asio::ip::tcp::resolver resolver(_context);
        const auto query = resolver.resolve(asio::ip::host_name(), "");
        asio::ip::basic_resolver_iterator<asio::ip::tcp> iter = query.begin();
        asio::ip::basic_resolver_iterator<asio::ip::tcp> end = query.end();
        while (iter != end)
        {
            if (iter->endpoint().address().is_v4()
            && !iter->endpoint().address().is_loopback()
            && !iter->endpoint().address().is_multicast()
            && !iter->endpoint().address().is_unspecified())
            {
            return iter->endpoint().address().to_string();
            }

            ++iter;
        }
        return "";
    }

}
}