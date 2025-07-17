#include "WebsocketServer.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>


using namespace Poco::Net;

// 前向声明，供工厂类使用
class WebSocketRequestHandler;
class WebSocketRequestHandlerFactory;

#ifndef WEBSOCKET_HANDLER_FACTORY_DEFINED
#define WEBSOCKET_HANDLER_FACTORY_DEFINED

// 提前完整定义
class WebSocketRequestHandler : public HTTPRequestHandler {
public:
    explicit WebSocketRequestHandler(WebsocketServer &server) : m_serverRef(server) {}
    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override;
private:
    WebsocketServer &m_serverRef;
};

class WebSocketRequestHandlerFactory : public HTTPRequestHandlerFactory {
public:
    explicit WebSocketRequestHandlerFactory(WebsocketServer &server) : m_serverRef(server) {}
    HTTPRequestHandler *createRequestHandler(const HTTPServerRequest &request) override;
private:
    WebsocketServer &m_serverRef;
};
#endif // WEBSOCKET_HANDLER_FACTORY_DEFINED



WebsocketServer::WebsocketServer(int port)
    : m_port(port),                // 初始化端口
      m_running(false),            // 原子变量初始化为false
      m_connected(false),          // 原子变量初始化为false
      m_serverSocket(nullptr),     // 初始化为nullptr
      m_httpServer(nullptr),       // 初始化为nullptr
      m_webSocket(nullptr),        // 初始化为nullptr
      m_onConnectedCallback(nullptr),      // 回调初始化为空
      m_onDisconnectedCallback(nullptr),   // 回调初始化为空
      m_onMessageCallback(nullptr),        // 回调初始化为空
      m_onErrorCallback(nullptr)           // 回调初始化为空
{
    try 
    {
        // 初始化ServerSocket实例
        m_serverSocket = std::make_unique<ServerSocket>(port);
        
    } catch (const Poco::Exception& e) {
        // 捕获Poco异常并重新抛出
        throw std::runtime_error("Failed to create server socket: " + e.displayText());
    } catch (...) {
        // 捕获未知异常
        throw std::runtime_error("Unknown error occurred while creating server socket");
    }
}

void WebsocketServer::startListening(bool block) {
    try {
        m_httpServer = std::make_unique<HTTPServer>(
            new WebSocketRequestHandlerFactory(*this),
            *m_serverSocket,
            new HTTPServerParams);
        m_httpServer->start();
        m_running = true;

        // 若请求阻塞，则循环等待直到第一次连接建立或服务器被停止
        std::cout << "等待连接中" << std::endl;
        if (block) {
            while (m_running && !m_connected) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            std::cout << std::endl; // 连接建立后换行
        }
    } catch (const Poco::Exception &e) {
        throw std::runtime_error("Failed to start HTTP server: " + e.displayText());
    } catch (...) {
        throw std::runtime_error("Unknown error occurred while starting HTTP server");
    }
}

void WebsocketServer::stopListening() {
    try 
    {
        m_httpServer->stop();
        m_running = false;
    } catch (const Poco::Exception& e) {
        throw std::runtime_error("Failed to stop HTTP server: " + e.displayText());
    } catch (...) {
        throw std::runtime_error("Unknown error occurred while stopping HTTP server");
    }
}

WebsocketServer::~WebsocketServer() {
    stopListening();
    std::cout << "WebsocketServer destroyed" << std::endl;
}

bool WebsocketServer::isRunning() const {
    return m_running;
}

bool WebsocketServer::isListening() const {
    return m_listening;
}

bool WebsocketServer::isConnected() const {
    return m_connected;
}

// 每次调用时开启一个短生命周期的发送线程
void WebsocketServer::sendMessage(const std::vector<uint8_t>& message)
{
    // 捕获 by value message，by pointer this
    std::thread([this, message]() {
        std::lock_guard<std::mutex> lock(m_wsMutex);
        if (!m_connected || m_webSocket == nullptr)
        {
            std::cerr << "[WebsocketServer] No active WebSocket connection, cannot send." << std::endl;
            return;
        }

        try
        {
            // std::cout << "send frame..." << std::endl;
            m_webSocket->sendFrame(message.data(), static_cast<int>(message.size()), WebSocket::FRAME_BINARY);
        }
        catch (const Poco::Exception &e)
        {
            if (m_onErrorCallback) m_onErrorCallback(e.displayText());
        }
    }).detach();
}

