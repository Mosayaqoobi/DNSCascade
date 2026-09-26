/* --- DnsMessage.cpp --- */

/* ------------------------------------------
author: undefined
date: 9/13/2026
------------------------------------------ */

#include "DnsMessage.h"
#include "DnsRecord.h"
#include <sstream>
#include <stdexcept>

namespace {
    void serializeRR(std::ostringstream& out, const std::string& tag, const ResourceRecord& rr) {
        out << tag << "|" << rr.name << "|" << rrTypeToString(rr.type)
        << "|" << rr.ttl << "|" << rr.rdata << "\n";
    }

    void requireFields(const std::vector<std::string>& fields, const size_t count) {
        if (fields.size() < count) {
            throw std::runtime_error("Malformed " + fields[0] + " line: expected " +
                                     std::to_string(count) + " fields, got " + std::to_string(fields.size()));
        }
    }

    ResourceRecord deserializeRR(const std::vector<std::string>& fields) {
        requireFields(fields, 5);
        ResourceRecord rr;
        rr.name = fields[1];
        rr.type = rrTypeFromString(fields[2]);
        rr.ttl = static_cast<uint32_t>(std::stoul(fields[3]));
        rr.rdata = fields[4];
        return rr;
    }

    std::vector<std::string> splitLine(const std::string& line, char delim) {
        std::vector<std::string> tokens;
        std::stringstream ss(line);
        std::string token;
        while (std::getline(ss, token, delim)) {
            tokens.push_back(token);
        }
        return tokens;
    }
}

std::string DnsMessage::serialize() const {
    std::ostringstream out;

    out << "HEADER|" << header.id << "|" << header.qr << "|" << static_cast<int>(header.opcode)
        << "|" << header.aa << "|" << header.tc << "|" << header.rd << "|" << header.ra
        << "|" << static_cast<int>(header.rcode) << "|" << header.qdcount << "|" << header.ancount
        << "|" << header.nscount << "|" << header.arcount << "\n";

    for (const auto& q : questions) {
        out << "QUESTION|" << q.qname << "|" << rrTypeToString(q.qtype) << "\n";
    }
    for (const auto& rr : answers)     serializeRR(out, "ANSWER", rr);
    for (const auto& rr : authority)   serializeRR(out, "AUTHORITY", rr);
    for (const auto& rr : additional)  serializeRR(out, "ADDITIONAL", rr);

    return out.str();
}

DnsMessage DnsMessage::deserialize(const std::string& data) {
    DnsMessage msg {};
    std::istringstream stream(data);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        auto fields = splitLine(line, '|');
        if (fields.empty()) continue;

        if (const std::string& tag = fields[0]; tag == "HEADER") {
            requireFields(fields, 13);
            msg.header.id = static_cast<uint16_t>(std::stoi(fields[1]));
            msg.header.qr = fields[2] == "1";
            msg.header.opcode = static_cast<uint8_t>(std::stoi(fields[3]));
            msg.header.aa = fields[4] == "1";
            msg.header.tc = fields[5] == "1";
            msg.header.rd = fields[6] == "1";
            msg.header.ra = fields[7] == "1";
            msg.header.rcode = static_cast<uint8_t>(std::stoi(fields[8]));
            msg.header.qdcount = static_cast<uint16_t>(std::stoi(fields[9]));
            msg.header.ancount = static_cast<uint16_t>(std::stoi(fields[10]));
            msg.header.nscount  = static_cast<uint16_t>(std::stoi(fields[11]));
            msg.header.arcount  = static_cast<uint16_t>(std::stoi(fields[12]));
        } else if (tag == "QUESTION") {
            requireFields(fields, 3);
            Question q;
            q.qname = fields[1];
            q.qtype = rrTypeFromString(fields[2]);
            msg.questions.push_back(q);
        }
        else if (tag == "ANSWER") msg.answers.push_back(deserializeRR(fields));
        else if (tag == "AUTHORITY") msg.authority.push_back(deserializeRR(fields));
        else if (tag == "ADDITIONAL") msg.additional.push_back(deserializeRR(fields));

    }
    return msg;
}
