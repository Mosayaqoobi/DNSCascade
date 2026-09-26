# Server base
`DnsServer`: the receive -> `handleQuery` -> reply loop every server inherits. Drops malformed packets instead of crashing and fills in the header section counts. Also holds `DnsName.h` (case-insensitive name helpers) and `ServerConfig.h` (the `Address` type used in configs).
