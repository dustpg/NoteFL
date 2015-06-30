#include <Ws2tcpip.h>
#include <Windows.h>
//#include <ShellScalingApi.h>
#include <Shlwapi.h>
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#undef min
#undef max
#define AutoLocker std::lock_guard<decltype(g_mutex)> locker(g_mutex)

// 全局互斥锁
std::mutex g_mutex;

// 帮助信息
static const char* HELP_MESSAGE = "[HELP]\r\nlist     \trefresh and list the member on the server\r\n"
                                  "send [id] \tsend file to this user\r\n"
                                  "help    \tshow help info\r\n"
                                  "tell [id] [msg]\tsend message to this user\r\n";
// IP地址
static const char* IP_ADDRESS = "210.41.229.50";
// 端口
static constexpr int IP_PORT = 6000;
// 发送大小_SERVER
static constexpr int SEND_MSG_LENGTH_SERVER = 50;
// 接收大小_SERVER
static constexpr int RECV_MSG_LENGTH_SERVER = 500;
// 文件每片大小
static constexpr int FILE_LEGNTH_IN_EACH_CLIP = 1024;
// 服务器消息
static constexpr int SERVER_MESSAGE = 101;
// 客户端消息
static constexpr int CLIENT_MESSAGE = 102;
// 消息等待超时时间 ms
static constexpr int MESSAGE_SEND_TIMEOUT = 233;
// 输入缓存
static constexpr int INPUT_BUFFER_SIZE = 128;
// 输入缓存
static constexpr int SLEEP_TIME = 20;
// 在线消息发送频率
static constexpr int SEND_OLMSG_TIMEDUR = 3000;
// 文件名缓存
static constexpr int PATH_BUFFER_LENGTH = 256;

// SENDMSGSER
struct SENDMSGSER { char control; char buffer[SEND_MSG_LENGTH_SERVER - 1]; };
// RECVMSG
struct RECVMSGSER { char control; char buffer[RECV_MSG_LENGTH_SERVER - 1]; };
// CLIENTDATA
struct CLIENTDATA { char ip[32]; int32_t port; char name[SEND_MSG_LENGTH_SERVER]; };
// FILEINFO
struct FILEINFO { uint32_t index; uint32_t file_len; char buffer[FILE_LEGNTH_IN_EACH_CLIP]; };

// 全局数据
std::vector<CLIENTDATA> g_clientdata, g_clientdatabackup;

// 初始化IP地址
void init_sockaddr_in(sockaddr_in& addr, const char* ip, int port) noexcept;

