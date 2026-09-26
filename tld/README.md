# TLD server
One process per TLD (`cfg/a.json` ... `cfg/d.json`). Maps each domain registered under its TLD to that domain's authoritative server and answers with a referral, or NXDOMAIN for an unregistered domain. Refuses names outside its own TLD.

    ./build/bin/tld_server tld/cfg/a.json
