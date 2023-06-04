# Projekt: Server pre vzdialenú kalkulačku (IPK Calculator)

Tento projekt implementuje server pre IPK Calculator Protocol, ktorý komunikuje s klientmi pomocou protokolov TCP alebo
UDP.

## Inštalácia

1. Skopírujte zdrojový kód, `Makefile` a tento README súbor do vášho projektového priečinka.
2. Otvorte terminál a prejdite do priečinka s projektom: `cd /path/to/project/folder`
3. Skompilujte projekt pomocou príkazu `make` (Týmto krokom vytvoríte spustiteľný súbor `ipkcpd`.)

## Spustenie servera

Spustite server s nasledujúcimi parametrami: `./ipkcpd -h <host> -p <port> -m <mode>`
Kde:

- `<host>`: IP adresa alebo doménové meno servera
- `<port>`: Port, na ktorom bude server počúvať
- `<mode>`: Môže byť "tcp" alebo "udp" pre vybratý prenosový protokol

Príklad: `./ipkcpd -h 127.0.0.1 -p 8080 -m tcp`

## Pripojenie k serveru

1. Ako klient sa pripojte k serveru pomocou nástrojov ako `telnet` alebo `nc` pre TCP alebo `nc` pre UDP:
   Pre TCP: `telnet <host> <port>` alebo `nc <host> <port>`
2. Pre UDP: `nc -u <host> <port>`  
   Po úspešnom pripojení môžete interagovať so serverom pomocou IPKCP protokolu.

## Implementácia

Server je napísaný v jazyku C a využíva nasledujúce knižnice:

- `<stdio.h>`  
  Štandardná knižnica vstupu/výstupu v jazyku C. Obsahuje funkcie pre prácu s reťazcami, súbormi,
  čítanie a zápis na štandardný vstup/výstup (stdin, stdout).
- `<stdlib.h>`  
  Štandardná knižnica pre všeobecné účely, ktorá poskytuje funkcie pre dynamickú alokáciu pamäte,
  konverziu reťazcov, generovanie náhodných čísel a spracovanie argumentov príkazového riadka.
- `<arpa/inet.h>`  
  Táto knižnica obsahuje funkcie a definície pre prácu s internetovými adresami
  (IPv4 a IPv6). Poskytuje funkcie ako inet_pton a inet_ntop, ktoré prevádzajú
  reťazce na štruktúry adries a naopak.
- `<netinet/in.h>`  
  Knižnica poskytuje definície a štruktúry pre adresy
  internetových domén (IPv4 a IPv6) a ich porty. Definuje štruktúry ako sockaddr_in a sockaddr_in6.
- `<pthread.h>`  
  Knižnica pre prácu s POSIX vláknami (pthreads).
  Umožňuje vytvárať a manipulovať s vláknami, ako aj s mutexmi a podmienkami.
- `<signal.h>`  
  Táto knižnica poskytuje funkcie a makrá na prácu so signálmi. Umožňuje definovať,
  ako váš program zareaguje na príchod signálov (napr. SIGINT).
- `<stdbool.h>`  
  Knižnica pre definíciu dátového typu bool, ktorý reprezentuje pravdivostné hodnoty true a false.
- `<string.h>`  
  Knižnica obsahuje funkcie pre manipuláciu a porovnávanie reťazcov, ako sú strcpy, strncpy, strcmp, strncmp, strcat,
  strncat, strlen atď.
- `<sys/socket.h>`  
  Knižnica poskytuje funkcie, definície a štruktúry pre vytváranie, používanie a spracovávanie socketov. Umožňuje
  vytvorenie, pripojenie, odpojenie a komunikáciu cez sockety.
- `<sys/types.h>`  
  Knižnica obsahuje základné dátové typy a konštanty používané v systémových volaniach a štruktúrach.
- `<unistd.h>`  
  Knižnica poskytuje funkcie a konštanty pre POSIX operačné systémy. Obsahuje funkcie ako read, write, close, getpid,
  fork a ďalšie.
- `<ctype.h>`  
  Knižnica poskytuje funkcie na klasifikáciu a manipuláciu so znakmi.

Implementácia servera zahŕňa nasledovné technológie a koncepty:

