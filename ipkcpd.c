#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>

//----------------------------------------------------Konstanty---------------------------------------------------------
#define MAX_BUFFER_SIZE 1024
#define MAX_CONNECTIONS 10

//----------------------------------------------------Struktury---------------------------------------------------------
typedef struct {
    int client_socket;
    struct sockaddr_in client_address;
    socklen_t client_address_length;
    bool tcp_mode;
} client_data;

//------------------------------------------------Deklaracie Funkcii----------------------------------------------------

/// \brief - Vyhodnocuje zadaný matematický výraz v reťazci
/// \param expr - Reťazec obsahujúci výraz na vyhodnotenie
/// \param index - Ukazovateľ na aktuálnu pozíciu v reťazci
/// \param error - Ukazovateľ na príznak chyby
/// \return - Výsledok vyhodnotenia výrazu
int evaluate_expression(char *expr, int *index, bool *error);

/// \brief - Vyhodnocuje zadaný operátor a jeho dva operandy
/// \param operator - Operátor na vyhodnotenie (+, -, *, /)
/// \param a - Prvý operand
/// \param b - Druhý operand
/// \return - Výsledok vyhodnotenia operátora a dvoch operandov
int evaluate_operator(char operator, int a, int b);

/// \brief - Spracováva textový formát požiadavky a generuje odpoveď
/// \param request - Vstupný reťazec s požiadavkou
/// \param response - Výstupný reťazec s odpoveďou
/// \param response_len - Maximálna dĺžka reťazca odpovede
void process_textual_request(char *request, char *response, size_t response_len);

/// \brief - Spracováva binárny formát požiadavky a generuje odpoveď
/// \param request - Vstupný reťazec s požiadavkou
/// \param request_len - Dĺžka vstupného reťazca s požiadavkou
/// \param response - Výstupný reťazec s odpoveďou
/// \param response_len - Ukazovateľ na dĺžku výstupného reťazca odpovede
void process_binary_request(char *request, int request_len, char *response, int *response_len);

/// \brief - Spracováva klienta a jeho požiadavky
/// \param data - Ukazovateľ na štruktúru s údajmi o klientovi
/// \return - Vracia NULL, keď je spracovanie dokončené
void *client_handler(void *data);

/// \brief - Obsluhuje signál SIGINT a nastavuje príznak pre ukončenie servera
/// \param signal - Číslo signálu (v tomto prípade SIGINT)
void handle_sigint(int signal);

//-----------------------------------------------Globalne Premenne------------------------------------------------------
int server_socket;
volatile sig_atomic_t stop_server = 0;

