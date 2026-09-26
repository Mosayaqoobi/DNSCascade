# DNSCascade
- I have been learning about the fundementals of networking. This includes the ISO model and how each layer functions. Because of that, I wanted to make a DNS simulator which starts off with a search from the client using their respective user agent (chrome, safari, etc), and ends up with an IP address. 

## How it works

Every component is its own process, and they only talk to each other over UDP on `127.0.0.1` — the same way the real DNS hierarchy does.

```
dns_client ──► resolver ──► root_server          "who handles .a?"          -> referral to TLD .a
                  │    ──► tld_server (.a)       "who handles example.a?"   -> referral to example.a
                  │    ──► authoritative_server  "what is www.example.a?"   -> 10.0.1.10
                  ▼
             answer (cached until its TTL runs out)
```

- **Client** (`client/`, built as `dns_client`) — takes a URL, strips it down to a hostname, and asks the resolver. It only knows the resolver's address.
- **Resolver** (`resolver/`) — answers from its cache when it can; otherwise walks root → TLD → authoritative itself, following each referral. Also follows aliases (CNAMEs) that point into other zones.
- **Root server** (`root/`) — knows which TLD server handles each TLD (`.a`, `.b`, `.c`, `.d`). Only ever refers.
- **TLD servers** (`tld/`, one process per TLD) — know which authoritative server handles each domain under their TLD. Only ever refer.
- **Authoritative servers** (`authoritative/`, one process per zone) — hold the actual records and give the final answer, NXDOMAIN, or "exists but no address".
- **`server/`** — the `DnsServer` base class every server above inherits (receive → `handleQuery` → reply), plus shared name/config helpers.
- **`common/`** — the message format, UDP socket wrapper, and config loader shared by everything.

Real DNS servers all listen on port 53, but this simulation runs every server on one machine, so each has its own port. A referral's glue record therefore carries `"ip:port"` instead of a bare IP.

## Build

Requires CMake 3.15+ and a C++20 compiler. nlohmann/json is downloaded automatically on the first configure.

```bash
cmake -S . -B build
cmake --build build
```

Binaries land in `build/bin/`.

## Run

Run everything from the repository root — config paths are relative to it.

```bash
# Terminal 1: root, all TLD servers, all authoritative servers, and the resolver
./scripts/run_servers.sh        # logs in logs/, Ctrl+C stops everything

# Terminal 2: the client
./build/bin/dns_client
```

To run a single server by hand instead:

```bash
./build/bin/root_server   root/cfg/root.json
./build/bin/tld_server    tld/cfg/a.json
./build/bin/authoritative_server authoritative/cfg/example.a.json
./build/bin/resolver      resolver/cfg/resolver.json
```

## Things to try

| Query | What it shows |
|---|---|
| `www.example.a` | Full walk root → TLD → authoritative |
| `www.example.a` again | Cache hit — no network hops, TTL counting down |
| `blog.example.a` | Alias resolved inside one zone |
| `shop.example.b` | Alias into another zone (`www.example.c`) — resolver starts a second walk |
| `old.example.d` | Alias chain across TLDs (`→ blog.example.a → www.example.a`) |
| `nope.example.a` | NXDOMAIN from the authoritative server |
| `www.unknown.b` | NXDOMAIN from the TLD server |
| `www.example.com` | NXDOMAIN from the root server (no `.com` TLD here) |
| `ns.example.c` | Name exists but has no address record |
| `https://WWW.Example.a/page` | URL reduced to a hostname before querying |

Stop one TLD server (e.g. `.c`) while the others run and query `api.example.c` to see a SERVFAIL.

## Configuration

| File | Contents |
|---|---|
| `client/cfg/client.json` | Resolver address, per-attempt timeout, retries |
| `resolver/cfg/resolver.json` | Listening port, root server address, upstream timeout, retries |
| `root/cfg/root.json` | Listening port, TLD → TLD server address |
| `tld/cfg/<tld>.json` | Listening port, which TLD it serves, domain → authoritative server address |
| `authoritative/cfg/<zone>.json` | Listening port, zone name, its records (`A`, `CNAME`, `NS`) |

To add a domain, e.g. `test.b`: add an entry to `tld/cfg/b.json` pointing at a new port, then create `authoritative/cfg/test.b.json` with that port and its records. `run_servers.sh` picks up every file in `tld/cfg/` and `authoritative/cfg/` automatically.

The client's timeout should stay larger than the resolver's worst case (timeout × (retries + 1) × 3 hops), or the client gives up while the resolver is still working.

## Ports

| Component | Port |
|---|---|
| resolver | 5300 |
| root | 5301 |
| TLD `.a` `.b` `.c` `.d` | 5401–5404 |
| `example.a` … `example.d` | 5501–5504 |