- **TCP** - spoľahlivý, orientovaný na spojenie protokol pre prenos dát medzi sieťovými zariadeniami
- **UDP** - nespoľahlivý, bez spojenia protokol pre prenos dát medzi sieťovými zariadeniami
- **Sokety** - abstrakcia komunikácie medzi dvoma sieťovými zariadeniami, v tomto projekte použité na vytvorenie servera
  a prijímanie pripojení od klientov
- **Vlákna (pthreads)** - pre paralelné vykonávanie úloh a spracovanie viacerých klientov súčasne

Server je navrhnutý tak, aby mohol súčasne spracovávať viacero klientov pomocou vlákien (`pthreads`). V závislosti od
zvoleného módu (TCP alebo UDP) server používa buď textový alebo binárny variant protokolu.


## TCP a UDP Sokety

TCP (Transmission Control Protocol) a UDP (User Datagram Protocol) sú dva základné transportné protokoly používané v
internetových komunikáciách.

### TCP Sokety

TCP je spojový protokol, ktorý poskytuje spoľahlivé, usporiadané a kontrolu toku dát medzi aplikáciami. TCP zaisťuje, že
dáta prídu na svoje miesto bez chýb a v správnom poradí. TCP je vhodný pre aplikácie, ktoré vyžadujú vysokú spoľahlivosť
komunikácie.

### UDP Sokety

UDP je nespojový protokol, ktorý nezaručuje doručenie dát, poradie alebo kontrolu toku dát. UDP je vhodný pre aplikácie,
ktoré vyžadujú rýchle prenosy a menej prenosovej spoľahlivosti.

### Pthreads

Pthreads (POSIX threads) sú štandardom pre viacvláknové programovanie v jazyku C na platformách podporujúcich POSIX.
Pthreads umožňujú vytvárať, spravovať a synchronizovať vlákna.

Opis implementácie
---------------------

- Funkcia `main`: Inicializuje server a počúva prichádzajúce spojenia.
- Funkcia `evaluate_expression`: Vyhodnocuje zadaný matematický výraz v reťazci
- Funkcia `evaluate_operator`: Vyhodnocuje zadaný operátor a jeho dva operandy
- Funkcia `process_textual_request`: Spracováva textový formát požiadavky a generuje odpoveď
- Funkcia `process_binary_request`: Spracováva binárny formát požiadavky a generuje odpoveď
- Funkcia `client_handler`: Spracováva klienta a jeho požiadavky
- Funkcia `handle_sigint`: Obsluhuje signál SIGINT a nastavuje príznak pre ukončenie servera

Testovanie
-------------

Testovanie bolo vykonané na nasledujúcom prostredí:

- OS: Ubuntu 20.04
- Kompilátor: GCC 9.3.0

Zdroje
---------
Primárnym zdrojom mi boli slidy k prednáškam,  
dokumentácie k použitým knižniciam a  
RFC dokumenty týkajúce sa TCP a UDP protokolov
Zoznam sekundárnych zdrojov je uvedený nižšie:

Zoznam sekundárnych zdrojov:

- TCP komunikácia: https://www.geeksforgeeks.org/tcp-connection-termination/
- UDP komunikácia: https://www.geeksforgeeks.org/user-datagram-protocol-udp/
- Multithreading in C: https://www.geeksforgeeks.org/multithreading-in-c/
- Socket Programming in C/C++: https://www.geeksforgeeks.org/socket-programming-cc/

## Testovacie prípady

1. **Testovacia situácia:** Platný výraz, ktorý obsahuje súčet dvoch celých čísel.

- **Vstupy:** "1+2"
- **Očakávaný výstup:** 3

2. **Testovacia situácia:** Platný výraz, ktorý obsahuje rozdiel dvoch celých čísel.

- **Vstupy:** "5-3"
- **Očakávaný výstup:** 2

3. **Testovacia situácia:** Platný výraz, ktorý obsahuje súčin dvoch celých čísel.

- **Vstupy:** "2*3"
- **Očakávaný výstup:** 6

4. **Testovacia situácia:** Platný výraz, ktorý obsahuje podiel dvoch celých čísel.

- **Vstupy:** "6/2"
- **Očakávaný výstup:** 3

