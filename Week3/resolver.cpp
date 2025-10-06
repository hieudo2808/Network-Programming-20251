#include <arpa/inet.h>
#include <netdb.h>

#include <iostream>
#include <set>

using namespace std;

bool isPotentialIP(const char* str) {
    int len = 0;
    int dots = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (isdigit(str[i]) || str[i] == '.') {
            if (str[i] == '.') dots++;
            len++;
        } else
            return false;
    }
    return true;
}

bool checkSpecialAddress(const in_addr& addr) {
    uint32_t ip = ntohl(addr.s_addr);  // Convert from network byte order to
                                       // host byte order - big endian

    if ((ip >> 24) == 127) {  // 127.x.x.x
        cout << "This is loopback address" << endl;
        return false;
    } else if ((ip >> 24) == 10 ||  // 10.x.x.x A-type private
               ((ip >> 20) == (172 << 4 | 1) && (ip >> 16 & 0xF) >= 0 &&
                (ip >> 16 & 0xF) <=
                    15) ||  // 172.16.x.x - 172.31.x.x B-type private
               ((ip >> 16) ==
                (192 << 8 | 168))) {  // 192.168.x.x C-type private
        cout << "This is private address" << endl;
        return false;
    } else if ((ip >> 24) == 169 &&
               ((ip >> 16) & 0xFF) == 254) {  // 169.254.x.x
        cout << "This is link-local address" << endl;
        return false;
    } else if ((ip >> 28) == 0xE) {  // 224.0.0.0/4
        cout << "This is multicast address" << endl;
        return false;
    } else if (ip == 0xFFFFFFFF) {  // 255.255.255.255
        cout << "This is broadcast address" << endl;
        return false;
    }
    return true;
}

void domainToIpv4Address(const char* domain) {
    addrinfo *res, hint{};
    hint.ai_family = AF_INET;

    int status = getaddrinfo(domain, NULL, &hint, &res);

    if (status != 0) {
        cerr << "Not found information, " << gai_strerror(status) << endl;
        return;
    }

    char ipstr[INET_ADDRSTRLEN];
    set<string> unique_ips;
    bool firstElement = true;

    for (addrinfo* p = res; p != nullptr; p = p->ai_next) {
        auto* ipv4 = (sockaddr_in*)p->ai_addr;
        inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
        if (firstElement) {
            cout << "Official IP: " << ipstr << endl;
            firstElement = false;
        } else
            unique_ips.insert(string(ipstr));
    }

    if (unique_ips.size() > 0) cout << "Alias IP:" << endl;

    for (const auto& ip : unique_ips) cout << ip << endl;

    freeaddrinfo(res);
}

void ipToDomain(in_addr addr) {
    if(!checkSpecialAddress(addr)) return;

    struct hostent* he = gethostbyaddr(&addr, sizeof(addr), AF_INET);
    if (!he) {
        cerr << "Not found information: " << hstrerror(h_errno) << endl;
        return;
    }

    cout << "Official name: " << he->h_name << endl;

    if (he->h_aliases && he->h_aliases[0]) {
        cout << "Alias names: " << endl;
        for (char** a = he->h_aliases; a && *a; ++a)
            cout << *a << endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Invalid input" << endl;
        return 0;
    }

    in_addr address{};

    if (isPotentialIP(argv[1])) {
        if (inet_pton(AF_INET, argv[1], &address) != 1)
            cerr << "Invalid address" << endl;
        else
            ipToDomain(address);
    } else
        domainToIpv4Address(argv[1]);

    return 0;
}