#include "websocket-client.h"
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/system/error_code.hpp>
#include <cstddef>
#include <functional>
#include <string>
#include <iostream>


WebSocketClient::WebSocketClient(boost::asio::io_context& ioc, const std::string& host, const std::string& path, const std::string& port) : 
m_ioc{ioc}, m_host{host}, m_path{path}, m_port{port}, m_ws{m_ioc}, m_resolver{m_ioc} {
    
}

void WebSocketClient::Connect(std::function<void (const boost::system::error_code&)> onConnect, std::function<void (const boost::system::error_code&, std::string&&)> onMessage, std::function<void (const boost::system::error_code&)> onConnectErrOrDisconnect) {
    m_onConnect = onConnect;
    m_onMessage = onMessage;
    m_onConnectErrOrDisconnect = onConnectErrOrDisconnect;

    // first resolve the host and port
    m_resolver.async_resolve(m_host, m_port, [this] (boost::system::error_code ec, boost::asio::ip::tcp::resolver::results_type resolverResults) {
        if (ec) {
            std::cout << "Could not resolve host and port" << std::endl;
            if (m_onConnectErrOrDisconnect)
                m_onConnectErrOrDisconnect(ec);
            return;
        }

        std::cout << "Resolved host and port" << std::endl;

        // connect tcp socket to endpoint
        m_ws.next_layer().async_connect(*resolverResults, [this] (boost::system::error_code ec) {
            if (ec) {
                std::cout << "Could not connect to endpoint" << std::endl;
                if (m_onConnectErrOrDisconnect)
                    m_onConnectErrOrDisconnect(ec);
                return;
            }

            std::cout << "Connected to endpoint" << std::endl;

            // perform ws handshake
            m_ws.async_handshake(m_host, m_path, [this] (boost::system::error_code ec) {
                if (ec) {
                    std::cout << "Could not perform ws handshake" << std::endl;
                    if (m_onConnectErrOrDisconnect)
                        m_onConnectErrOrDisconnect(ec);
                    return;
                }

                std::cout << "Performed ws handshake" << std::endl;

                // start recursively listening for messages
                ReadRecursively();

                if (m_onConnect)
                    m_onConnect(ec);
            });
        });
    });
}

void WebSocketClient::ReadRecursively() {
    m_ws.async_read(m_rBuffer, [this] (boost::system::error_code ec, size_t bytesRead) {
        if (ec == boost::asio::error::operation_aborted) {
            // this ec is for a closed connection, so we stop recursing
            return;
        }

        if (ec) {
            std::cout << "Error reading buffer" << std::endl;
            if (m_onConnectErrOrDisconnect)
                m_onConnectErrOrDisconnect(ec);
        } else {
            std::cout << "Reading buffer" << std::endl;

            if (m_onMessage)
                m_onMessage(ec, std::move(boost::beast::buffers_to_string(m_rBuffer.data())));

            m_rBuffer.consume(bytesRead);
        }

        ReadRecursively();
    });
}

// we dont want msg to be a temporary hence not const& or &&
void WebSocketClient::Send(std::string& msg, std::function<void(const boost::system::error_code&)> onSend) {
    m_wBuffer = boost::asio::const_buffer {msg.c_str(), msg.size()};

    m_ws.async_write(m_wBuffer, [this, onSend] (boost::system::error_code ec, size_t bytesRead) {
        if (ec) {
            std::cout << "Error sending message" << std::endl;
            if (m_onConnectErrOrDisconnect)
                m_onConnectErrOrDisconnect(ec);
            return;
        }

        std::cout << "Sent message" << std::endl;

        if (onSend)
            onSend(ec);
    });
}

void WebSocketClient::Close(std::function<void(const boost::system::error_code&)> onClose) {
    m_ws.async_close(boost::beast::websocket::close_code::none, [this, onClose] (boost::system::error_code ec) {
        if (ec) {
            std::cout << "Error closing connection" << std::endl;
            if (m_onConnectErrOrDisconnect)
                m_onConnectErrOrDisconnect(ec);
            return;
        }

        std::cout << "Closed connection" << std::endl;

        if (onClose)
            onClose(ec);
    });
}