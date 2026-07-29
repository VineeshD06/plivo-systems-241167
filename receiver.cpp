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
    in_addr.sin_port = htons(47002);
    in_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    bind(in_fd, (struct sockaddr *)&in_addr, sizeof(in_addr));

    SOCKET out_fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in player = {0};
    player.sin_family = AF_INET;
    player.sin_port = htons(47020);
    player.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Buffers just to hold data for potential XOR math
    std::map<uint32_t, std::vector<char>> data_buf;
    std::map<uint32_t, std::vector<char>> parity_buf;

    char buf[2048];
    for (;;) {
        int n = recvfrom(in_fd, buf, sizeof(buf), 0, NULL, NULL);
        if (n <= 0) continue;

        uint32_t raw_seq;
        memcpy(&raw_seq, buf, 4);
        raw_seq = ntohl(raw_seq);
        
        // Strip the parity flag to get the real sequence number
        bool is_parity = (raw_seq & 0x80000000) != 0;
        uint32_t seq = raw_seq & 0x7FFFFFFF;

        if (is_parity) {
            parity_buf[seq] = std::vector<char>(buf + 4, buf + n);
            uint32_t p0 = seq - 1; // The Even partner
            uint32_t p1 = seq;     // The Odd partner

            // If we have Even but missed Odd, recover Odd instantly
            if (data_buf.count(p0) && !data_buf.count(p1)) {
                char out_buf[164];
                uint32_t net_seq = htonl(p1);
                memcpy(out_buf, &net_seq, 4);
                for(int i = 0; i < 160; i++) out_buf[4+i] = parity_buf[seq][i] ^ data_buf[p0][i];
                sendto(out_fd, out_buf, 164, 0, (struct sockaddr *)&player, sizeof(player));
                data_buf[p1] = std::vector<char>(out_buf+4, out_buf+164); // Save so we don't double-recover
            }
            // If we have Odd but missed Even, recover Even instantly
            if (data_buf.count(p1) && !data_buf.count(p0)) {
                char out_buf[164];
                uint32_t net_seq = htonl(p0);
                memcpy(out_buf, &net_seq, 4);
                for(int i = 0; i < 160; i++) out_buf[4+i] = parity_buf[seq][i] ^ data_buf[p1][i];
                sendto(out_fd, out_buf, 164, 0, (struct sockaddr *)&player, sizeof(player));
                data_buf[p0] = std::vector<char>(out_buf+4, out_buf+164);
            }
        } else {
            // It's a normal packet. Send it to the player immediately!
            if (!data_buf.count(seq)) {
                sendto(out_fd, buf, n, 0, (struct sockaddr *)&player, sizeof(player));
                data_buf[seq] = std::vector<char>(buf + 4, buf + n);
            }

            // Check if this new packet allows us to recover its missing partner
            uint32_t p_seq = (seq % 2 == 1) ? seq : seq + 1;
            uint32_t partner = (seq % 2 == 1) ? seq - 1 : seq + 1;

            if (parity_buf.count(p_seq) && !data_buf.count(partner)) {
                char out_buf[164];
                uint32_t net_seq = htonl(partner);
                memcpy(out_buf, &net_seq, 4);
                for(int i = 0; i < 160; i++) out_buf[4+i] = parity_buf[p_seq][i] ^ data_buf[seq][i];
                sendto(out_fd, out_buf, 164, 0, (struct sockaddr *)&player, sizeof(player));
                data_buf[partner] = std::vector<char>(out_buf+4, out_buf+164);
            }
        }
    }
    return 0;
}