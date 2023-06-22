Onel Alexandru 322CB
	Am avut probleme cu checkerul, cand imi functioneaza acestea
	sunt testele trecute. https://imgur.com/a/MMJCFtt
    Am inceput tema prin crearea unui server UDP, ulterior am 
incercat sa fac unul tcp. Dupa ce am inteles cum functioneaza am
cautat despre multiplexare si am ajuns sa folosesc poll pentru
aceasta. Primii pasi pentru server au fost crearea unui socket 
pentru tcp si un socket pentru UDP. Le am dat bind la un file 
descriptor tcpFd si udpFd. Ulterior a inceput partea de multiplexare
mi am pus udpFd, tcpFd si Stdin. Am dezactivat algoritmul lui 
NAGLE pentru TCP. Am verificat daca am eveniment la tastura folosind
poll si pentru tastura citesc inputul si daca nu este "exit" afisez 
"Comanda invalida". Daca este comanda "exit" atunci inchid toate 
conexiunile deschise si eliberez memoria. Pentru cazul de mesaj 
primit pe UDP atunci salvez tot mesajul intr un char* stocare
si extrag topicul pentru a il adauga in array ul stocare. Parcurg
toti clientii abonati la acel topic si le trimit pachetul initial
primi de UDP, iar clientul il va decripta. Daca nu a intrat nici pe
UDP nici pe Stdin atunci caut pe tcp si pe clientii noi adaugati.
Parcurg cu un for pana la dimensiunea clientilor si iau primul 
file descriptor care are eveniment. Daca este cel de tcp inseamna
ca avem o noua conexiune si o accept. Adaugam noul socket al 
clientului in allPollfds si incrementam numarul de clienti.
Se verifica ceea ce primim de pe client_sock si daca nu primim nimic
inseamna ca s a deconectat clientul. Il setam offline in array ul
online si stergem id ul clientului din array ul perechi(tine 
corespondenta id<->fd). Daca e o conexiune noua avem 3 cazuri:
este un client nou, este un client vechi caruia ii actualizez
informatiile sau este un client deja online. Am tratat aceste cazuri
folosind functiile isNewClient() care imi returneaza 1 daca e nou,
functia isOnline() aceasi idee si functia updateClientInfo imi 
actualizeaza in array ul perechi id<->fd. Ulterior am avut cazurile
pentru mesajul primit este de subscribe sau unsubscribe.
Conventia mea a fost ca clientul tcp imi va trimite datele in 
acest format: idClient + 1 octet + struct MesajSubscribe.
Acel 1 octet este setat ori la 1 si reprezinta ca voi avea un mesaj
subscribe, ori pe 2 si reprezinta un mesaj de unsubscribe.
Le am parsat si am apelat functiile de addSubscribe() si unSubscribe().
Aceste functii se folosesc de array ul stocareTopic care 
este de tip struct Stocare si imi retine toate contenturile primite
si id urile abonate la acestea.
Pentru client a fost putin mai simplu, am creat un socket tcpFd, am 
setat portul si adresa ip extrase din argv. M am conectat la server
si am trimis un prim pachet catre server pentru a afisa mesajul 
New client connected.. (ca sa aiba id ul serverul). Apoi am 
dezactivat NAGLE si am folosit tot multiplexare prin poll.
Am adaugat pentru multiplexare doar tcpFd si Stdin. Cand aveam
eveniment pentru tcpFd primeam mesajul exact cum il trimitea cel UDP.
Deci era nevoie de parsarea acestuia. Am extras topicul din mesaj,
am mai extras si tipul si am inceput sa o iau pe cazuri. Prin 
structura MessageInfo am putut sa mi extrag tip ul si topicul, am 
mai avut niste bug uri. Dupa ce am extras datele am afisat mesajul 
corespunzator. Pentru cazul cand am eveniment la tastatura am 
verificat daca am primit "exit" si am inchis conexiunea tcp.
Daca am primit subscribe am obtinut cu ajutorul strtok cuvintele 
citite si mi am creat un packet asemanator cu tema router.
Am pus prima data idClientului, apoi am pus pe pozitia 11 "1" pentru
mesaj de tip subscribe si "2" pentru mesaj de tip unsubscribe.
In functie de caz am pus toata structura MesajSubscribe(subscribe) si 
pentru unsubscribe am pus doar topicul, dar toate send urile au 
aceasi dimensiune deoarece pastrez protocolul meu. 
    Am tratat si toate cazurile de input invalid si toate
alocarile si apelurile de sistem.
