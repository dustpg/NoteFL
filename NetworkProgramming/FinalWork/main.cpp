#define _CRT_SECURE_NO_WARNINGS
#include <Ws2tcpip.h>
#include <Windows.h>
#include <ShellScalingApi.h>
#include <Shlwapi.h>
#include <cstdio>
#include <cassert>
#include <thread>
#include <vector>
#include <mutex>

// 帮助信息
static const char* HELP_MESSAGE = "[HELP]\r\n";
// IP地址
static const char* IP_ADDRESS = "210.41.229.50";
// 端口
static constexpr int IP_PORT = 6000;
// 发送大小
static constexpr int SEND_MSG_LENGTH = 50;
// 接收大小
static constexpr int RECV_MSG_LENGTH = 500;
// 服务器消息
static constexpr int SERVER_MESSAGE = 101;
// 客户端消息
static constexpr int CLIENT_MESSAGE = 102;
// 消息等待超时时间 ms
static constexpr int MESSAGE_SEND_TIMEOUT = 2000;
// 消息
struct SENDMSG {
    char        fisrt;
    char        buffer[SEND_MSG_LENGTH - 1];
};
static_assert(sizeof(SENDMSG) == SEND_MSG_LENGTH, "NO!");
// 消息
struct RECVMSG {
    char        fisrt;
    char        buffer[RECV_MSG_LENGTH - 1];
};
static_assert(sizeof(RECVMSG) == RECV_MSG_LENGTH, "NO!");

// 全局互斥锁
std::mutex g_mutex;
// 数据信息
struct MsgData {
    // 地址信息
    sockaddr_in     addr;
    // IP
    char            ip[32];
    // port
    int32_t         port;
    // 名称
    char            name[SEND_MSG_LENGTH];
};
// 全局数据
std::vector<MsgData> g_data, g_databackup;

auto save_message(const char* str) {
    auto file = ::fopen("message.txt", "wb+");
    ::fprintf(file, "%s\r\n", str);
    ::fclose(file);
}

