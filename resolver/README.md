# Resolver
The local recursive resolver. Answers from `DnsCache` when it can (entries expire with their TTL); otherwise walks root -> TLD -> authoritative, following each referral, and chases CNAMEs that point into other zones. Uses a second socket for its own outbound queries so upstream replies never mix with incoming client queries. Settings in `cfg/resolver.json`.
