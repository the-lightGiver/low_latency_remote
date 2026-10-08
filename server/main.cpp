// Stage 1: the smallest possible UDP server.
//
// Deliberate simplification for this version: no protocol framing, no
// sequence numbers, no reliability. This exists only to prove the raw
// socket plumbing works before anything else is layered on top of it.
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char** argv) {
    int port = argc > 1 ? std::atoi(argv[1]) : 5555;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    printf("[server] listening on udp/%d\n", port);
    fflush(stdout);

    char buf[1024];
    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        ssize_t n = recvfrom(sock, buf, sizeof(buf) - 1, 0,
                              reinterpret_cast<sockaddr*>(&client_addr), &client_len);
        if (n < 0) {
            perror("recvfrom");
            continue;
        }
        buf[n] = '\0';
        printf("[server] received: \"%s\"\n", buf);
        fflush(stdout);

        const char* reply = "hello received";
        sendto(sock, reply, strlen(reply), 0,
               reinterpret_cast<sockaddr*>(&client_addr), client_len);
    }
    return 0;
}