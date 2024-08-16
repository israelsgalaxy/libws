#ifndef LIB_WS
#define LIB_WS

#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <boost/system/error_code.hpp>
#include <functional>
#include <boost/asio/io_context.hpp>

class WebSocketClient {
public:
    WebSocketClient(boost::asio::io_context&, const std::string&, const std::string&, const std::string&);
    ~WebSocketClient() = default;

    void Connect(std::function<void(const boost::system::error_code&)> = nullptr, std::function<void(const boost::system::error_code&, std::string&&)> = nullptr, std::function<void(const boost::system::error_code&)> = nullptr);
    void Send(std::string&, std::function<void(const boost::system::error_code&)> = nullptr);
    void Close(std::function<void(const boost::system::error_code&)> = nullptr);

private:
    boost::asio::io_context& m_ioc;
    std::string m_host;
    std::string m_path;
    std::string m_port;

    boost::beast::websocket::stream<boost::beast::tcp_stream> m_ws;
    boost::asio::ip::tcp::resolver m_resolver;
    boost::beast::flat_buffer m_rBuffer {};
    boost::asio::const_buffer m_wBuffer {};

    std::function<void(const boost::system::error_code&)> m_onConnect;
    std::function<void(const boost::system::error_code&, std::string&&)> m_onMessage;
    std::function<void(const boost::system::error_code&)> m_onConnectErrOrDisconnect;

    void ReadRecursively();
};

#endif