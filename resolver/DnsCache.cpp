/* --- DnsCache.cpp --- */

/* ------------------------------------------
author: undefined
date: 9/13/2026
------------------------------------------ */

#include "DnsCache.h"
#include <algorithm>


std::string DnsCache::normalize(const std::string& qname) {
    std::string lower = qname;
    std::ranges::transform(lower, lower.begin(), [](const unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return lower;
}

std::optional<ResourceRecord> DnsCache::lookup(const std::string& qname, const RRType& qtype) {
    const Key key {
        .qname = normalize(qname),
        .qtype = qtype
    };

    const auto it = entries_.find(key);
    if (it == entries_.end()) {
        return std::nullopt;
    }
    const auto now = std::chrono::steady_clock::now();

    if (now >= it->second.expiresAt) {
        entries_.erase(it);
        return std::nullopt;
    }
    ResourceRecord result = it->second.record;

    const auto remaining = std::chrono::duration_cast<std::chrono::seconds>(it->second.expiresAt - now);
    result.ttl = static_cast<uint32_t>(remaining.count());
    return result;
}

void DnsCache::insert(const ResourceRecord& record) {
    const Key key {
        .qname = normalize(record.name),
        .qtype = record.type
    };

    const auto expiresAt = std::chrono::steady_clock::now() + std::chrono::seconds(record.ttl);
    entries_[key] = Entry {.record = record, .expiresAt = expiresAt};


}