5. **Testovacia situácia:** Platný výraz so zátvorkami.

- **Vstupy:** "(2+3)*4"
- **Očakávaný výstup:** 20

6. **Testovacia situácia:** Neplatný výraz so zlým operátorom.

- **Vstupy:** "1#2"
- **Očakávaný výstup:** Chyba výrazu

7. **Testovacia situácia:** Neplatný výraz so zlým umiestnením zátvorky.

- **Vstupy:** "2+3)*4"
- **Očakávaný výstup:** Chyba výrazu

8. **Testovacia situácia:** Delenie nulou.

- **Vstupy:** "4/0"
- **Očakávaný výstup:** Chyba delenia nulou

9. **Testovacia situácia:** Platný výraz s viacerými operátormi a zátvorkami.

- **Vstupy:** "((2+3)*4)-(7*2)"
- **Očakávaný výstup:** 6

10. **Testovacia situácia:** Platný výraz s veľkými číslami.

- **Vstupy:** "12345+67890"
- **Očakávaný výstup:** 80235

11. **Testovacia situácia:** Spustenie servera v TCP režime na platnom porte.

- **Vstupy:** Spustenie servera s argumentmi: 12345 tcp
- **Očakávaný výstup:** Server started on port 12345 (TCP mode)

12. **Testovacia situácia:** Spustenie servera v UDP režime na platnom porte.

- **Vstupy:** Spustenie servera s argumentmi: 12345 udp
- **Očakávaný výstup:** Server started on port 12345 (UDP mode)

13. **Testovacia situácia:** Spustenie servera s neplatným režimom.

- **Vstupy:** Spustenie servera s argumentmi: 12345 abc
- **Očakávaný výstup:** Chybová správa o neplatnom režime

14. **Testovacia situácia:** Spustenie servera s neplatným portom.

- **Vstupy:** Spustenie servera s argumentmi: 70000 tcp
- **Očakávaný výstup:** Chybová správa o neplatnom porte

15. **Testovacia situácia:** Pripojenie klienta k serveru v TCP režime.

- **Vstupy:** Server beží na porte 12345 v TCP režime, klient sa pokúsi pripojiť na port 12345
- **Očakávaný výstup:** Klient úspešne pripojený, server spracováva požiadavky klienta

16. **Testovacia situácia:** Pripojenie klienta k serveru v UDP režime.

- **Vstupy:** Server beží na porte 12345 v UDP režime, klient sa pokúsi pripojiť na port 12345
- **Očakávaný výstup:** Klient úspešne pripojený, server spracováva požiadavky klienta

17. **Testovacia situácia:** Pripojenie viacerých klientov súčasne k serveru v TCP režime.

- **Vstupy:** Server beží na porte 12345 v TCP režime, viacerí klienti sa pokúšajú pripojiť na port 12345
- **Očakávaný výstup:** Všetci klienti úspešne pripojení, server spracováva požiadavky klientov paralelne

18. **Testovacia situácia:** Pripojenie viacerých klientov súčasne k serveru v UDP režime.

- **Vstupy:** Server beží na porte 12345 v UDP režime, viacerí klienti sa pokúšajú pripojiť na port 12345
- **Očakávaný výstup:** Všetci klienti úspešne pripojení, server spracováva požiadavky klientov paralelne

19. **Testovacia situácia:** Ukončenie servera pomocou Ctrl+C v TCP režime.

- **Vstupy:** Server beží na porte 12345 v TCP režime, užívateľ stlačí Ctrl+C
- **Očakávaný výstup:** Správa "Server has been terminated." a server sa ukončí

20. **Testovacia situácia:** Ukončenie servera pomocou Ctrl+C v UDP režime.

- **Vstupy:** Server beží na porte 12345 v UDP režime, užívateľ stlačí Ctrl+C
- **Očakávaný výstup:** Správa "Server has been terminated." a server sa ukončí

## Licencia

Tento projekt je chránený licenciou, pozrite si súbor [LICENSE](LICENSE) pre viac informácií.

## Zmeny a obmedzenia

Podrobnosti o zmene a obmedzeniach nájdete v súbore [CHANGELOG.md](CHANGELOG.md).