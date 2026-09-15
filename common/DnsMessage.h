/* --- DnsMessage.h --- */

/* ------------------------------------------
Author: undefined
Date: 9/13/2026
------------------------------------------ */

#ifndef DNSMESSAGE_H
#define DNSMESSAGE_H

#include "DnsRecord.h"
#include <string>
#include <vector>

struct DnsMessage {
    DnsHeader header;
    std::vector<Question> questions;
    std::vector<ResourceRecord> answers;
    std::vector<ResourceRecord> authority;
    std::vector<ResourceRecord> additional;

    std::string serialize() const;
    static DnsMessage deserialize(const std::string& data);
};

#endif // DNSMESSAGE