// 文本消息重载：将字符串转换为字节数组后复用二进制发送逻辑
void WebsocketServer::sendMessage(const std::string &message)
{
    std::vector<uint8_t> data(message.begin(), message.end());
    sendMessage(data);
}

// 回调注册实现
void WebsocketServer::setOnConnectedCallback(const ConnectionCallback& cb)
{
    m_onConnectedCallback = cb;
}

void WebsocketServer::setOnDisconnectedCallback(const ConnectionCallback& cb)
{
    m_onDisconnectedCallback = cb;
}

void WebsocketServer::setOnMessageCallback(const MessageCallback& cb)
{
    m_onMessageCallback = cb;
}

void WebsocketServer::setOnErrorCallback(const ErrorCallback& cb)
{
    m_onErrorCallback = cb;
}


HTTPRequestHandler *WebSocketRequestHandlerFactory::createRequestHandler(const HTTPServerRequest &request) {
    try {
        // 收到连接后直接创建 handler 实例，额外逻辑留空
        return new WebSocketRequestHandler(m_serverRef);
    } catch (const Poco::Exception &e) {
        std::cerr << "Error in createRequestHandler: " << e.displayText() << std::endl;
        return nullptr;
    }
}

void WebSocketRequestHandler::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) {
    try {
        WebSocket ws(request, response);

        // 记录活动连接并触发连接建立回调
        {
            std::lock_guard<std::mutex> lock(m_serverRef.m_wsMutex);
            m_serverRef.m_webSocket = &ws;
        }
        m_serverRef.m_connected = true;
        if (m_serverRef.m_onConnectedCallback) {
            m_serverRef.m_onConnectedCallback();
        }

        std::cout << "WebSocket connection established." << std::endl;
        char buffer[1024];
        int flags;
        int n;
        do {
            {
                std::lock_guard<std::mutex> lock(m_serverRef.m_wsMutex);
                n = ws.receiveFrame(buffer, sizeof(buffer), flags);
            }
            std::cout << "Frame received (length=" << n << ", flags=0x" << std::hex << unsigned(flags) << ")." << std::endl;

            // 如果是 Close 帧，额外输出提示
            int opcode = flags & WebSocket::FRAME_OP_BITMASK;
            if (opcode == WebSocket::FRAME_OP_CLOSE) {
                std::cout << "收到 Close 帧 (客户端请求关闭连接)" << std::endl;
            }
            // 调用消息回调
            if (n > 0 && m_serverRef.m_onMessageCallback) {
                std::vector<uint8_t> msg(buffer, buffer + n);
                m_serverRef.m_onMessageCallback(msg);
            }
        } while (n > 0 || (flags & WebSocket::FRAME_OP_BITMASK) != WebSocket::FRAME_OP_CLOSE);
        std::cout << "WebSocket connection closed." << std::endl;

        // 连接关闭，触发断开回调并清理
        {
            std::lock_guard<std::mutex> lock(m_serverRef.m_wsMutex);
            m_serverRef.m_connected = false;
            m_serverRef.m_webSocket = nullptr;
        }
        if (m_serverRef.m_onDisconnectedCallback) {
            m_serverRef.m_onDisconnectedCallback();
        }
    } catch (Poco::Net::WebSocketException &exc) {
        std::string errorMsg = exc.displayText();

        // 触发用户注册的错误回调
        if (m_serverRef.m_onErrorCallback) {
            m_serverRef.m_onErrorCallback(errorMsg);
        }

        // 针对握手失败的情况，返回 HTTP 错误状态
        switch (exc.code()) {
            case WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
                response.set("Sec-WebSocket-Version", WebSocket::WEBSOCKET_VERSION);
                [[fallthrough]];
            case WebSocket::WS_ERR_NO_HANDSHAKE:
            case WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
            case WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
                response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST);
                response.setContentLength(0);
                response.send();
                break;
            default:
                break;
        }

        // 出现异常同样视为连接断开，更新状态并停止发送线程
        {
            std::lock_guard<std::mutex> lock(m_serverRef.m_wsMutex);
            m_serverRef.m_connected = false;
            m_serverRef.m_webSocket = nullptr;
        }
        if (m_serverRef.m_onDisconnectedCallback) {
            m_serverRef.m_onDisconnectedCallback();
        }
    }
}






