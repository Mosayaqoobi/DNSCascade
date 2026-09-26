# Common
Shared by every component: `DnsMessage` (the text wire format), `DnsRecord.h` (record types and response codes), `UdpSocket`, and `ConfigLoader.h`. `tests/test_roundtrip.cpp` sends a message to itself over UDP and parses it back.
