#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define DIM_BUFFER 512

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Utilizzo: %s <indirizzo_server> <numero_porta>\n", argv[0]);
        exit(1);
    }

    int simpleSocket = 0;
    int simplePorta = atoi(argv[2]);
    int returnStatus = 0;
    char buffer[DIM_BUFFER] = "";
    struct sockaddr_in simpleServer;

    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (simpleSocket == -1)
    {
        perror("Impossibile creare il socket");
        exit(1);
    }

    memset(&simpleServer, '\0', sizeof(simpleServer));
    simpleServer.sin_family = AF_INET;
    simpleServer.sin_addr.s_addr = inet_addr(argv[1]);
    simpleServer.sin_port = htons(simplePorta);

    returnStatus = connect(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));
    if (returnStatus == 0)
    {
        printf("Connessione riuscita!\n");
    }
    else
    {
        perror("Impossibile connettersi all'indirizzo");
        close(simpleSocket);
        exit(1);
    }

    // Ricezione e gestione del messaggio di benvenuto dal server
    returnStatus = read(simpleSocket, buffer, sizeof(buffer));
    if (returnStatus > 0)
    {
        buffer[returnStatus] = '\0';
        printf("Messaggio di benvenuto dal server: %s\n", buffer);
    }
    else
    {
        fprintf(stderr, "Errore durante la ricezione del messaggio di benvenuto\n");
        close(simpleSocket);
        exit(1);
    }

    // Loop principale per l'interazione con il server
    while (1)
    {
        // Richiesta input dell'utente e invio dei dati al server
        printf("Inserisci il numero di dati seguito dai valori separati da spazi (0 per terminare): ");
        fgets(buffer, sizeof(buffer), stdin);

        // Rimuovi il carattere di nuova riga dalla fine dell'input
        buffer[strcspn(buffer, "\n")] = '\0';

        // Verifica se l'utente vuole terminare il processo
        if (strcmp(buffer, "0") == 0)
        {
            // Invio messaggio di fine dati al server
            strcat(buffer, "\n");                            
            write(simpleSocket, buffer, strlen(buffer) + 1);
            printf("Terminazione del processo client.\n");

            // Ricezione e stampa della risposta dal server
            returnStatus = read(simpleSocket, buffer, sizeof(buffer));
            if (returnStatus > 0)
            {
                buffer[returnStatus] = '\0';
                printf("%s\n", buffer);
            }
            else
            {
                fprintf(stderr, "Errore durante la ricezione della risposta\n");
            }

            break;
        }

        // Invio dei dati al server
        strcat(buffer, "\n");                            
        write(simpleSocket, buffer, strlen(buffer) + 1);

        // Ricezione e stampa della risposta dal server
        returnStatus = read(simpleSocket, buffer, sizeof(buffer));
        if (returnStatus > 0)
        {
            buffer[returnStatus] = '\0';
            printf("%s\n", buffer);
        }
        else
        {
            fprintf(stderr, "Errore durante la ricezione della risposta\n");
            close(simpleSocket);
            break;
        }
    }

    // Chiusura del socket ed uscita
    close(simpleSocket);
    return 0;
}
