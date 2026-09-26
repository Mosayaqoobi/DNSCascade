#ifndef DNSRECORD_H
#define DNSRECORD_H

#include <string>
#include <cstdint>
#include <stdexcept>

/**
 * @brief This enum class represents the different types of the records.
 *
 */
enum class RRType : uint16_t {
    A,      ///< DnsRecord of type A
    NS,     ///< DnsRecord of type NS
    CNAME = 5,  ///< DnsRecord of type CNAME
};

inline std::string rrTypeToString(const RRType type) {
    switch (type) {
        case RRType::A: return "A";
        case RRType::NS: return "NS";
        case RRType::CNAME: return "CNAME";
    }
    throw std::runtime_error("Unknown RRType");
}

inline RRType rrTypeFromString(const std::string& s) {
    if (s == "A") return RRType::A;
    if (s == "NS") return RRType::NS;
    if (s == "CNAME") return RRType::CNAME;
    throw std::runtime_error("Unknown RRType string: " + s);
}

/**
 * @brief Response codes carried in DnsHeader::rcode (RFC 1035 values).
 */
namespace Rcode {
    constexpr uint8_t NOERROR  = 0;
    constexpr uint8_t FORMERR  = 1;
    constexpr uint8_t SERVFAIL = 2;
    constexpr uint8_t NXDOMAIN = 3;
    constexpr uint8_t REFUSED  = 5;
}

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
    uint16_t arcount;   ///< Number of resource records in the additional section

    uint16_t id;        ///< identification of the message
    uint8_t rcode;      ///< Response code, see Rcode
    uint8_t opcode;     ///< Type of the query that is carried by this message

    bool qr;            ///< Query or a Response
    bool aa;            ///< (authoritative answer) either an authoritative answer or not
    bool tc;            ///< (truncation) If the length of the message exceeded the max amount
    bool rd;            ///< (recursion desired) If 1, answer question recursively
    bool ra;            ///< (recursive available) whether recursion is available for the response
};

#endif // DNSRECORD_H
