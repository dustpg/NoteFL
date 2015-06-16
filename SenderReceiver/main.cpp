#include <Ws2tcpip.h>
#include <Windows.h>
#include <Shlwapi.h>
#include <cstdio>
#include <cassert>

// 消息缓冲区大小
constexpr size_t MSG_BUFFER_LENGTH = 512;
// 文件缓冲区大小
constexpr size_t FILE_BUFFER_LENGTH = 1024 * 8;
// 文件名缓存
constexpr size_t PATH_BUFFER_LENGTH = 256;

void send_file(const char*, int);
void receive_file(const char*, int);

// 应用程序入口
int main(int argc, char* argv[]) {
    ::wprintf(L"help:\r\n\tput <ip> : <port> , to put file\r\n\tget <ip> : <port> , to get file\r\n");
    char op[64];char ipbuf[64]; int port = 0;
    ::scanf("%s %s : %d", op, ipbuf, &port);
    if (!::strcmp(op, "put")) {
        send_file(ipbuf, port);

    }
    else if (!::strcmp(op, "get")) {
        receive_file(ipbuf, port);
    }
    return EXIT_SUCCESS;
}


// 发送文件
void send_file(const char* ip_, int port_) {
    WSAData sender_data; ::WSAStartup(MAKEWORD(2, 2), &sender_data);
    SOCKET sender_sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);;
    sockaddr_in udp_receiver, udp_sender;
    {
        udp_sender.sin_family = AF_INET;
        ::inet_pton(AF_INET, ip_, &udp_sender.sin_addr.s_addr);
        udp_sender.sin_port = ::htons(port_);
        // 绑定
        auto status = ::bind(
            sender_sock,
            reinterpret_cast<sockaddr*>(&udp_sender),
            sizeof(udp_sender)
            );

        char buffer[MSG_BUFFER_LENGTH];
        // 接收待机消息
        int addrlen = sizeof(udp_receiver);;
        ::Sleep(60);
        status = ::recvfrom(
            sender_sock, buffer, MSG_BUFFER_LENGTH, 0,
            reinterpret_cast<sockaddr*>(&udp_receiver),
            &addrlen
            );
        assert(::strcmp("STANDBY", buffer) == 0 && "failed");
    }
    wchar_t file_path[PATH_BUFFER_LENGTH]; ZeroMemory(file_path, sizeof(file_path));
    {
        // 选择文件

    }
    // 打开文件
    auto file = ::_wfopen(file_path, L"rb");
    if (file) {
        // 显示文件大小
        ::fseek(file, 0, SEEK_END);
        const auto file_size = ::ftell(file);
        ::fseek(file, 0, SEEK_SET);
        ::wprintf(L"file[%ls] @ %ldbytes\r\n", file_path, file_size);
        {
            char buffer[MSG_BUFFER_LENGTH];
            // 准备完毕
            ::PathStripPathW(file_path);
            ::sprintf(buffer, "READY %ld, %ls\r\n", file_size, file_path);
            ::sendto(
                sender_sock, buffer, ::strlen(buffer) + 1, 0,
                reinterpret_cast<sockaddr*>(&udp_receiver),
                sizeof(udp_receiver)
                );
            // 等待握手
            ZeroMemory(buffer, sizeof(buffer));
            int addrlen = sizeof(udp_receiver);;
            auto status = ::recvfrom(
                sender_sock, buffer, sizeof(buffer), 0,
                reinterpret_cast<sockaddr*>(&udp_receiver),
                &addrlen
                );
            if (::strncmp(buffer, "OK", 2)) {
                ::wprintf(L"<%s> failed:%s\r\n", __func__, buffer);
            }
        }
        // 正式发送文件
        {
            char buffer[FILE_BUFFER_LENGTH];
            long totalSendBytes = 0l;
            while (totalSendBytes < file_size) {
                auto readBytes = ::fread(buffer, 1, FILE_BUFFER_LENGTH, file);
                auto sendBytes = 0u;
                while (sendBytes < readBytes) {
                    sendBytes += ::sendto(
                        sender_sock,
                        buffer + sendBytes, readBytes - sendBytes,
                        0,
                        reinterpret_cast<sockaddr*>(&udp_receiver),
                        sizeof(udp_receiver)
                        );
                }
                totalSendBytes += sendBytes;
                // 等待
                int addrlen = sizeof(udp_receiver);
                auto status = ::recvfrom(
                    sender_sock, buffer, sizeof(buffer), 0,
                    reinterpret_cast<sockaddr*>(&udp_receiver),
                    &addrlen
                    );
            }
        }
        ::fclose(file);
        file = nullptr;
    }
}

