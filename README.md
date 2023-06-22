# TCP-UDP-server-client

    RULARE SERVER: ./server PORT_DORIT
  
    RULARE CLIENT: ./subscriber ID_CLIENT IP_SERVER PORT_DORIT
  
Am reusit sa implementez un server care accepta conexiuni TCP si UDP. Prin partea de UDP serverul
primeste actualizari despre un topic de la un client UDP furnizat de checkerul temei. Prin partea
de TCP, adica client.c acestia se pot abona si dezabona la diverse topicuri si primesc actualizarile
despre acele topicuri cat timp sunt porniti. Am folosit si multiplexare pentru a avea acces la UDP, TCP si STDIN.
O tema interesanta, datorita utilizarii atat TCP, cat si UDP si invatarea pentru prima data utilizarea de 
multiplexare.

Clientul de UDP nu este furnizat in cod deocamdata.
