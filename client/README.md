# Client
The user side of the simulation. Prompts for a URL, reduces it to a hostname, sends one query to the local resolver, and prints the result. It only ever talks to the resolver; its address, timeout and retry count come from `cfg/client.json`.
