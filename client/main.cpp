// Stage 1: the smallest possible UDP client.
//
// Deliberate simplification for this version: sends exactly one message
// and waits for exactly one reply, with no timeout and no retry. If the
// packet or the reply is lost, this client hangs forever - that failure
// mode is intentional at this stage; it's the exact motivation for every
// stage that follows.
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }
    const char* server_ip = argv[1];
    int port = std::atoi(argv[2]);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "invalid server IP: %s\n", server_ip);
        return 1;
    }

    const char* msg = "hello";
    sendto(sock, msg, strlen(msg), 0,
           reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
    printf("[client] sent: \"%s\"\n", msg);

    char buf[1024];
    ssize_t n = recvfrom(sock, buf, sizeof(buf) - 1, 0, nullptr, nullptr);
    if (n < 0) {
        perror("recvfrom");
        return 1;
    }
    buf[n] = '\0';
    printf("[client] received: \"%s\"\n", buf);

    return 0;
}