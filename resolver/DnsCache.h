/* --- DnsCache.h --- */

/* ------------------------------------------
Author: undefined
Date: 9/13/2026
------------------------------------------ */

#ifndef DNSCACHE_H
#define DNSCACHE_H

#include <chrono>
#include <optional>
#include <string>
#include <unordered_map>

#include "DnsRecord.h"


class DnsCache {
public:

    std::optional<ResourceRecord> lookup(const std::string& qname, const RRType& qtype);

    void insert(const ResourceRecord& record);

private:
    struct Key {
        std::string qname;
        RRType qtype;

        bool operator==(const Key& other) const {
            return qtype == other.qtype && qname == other.qname;
        }
    };

    struct KeyHash {
        size_t operator()(const Key& key) const {
            const size_t h1 = std::hash<std::string>{}(key.qname);
            const size_t h2 = std::hash<uint16_t>{}(static_cast<uint16_t>(key.qtype));
            return h1 ^ (h2 * 0x9e3779b97f4a7c15ULL);
        }
    };

    struct Entry {
        ResourceRecord record;
        std::chrono::steady_clock::time_point expiresAt;
    };

    static std::string normalize(const std::string& qname);

    std::unordered_map<Key, Entry, KeyHash> entries_;

};

#endif // DNSCACHE_H
