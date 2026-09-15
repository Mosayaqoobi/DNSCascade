/* --- UdpSocket.h --- */

/* ------------------------------------------
Author: undefined
Date: 9/13/2026
------------------------------------------ */

#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include <string>

class UdpSocket {
public:
    UdpSocket();
    ~UdpSocket();

    /**
     * @brief Listen on a specific port for any messages (for servers only)
     * 
     * @param port the port to listen on
     */
    void bind(int port);

    /**
     * @brief send raw bytes to the respective ip and port
     * 
     * @param data to send to the process
     * @param ip address
     * @param port location
     */
    void sendTo(const std::string& data, const std::string& ip, int port);

    /**
     * @brief block until data arrives
     * 
     * @param outSenderIp the ip that the message came from
     * @param outSenderPort the port it came from
     * @return false on timeout, or true on data received
     */
    bool receiveFrom(std::string& outData, std::string& outSenderIp, int& outSenderPort);

    /**
     * @brief Set the Timout object
     * 
     * @param seconds number of seconds to timeout
     */
    void setTimout(int seconds);

private:
    int sockfd_;
};

#endif // UDPSOCKET_H
