#include <stdlib.h>

#include <unistd.h>

#include <string.h>

#include <sys/types.h>

#include <sys/socket.h>

#include <arpa/inet.h>

#include <netinet/in.h>

#include <stdio.h>

#include <poll.h>

#include <netinet/tcp.h>

// structura pentru trimiterea datelor catre server

struct MesajSubscribe

{

    char topic[50];

    char sf;

};

// structura pentru parsarea datelor primite

struct MessageInfo

{

    char topic[50];

    char dataType;

};

int main(int argc, char *argv[])

{

    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    if (argc < 4)

    {

        printf("Clientul are nevoie de ID,IP,PORT!\n");

        return -1;

    }

    // id ul primit parametru

    char idClient[11];

    strcpy(idClient, argv[1]);

    // ip ul primit parametru

    char ip[INET_ADDRSTRLEN];

    strcpy(ip, argv[2]);

    // port ul primit parametru

    int port = atoi(argv[3]);

    // socket pentru conexiunea TCP

    int tcpFd;

    struct sockaddr_in server_addr;

    // char message[50]; ///POSIBIL STRICACIOS



    // socket pentru TCP

    tcpFd = socket(AF_INET, SOCK_STREAM, 0);

    if (tcpFd < 0)

    {

        printf("Eroare socket!\n");

        return -1;

    }



    //  initializam datele pentru server_addr

    memset(&server_addr, 0, sizeof(server_addr));

    // folosim IPV4

    server_addr.sin_family = AF_INET;

    server_addr.sin_addr.s_addr = inet_addr(ip);

    server_addr.sin_port = htons(port);

    // am folosit poll pentru multiplexare

    // am pus sa asculte pe tcpFd si pe STDIN adica mesajele scrise in consola

    struct pollfd allPollFds[3];

    allPollFds[0].events = POLLIN;

    allPollFds[0].fd = tcpFd;

    allPollFds[1].events = POLLIN;

    allPollFds[1].fd = STDIN_FILENO;



    // socketul este conectat la server

    if (connect(tcpFd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)

    {

        printf("Eroare conectare\n");

        return -1;

    }

    // trimit un pachet cu id ul meu ca prim pachet

    char *firstPacket = calloc(1, 1 + sizeof(struct MesajSubscribe) + sizeof(idClient));

    memcpy(firstPacket, idClient, sizeof(idClient));

    if (send(tcpFd, firstPacket, sizeof(struct MesajSubscribe) + 1 + sizeof(idClient), 0) < 0)

    {

        printf("Eroare send");

        return -1;

    }

    // dezactivare NAGLE

    int flag = 1;

    int result = setsockopt(tcpFd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag,

                            sizeof(int));

    while (1)

    {

        // poll are 2 pentru nfds pentru ca am doar STDIN si tcpFd

        if (poll(&allPollFds, 2, 0) == -1)

        {

            printf("Poll malfunction!\n");

            return -1;

        }

        // evenimentul pentru tcpFd

        if (allPollFds[0].revents & POLLIN)

        {

            char stocare[1551];

            // retRecv este numarul de octeti citit

            int retRecv = recv(tcpFd, stocare, sizeof(stocare), 0);

            // cazul de eroare

            if (retRecv == -1)

            {

                printf("Eroare la primirea mesajului!\n");

                return -1;

            }

            if (retRecv == 0)

            {

                // inchid clientul

                close(tcpFd);

                return 0;

            }

            // partea de decodare a mesajului

            char topic[50];

            int cnt = 0;

            while (cnt != 50)

            {

                topic[cnt] = stocare[cnt];

                cnt++;

            }

            char tipChar = stocare[cnt];

            int tip = (int)tipChar;

            cnt++;



            char continut[1500];

            int cntContinut = 0;

            while (cnt < 1552)

            {

                continut[cntContinut] = stocare[cnt];

                cnt++;

                cntContinut++;

            }



            struct MessageInfo *message = (struct MessageInfo *)stocare;

            // Cazurile de decodare a mesajului in functie de tipul acestuia

            if (tip == 0)

            {

                // Cazul pentru INT

                char *semn = stocare + sizeof(struct MessageInfo);

                uint32_t *numar = stocare + sizeof(struct MessageInfo) + 1;

                uint32_t numarFinal = ntohl(*numar);



                if (*semn == 1)

                    numarFinal *= -1;



                char ip_str[INET_ADDRSTRLEN];

                inet_ntop(AF_INET, &(server_addr.sin_addr), ip_str, INET_ADDRSTRLEN);

                printf("%s:%d - %s - INT - %d\n", ip_str, ntohs(server_addr.sin_port), message->topic, numarFinal);

            }

            if (tip == 1)

            {

                // Cazul pentru SHORT_REAL

                char *str = malloc(3);

                memcpy(str, stocare + sizeof(struct MessageInfo) + 1, 1);

                memcpy(str + 1, stocare + sizeof(struct MessageInfo), 1);

                uint16_t num;

                memcpy(&num, str, sizeof(uint16_t));

                float numFinal = ((float)num / 100);



                char ip_str[INET_ADDRSTRLEN];

                inet_ntop(AF_INET, &(server_addr.sin_addr), ip_str, INET_ADDRSTRLEN);

                printf("%s:%d - %s - SHORT_REAL - %.2f\n", ip_str, ntohs(server_addr.sin_port), message->topic, numFinal);

            }

            if (tip == 2)

            {

                // Cazul pentru FLOAT

                char *semn = stocare + sizeof(struct MessageInfo);

                uint32_t *numarPointer = stocare + sizeof(struct MessageInfo) + 1;

                uint8_t *virgulaPointer = stocare + sizeof(struct MessageInfo) + 1 + sizeof(uint32_t);



                float numar = (float)ntohl(*numarPointer);

                if (*semn == 1)

                    numar *= -1;

                uint8_t virgula = *virgulaPointer;

                u_int8_t copieVirgula = virgula;



                while (copieVirgula != 0)

                {

                    numar /= 10;

                    copieVirgula--;

                }



                char ip_str[INET_ADDRSTRLEN];

                inet_ntop(AF_INET, &(server_addr.sin_addr), ip_str, INET_ADDRSTRLEN);

                printf("%s:%d - %s - FLOAT - %.*f\n", ip_str, ntohs(server_addr.sin_port), message->topic, virgula, numar);

            }

            if (tip == 3)

            {

                /// Cazul cand este string

                char *mesaj;

                mesaj = stocare + sizeof(struct MessageInfo);



                char ip_str[INET_ADDRSTRLEN];

                inet_ntop(AF_INET, &(server_addr.sin_addr), ip_str, INET_ADDRSTRLEN);

                printf("%s:%d - %s - STRING - %s\n", ip_str, ntohs(server_addr.sin_port), message->topic, mesaj);

            }

        }

        if (allPollFds[1].revents & POLLIN)

        {

            /// Primesc de la tastatura

            char inputStdin[100];

            fgets(inputStdin, sizeof(inputStdin), stdin);

            if (strcmp("exit\n", inputStdin) == 0)

            {

                // inchid conexiunea tcp

                close(tcpFd);

                return 0;

            }

            else

            {

                // copie inputStdin deoarece strtok distruge sursa

                char *cpinputStdin = malloc(sizeof(inputStdin));

                memcpy(cpinputStdin, inputStdin, sizeof(inputStdin));

                // primul cuvant din propozitia separata cu

                // delimitatorul spatiu

                char *primul = strtok(cpinputStdin, " ");

                if (strcmp(primul, "subscribe") == 0)

                {

                    char *topic = strtok(NULL, " ");

                    char *sf = strtok(NULL, " ");



                    struct MesajSubscribe *mesaj = malloc(sizeof(struct MesajSubscribe));

                    mesaj->sf = atoi(sf);

                    memcpy(mesaj->topic, topic, strlen(topic));

                    // creez pachetul pe care il voi trimite

                    // packet va contine idClient + 1/2(in functie de tipul

                    // mesajului) + structura MesajSubscribe

                    char *packet = calloc(1, 1 + sizeof(struct MesajSubscribe) + sizeof(idClient));

                    // copiez idClient in pachet

                    memcpy(packet, idClient, sizeof(idClient));

                    // setez pe pozitia 11 din packet cu valoarea 1 care

                    // reprezinta faptul ca e un mesaj de tip subscribe

                    packet[11] = 1;

                    // adaug structura MesajSubscribe formata din inputul de

                    // la tastatura

                    memcpy((void *)(packet + 1 + sizeof(idClient)), (void *)mesaj, sizeof(struct MesajSubscribe));

                    // trimit pachetul si verific daca a fost trimis

                    if (send(tcpFd, packet, sizeof(struct MesajSubscribe) + 1 + sizeof(idClient), 0) < 0)

                    {

                        printf("Eroare send!\n");

                        return -1;

                    }

                    printf("Subscribed to topic.\n");

                }

                else if (strcmp(primul, "unsubscribe") == 0)

                {

                    // Parsez mesajul de unsubscribe

                    char *topic = strtok(NULL, " ");

                    // aloc pachetul pe care il trimit

                    char *packet = calloc(1, 1 + sizeof(struct MesajSubscribe) + sizeof(idClient));

                    // pun id ul in packet

                    memcpy(packet, idClient, sizeof(idClient));

                    // setez pozitia 11 cu valoarea 2 pt a arata

                    // ca e de tip unsubscribe

                    packet[11] = 2;

                    // adaug doar topicul pentru mesajul de tip unsubscribe

                    memcpy((void *)(packet + 1 + sizeof(idClient)), (void *)topic, 50);

                    // verific daca send ul a functionat sau are eroare

                    if (send(tcpFd, packet, sizeof(struct MesajSubscribe) + 1 + sizeof(idClient), 0) < 0)

                    {

                        printf("Eroare send!\n");

                        return -1;

                    }

                    printf("Unsubscribed from topic.\n");

                }

                else

                {

                    printf("Comanda invalida!\n");

                }

            }

        }

    }

    return 0;

}
