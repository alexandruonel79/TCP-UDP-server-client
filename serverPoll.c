#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define CONEXIUNI 100

#define DIM_PACHET 1551

// aceasi structura cu cea din client

// pentru a parsa mesajul de tip subscribe

struct MesajSubscribe

{
  char topic[51];

  char sf;
};

// corespondenta dintre id si file descriptor

struct Pereche

{
  char id[11];

  int fd;
};

// structura retine topic urile, content urile si id urile abonatilor

struct Stocare

{
  char topic[51];

  char content[CONEXIUNI][DIM_PACHET];

  int nrContent;

  char idAbonati[CONEXIUNI][12];

  int cntAbonati;
};

struct Pereche perechi[CONEXIUNI];

int cntPerechi = 0;

char online[65535][12];

int cntOnline = 0;

struct Stocare stocareTopic[600];

int cntStocare = 0;

// functia imi returneaza file descriptorul corespunzator

// clientului din variabila perechi

int getFd(char *idClient)

{
  for (int i = 0; i < cntPerechi; i++)

  {
    if (strncmp(perechi[i].id, idClient, strlen(idClient)) == 0)

    {
      return perechi[i].fd;
    }
  }

  return -1;
}

// functia initializeaza contoarele din stocareTopic cu 0

void initStocare()

{
  for (int i = 0; i < CONEXIUNI; i++)

  {
    stocareTopic[i].cntAbonati = 0;

    stocareTopic[i].nrContent = 0;
  }
}

// functia adauga un abonament nou

void addSubscribe(char *topic, char *idClient)

{
  int exists = 0;

  for (int i = 0; i <= cntStocare; i++)

  {
    stocareTopic[i].topic[51] = '\0';

    topic[51] = '\0';

    if (strcmp(stocareTopic[i].topic, topic) == 0)

    {
      for (int j = 0; j < stocareTopic[i].cntAbonati; j++)

      {
        if (strcmp(stocareTopic[i].idAbonati[j], idClient) == 0)

        {
          return;
        }
      }

      // ma abonez la topic

      exists = 1;

      memcpy(stocareTopic[i].idAbonati[stocareTopic[i].cntAbonati], idClient,
             11);

      stocareTopic[i].idAbonati[stocareTopic[i].cntAbonati][12] = '\0';

      stocareTopic[i].cntAbonati++;
    }
  }

  if (exists == 0)

  {
    // acest topic nu exista deci il adaug

    memcpy(stocareTopic[cntStocare].topic, topic, 50);

    memcpy(
        stocareTopic[cntStocare].idAbonati[stocareTopic[cntStocare].cntAbonati],
        idClient, 11);

    stocareTopic[cntStocare]
        .idAbonati[stocareTopic[cntStocare].cntAbonati][12] = '\0';

    stocareTopic[cntStocare].cntAbonati = 1;

    cntStocare++;
  }
}

// functia elimina un abonament

void unSubscribe(char *topic, char *idClient)

{
  for (int i = 0; i < cntStocare; i++)

  {
    // verific daca este topic ul cautat

    if (strncmp(stocareTopic[i].topic, topic, 50) == 0)

    {
      for (int j = 0; j < stocareTopic[i].cntAbonati; j++)

      {
        // verific daca este idClient

        if (strncmp(stocareTopic[i].idAbonati[j], idClient, 11) == 0)

        {
          // deplaseaza toate elementele peste client_sock

          for (int h = j + 1; h < stocareTopic[i].cntAbonati; h++)

          {
            memcpy(stocareTopic[i].idAbonati[h - 1],
                   stocareTopic[i].idAbonati[h], 11);
          }

          stocareTopic[i].cntAbonati--;

          break;
        }
      }
    }
  }
}

// functia adauga content in variabila stocare

void addContent(char *topic, char *content)

{
  int exists = 0;

  for (int i = 0; i < cntStocare; i++)

  {
    if (strncmp(stocareTopic[i].topic, topic, 50) == 0)

    { // topicul deja exista, adaug mai mult content

      exists = 1;

      memcpy(stocareTopic[i].content[stocareTopic[i].nrContent], content,
             strlen(content));

      stocareTopic[i].nrContent++;
    }
  }

  if (exists == 0)

  {
    // topicul nu exista, il creez si adaug contentul

    memcpy(stocareTopic[cntStocare].topic, topic, strlen(topic));

    memcpy(stocareTopic[cntStocare].content[stocareTopic[cntStocare].nrContent],
           content, strlen(content));

    stocareTopic[cntStocare].nrContent++;
  }

  cntStocare++;
}

