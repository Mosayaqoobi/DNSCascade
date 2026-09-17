#include "DnsClient.h"

int main() {
    DnsClient client("127.0.0.1", 5300);
    client.run();
    return 0;
}