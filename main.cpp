#include "ConfigLoader.h"
#include "DnsClient.h"
#include <exception>
#include <iostream>

int main(const int argc, char* argv[]) {
    const std::string cfgPath = argc > 1 ? argv[1] : "client/cfg/client.json";
    try {
        DnsClient client(loadConfig<ClientConfig>(cfgPath));
        client.run();
    } catch (const std::exception& e) {
        std::cerr << "dns_client: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