// functia seteaza un client primind un id offline

void setOffline(char *idClient)

{
  int indexElim = -1;

  for (int i = 0; i < cntOnline; i++)

  {
    if (strncmp(online[i], idClient, 11) == 0)

    {
      indexElim = i;

      break;
    }
  }

  if (indexElim != -1)

  {
    // sterg id ul din online

    for (int i = indexElim; i < cntOnline - 1; i++)

    {
      strncpy(online[i], online[i + 1], 11);
    }

    cntOnline--;
  }
}

// setez un idClient online

void setOnline(char *idClient)

{
  int found = -1;

  for (int i = 0; i < cntOnline; i++)

  {
    if (strncmp(online[i], idClient, 11) == 0)

    {
      found = 1;

      break;
    }
  }

  if (found == -1)

  {
    memcpy(online[cntOnline], idClient, 11);

    online[cntOnline][12] = '\0';

    cntOnline++;
  }
}

// functia verifica daca un client e online

// cauta in online si daca gaseste inseamna ca e online

int isOnline(char *idClient)

{
  int found = -1;

  for (int i = 0; i < cntOnline; i++)

  {
    if (strncmp(online[i], idClient, 11) == 0)

    {
      found = 1;

      break;
    }
  }

  if (found == 1)

  {
    return 1;
  }

  else

  {
    return 0;
  }
}

// elimina un fd si intoarce id ul

char *removeFd(int client_sock)

{
  for (int i = 0; i < cntPerechi; i++)

  {
    if (perechi[i].fd == client_sock)

    {
      perechi[i].fd = -1;

      return perechi[i].id;
    }
  }
}

// verifica daca e un client nou

// cauta in perechi daca gaseste corespondentul

int isNewClient(char *idClient)

{
  for (int i = 0; i < cntPerechi; i++)

  {
    if (strcmp(idClient, perechi[i].id) == 0)

    {
      return 0;
    }
  }

  return 1;
}

// actualizeaza perechea client_sock si idClient

void updateClientInfo(char *idClient, int client_sock)

{
  for (int i = 0; i < cntPerechi; i++)

  {
    if (strcmp(idClient, perechi[i].id) == 0)

    {
      perechi[i].fd = client_sock;

      return;
    }
  }
}

// functia sterge o pereche primind un clientId

void removeId(char *clientId)

{
  int i, j;

  for (i = 0; i < cntPerechi; i++)

  {
    if (strcmp(perechi[i].id, clientId) == 0)

    {
      // am gasit deci mutam totul la stanga

      for (j = i; j < cntPerechi - 1; j++)

      {
        perechi[j] = perechi[j + 1];
      }

      cntPerechi--;

      break;
    }
  }
}

int main(int argc, char *argv[])