// 应用程序入口
int main(int argc, char* argv[]) {
    //::SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    g_clientdata.reserve(16);
    g_clientdatabackup.reserve(16);
    // 初始化
    WSAData wsdata; ::WSAStartup(MAKEWORD(2, 2), &wsdata);
    // 解析端口
    const int port_init = argc >= 2 ? std::atoi(argv[1]) : 5000;
    // 初始化SOCKET
    SOCKET sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    assert(sock != INVALID_SOCKET);
    // 服务器地址
    sockaddr_in server_addr; init_sockaddr_in(server_addr, IP_ADDRESS, IP_PORT);
    // 本机绑定地址
    {
        sockaddr_in sockaddrs; init_sockaddr_in(sockaddrs, "0.0.0.0", port_init);
        auto status = ::bind(sock, reinterpret_cast<sockaddr*>(&sockaddrs), sizeof(sockaddrs));
        status = 0;
    }
    // 退出信号
    std::atomic_bool loop4ever = true;
    // 名称
    SENDMSGSER this_name; this_name.control = SERVER_MESSAGE;
    ::strcpy_s(this_name.buffer, argc >= 3 ? argv[2] : "Dust Loong");
    // 发送信息
    struct SFINFO {
        FILE*       file;
        uint32_t    file_len;
        sockaddr_in addr;
        char        raw_name_inwide[PATH_BUFFER_LENGTH];
        CLIENTDATA  data;
    } send_file_info; ::memset(&send_file_info, 0, sizeof(send_file_info));
    // 输入线程
    std::thread input_thread([&loop4ever, &send_file_info]() noexcept {
        while (true) {
            char buffer[INPUT_BUFFER_SIZE];
            // 获取输入
            ::fgets(buffer, INPUT_BUFFER_SIZE, stdin);
            // 退出
            if (!::strncmp(buffer, "exit", 4)) {
                loop4ever = false;
                break;
            }
            // 帮助
            else if (!::strncmp(buffer, "help", 4)) {
                ::printf(HELP_MESSAGE);
            }
            // 列表
            else if (!::strncmp(buffer, "list", 4)) {
                {
                    AutoLocker;
                    g_clientdatabackup = g_clientdata;
                }
                if (g_clientdatabackup.size()) {
                    int index = 1;
                    for (auto& data : g_clientdatabackup) {
                        ::printf("编号: %2d -- IP: %16s:%5d 名称: %s\r\n", index, data.ip, int(data.port), data.name);
                        ++index;
                    }
                }
                else {
                    ::printf("等待服务器发送消息，请等待数秒\r\n");
                }
            }
            // 发送
            else if (!::strncmp(buffer, "send ", 5)) {
                // 检查已经
                auto index = uint32_t(std::atoi(buffer + 5));
                assert(index > 0 && index <= g_clientdatabackup.size());
                const auto& data = g_clientdatabackup[index-1];
                ::printf("准备发送文件给: %s, 请选择文件\r\n", data.name);
                wchar_t file_path[PATH_BUFFER_LENGTH]; ZeroMemory(file_path, sizeof(file_path));
                {
                    // 选择文件
                    OPENFILENAMEW ofn; ZeroMemory(&ofn, sizeof(ofn));
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = ::GetConsoleWindow();
                    ofn.lpstrFile = file_path;
                    ofn.nMaxFile = PATH_BUFFER_LENGTH;
                    if (!::GetOpenFileNameW(&ofn)) {
                        ::printf("发送取消\r\n");
                        continue;
                    }
                }
                {
                    AutoLocker;
                    assert(!send_file_info.file);
                    ::_wfopen_s(&send_file_info.file, file_path, L"rb");
                    assert(send_file_info.file);
                    // 去除路径信息
                    ::PathStripPathW(file_path);
                    ::memcpy(send_file_info.raw_name_inwide, file_path, sizeof(wchar_t)*(::wcslen(file_path) + 1));
                    // 计算文件大小
                    ::fseek(send_file_info.file, 0, SEEK_END);
                    send_file_info.file_len = ::ftell(send_file_info.file);
                    ::fseek(send_file_info.file, 0, SEEK_SET);
                    init_sockaddr_in(send_file_info.addr, data.ip, data.port);
                    send_file_info.data = data;
                }
            }

        }
    });
    // 主循环
    auto sendto_time = 0;
    // 接受文件信息
    class {
    public:
        // got XXX message
        auto Next(DWORD index) noexcept {
            if (index == m_dwIndex) {
                ++m_dwIndex;
                m_wSend = true;
            }
        }
        // update
        bool Update(SOCKET sock, SFINFO& sfinfo) noexcept {
            if (m_wEnd) return false;
            // 刷新
            auto time = ::timeGetTime();
            if (time - m_dwTime > MESSAGE_SEND_TIMEOUT) {
                m_dwTime = time;
                m_wSend = true;
            }
            // 初始化
            bool rcode = false;
            if (m_wSend) {
                FILEINFO nfinfo;
                nfinfo.index = m_dwIndex;
                nfinfo.file_len = sfinfo.file_len;
                if (m_dwIndex == DWORD(-1)) {
                    ::printf("发送文件信息, 等待接收, 重试%d次\r", int(m_dwRetry));
                    ::memcpy(nfinfo.buffer, sfinfo.raw_name_inwide, sizeof(sfinfo.raw_name_inwide));
                    if (m_dwRetry) rcode = true;
                    ++m_dwRetry;
                }
                // 发送文件数据
                else {
                    if (m_dwRetry) {
                        ::putchar('\n');
                        m_dwRetry = 0;
                    }
                    if (FILE_LEGNTH_IN_EACH_CLIP * m_dwIndex > sfinfo.file_len) {
                        // 不再发送
                        this->End(sfinfo);
                        //this->Start();
                        return false;
                    }
                    ::fseek(sfinfo.file, FILE_LEGNTH_IN_EACH_CLIP * m_dwIndex, SEEK_SET);
                    ::fread(nfinfo.buffer, 1, FILE_LEGNTH_IN_EACH_CLIP, sfinfo.file);
                    ::printf("发送: %.2f%%\r", float(FILE_LEGNTH_IN_EACH_CLIP * m_dwIndex) / float(sfinfo.file_len) * 100.f);
                }
                auto status = ::sendto(
                    sock, reinterpret_cast<char*>(&nfinfo), sizeof(nfinfo), 0,
                    reinterpret_cast<sockaddr*>(&sfinfo.addr),
                    sizeof(sfinfo.addr)
                    );
                status = 0;
            }
            m_wSend = false;
            return rcode;
        }
        // start mission
        void Start() noexcept {
            m_dwTime = ::timeGetTime();
            m_dwIndex = DWORD(-1);
            m_wSend = true;
            m_wEnd = false;
            m_dwRetry = 0;
        }
        // end mission
        void End(SFINFO& info) noexcept {
            if (info.file) {
                ::fclose(info.file);
                ::printf("\n---发送完毕---\r\n");
            }
            ::memset(&info, 0, sizeof(info));
            m_dwTime = 0;
            m_dwIndex = 0;
            m_wSend = false;
            m_wEnd = true;
            m_dwRetry = 0;
        }
        auto IsEnd() const noexcept { return m_wEnd; }
    private:
        // time
        DWORD       m_dwTime;
        // index
        DWORD       m_dwIndex;
        // index
        DWORD       m_dwRetry;
        // send msg
        WORD        m_wSend;
        // is end
        WORD        m_wEnd;
    } send_file_helper;
    // 初始化
    send_file_helper.End(send_file_info);
    // 接收
    struct { FILE* file; uint32_t index; } recv_file_info; 
    recv_file_info.file = nullptr;
    recv_file_info.index = uint32_t(-2);
    while (loop4ever) {
        // 发送在线消息
        {
            auto time = ::timeGetTime();
            if (time - sendto_time > SEND_OLMSG_TIMEDUR) {
                sendto_time = time;
                auto status = ::sendto(
                    sock, &this_name.control, sizeof(this_name), 0,
                    reinterpret_cast<sockaddr*>(&server_addr),
                    sizeof(server_addr)
                    );
                status = 0;
            }
        }
        // 发送文件消息
        auto sendccctoserver = [&]() {
            SENDMSGSER ccc; ccc.control = CLIENT_MESSAGE;
            ::sprintf_s(ccc.buffer, "%s:%d", send_file_info.data.ip, int(send_file_info.data.port));
            // 发送打洞消息
            ::sendto(sock, &ccc.control, sizeof(ccc), 0, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
        };
        {
            g_mutex.lock();
            if (send_file_info.file && send_file_helper.IsEnd()) {
                // 先解锁
                g_mutex.unlock();
                ::Sleep(0);
                sendccctoserver();
                // 等待
                ::Sleep(100);
                g_mutex.lock();
                send_file_helper.Start();
            }
            // 刷新
            if (send_file_helper.Update(sock, send_file_info)) {
                g_mutex.unlock();
                ::Sleep(0);
                sendccctoserver();
                ::Sleep(100);
                g_mutex.lock();
            }
            g_mutex.unlock();
        }
        // 接收消息
        union { RECVMSGSER msg; FILEINFO fileinfo; } recv_data;
        static_assert(sizeof(RECVMSGSER) != sizeof(FILEINFO), "coded via length");
        sockaddr_in target_addr; int addrlen = sizeof(target_addr);
        // 要求异步
        //u_long flag = 1; ::ioctlsocket(sock, FIONBIO, &flag);
        int time_out = SLEEP_TIME; ::setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&time_out), sizeof(time_out));
        auto status = ::recvfrom(
            sock, &recv_data.msg.control, sizeof(recv_data), 0,
            reinterpret_cast<sockaddr*>(&target_addr),
            &addrlen
            );
        // 失败
        if (status == -1)  continue;
        if (status == sizeof(RECVMSGSER)) {
            // 服务器消息?
            if (recv_data.msg.control == SERVER_MESSAGE) {
                char* src_token = recv_data.msg.buffer;
                AutoLocker;
                g_clientdata.clear();
                while (*src_token) {
                    CLIENTDATA data;
                    // 解析IP地址
                    {
                        auto end = ::strchr(src_token, ':'); *end = 0;
                        ::strcpy_s(data.ip, src_token);
                        src_token = end + 1;
                    }
                    // 解析端口
                    {
                        auto end = ::strchr(src_token, ' '); *end = 0;
                        data.port = ::atoi(src_token);
                        src_token = end + 1;
                    }
                    // 解析名称
                    {
                        auto length = ::strstr(src_token, "\r\n") - src_token;
                        ::memcpy(data.name, src_token, length); data.name[length] = 0;
                        src_token += length + 2;
                    }
                    // 新加入
                    g_clientdata.push_back(data);
                }
            }
            // 客户端消息?
            else if (recv_data.msg.control == CLIENT_MESSAGE) {
                sockaddr_in client_addr;;
                // 发送消息
                auto end = ::strchr(recv_data.msg.buffer, ':'); *end = 0;
                init_sockaddr_in(client_addr, recv_data.msg.buffer, ::atoi(end + 1));
                // 发送打洞
                ::sendto(sock, "CCC", 4, 0, reinterpret_cast<sockaddr*>(&client_addr), sizeof(client_addr));
                //::printf("send CCC to %s:%s\r\n", recv_data.msg.buffer, end + 1);
            }
        }
        // 文件消息
        else if (status == sizeof(FILEINFO)) {
            // 发送回馈
            uint32_t data[] = { 0xABCDEF, recv_data.fileinfo.index };
            register auto len_total = recv_data.fileinfo.file_len;
            float rate = float(FILE_LEGNTH_IN_EACH_CLIP * (data[1]+1)) / float(len_total);
            // 检查
            ::printf("接收: %.2f%%\r", std::min(rate, 1.f) * 100.f);
            auto length_this_clip = FILE_LEGNTH_IN_EACH_CLIP;
            auto end_of_stream = false;
            if (FILE_LEGNTH_IN_EACH_CLIP * (data[1] + 1) >= len_total) {
                length_this_clip = len_total - FILE_LEGNTH_IN_EACH_CLIP * data[1];
                end_of_stream = true;
                // 接收完毕
                ::printf("\n---接收完毕---\r\n");
            }
            // 检查
            if (data[1] == uint32_t(-1)) {
                // 选择文件
                if (!recv_file_info.file) {
                    wchar_t file_path[PATH_BUFFER_LENGTH];
                    ::wcscpy_s(file_path, reinterpret_cast<wchar_t*>(recv_data.fileinfo.buffer));
                    OPENFILENAMEW ofn; ZeroMemory(&ofn, sizeof(ofn));
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = ::GetConsoleWindow();
                    ofn.lpstrFile = file_path;
                    ofn.nMaxFile = PATH_BUFFER_LENGTH;
                    if (::GetSaveFileNameW(&ofn)) {
                        ::_wfopen_s(&recv_file_info.file, file_path, L"wb");
                        recv_file_info.index = 0;
                    }
                }
            }
            // 需求
            else if (data[1] == recv_file_info.index) {
                ::fwrite(recv_data.fileinfo.buffer, 1, length_this_clip, recv_file_info.file);
                ++recv_file_info.index;
            }
            // EOS?
            if (end_of_stream && recv_file_info.file) {
                ::fclose(recv_file_info.file);
                recv_file_info.file = nullptr;
                recv_file_info.index = uint32_t(-2);
            }
            // 回馈
            ::sendto(sock, reinterpret_cast<char*>(data), sizeof(data), 0, reinterpret_cast<sockaddr*>(&target_addr), sizeof(target_addr));
        }
        // 消息
        else if(status == sizeof(uint32_t)*2){
            uint32_t control = reinterpret_cast<uint32_t*>(&recv_data)[0];
            uint32_t index = reinterpret_cast<uint32_t*>(&recv_data)[1];
            // 收到XXX消息
            if (control == 0xABCDEF) {
                send_file_helper.Next(index);
            }
        }
        else if (!::strcmp(reinterpret_cast<char*>(&recv_data), "CCC")) {
            //::printf("got CCC \r\n");
        }
    }
    input_thread.join();
    // 扫尾处理
    ::closesocket(sock);
    ::WSACleanup();
    return EXIT_SUCCESS;
}

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"shlwapi.lib")
//#pragma comment(lib,"Shcore.lib")
#pragma comment(lib,"Winmm.lib")

#ifdef _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

// 初始化IP地址
void init_sockaddr_in(sockaddr_in& addr, const char* ip, int port) noexcept {
    ::ZeroMemory(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    ::inet_pton(AF_INET, ip, &addr.sin_addr.s_addr);
    addr.sin_port = ::htons(port);
}