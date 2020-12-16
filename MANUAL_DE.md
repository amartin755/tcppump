# tcppump

tcppump ist ein Packetgeneerator der blabla

Usage

    tcppump -i IFC [OPTIONS] packets
    tcppump -i IFC [OPTIONS] -s scriptfiles

Die zu versendenden Netwerkpakete k�nnen sowohl direkt als Programmparameter, als auch �ber Skriptfiles definiert werden. M�chte man nur einzelne Packete versenden, ist die erst genannte Methode sicherlich die einfachste M�glichkeit. Skriptfiles bieten sich dann an, wenn man ganze Packet-Sequenzen generieren und/oder Packete zeitgesteuert versenden m�chte. Sollen Skriptfiles verwendet werden, muss der Parameter `-s` bzw. `--script` gefolgt vom Dateinamen des Skriptfiles angegeben werden. Es k�nnen auch mehrere Dateien �bergeben werden.

tcppump ben�tigt immer ein Netzwerkinterface �ber das die Packete versendet werden sollen. Deshalb ist der Parameter `-i` bzw. `--interface` gefolgt vom Namen des Netzwerkadapters verpflichtend. Unter Windows kann als Name entweder der sog. "Friendly-Name" (ermittelbar via `ipconfig`), als auch GUID des Netzwerkadapters angegeben werden. Unter Linux ist Name des Netzwerk-Devices anzugeben (siehe output von `ip addr`);

Beispiele:

    tcppump -i eth0 "arp(dip=11.22.33.44)"
    tcppump -i "Ethernet 2" "raw(payload=12345678)" "arp(dip=11.22.33.44)"
    tcppump -i WiFi -s myscript.txt
    tcppump -i WiFi -s myscript.txt mystript2.txt

Es ist grunds�tzlich eine gute Idee die Definitionen der Netzwerkpackete mit Anf�hrungszeichen zu versehen, da die Definition auch Leerzeichen enthalten k�nnten und somit f�lschlicherweise als mehrere Kommandozeilenparameter erkannt werden.

Die genaue Syntax der Netzwerkpackete (`packets`) und die verschiedenen unterst�tzten Protokolle sind [hier](./PACKET_REFERENCE.md) definiert.


## Packet Source-Adressen (MAC, IPv4)

tcppump bietet mehrere M�glichkeiten die Absendeadressen zu beeinflussen. Sofern im Packet nicht explizit die Absendeadresse angegeben wurde, verwendet tcppump die Adressen des Netzwerkadapters. Das gilt sowohl f�r die MAC, als auch die IPv4 Adresse. Die Adressen des Netzwerkadapters k�nnen wiederum explizit mit den Parameter `--mymac` und `--mymipv4` �berschrieben werden. Die Einstellungen der Netzwerkkarten werden nat�rlich nicht ver�ndert.

Dadurch ergibt sich folgendes Vergabestrategie (mit absteigender Priorit�t)

1. Direkt in der Packetdefinition angegebene source adresse(n)
2. Adressen der Parameter `--mymac` und `--mymipv4`
3. Source Adressen des Netzwerkadapters