{
  setvbuf(stdout, NULL, _IONBF, BUFSIZ);

  // initializez structura stocare, int urile le setez cu 0

  initStocare();

  // daca primeste doar un argument afisez mesajul de eroare

  if (argc == 1)

  {
    printf("Serverul are nevoie de un PORT!\n");

    return -1;
  }

  uint16_t port = atoi(argv[1]);

  char *stocare = malloc(DIM_PACHET * sizeof(char));

  // daca alocarea e nereusita afisez mesaj de eroare

  if (stocare == NULL)

  {
    printf("Malloc stocare failed!\n");

    return -1;
  }

  struct sockaddr_in server_addr, client_addr;

  // initializam datele pentru server_addr

  server_addr.sin_family = AF_INET;

  server_addr.sin_port = ntohs(port);

  server_addr.sin_addr.s_addr = INADDR_ANY;

  // creez socket de tip UDP

  int udpFd = socket(AF_INET, SOCK_DGRAM, 0);

  // verific daca a reusit

  if (udpFd == -1)

  {
    printf("Server failed to create UDP socket\n");

    return -1;
  }

  // creez socket de tip TCP

  int tcpFd = socket(AF_INET, SOCK_STREAM, 0);

  // verific daca a reusit

  if (tcpFd == -1)

  {
    printf("Server failed to create TCP socket\n");

    return -1;
  }

  // asociez socketul UDP cu server_addr

  int bindUdp = bind(udpFd, (const struct sockaddr *)&server_addr,

                     sizeof(server_addr));

  // verific daca a reusit bind ul

  if (bindUdp == -1)

  {
    printf("Server failed to bind to UDP socket\n");

    return -1;
  }

  // asociez socketul TCP cu server_addr

  int bindTcp = bind(tcpFd, (const struct sockaddr *)&server_addr,

                     sizeof(server_addr));

  // verific daca a reusit bind ul

  if (bindTcp == -1)

  {
    printf("Server failed to bind to TCP socket\n");

    return -1;
  }

  // ascult pe socketul tcp cu un numar maxim de conexiuni

  if (listen(tcpFd, CONEXIUNI) == -1)
    printf("Eroare listen!\n");

  // incepe de la doi deoarece sar peste udpFd,tcpFd si STDIN

  int numarClientiTcp = 2;

  // salvez in allPollFds toti fd pentru multiplexare

  struct pollfd allPollFds[CONEXIUNI];

  allPollFds[0].events = POLLIN;

  allPollFds[0].fd = udpFd;

  allPollFds[1].events = POLLIN;

  allPollFds[1].fd = STDIN_FILENO;

  allPollFds[2].events = POLLIN;

  allPollFds[2].fd = tcpFd;

  // dezactivare NAGLE

  int flag = 1;

  int result = setsockopt(tcpFd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag,

                          sizeof(int));

  while (1)

  {
    if (poll(&allPollFds, numarClientiTcp + 1, 0) == -1)

    {
      printf("Poll malfunction!\n");

      return -1;
    }

    // verific daca e eveniment la tastatura

    if (allPollFds[1].revents & POLLIN)

    {
      char inputStdin[100];

      fgets(inputStdin, sizeof(inputStdin), stdin);

      if (strcmp("exit\n", inputStdin) == 0)

      {
        // inchid toate conexiunile dechise

        for (int i = 0; i <= numarClientiTcp; i++)

        {
          if (allPollFds[i].fd != -1 && allPollFds[i].fd != 0)

            close(allPollFds[i].fd);
        }

        // eliberez memoria

        free(stocare);

        return 0;
      }

      else

      {
        printf("Comanda invalida!\n");
      }
    }

    else if (allPollFds[0].revents & POLLIN)

    {
      // Procesez mesajele UDP

      memset(stocare, 0, DIM_PACHET);

      int length = recv(udpFd, (char *)stocare, DIM_PACHET, 0);

      char topic[51];

      int cnt = 0;

      while (cnt != 50)

      {
        topic[cnt] = stocare[cnt];

        cnt++;
      }

      topic[51] = '\0';

      // adaug contentul in stocare

      addContent(topic, stocare);

      /// am primit topicul si acum trimit tot mesajul

      // catre clientii abonati

      for (int i = 0; i < cntStocare; i++)

      {
        if (topic != NULL && stocareTopic[i].topic != NULL &&
            strncmp(stocareTopic[i].topic, topic, 50) == 0)

        {
          for (int j = 0; j < stocareTopic[i].cntAbonati; j++)

          {
            // iau toate fd urile corespunzatoare id urilor abonatilor

            int fdSend = getFd(stocareTopic[i].idAbonati[j]);

            if (fdSend != -1)

            {
              if (send(fdSend, stocare, DIM_PACHET, 0) == -1)

                printf("Eroare send!\n");
            }
          }
        }
      }
    }

    // caut daca e vreun tcpfd care are ceva nou primit

    int fileDescriptor = -1;

    for (int i = 2; i <= numarClientiTcp; i++)

    {
      if (allPollFds[i].revents & POLLIN)

      {
        // il aleg pe primul activ

        fileDescriptor = allPollFds[i].fd;

        break;
      }
    }

    if (fileDescriptor != -1)

    {
      // Procesez cazul de TCP

      socklen_t addr_size = sizeof(client_addr);

      int client_sock = fileDescriptor;

      int conexiuneNoua = 0;

      if (fileDescriptor == tcpFd)

      {
        // daca am primit pe tcpFd inseamna ca e doar de

        // acceptare a conexiunii

        // de acolo lucrez cu socketul dedicat special

        // clientului

        if (fileDescriptor == tcpFd)

          client_sock = accept(fileDescriptor, (struct sockaddr *)&client_addr,
                               &addr_size);

        if (client_sock == -1)

        {
          printf("Error accepting client connection\n");

          return -1;
        }

        // adaugam un nou client

        conexiuneNoua = 1;

        numarClientiTcp++;

        allPollFds[numarClientiTcp].events = POLLIN;

        allPollFds[numarClientiTcp].fd = client_sock;
      }

      // aici voi salva id ul Clientului

      char *idClient = malloc(11);

      if (idClient == NULL)

      {
        printf("Malloc idCLient failed!\n");

        return -1;
      }

      // aici voi salva tot mesajul primit

      char *mesajTcp = malloc(sizeof(struct MesajSubscribe) + 1 + 11);

      if (mesajTcp == NULL)

      {
        printf("Malloc mesajTcp failed!\n");

        return -1;
      }

      int raspRecv = recv(client_sock, mesajTcp,
                          sizeof(struct MesajSubscribe) + 1 + 11, 0);

      if (raspRecv == -1)
        printf("EROARE RECV\n");

      // extrag id ul din mesajTcp daca recv a functionat

      if (raspRecv > 0)
        memcpy(idClient, mesajTcp, 11);

      // daca raspunsul este egal cu 0 inseamna ca s a dus conexiunea

      if (raspRecv == 0)

      {
        strcpy(idClient, removeFd(client_sock));

        printf("Client %s disconnected.\n", idClient);

        // clientul devine offline

        setOffline(idClient);

        // sterg id ul clientului

        removeId(idClient);

        // inchid socketul pentru client

        close(client_sock);

        for (int i = 3; i <= numarClientiTcp; i++)

        {
          if (allPollFds[i].fd == client_sock)

          {
            // deplaseaza toate elementele peste client_sock

            for (int j = i + 1; j <= numarClientiTcp; j++)

            {
              allPollFds[j - 1] = allPollFds[j];
            }

            numarClientiTcp--;

            break;
          }
        }
      }

      if (conexiuneNoua == 1)

      {
        // verific daca e client nou

        if (isNewClient(idClient) == 1)

        {
          // adaug perechea id fd in perechi

          perechi[cntPerechi].fd = client_sock;

          strcpy(perechi[cntPerechi].id, idClient);

          cntPerechi++;

          char ip_str[INET_ADDRSTRLEN];

          inet_ntop(AF_INET, &(client_addr.sin_addr), ip_str, INET_ADDRSTRLEN);

          printf("New client %s connected from %s:%d.\n", idClient, ip_str,
                 ntohs(client_addr.sin_port));

          // il setez online

          setOnline(idClient);

          continue;
        }

        else

        {
          // daca nu e online actualizez informatiile

          if (isOnline(idClient) == 0)

            updateClientInfo(idClient, client_sock);

          else

          {
            printf("Client %s already connected.\n", idClient);

            // ii inchid conexiunea

            close(client_sock);
          }
        }
      }

      // conventia povestita in README

      // daca pe pozitia 11 am 1 -> subscribe

      // daca pe pozitia 11 am 2 -> unsubscribe

      if (mesajTcp[11] == 1)

      {
        // Parsare mesaj subscribe

        struct MesajSubscribe *mesaj = malloc(sizeof(struct MesajSubscribe));

        if (mesaj == NULL)

        {
          printf("Malloc mesaj failed!\n");

          return -1;
        }

        mesaj = (struct MesajSubscribe *)(mesajTcp + 11 + 1);

        mesaj->topic[51] = '\0';

        // functia se ocupa de subscribe ul primit

        addSubscribe(mesaj->topic, idClient);
      }

      if (mesajTcp[11] == 2)

      {
        /// Parsare mesaj unsubscribe

        char *topic = malloc(51);

        if (topic == NULL)

        {
          printf("Malloc topic failed!\n");

          return -1;
        }

        memcpy(topic, mesajTcp + 11 + 1, 50);

        topic[strlen(topic) - 1] = '\0';

        topic[51] = '\0';

        // functia se ocupa de unsubscribe ul primit

        unSubscribe(topic, idClient);
      }
    }
  }

  return 0;
}
