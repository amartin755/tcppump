#/usr/bin/bash

mergecap -a -s 65535 -F pcap -w all-raw.pcap raw*.pcap
mergecap -a -s 65535 -F pcap -w all-eth.pcap eth-01.pcap  eth-03.pcap  eth-06.pcap  eth-07.pcap  eth-08.pcap eth-08.pcap eth-11.pcap eth-11.pcap  eth-14.pcap eth-14.pcap  eth-17.pcap  eth-20.pcap  eth-21.pcap  eth-22.pcap  eth-24.pcap  eth-25.pcap eth-25.pcap  eth-27.pcap  eth-28.pcap eth-28.pcap eth-33.pcap  eth-35.pcap  eth-36.pcap  eth-37.pcap  eth-39.pcap eth-40.pcap
mergecap -a -s 65535 -F pcap -w all-arp.pcap arp-01.pcap arp-01.pcap  arp-04.pcap  arp-05.pcap  arp-07.pcap  arp-08.pcap  arp-09.pcap  arp-10.pcap  arp-11.pcap  arp-12.pcap  arp-13.pcap
mergecap -a -s 65535 -F pcap -w all-ipv4.pcap ipv4-04.pcap  ipv4-05.pcap ipv4-04.pcap ipv4-07.pcap ipv4-07.pcap ipv4-10.pcap ipv4-10.pcap  ipv4-13.pcap ipv4-14.pcap ipv4-14.pcap  ipv4-17.pcap  ipv4-19.pcap  ipv4-20.pcap  ipv4-21.pcap  ipv4-22.pcap  ipv4-23.pcap ipv4-24.pcap
mergecap -a -s 65535 -F pcap -w all-udp.pcap udp-05.pcap  udp-06.pcap  udp-08.pcap  udp-10.pcap  udp-11.pcap  udp-13.pcap  udp-14.pcap  udp-15.pcap  udp-16.pcap  udp-17.pcap udp-18.pcap udp-19.pcap udp-20.pcap
mergecap -a -s 65535 -F pcap -w all-vrrp.pcap vrrp-06.pcap vrrp-07.pcap vrrp-06.pcap vrrp-09.pcap vrrp-10.pcap vrrp-10.pcap vrrp-13.pcap vrrp-10.pcap vrrp-15.pcap vrrp-16.pcap vrrp-18.pcap vrrp-19.pcap vrrp-21.pcap vrrp-22.pcap vrrp-23.pcap vrrp-24.pcap
mergecap -a -s 65535 -F pcap -w all-vrrp3.pcap vrrp3-06.pcap vrrp3-07.pcap vrrp3-06.pcap vrrp3-09.pcap vrrp3-10.pcap vrrp3-10.pcap vrrp3-13.pcap vrrp3-10.pcap vrrp3-15.pcap vrrp3-16.pcap vrrp3-18.pcap vrrp3-19.pcap vrrp3-21.pcap vrrp3-22.pcap vrrp3-23.pcap vrrp3-24.pcap
mergecap -a -s 65535 -F pcap -w all.pcap all-raw.pcap all-eth.pcap all-arp.pcap all-ipv4.pcap all-udp.pcap all-vrrp.pcap all-vrrp3.pcap