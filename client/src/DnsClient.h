/* --- DNSClient.h --- */

/* ------------------------------------------
Author: undefined
Date: 9/13/2026
------------------------------------------ */

#ifndef DNSCLIENT_H
#define DNSCLIENT_H

#include "DnsMessage.h"
#include "UdpSocket.h"
#include <string>


/**
 * @brief Object for the DNSClient that handles the initiation of the dns simulation
 * 
 */
class DnsClient {

public:
    DnsClient(std::string resolverIP, int resolverPort);
    
    /**
     * @brief Main loop that allows for prompting, validating, sending, displaying and repeating the 
     simulation 
     * 
     */
    void run();

private:
    /**
     * @brief Allows the user to enter in a hostname from the terminal
     * 
     * @return std::string 
     */
    static std::string enterHost();
    
    /**
     * @brief Check if the user-entered hostname is valid
     * 
     * @param host the thing to validate
     * @return true if host is valid
     * @return false if host is not valid
     */
    static bool validateHost(const std::string& host);
    /**
     * @brief If the host is valid, then send create a dnsmessage and send it to the local dns server
     * 
     * @param host the hostname to get the ip
     * @return DnsMessage object that the local dns understands
     */
    std::optional<DnsMessage> sendQuery(const std::string& host);
    
    /**
     * @brief Breifly displays the result of the query
     * 
     * @param response the response of the query
     */
    static void displayResult(const DnsMessage& response) ;

    std::string resolverIp_;
    int resolverPort_;
    UdpSocket sockfd_;
    uint16_t nextId_;
    int maxRetries_;

};

#endif // DNSCLIENT_H
