#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <map>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET in_fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in in_addr = {0};
    in_addr.sin_family = AF_INET;
    in_addr.sin_port = htons(47010);
    in_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    bind(in_fd, (struct sockaddr *)&in_addr, sizeof(in_addr));

    SOCKET out_fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in relay = {0};
    relay.sin_family = AF_INET;
    relay.sin_port = htons(47001);
    relay.sin_addr.s_addr = inet_addr("127.0.0.1");

    std::map<uint32_t, std::vector<char>> history;
    char buf[2048];

    for (;;) {
        int n = recvfrom(in_fd, buf, sizeof(buf), 0, NULL, NULL);
        if (n <= 0) continue;
        
        uint32_t seq;
        memcpy(&seq, buf, 4);
        seq = ntohl(seq);
        
        history[seq] = std::vector<char>(buf + 4, buf + n);
        
        // 1. Send original immediately
        sendto(out_fd, buf, n, 0, (struct sockaddr *)&relay, sizeof(relay));
        
        // 2. Immediate XOR Parity (pairing 0&1, 2&3, etc.)
        if (seq % 2 == 1) { 
            uint32_t pair_seq1 = seq;
            uint32_t pair_seq2 = seq - 1;
            
            if (history.count(pair_seq1) && history.count(pair_seq2)) {
                char parity_buf[164];
                // Flag the highest bit to signify this is a parity packet
                uint32_t p_seq_net = htonl(pair_seq1 | 0x80000000); 
                memcpy(parity_buf, &p_seq_net, 4);
                
                for (int i = 0; i < 160; i++) {
                    parity_buf[4 + i] = history[pair_seq1][i] ^ history[pair_seq2][i];
                }
                
                sendto(out_fd, parity_buf, 164, 0, (struct sockaddr *)&relay, sizeof(relay));
                
                // Keep memory clean
                history.erase(pair_seq1);
                history.erase(pair_seq2);
            }
        }
    }
    return 0;
}