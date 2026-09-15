#include "DnsMessage.h"
#include "DnsRecord.h"
#include "UdpSocket.h"
#include <iostream>

/**
 * @brief Tests if the UdpSocket actually sends the required data
 * 
 */
int main() {
    UdpSocket receiver;
    receiver.bind(5999);
    receiver.setTimout(3);

    UdpSocket sender;

    DnsMessage query;
    query.header.id = 42;
    query.header.qr = false;
    query.header.rd = true;
    query.header.qdcount = 1;
    query.questions.push_back({"www.foo.test", RRType::A});

    std::string wireData = query.serialize();
    std::cout << "Sending: \n" << wireData << "\n";

    sender.sendTo(wireData, "127.0.0.1", 5999);

    std::string received;
    std::string senderIp;
    int senderPort;

    bool ok = receiver.receiveFrom(received, senderIp, senderPort);
    if (!ok) {
        std::cout << "Timed out waiting for data! \n";
        return 1;
    }

    std::cout << "Received from " << senderIp << ":" << senderPort << "\n";
    std::cout << "Raw:\n" << received << "\n";

    DnsMessage parsed = DnsMessage::deserialize(received);
    std::cout << "Parsed ID: " << parsed.header.id << "\n";
    std::cout << "Parsed hostname: " << parsed.questions[0].qname << "\n";

    return 0;

}