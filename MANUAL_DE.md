# tcppump

tcppump ist ein Packetgeneerator der blabla

Usage

    tcppump -i IFC [OPTIONS] packets
    tcppump -i IFC [OPTIONS] -s scriptfiles

Die zu versendenden Netwerkpakete können sowohl direkt als Programmparameter, als auch über Skriptfiles definiert werden. Möchte man nur einzelne Packete versenden, ist die erst genannte Methode sicherlich die einfachste Möglichkeit. Skriptfiles bieten sich dann an, wenn man ganze Packet-Sequenzen generieren und/oder Packete zeitgesteuert versenden möchte. Sollen Skriptfiles verwendet werden, muss der Parameter `-s` bzw. `--script` gefolgt vom Dateinamen des Skriptfiles angegeben werden. Es können auch mehrere Dateien übergeben werden.

tcppump benötigt immer ein Netzwerkinterface über das die Packete versendet werden sollen. Deshalb ist der Parameter `-i` bzw. `--interface` gefolgt vom Namen des Netzwerkadapters verpflichtend. Unter Windows kann als Name entweder der sog. "Friendly-Name" (ermittelbar via `ipconfig`), als auch GUID des Netzwerkadapters angegeben werden. Unter Linux ist Name des Netzwerk-Devices anzugeben (siehe output von `ip addr`);

Beispiele:

    tcppump -i eth0 "arp(dip=11.22.33.44)"
    tcppump -i "Ethernet 2" "raw(payload=12345678)" "arp(dip=11.22.33.44)"
    tcppump -i WiFi -s myscript.txt
    tcppump -i WiFi -s myscript.txt mystript2.txt

Es ist grundsätzlich eine gute Idee die Definitionen der Netzwerkpackete mit Anführungszeichen zu versehen, da die Definition auch Leerzeichen enthalten könnten und somit fälschlicherweise als mehrere Kommandozeilenparameter erkannt werden.

Die genaue Syntax der Netzwerkpackete (`packets`) und die verschiedenen unterstützten Protokolle sind [hier](./PACKET_REFERENCE.md) definiert.


## Packet Source-Adressen (MAC, IPv4)

tcppump bietet mehrere Möglichkeiten die Absendeadressen zu beeinflussen. Sofern im Packet nicht explizit die Absendeadresse angegeben wurde, verwendet tcppump die Adressen des Netzwerkadapters. Das gilt sowohl für die MAC, als auch die IPv4 Adresse. Die Adressen des Netzwerkadapters können wiederum explizit mit den Parameter `--mymac` und `--mymipv4` überschrieben werden. Die Einstellungen der Netzwerkkarten werden natürlich nicht verändert.

Dadurch ergibt sich folgendes Vergabestrategie (mit absteigender Priorität)

1. Direkt in der Packetdefinition angegebene source adresse(n)
2. Adressen der Parameter `--mymac` und `--mymipv4`
3. Source Adressen des Netzwerkadapters
