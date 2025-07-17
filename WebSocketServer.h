#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/WebSocket.h>
#include <Poco/Net/NetException.h>
#include <functional>
#include <memory>
#include <vector>
#include <string>
#include <atomic>
#include <mutex>


using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServer;
using Poco::Net::ServerSocket;
using Poco::Net::WebSocket;
using std::vector;
using std::string;

class WebsocketServer{
    public:
        /**
         * @brief 构造函数
         * @param port 监听端口号
         */
        WebsocketServer(int port);
        ~WebsocketServer();

        bool isRunning() const;
        
        /**
         * 开始监听
         * @param block  阻塞当前线程，直到第一次 WebSocket 连接建立
         */
        void startListening(bool block = true);
        void stopListening();

        bool isListening() const;

        bool isConnected() const;

        // 发送二进制数据
        void sendMessage(const std::vector<uint8_t>& message);

        // 发送文本数据（自动转换为字节序列）
        void sendMessage(const std::string& message);

        // 注册回调函数
        using ConnectionCallback = std::function<void()>;  // 连接建立/断开回调
        using MessageCallback = std::function<void(const std::vector<uint8_t>&)>;  // 数据接收回调
        using ErrorCallback = std::function<void(const std::string&)>;  // 错误处理回调

        void setOnConnectedCallback(const ConnectionCallback& callback);
        void setOnDisconnectedCallback(const ConnectionCallback& callback);
        void setOnMessageCallback(const MessageCallback& callback);
        void setOnErrorCallback(const ErrorCallback& callback);

    // 声明友元，方便访问私有成员
    friend class WebSocketRequestHandler;
    friend class WebSocketRequestHandlerFactory;

    private:
        // 成员变量
        int m_port;
        std::atomic<bool> m_running = false;        // 服务器运行状态标志
        std::atomic<bool> m_connected = false;      // 当前连接状态标志
        std::atomic<bool> m_listening = false;      // 监听状态标志

        std::unique_ptr<Poco::Net::ServerSocket> m_serverSocket;
        std::unique_ptr<Poco::Net::HTTPServer> m_httpServer;
        Poco::Net::WebSocket* m_webSocket = nullptr;  // 当前活动连接

        ConnectionCallback m_onConnectedCallback;
        ConnectionCallback m_onDisconnectedCallback;
        MessageCallback m_onMessageCallback;
        ErrorCallback m_onErrorCallback;

        // 保护 WebSocket 对象的互斥量
        std::mutex m_wsMutex;

};










#endif // WEBSOCKET_SERVER_H