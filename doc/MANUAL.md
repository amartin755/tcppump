
# tcppump

tcppump is a command-line tool for generating Ethernet network packets. The user has full control over every detail of the packets created. All network packets are generated directly by tcppump and not by the operating system, which also makes it possible to create invalid or malformed packets. It was developed with the goal of providing a simple tool for testing protocol stacks and firewalls.

**Usage:**

```
tcppump -i IFC [OPTIONS] packets
tcppump -i IFC [OPTIONS] -s scriptfiles
```

The network packets to be sent can be defined either directly as program parameters or via script files. If you only want to send individual packets, the first method is certainly the simplest option. Script files are useful when you want to generate entire packet sequences and/or send packets on a schedule. To use script files, specify the `-s` or `--script` parameter followed by the filename of the script file. You can also provide multiple files.

tcppump always requires a network interface through which packets will be sent. Therefore, the `-i` or `--interface` parameter followed by the name of the network adapter is mandatory. On Windows, you can specify either the so-called "friendly name" (found via `ipconfig`) or the adapter’s GUID. On Linux, specify the name of the network device (see the output of `ip addr`).

**Examples:**

```
tcppump -i eth0 "arp(dip=11.22.33.44)"
tcppump -i "Ethernet 2" "raw(payload=12345678)" "arp(dip=11.22.33.44)"
tcppump -i WiFi -s myscript.txt
tcppump -i WiFi -s myscript.txt myscript2.txt
```

It’s generally a good idea to enclose packet definitions in quotes, as definitions may contain spaces and could otherwise be mistakenly interpreted as multiple command-line parameters.

The exact syntax for network packets (`packets`) and the various supported protocols is defined [here](./PACKET_REFERENCE.md).

---

## Packet Source Addresses (MAC, IPv4, IPv6)

tcppump offers several ways to control source addresses. If a source address is not explicitly specified in the packet definition, tcppump uses the addresses of the network adapter. This applies to both the MAC and IPv4 addresses. The addresses of the network adapter can also be explicitly overridden using the `--mymac` and `--myipv4`/`--myipv6` parameters. The network card’s configuration itself is, of course, not modified.

This results in the following address selection strategy (in descending priority):

1. Source address(es) explicitly specified in the packet definition  
2. Addresses specified via `--mymac` and `--myipv4`/`--myipv6`  
3. Source addresses of the network adapter  
