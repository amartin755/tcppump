#/usr/bin/bash

mergecap -a -s 65535 -F pcap -w all-raw.pcap raw*.pcap
mergecap -a -s 65535 -F pcap -w all-eth.pcap eth-01.pcap  eth-03.pcap  eth-06.pcap  eth-07.pcap  eth-08.pcap eth-08.pcap eth-11.pcap eth-11.pcap  eth-14.pcap eth-14.pcap  eth-17.pcap  eth-20.pcap  eth-21.pcap  eth-22.pcap  eth-24.pcap  eth-25.pcap eth-25.pcap  eth-27.pcap  eth-28.pcap eth-28.pcap eth-33.pcap  eth-35.pcap  eth-36.pcap  eth-37.pcap  eth-39.pcap
mergecap -a -s 65535 -F pcap -w all-arp.pcap arp-01.pcap arp-01.pcap  arp-04.pcap  arp-05.pcap  arp-07.pcap  arp-08.pcap  arp-09.pcap  arp-10.pcap  arp-11.pcap  arp-12.pcap  arp-13.pcap
mergecap -a -s 65535 -F pcap -w all-ipv4.pcap ipv4-04.pcap  ipv4-05.pcap ipv4-04.pcap ipv4-07.pcap ipv4-07.pcap ipv4-10.pcap ipv4-10.pcap  ipv4-13.pcap ipv4-14.pcap ipv4-14.pcap  ipv4-17.pcap  ipv4-19.pcap  ipv4-20.pcap  ipv4-21.pcap
mergecap -a -s 65535 -F pcap -w all-udp.pcap udp-05.pcap  udp-06.pcap  udp-08.pcap  udp-10.pcap  udp-11.pcap  udp-13.pcap  udp-14.pcap  udp-15.pcap  udp-16.pcap  udp-17.pcap
mergecap -a -s 65535 -F pcap -w all.pcap all-raw.pcap all-eth.pcap all-arp.pcap all-ipv4.pcap all-udp.pcap