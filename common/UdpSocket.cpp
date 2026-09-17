/* --- UdpSocket.cpp --- */

/* ------------------------------------------
author: undefined
date: 9/13/2026
------------------------------------------ */

#include "UdpSocket.h"
#include <cerrno>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <iostream>

UdpSocket::UdpSocket() {
    sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_ < 0) {
        throw std::runtime_error("Failed to create UDP socket");
    }
}
UdpSocket::~UdpSocket() {
    if (sockfd_ >= 0) {
        std::cout << "closing the UDP connection..." << "\n";
        close(sockfd_);
    }
}

void UdpSocket::bind(int port) const {
    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;  // listen on all local interfaces
    addr.sin_port = htons(port); // host-to-network byte order
    
    if (::bind(sockfd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        throw std::runtime_error("Failed to bind UDP socket to port: " + std::to_string(port));
    }
}

void UdpSocket::sendTo(const std::string& data, const std::string& ip, int port) const {
    sockaddr_in destAddr {};
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &destAddr.sin_addr) <= 0) {
        throw std::runtime_error("Invalid IP Address: " + ip);
    }

    if (const ssize_t sent = ::sendto(sockfd_, data.c_str(), data.size(), 0, reinterpret_cast<sockaddr*>(&destAddr), sizeof(destAddr)); sent < 0) {
        throw std::runtime_error("sendto() failed");
    }
}
void UdpSocket::setTimout(const int seconds) const {
    timeval tv {};
    tv.tv_sec = seconds;
    tv.tv_usec = 0;

    if (setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        throw std::runtime_error("Failed to set socket timeout");
    }
}

bool UdpSocket::receiveFrom(std::string& outData, std::string& outSenderIp, int outSenderPort) const {
    char buffer[65536];
    sockaddr_in senderAddr {};
    socklen_t senderLen = sizeof(senderAddr);

    ssize_t received = ::recvfrom(sockfd_, buffer, sizeof(buffer), 0, reinterpret_cast<sockaddr*>(&senderAddr), &senderLen);

    if (received < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return false;
        }
        throw std::runtime_error("recvfrom() failed");
    }

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &senderAddr.sin_addr, ipStr, sizeof(ipStr));
    outSenderIp = ipStr;
    outSenderPort = ntohs(senderAddr.sin_port);
    outData = std::string(buffer, received);
    
    return true;
}