// 接收文件
void receive_file(const char* ip_, int port_) {
    WSAData receiver_data; ::WSAStartup(MAKEWORD(2, 2), &receiver_data);
    SOCKET receiver_sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);;
    sockaddr_in udp_receiver, udp_sender;
    // 检查
    udp_receiver.sin_family = AF_INET;
    ::inet_pton(AF_INET, ip_, &udp_receiver.sin_addr.s_addr);
    udp_receiver.sin_port = ::htons(port_);
    ::wprintf(L"waitting for connecting...\r\n");
    int addrle = sizeof(udp_sender);;
    long file_size = 0;
    {
        // 发送一次消息
        {
            auto statu = ::sendto(
                receiver_sock, "STANDBY", 8, 0,
                reinterpret_cast<sockaddr*>(&udp_receiver), sizeof(udp_receiver)
                );
            int a = 9;
        }
        char buffer[MSG_BUFFER_LENGTH];
        // 收到READY消息
        auto status = ::recvfrom(
            receiver_sock, buffer, MSG_BUFFER_LENGTH, 0,
            reinterpret_cast<sockaddr*>(&udp_sender), &addrle
            );
        // 显示连接
        {
            char ip[64];
            ::inet_ntop(AF_INET, &(udp_sender.sin_addr), ip, INET_ADDRSTRLEN);
            int port = ntohs(udp_sender.sin_port);
            ::wprintf(L"connect from: %s:%d\r\n", ip, port);
        }
        wchar_t file_name[PATH_BUFFER_LENGTH];
        ::sscanf(buffer, "READY %ld, %ls\r\n", &file_size, file_name);
        {
            int a = 9;
        }
    }
    // 准备完毕
    {
        auto statu = ::sendto(
            receiver_sock, "OK", 3, 0,
            reinterpret_cast<sockaddr*>(&udp_receiver), sizeof(udp_receiver)
            );
        int a = 9;
    }
    // 写入数据
    FILE* file = ::_wfopen(L"D:\\22.jpg", L"wb");
    if (file) {
        auto totalReadBytes = 0l;
        // 足够
        while (totalReadBytes < file_size) {
            ::Sleep(100);
            ::wprintf(L"\r%2.2f%%", float(totalReadBytes) / float(file_size) * 100.f);
            // 缓冲区
            char file_buffer[FILE_BUFFER_LENGTH];
            int addrle = sizeof(udp_sender);
            auto readBytes = ::recvfrom(
                receiver_sock, file_buffer, FILE_BUFFER_LENGTH, 0,
                reinterpret_cast<sockaddr*>(&udp_sender), &addrle
                );
            auto writeBytes = 0;
            while (writeBytes < readBytes) {
                writeBytes += fwrite(file_buffer + writeBytes, 1, readBytes - writeBytes, file);
            }
            totalReadBytes += writeBytes;
            // 发送
            ::sendto(receiver_sock, "STANDBY", 8, 0,
                reinterpret_cast<sockaddr*>(&udp_receiver), sizeof(udp_receiver)
                );
        }
        ::fclose(file);
        file = nullptr;
    }
}

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"shlwapi.lib")