//------------------------------------------------Definicie Funkcii-----------------------------------------------------
int main(int argc, char *argv[]) {
    // Registrácia obsluhy signálu SIGINT
    signal(SIGINT, handle_sigint);
    // Kontrola počtu argumentov
    if (argc != 7) {
        printf("Usage: ipkcpd -h <host> -p <port> -m <mode>\n");
        exit(EXIT_FAILURE);
    }

    // Načítanie hodnôt z argumentov
    char *host = argv[2];
    int port = atoi(argv[4]);
    bool tcp_mode = strcmp(argv[6], "tcp") == 0;

    // Inicializácia štruktúry server_address
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &server_address.sin_addr) <= 0) {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    // Vytvorenie socketu
    int domain = tcp_mode ? SOCK_STREAM : SOCK_DGRAM;
    server_socket = socket(AF_INET, domain, 0);
    if (server_socket < 0) {
        perror("Cannot create socket");
        exit(EXIT_FAILURE);
    }

    // Pripojenie (bind) socketu k adrese
    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d (%s mode)\n", port, tcp_mode ? "TCP" : "UDP");

    // Prepnutie socketu do pasívneho režimu (pre TCP)
    if (tcp_mode && listen(server_socket, MAX_CONNECTIONS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    // Registrácia obsluhy signálu SIGINT (opätovne)
    signal(SIGINT, handle_sigint);

    // Hlavný cyklus servera
    while (!stop_server) {
        // Alokácia a inicializácia dát klienta
        client_data *data = (client_data *) malloc(sizeof(client_data));
        data->client_address_length = sizeof(data->client_address);
        data->tcp_mode = tcp_mode;

        // Akceptovanie prichádzajúceho spojenia (pre TCP)
        if (tcp_mode) {
            data->client_socket = accept(server_socket, (struct sockaddr *) &data->client_address,
                                         &data->client_address_length);
            if (data->client_socket < 0) {
                perror("Accept failed");
                continue;
            }
        } else {
            data->client_socket = server_socket;
        }

        // Vytvorenie vlákna pre obsluhu klienta
        pthread_t thread;
        if (pthread_create(&thread, NULL, client_handler, data) < 0) {
            perror("Thread creation failed");
            continue;
        }
        // Ukončenie vlákna po dokončení (pre TCP) alebo čakanie na jeho ukončenie (pre UDP)
        if (tcp_mode) {
            pthread_detach(thread);
        } else {
            pthread_join(thread, NULL);
        }
    }


    // Uvoľnenie zdrojov
    close(server_socket);

    printf("Server has been terminated.\n");
    return 0;
}

void *client_handler(void *data) {
    // Prevod void dát na štruktúru client_data
    client_data *client = (client_data *) data;

    if (client->tcp_mode) {
        // Inicializácia buffera pre prijímanie dát
        char buffer[MAX_BUFFER_SIZE];
        int bytes_received;
        bool done = false;

        // Cyklus na čítanie a spracovanie správ klienta
        while (!done && (bytes_received = recv(client->client_socket, buffer, MAX_BUFFER_SIZE, 0)) > 0) {
            buffer[bytes_received] = '\0';

            // Vytvorenie buffera pre odpoveď
            char response[MAX_BUFFER_SIZE];

            // Spracovanie textovej požiadavky klienta
            process_textual_request(buffer, response, MAX_BUFFER_SIZE);

            // Kontrola, či odpoveď obsahuje "BYE", čo ukončí komunikáciu
            if (strncmp(response, "BYE", 3) == 0) { done = true; }

            // Odoslanie odpovede klientovi
            send(client->client_socket, response, strlen(response), 0);
        }
    } else {    // Obsluha klienta v režime UDP

        // Inicializácia buffera pre žiadosť a odpoveď
        char request_buffer[MAX_BUFFER_SIZE];
        char response_buffer[MAX_BUFFER_SIZE];
        int request_len, response_len;

        // Štruktúra pre adresu klienta
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        // Cyklus na čítanie a spracovanie správ klienta
        while ((request_len = recvfrom(client->client_socket, request_buffer, MAX_BUFFER_SIZE, 0,
                                       (struct sockaddr *) &client_addr, &client_addr_len)) > 0) {

            // Spracovanie binárnej požiadavky klienta
            process_binary_request(request_buffer, request_len, response_buffer, &response_len);

            // Odoslanie odpovede klientovi
            sendto(client->client_socket, response_buffer, response_len, 0,
                   (struct sockaddr *) &client_addr, client_addr_len);
        }
    }

    // Zatvorenie socketu klienta v režime TCP
    if (client->tcp_mode) { close(client->client_socket); }

    // Uvoľnenie pamäti pre dáta klienta
    free(client);

    return NULL;
}

void process_textual_request(char *request, char *response, size_t response_len) {
    // Príznaky pre chyby a neplatné žiadosti
    bool error = false;
    bool invalid_request = false;

    // Kontrola, či žiadosť začína slovom "HELLO"
    if (strncmp(request, "HELLO", 5) == 0) {
        // Kopírovanie odpovede "HELLO\n" do buffer-a odpovede
        strncpy(response, "HELLO\n", response_len - 1);
    } else if (strncmp(request, "SOLVE", 5) == 0) {
        // Preskočenie "SOLVE " a začatie spracovania výrazu
        int index = 6;

        // Vyhodnotenie výrazu
        int result = evaluate_expression(request, &index, &error);

        // Kontrola chyby pri vyhodnotení výrazu
        if (error) { invalid_request = true; }
        else {
            snprintf(response, response_len, "RESULT %d\n", result);
        } // Vytvorenie odpovede "RESULT " + hodnota výrazu
    } else if (strncmp(request, "BYE", 3) == 0) {
        // Kopírovanie odpovede "BYE\n" do buffera odpovede
        strncpy(response, "BYE\n", response_len - 1);
    } else {
        // Ak žiadosť nie je platná, nastavenie príznaku invalid_request
        invalid_request = true;
    }

    // Ak je žiadosť neplatná, nastavenie odpovede na "BYE\n"
    if (invalid_request) { strncpy(response, "BYE\n", response_len - 1); }

    // Uistenie, že reťazec odpovede je ukončený nulou
    response[response_len - 1] = '\0';
}

void process_binary_request(char *request, int request_len, char *response, int *response_len) {
    // Overenie, či dĺžka žiadosti je aspoň 3 a prvý znak žiadosti je 0
    if (request_len < 3 || request[0] != 0) { return; }

    // Príznak pre chyby
    bool error = false;

    // Indexovanie vstupného reťazca
    int index = 2;

    // Vyhodnotenie výrazu
    int result = evaluate_expression(request, &index, &error);

    // Nastavenie prvých dvoch bytov odpovede
    response[0] = 1;
    response[1] = error ? 1 : 0;

    // Inicializácia indexov pre dĺžku a obsah odpovede
    int payload_len_index = 2;
    int payload_index = 3;
    int length = 0;

    // Vytvorenie odpovede v závislosti od toho, či nastala chyba alebo nie
    if (error) {
        // Ak nastala chyba, nastaví sa odpoveď na "Invalid expression"
        length = snprintf(response + payload_index, MAX_BUFFER_SIZE - payload_index, "Invalid expression");
    } else {
        // Ak nebola chyba, nastaví sa odpoveď na hodnotu výsledku výrazu
        length = snprintf(response + payload_index, MAX_BUFFER_SIZE - payload_index, "%d", result);
    }

    // Uloženie dĺžky obsahu odpovede
    response[payload_len_index] = length;

    // Nastavenie celkovej dĺžky odpovede
    *response_len = payload_index + length;

}

int evaluate_expression(char *expr, int *index, bool *error) {
    // Ak už nastala chyba, vráti 0
    if (*error) { return 0; }

    // Načítanie operátora zo vstupného reťazca
    char operator = expr[*index];
    (*index) += 2;

    // Preskočenie otváracej zátvorky, ak je prítomná
    if (operator == '(') {
        operator = expr[*index];
        (*index) += 2;
    }

    // Overenie, či je operátor platný
    if (strchr("+-*/", operator) == NULL) {
        *error = true;
        return 0;
    }

    // Načítanie prvého operandu
    int a, b;
    if (isdigit(expr[*index])) {
        sscanf(expr + *index, "%d", &a);
        while (isdigit(expr[*index])) { (*index)++; }
    } else { a = evaluate_expression(expr, index, error); }

    // Ak nastala chyba, vráti 0
    if (*error) { return 0; }
    (*index)++;

    // Načítanie druhého operandu
    if (isdigit(expr[*index])) {
        sscanf(expr + *index, "%d", &b);
        while (isdigit(expr[*index])) { (*index)++; }
    } else { b = evaluate_expression(expr, index, error); }

    // Ak nastala chyba, vráti 0
    if (*error) { return 0; }

    // Overenie, či je zátvorka správne uzavretá
    if (expr[*index] == ')') { (*index) += 2; }
    else {
        *error = true;
        return 0;
    }

    // Overenie delenia nulou
    if (operator == '/' && b == 0) {
        *error = true;
        return 0;
    }

    // Vráť výsledok vyhodnotenia operátora a dvoch operandov
    return evaluate_operator(operator, a, b);

}

int evaluate_operator(char operator, int a, int b) {
    // Vyhodnotenie operátora podľa jeho typu
    switch (operator) {
        case '+':
            return a + b;
        case '-':
            return a - b;
        case '*':
            return a * b;
        case '/':
            return a / b;
        default:
            return 0;
    }
}

void handle_sigint(int sig) {
    // Nastavenie príznaku pre ukončenie servera na 1
    (void) sig;
    stop_server = 1;
}
