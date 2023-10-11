#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>
#include <ctype.h>

#define DIM_BUFFER 512

double calcola_media(const int *dati, int dimensione)
{
    double somma = 0.0;
    for (int i = 0; i < dimensione; i++)
    {
        somma += (double)dati[i];
    }
    return somma / dimensione;
}

double calcola_varianza(const int *dati, int dimensione, double media)
{
    if (dimensione <= 1)
    {
        return NAN;
    }
    double varianza = 0.0;
    for (int i = 0; i < dimensione; i++)
    {
        varianza += ((double)dati[i] - media) * ((double)dati[i] - media);
    }
    return varianza / (dimensione - 1);
}

void gestisci_connessione(int clientSocket)
{
    char buffer[DIM_BUFFER] = "";
    const char *messaggioBenvenuto = "Benvenuto, mandami i tuoi dati\n";
    write(clientSocket, messaggioBenvenuto, strlen(messaggioBenvenuto));

    int numDati = 0;
    int dati[DIM_BUFFER];

    while (1)
    {
        int returnStatus = read(clientSocket, buffer, sizeof(buffer));
        if (returnStatus <= 0)
        {
            printf("Connessione chiusa in modo anomalo\n");
            break;
        }

        buffer[returnStatus] = '\0';

        printf("Buffer ricevuto: %s\n", buffer);

        if (strcmp(buffer, "0\n") == 0)
        {
            if (numDati >= 2)
            {
                double media = calcola_media(dati, numDati);
                double varianza = calcola_varianza(dati, numDati, media);

                char response[DIM_BUFFER];
                snprintf(response, sizeof(response), "Risultati dei valori: \nNumero dei valori: %d\nMedia: %.2f\nVarianza: %.2f\n", numDati, media, varianza);
                write(clientSocket, response, strlen(response));
            }
            else
            {
                const char *messaggioErrore = "Errore nei risultati: non posso calcolare la varianza con meno di 2 campioni\n";
                write(clientSocket, messaggioErrore, strlen(messaggioErrore));
            }

            break;
        }

        // Verifica se la stringa contiene solo caratteri numerici e spazi
        int isValidInput = 1;
        int consecutiveSpaces = 0;
        for (int i = 0; i < strlen(buffer); i++)
        {
            if (isdigit(buffer[i]))
            {
                consecutiveSpaces = 0;          // Azzera il conteggio quando trova un carattere numerico
            }
            else if (isspace(buffer[i]))
            {
                consecutiveSpaces++;
                if (consecutiveSpaces > 1)
                {
                    isValidInput = 0;           // Troppi spazi consecutivi
                    break;
                }
            }
            else
            {
                isValidInput = 0;               // Carattere non valido
                break;
            }
        }

        if (isValidInput)
        {
            char *token = strtok(buffer, " ");
            while (token != NULL && numDati < DIM_BUFFER)
            {
                int valore;
                if (sscanf(token, "%d", &valore) != 1)
                {
                    const char *messaggioErrore = "Errore nei dati: valore non valido\n";
                    write(clientSocket, messaggioErrore, strlen(messaggioErrore));
                    break;
                }
                else
                {
                    dati[numDati++] = valore;
                }

                token = strtok(NULL, " ");
            }

            const char *messaggioOKData = "Dato accettabile\n";
            write(clientSocket, messaggioOKData, strlen(messaggioOKData));
        }
        else
        {
            const char *messaggioErrore = "Errore: dato non valido\n";
            write(clientSocket, messaggioErrore, strlen(messaggioErrore));
        }
    }

    close(clientSocket);
    printf("Connessione chiusa\n");
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Utilizzo: %s <porta>\n", argv[0]);
        exit(1);
    }

    int simpleSocket = 0;
    int simplePorta = atoi(argv[1]);
    int returnStatus = 0;
    struct sockaddr_in simpleServer;

    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (simpleSocket == -1)
    {
        perror("Impossibile creare il socket");
        exit(1);
    }

    memset(&simpleServer, '\0', sizeof(simpleServer));
    simpleServer.sin_family = AF_INET;
    simpleServer.sin_addr.s_addr = htonl(INADDR_ANY);
    simpleServer.sin_port = htons(simplePorta);

    returnStatus = bind(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));
    if (returnStatus == -1)
    {
        perror("Impossibile effettuare il bind");
        close(simpleSocket);
        exit(1);
    }

    returnStatus = listen(simpleSocket, 5);
    if (returnStatus == -1)
    {
        perror("Impossibile mettersi in ascolto");
        close(simpleSocket);
        exit(1);
    }

    printf("Server in ascolto sulla porta %d...\n", simplePorta);

    while (1)
    {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int simpleChildSocket = accept(simpleSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);

        if (simpleChildSocket == -1)
        {
            perror("Impossibile accettare la connessione dal client");
            continue;
        }

        printf("Connessione accettata da %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        gestisci_connessione(simpleChildSocket);

        printf("Connessione con %s:%d chiusa\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
    }

    close(simpleSocket);
    return 0;
}
