/* --- DnsName.h --- */

#ifndef DNSNAME_H
#define DNSNAME_H

#include <algorithm>
#include <cctype>
#include <string>

/**
 * @brief Helpers for comparing domain names. DNS names are case-insensitive and
 * "example.a." (trailing dot) is the same name as "example.a".
 */
namespace DnsName {

    inline std::string normalize(std::string name) {
        if (!name.empty() && name.back() == '.') {
            name.pop_back();
        }
        std::ranges::transform(name, name.begin(),
                               [](const unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return name;
    }

    /**
     * @brief The last `count` dot-separated labels of a normalized name,
     * e.g. lastLabels("www.example.a", 1) == "a", lastLabels("www.example.a", 2) == "example.a".
     * Returns the whole name if it has fewer labels than requested.
     */
    inline std::string lastLabels(const std::string& name, int count) {
        size_t pos = name.size();
        while (count-- > 0) {
            if (pos == 0) return name;
            const auto dot = name.rfind('.', pos - 1);
            if (dot == std::string::npos) return name;
            pos = dot;
        }
        return name.substr(pos + 1);
    }

    /**
     * @brief True if `name` is `zone` itself or a subdomain of it (both normalized).
     */
    inline bool isInZone(const std::string& name, const std::string& zone) {
        if (name == zone) return true;
        return name.size() > zone.size() &&
               name.ends_with(zone) &&
               name[name.size() - zone.size() - 1] == '.';
    }
}

#endif // DNSNAME_H
