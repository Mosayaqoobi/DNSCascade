#ifndef DNSRECORD_H
#define DNSRECORD_H

#include <string>
#include <cstdint>

/**
 * @brief This enum class represents the different types of the records.
 * 
 */
enum class RRType : uint16_t {
    A,      ///< DnsRecord of type A
    NS,     ///< DnsRecord of type NS
    CNAME = 5,  ///< DnsRecord of type CNAME
};

struct ResourceRecord {
    std::string name;   ///< Could be a hostname or a domain or an alias hostname depending on the RRtype
    RRType type;        ///< the type of Resource record
    uint32_t ttl;       ///< time to live of the resource record for when it should be removed from cache
    std::string rdata;  ///< Could be a the Ip address or a hostname
};

struct Question {
    std::string qname;  ///< Identifier of the question name
    RRType qtype;       ///< the question type
};

struct DnsHeader {
    uint16_t qdcount;   ///< Number of questions
    uint16_t ancount;   ///< Number of answers
    uint16_t nscount;   ///< Number of resource records in the authoritative section
    uint16_t arcount;   

    uint16_t id;        ///< identification of the message
    uint8_t rcode;      ///< Response code (0, 1, 2, 3, 4, 5)
    uint8_t opcode;     ///< Type of the query that is carried by this message

    bool qr;            ///< Query or a Response
    bool aa;            ///< (authoritative answer) either an authoritative answer or not 
    bool tc;            ///< (truncation) If the length of the message exceeded the max amount
    bool rd;            ///< (recursion desired) If 1, answer question recursively
    bool ra;            ///< (recursive available) whether recursion is available for the response
};

#endif // DNSRECORD_H 
