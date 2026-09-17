/* --- DNSClient.cpp --- */

/* ------------------------------------------
author: undefined
date: 9/13/2026
------------------------------------------ */

#include "DnsClient.h"
#include "DnsMessage.h"
#include "DnsRecord.h"
#include "UdpSocket.h"
#include <optional>
#include <regex>
#include <iostream>
#include <thread>

DnsClient::DnsClient(std::string resolverIP, const int resolverPort) :
    resolverIp_(std::move(resolverIP)), resolverPort_(resolverPort), nextId_(0), maxRetries_(3) {
        // set the timeout for receiving a reply for 3 seconds;
        sockfd_.setTimout(3);
}

std::string DnsClient::enterHost() {
    std::string host;
    std::cout << "Enter a URL: ";
    std::cin >> host;
    while (!validateHost(host)) {
        std::cout << "Please enter a valid URL. Starting with www... or https...\n";
        std::cin>> host;
    }
    return host;
}

bool DnsClient::validateHost(const std::string& host) {
    if (host.empty()) {
        return false;
    }
    const std::regex urlPattern(
    R"(^(https?://)?(www\.)?[a-zA-Z0-9-]+(\.[a-zA-Z0-9-]+)*\.(com|org|net|test)(/[a-zA-Z0-9.,_@%?&=~+#-]*)*$)"
    );
    return std::regex_match(host, urlPattern);
}

std::optional<DnsMessage> DnsClient::sendQuery(const std::string& host) {
    DnsHeader header {
        .qdcount = 1, 
        .ancount = 0,
        .nscount = 0,
        .arcount = 0, 
        .id = nextId_++,
        .rcode = 0,
        .opcode = 0,
        .qr = false,
        .aa = false,
        .tc = false,
        .rd = true,
        .ra = false
    };

    std::vector<Question> questions {{host, RRType::A}};
    DnsMessage msg = {
        .header = header,
        .questions = questions,
    };

    const std::string serializedMsg = msg.serialize();
    for (int attempt = 0; attempt < maxRetries_; attempt++) {
        std::cout << "Attempt #" << attempt + 1 << " to send query to local resolver...\n";
        sockfd_.sendTo(serializedMsg, resolverIp_, resolverPort_);    
        std::string responseData {};
        std::string senderIp {};
        int senderPort {};

        if (!sockfd_.receiveFrom(responseData, senderIp, senderPort)) {
            std::cout << "Attempt #" << attempt + 1 << " timed out, trying again...\n\n";
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        DnsMessage response = DnsMessage::deserialize(responseData);
        //not the correct response
        if (response.header.id != header.id) {
            continue;
        }
        return response;
    }
    return std::nullopt;
}

void DnsClient::displayResult(const DnsMessage& response) {
    if (response.header.rcode != 0 || response.answers.empty()) {
        std::cout << "Could not resolve host (rcode " << static_cast<int>(response.header.rcode) << ")\n";
        return;
    }
    for (const auto& rr : response.answers) {
        if (rr.type == RRType::A) {
            std::cout << rr.name << " -> " << rr.rdata << "\n";
            return;
        }
    }
    std::cout << "No A record in response\n";

}

void DnsClient::run() {
    while (true) {
        std::string host = enterHost();

        if (auto response = sendQuery(host); !response.has_value()) {
            std::cout << "No response frome resolver (request timed out)\n";
        } else {
            displayResult(response.value());
        }
        std::cout << "Resolve another? (y/n)\n";
        std::string again;
        std::cin >> again;
        if (again != "y" && again != "Y") {
            break;
        }
    }
}
