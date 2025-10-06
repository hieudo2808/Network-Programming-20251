#include <arpa/inet.h>
#include <netdb.h>

#include <iostream>
#include <set>
#include <string.h>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <chrono>

using namespace std;
using namespace std::chrono;

void logToFile(const string& content) {
    ofstream log;
    log.open("resolver.txt", ios::app);
    if (log.is_open()) {
        log << content << endl;
        log.close();
    }
}

string currentTime() {
    time_t now = time(0);
    char* dt = ctime(&now);
    dt[strlen(dt) - 1] = '\0';
    return string(dt);
}

bool isPotentialIP(const char* str) {
    int len = 0;
    int dots = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (isdigit(str[i]) || str[i] == '.') {
            if (str[i] == '.') dots++;
            len++;
        } else return false;
    }
    return true;
}

bool checkSpecialAddress(const in_addr& addr) {
    uint32_t ip = ntohl(addr.s_addr);  // Convert from network byte order to
                                       // host byte order - big endian
    if ((ip >> 24) == 127) {
        cout << "Warning: This is loopback address — may not have DNS record" << endl;
        return false;
    } else if ((ip >> 24) == 10 ||  // 10.x.x.x A-type private
               ((ip >> 20) == (172 << 4 | 1) && (ip >> 16 & 0xF) >= 0 &&
                (ip >> 16 & 0xF) <= 15) ||  // 172.16.x.x - 172.31.x.x B-type private
               ((ip >> 16) == (192 << 8 | 168))) {
        cout << "Warning: This is private address — may not have DNS record" << endl;
        return false;
    } else if ((ip >> 24) == 169 &&
               ((ip >> 16) & 0xFF) == 254) {  // 169.254.x.x
        cout << "Warning: This is link-local address — may not have DNS record" << endl;
        return false;
    } else if ((ip >> 28) == 0xE) {  // 224.0.0.0/4
        cout << "Warning: This is multicast address — may not have DNS record" << endl;
        return false;
    } else if (ip == 0xFFFFFFFF) {  // 255.255.255.255
        cout << "Warning: This is broadcast address — may not have DNS record" << endl;
        return false;
    }
    return true;
}

void domainToAddress(const char* domain) {
    addrinfo *res, hint{};
    hint.ai_family = AF_UNSPEC;

    auto start = high_resolution_clock::now();
    int status = getaddrinfo(domain, NULL, &hint, &res);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();

    if (status != 0) {
        cerr << "Not found information, " << gai_strerror(status) << endl;
        logToFile("Not found information: " + string(gai_strerror(status)));
        return;
    }

    set<string> unique_ips;
    char ipstr[INET6_ADDRSTRLEN];
    bool firstElement = true;

    for (addrinfo* p = res; p != nullptr; p = p->ai_next) {
        void* addr;
        if (p->ai_family == AF_INET) {
            sockaddr_in* ipv4 = (sockaddr_in*)p->ai_addr;
            addr = &(ipv4->sin_addr);
        } else if (p->ai_family == AF_INET6) {
            sockaddr_in6* ipv6 = (sockaddr_in6*)p->ai_addr;
            addr = &(ipv6->sin6_addr);
        }

        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));

        if (firstElement) {
            cout << "Official IP: " << ipstr << endl;
            logToFile("Official IP: " + string(ipstr));
            if (p->ai_canonname) {
                cout << "Canonical name: " << p->ai_canonname << endl;
                logToFile("Canonical name: " + string(p->ai_canonname));
            }
            firstElement = false;
        } else {
            unique_ips.insert(string(ipstr));
        }
    }

    if (!unique_ips.empty()) {
        cout << "Alias IP:" << endl;
        logToFile("Alias IP:");
        for (const auto& ip : unique_ips) {
            logToFile(ip);
            cout << ip << endl;
        }
    }

    cout << "Query time: " << duration << " ms" << endl;
    logToFile("Query time: " + to_string(duration) + " ms");
    freeaddrinfo(res);
}

void ipToDomain(in_addr addr) {
    if(!checkSpecialAddress(addr)) {
        logToFile("Warning: Special IP address — may not have DNS record");
        return;
    }

    auto start = high_resolution_clock::now();
    struct hostent* he = gethostbyaddr(&addr, sizeof(addr), AF_INET);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();

    if (!he) {
        cerr << "Not found information: " << hstrerror(h_errno) << endl;
        logToFile("Not found information: " + string(hstrerror(h_errno)));
        return;
    }

    cout << "Official name: " << he->h_name << endl;
    logToFile("Official name: " + string(he->h_name));
    if (he->h_aliases && he->h_aliases[0]) {
        cout << "Alias names:" << endl;
        logToFile("Alias names:");
        for (char** a = he->h_aliases; a && *a; ++a) {
            cout << *a << endl;
            logToFile(*a);
        }
    }
    cout << "Query time: " << duration << " ms" << endl;
    logToFile("Query time: " + to_string(duration) + " ms");
}

vector<string> splitInput(const string& line) {
    istringstream iss(line);
    vector<string> tokens;
    string token;
    while (iss >> token)
        tokens.push_back(token);
    return tokens;
}

void processInput(const string& input) {
    vector<string> queries = splitInput(input);
    for (auto& query : queries) {
        logToFile("[" + currentTime() + "] Lookup: " + query);
        cout << "----Get infomation for " << query << "----" << endl;

        in_addr address{};
        if (isPotentialIP(query.c_str())) {
            if (inet_pton(AF_INET, query.c_str(), &address) != 1) {
                cerr << "Invalid address" << endl;
                logToFile("Invalid address\n");
            } else ipToDomain(address);
        } else domainToAddress(query.c_str());

        cout << "----------End of query----------" << endl << endl;
        logToFile("----------End of query----------\n");
    }
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        ifstream inputFile(argv[1]);
        if (!inputFile) {
            cerr << "Error when opening file" << endl;
            return 1;
        }
        string line;
        while (getline(inputFile, line)) {
            if (line.empty()) continue;
            processInput(line);
        }
        return 0;
    }

    string line;
    while (true) {
        cout << "> ";
        getline(cin, line);
        if (line.empty()) break;
        processInput(line);
    }

    cout << "Exiting..." << endl;
    return 0;
}