// 应用程序入口
int main(int argc, char* argv[]) {
    g_data.reserve(16);
    // 显示帮助
    ::printf("输入help打印命令帮助\r\n");
    SENDMSG sendmsg;
    sendmsg.fisrt = SERVER_MESSAGE; ::strcpy_s(sendmsg.buffer, "Dust Loong");
    // 初始化SOCKET
    WSAData sender_data; 
    ::WSAStartup(MAKEWORD(2, 2), &sender_data);
    SOCKET sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockaddr_in udp_addr;
    udp_addr.sin_family = AF_INET;
    ::inet_pton(AF_INET, IP_ADDRESS, &udp_addr.sin_addr.s_addr);
    udp_addr.sin_port = ::htons(IP_PORT);
    //
    if (sock) {
        // 绑定
        {
            sockaddr_in sockaddrs;
            ::ZeroMemory(&sockaddrs, sizeof(sockaddrs));
            sockaddrs.sin_family = AF_INET;
            sockaddrs.sin_addr.s_addr = ::htonl(INADDR_ANY);
            sockaddrs.sin_port = ::htons(argc == 1 ? 5000 : 5001);
            auto status = ::bind(sock, reinterpret_cast<sockaddr*>(&sockaddrs), sizeof(sockaddrs));
            status = 0;
        }
        // 在线线程
        std::thread onlinethread([&]() {
            constexpr uint32_t TIMESLEEP = 1000;
            while (true) {
                {
                    std::lock_guard<decltype(g_mutex)> locker(g_mutex);
                    // 发送在线消息
                    auto status = ::sendto(
                        sock, &sendmsg.fisrt, SEND_MSG_LENGTH, 0,
                        reinterpret_cast<sockaddr*>(&udp_addr),
                        sizeof(udp_addr)
                        );
                    if (status == -1) break;
                }
                ::Sleep(TIMESLEEP);
            }
        });
        // 接收线程
        std::thread recvthread(
            [sock, &sendmsg]() {
                while (true) {
                    sockaddr_in target_addr; //::memcpy(&target_addr, &udp_addr, sizeof(udp_addr));
                    ::ZeroMemory(&target_addr, sizeof(target_addr));
                    // 接收消息
                    RECVMSG msg;
                    int addrlen = sizeof(target_addr);
                    auto status = ::recvfrom(
                        sock, &msg.fisrt, RECV_MSG_LENGTH, 0,
                        reinterpret_cast<sockaddr*>(&target_addr),
                        &addrlen
                        );
                    // 异常退出
                    if (status == -1) break;
                    // 服务器消息?
                    if (msg.fisrt == SERVER_MESSAGE) {
                        // 保存到文本文件
                        {
                            auto file = ::fopen("saved.txt", "wb");
                            if (file) {
                                ::fwrite(msg.buffer, 1, ::strlen(msg.buffer) + 1, file);
                                ::fclose(file);
                            }
                        }
                        std::lock_guard<decltype(g_mutex)> locker(g_mutex);
                        // 分析消息
                        //::printf("SERVER_MESSAGE: %s", msg.buffer);
                        /*// 在线
                        ::sendto(
                            sock, &sendmsg.fisrt, SEND_MSG_LENGTH, 0,
                            reinterpret_cast<sockaddr*>(&target_addr),
                            sizeof(target_addr)
                            );*/
                        // 分析消息
                        g_data.clear();
                        char* src_token = msg.buffer;
                        while (*src_token) {
                            MsgData data;
                            data.addr.sin_family = AF_INET;
                            // sscanf
                            // 解析IP地址
                            {
                                auto end = ::strchr(src_token, ':'); *end = 0;
                                ::strcpy_s(data.ip, src_token);
                                ::inet_pton(AF_INET, src_token, &data.addr.sin_addr.s_addr);
                                src_token = end + 1;
                            }
                            // 解析端口
                            {
                                auto end = ::strchr(src_token, ' '); *end = 0;
                                data.port = ::atoi(src_token);
                                data.addr.sin_port = ::htons(data.port);
                                src_token = end + 1;
                            }
                            // 解析名称
                            {
                                auto length = ::strstr(src_token, "\r\n") - src_token;
                                ::memcpy(data.name, src_token, length); data.name[length] = 0;
                                src_token += length + 2;
                            }
                            /*{
                                // 绑定
                                static bool first = true;
                                if (first && !::strcmp(data.name, sendmsg.buffer)) {
                                    //
                                    auto status = ::bind(sock, reinterpret_cast<sockaddr*>(&data.addr), sizeof(data.addr));
                                    auto ddd = ::WSAGetLastError();
                                    first = false;
                                }
                            }*/
                            // 新加入
                            g_data.push_back(data);
                        }
                    }
                    // 客户端消息?
                    else if (msg.fisrt == CLIENT_MESSAGE) {
                        ::printf("CLIENT_MESSAGE: %s\r\n", msg.buffer);
                        // 分析消息
                        std::lock_guard<decltype(g_mutex)> locker(g_mutex);
                        sockaddr_in client_addr;ZeroMemory(&client_addr, sizeof(client_addr));
                        client_addr.sin_family = AF_INET;
                        // 发送消息
                        auto end = ::strchr(msg.buffer, ':'); *end = 0;
                        ::inet_pton(AF_INET, msg.buffer, &client_addr.sin_addr.s_addr);
                        client_addr.sin_port = ::htons(::atoi(end + 1));
                        // 发送打洞
                        ::sendto(sock, "CCC", 4, 0, reinterpret_cast<sockaddr*>(&client_addr), sizeof(client_addr));
                    }
                    // 未知消息
                    else {
                        if (!::strncmp(&msg.fisrt, "CCC", 4)) {

                        }
                        else {
                            // 显示消息
                            printf("收到消息: %s\n", &msg.fisrt);
                            save_message(&msg.fisrt);
                        }
                    }
                }
            }
            );
        {
            // 输入
            while (true) {
                char buffer[RECV_MSG_LENGTH];
                // 检查命令
                ::fgets(buffer, RECV_MSG_LENGTH, stdin);
                // 退出
                if (!::strncmp(buffer, "exit", 4)) {
                    break;
                }
                // 帮助
                else if(!::strncmp(buffer, "help", 4)) {
                    std::lock_guard<decltype(g_mutex)> locker(g_mutex);
                    ::printf(HELP_MESSAGE);
                }
                // 列表
                else if (!::strncmp(buffer, "list", 4)) {
                    {
                        std::lock_guard<decltype(g_mutex)> locker(g_mutex);
                        // 复制
                        g_databackup = g_data;
                    }
                    // 显示
                    int index = 1;
                    for (auto& data : g_databackup) {
                        ::printf("编号: %2d -- IP: %16s:%5d 名称: %s\r\n", index, data.ip, int(data.port), data.name);
                        ++index;
                    }
                }
                // 修改名字
                else if (!::strncmp(buffer, "rename ", 7)) {
                    std::lock_guard<decltype(g_mutex)> locker(g_mutex);
                    ::strcpy_s(sendmsg.buffer, buffer + 7);
                    // 去除\n
                    sendmsg.buffer[::strlen(sendmsg.buffer)-1] = 0;
                }
                // 发送消息
                else if (!::strncmp(buffer, "send ", 5)) {
                    int index = 0;
                    auto src_token = buffer + 5;
                    {
                        auto end = ::strchr(src_token, ' '); *end = 0;
                        index = ::atoi(src_token) - 1;
                        src_token = end + 1;
                        assert(*src_token);
                    }
                    sockaddr_in target_client;
                    {
                        std::lock_guard<decltype(g_mutex)> locker(g_mutex);
                        auto& data = g_databackup[index];
                        target_client = data.addr;
                        SENDMSG msg; msg.fisrt = CLIENT_MESSAGE;
                        ::sprintf_s(msg.buffer, "%s:%d", data.ip, int(data.port));
                        // 发送服务器提示消息
                        auto status = ::sendto(
                            sock, &msg.fisrt, SEND_MSG_LENGTH, 0,
                            reinterpret_cast<sockaddr*>(&udp_addr),
                            sizeof(udp_addr)
                            );
                        status = 0;
                    }
                    // 等待
                    ::Sleep(100);
                    // 发送
                    {
                        save_message(src_token);
                        auto status = ::sendto(
                            sock, src_token, ::strlen(src_token)+1, 0,
                            reinterpret_cast<sockaddr*>(&target_client),
                            sizeof(target_client)
                            );
                        status = 0;
                    }
                }
                else {
                    std::lock_guard<decltype(g_mutex)> locker(g_mutex);
                    ::printf("无效命令, 输入help打印命令帮助\r\n");
                }
            }
        }
        // 清理
        ::closesocket(sock);
        // 加入
        onlinethread.join();
        recvthread.join();
    }
    ::WSACleanup();
    return EXIT_SUCCESS;
}


#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"Shcore.lib")

#ifdef _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif