/* --- DNSClient.h --- */

/* ------------------------------------------
Author: undefined
Date: 9/13/2026
------------------------------------------ */

#ifndef DNSCLIENT_H
#define DNSCLIENT_H

#include "DnsMessage.h"
#include "UdpSocket.h"
#include <nlohmann/json.hpp>
#include <cstdint>
#include <optional>
#include <string>

/**
 * @brief Mirrors client/cfg/client.json.
 */
struct ClientConfig {
    std::string resolverIp;
    int resolverPort;
    int timeoutSeconds;   ///< how long to wait for the resolver on each attempt
    int maxRetries;       ///< resends after the first attempt times out

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ClientConfig, resolverIp, resolverPort, timeoutSeconds, maxRetries)
};

/**
 * @brief Object for the DNSClient that handles the initiation of the dns simulation
 * 
 */
class DnsClient {

public:
    explicit DnsClient(const ClientConfig& config);
    
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
     * @return the entered URL, or "" once input is exhausted
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
     * @brief Reduce a URL to the bare hostname DNS resolves,
     * e.g. "https://WWW.Example.a/page" -> "www.example.a".
     */
    static std::string toHostname(const std::string& url);

    /**
     * @brief Build a query for `host` and send it to the local resolver, retrying on timeout
     * 
     * @param host the hostname to get the ip
     * @return the resolver's reply, or nullopt if it never answered
     */
    std::optional<DnsMessage> sendQuery(const std::string& host);
    
    /**
     * @brief Briefly displays the result of the query
     * 
     * @param host the hostname that was asked about
     * @param response the response of the query
     */
    static void displayResult(const std::string& host, const DnsMessage& response);

    std::string resolverIp_;
    int resolverPort_;
    UdpSocket sockfd_;
    uint16_t nextId_ = 0;
    int maxRetries_;

};

#endif // DNSCLIENT_H
