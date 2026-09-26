# Authoritative server
One process per zone (`cfg/example.a.json` ...). The only server that holds real records. Answers with the records (following CNAMEs inside its own zone), NODATA when the name exists without that record type, or NXDOMAIN. Sets the authoritative-answer flag.

    ./build/bin/authoritative_server authoritative/cfg/example.a.json
