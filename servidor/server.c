#include <sys/select.h>         // select
#include <sys/stat.h>           // mkfifo
#include <time.h>               // sleep
#include <fcntl.h>              // O_NONBLOCK, etc
#include <stdio.h>              // printf
#include <sys/queue.h>          // list macros
#include <stdlib.h>             // malloc, free
#include <stdbool.h>            // bool, true, false
#include <unistd.h>             // unlink
#include <string.h>             // strlen

#define MSG_LEN 500
#define STATUS_LEN 100
#define BASIC_PERMISSIONS 0666
#define N 20

// crear contenedor de usuarios
struct client {
    char username[30];
	char status[140];
    int friends[N];
    int in_fd, out_fd;
} clients[N];

int open_fifo(const char *fifo_name);
void initialize();
void sendMessage(char username[], char message[]);
void write_full(char *token, char dst[]);
void add_friend(struct client *c, char username[]);
void delete_friend(struct client *c, char username[]);
void login(char username[], int in_fd, int out_fd);
void logout(char username[]);
void notify_friends(struct client c, char status[]);
int get_id(char username[]);
void print_array(int arr[]);


/*
    Hacen falta funciones para:
    - quien:    Conocer los usuarios conectados
    - escribir: Mandar mensaje a los usuarios conectados
    - estoy:    Poner un estado
    - salir:    Salir del chat

    Ademas faltan las funciones:
    - Agregar un usuario a la lista de estructuras
    - Eliminar un usuario de la lista de usuarios (Debe incluir cerrar pipes)
    - Comparar dos usuarios para ver si son o no identicos
    - Buscar usuarios
    - funciones para enviar y recibir datos por el pipe
    - Verificar si el nombre de un usuario ya existe
*/

int main(int argc, char *argv[]) {
    struct timeval tv;          // estructura para indicar el timeout de select
    int n;                      // 1 + file descriptor mas alto
    int rv;                     // return value del select, sirve para saber si
                                // retornó algo útil (>0),
                                // si hubo error (-1) o si hubo timeout (0)
    char message[MSG_LEN];      // string de datos enviados desde los clientes
                                // al servidor y viceversa
    char tmp[MSG_LEN];          // string temporal para agregar origen de mensaje
    int fifo;                   // fd del pipe del servidor
    fd_set fdset, error_fdset;  // crear sets de pipes nominales
    char in_file_name[11],     	// nombre de los pipes de entrada
         out_file_name[11];
    int in_fd, out_fd;          // file descriptors temporales para clientes
    char username[30];          // nombre temporal para clientes
    char server_pipe_name[100]; // nombre del pipe nominal del servidor
    int i, j;                   // variables de iteración simple
    char *token;

    if (argc == 2) {
        strcpy(server_pipe_name, argv[1]);
    }
    else {
        strcpy(server_pipe_name, "/tmp/servidor");
    }

    printf("Iniciando servidor!\n");

    fifo = open_fifo(server_pipe_name);
    tv.tv_sec = 1;                  // 1 seg de timeout con 0 microsegs
    tv.tv_usec = 0;
    initialize();

    while (true) {
        printf("escuchando conexiones...\n");

        FD_ZERO(&fdset);                // limpiar el set de pipes nominales
        FD_ZERO(&error_fdset);
        FD_SET(fifo, &fdset);           // agregar fifo al set de pipes
        n = 100;                        // sustituir por: fd mas alto +1

        strcpy(message, "");

        for (i=0; i<N; i++){             // agregar pipes de todos los usuarios al fd_set
            if (strlen(clients[i].username) > 0) {
                FD_SET(clients[i].in_fd, &fdset);
            }
        }

        rv = select(n, &fdset, NULL, &error_fdset, &tv);

        if (rv == -1) {
            perror("rvError");
            for (i=0; i<N; i++) {
                if (FD_ISSET(clients[i].in_fd, &error_fdset)) {
                    printf("Error on: %s| %d | %d\n", clients[i].username, 
                                                      clients[i].in_fd,
                                                      clients[i].out_fd);
                }
            }
        }
        else if (rv > 0) {              // existen archivos con datos para leer
            // si existe una nueva solicitud de conexion en el pipe 'fifo'
            if(FD_ISSET(fifo, &fdset)) {
                read(fifo, message, MSG_LEN);
                sscanf(message, "%s %s %s\n", username, out_file_name, in_file_name);

                //printf("%s %s %s\n", username, in_file_name, out_file_name);
                close(fifo);
                fifo = open_fifo(server_pipe_name);
                in_fd = open(in_file_name, O_RDONLY | O_NONBLOCK);
                out_fd = open(out_file_name, O_WRONLY | O_NONBLOCK);
                login(username, in_fd, out_fd);
            }

            for (i=0; i<N; i++) {
                if (FD_ISSET(clients[i].in_fd, &fdset)) {
                    printf("comando recibido de |%s|\n", clients[i].username);
                    strcpy(message, "");
                    read(clients[i].in_fd, message, MSG_LEN);
                    printf("message = |%s|\n", message);
                    token = strtok(message, " ");      // token = primera palabra del comando
                    printf("token = |%s|\n", token);
                    if (token == NULL) {
                        printf("null token\n");
                        logout(clients[i].username);
                        continue;
                    }
                    if (strcmp(token, "-estoy") == 0) {
                        token = strtok(NULL, " ");
                        write_full(token, message);
                        printf("client status = |%s|\n", message);
                        strcpy(clients[i].status, message);
                        notify_friends(clients[i], message);
                    }
                    else if (strcmp(token, "-quien") == 0) {
                        strcpy(message, "");

                        for (j=0; j<N; j++) {
                            if (strlen(clients[j].username) > 0) {
                                strcat(message, clients[j].username);
                                strcat(message, ":");
                                strcat(message, clients[j].status);
                                strcat(message, "|");
                            }
                        }
                        write(clients[i].out_fd, message, MSG_LEN);
                    }
                    else if (strcmp(token, "-escribir") == 0) {
                        token = strtok(NULL, " ");
                        strcpy(username, token);
                        token = strtok(NULL, " ");
                        write_full(token, message);
                        sprintf(tmp, "mensaje de %s: %s", clients[i].username, message);
                        sendMessage(username, tmp);
                        add_friend(&clients[i], username);
                        add_friend(&clients[get_id(username)], clients[i].username);
                        //print_array(clients[i].friends);
                        //print_array(clients[get_id(username)].friends);
                    }
                    else if (strcmp(token, "-salir") == 0) {
                        printf("logging out\n");
                        notify_friends(clients[i], "desconectado");
                        write(clients[i].out_fd, "-salir", MSG_LEN);
                        logout(clients[i].username);
                    }
                }
            }
        }
        sleep(1);

        int i;
        for (i=0;i<N;i++) {
            printf("%s, ", clients[i].username);
        }

    }

}

    // FUNCIONES PARA REALIZAR DISTINTAS TAREAS DEL SERVIDOR

// crea y abre el pipe nominal fifo_name
// retorna el file descriptor del pipe creado
int open_fifo(const char *fifo_name) {
    int fifo;

    // eliminar el pipe nominal creado en alguna otra ejecución del server
    unlink(fifo_name);
    // esperar 1 seg para que el SO lo elimine completamente
    sleep(1);
    // crear pipe (nominal) de conexiones nuevas
    mkfifo(fifo_name, BASIC_PERMISSIONS | O_NONBLOCK);
    // abrir el pipe para leer conexiones entrantes
    fifo = open(fifo_name, O_RDONLY | O_NONBLOCK);

    if (fifo == -1) perror("server mkfifo");

    return fifo;
}

// Inicializa el arreglo de clientes
void initialize() {
    int i, j;

    for (i=0; i<N; i++) {
        strcpy(clients[i].username, "");
        strcpy(clients[i].status, "");
        clients[i].in_fd = 0;
        clients[i].out_fd = 0;
        for (j=0; j<N; j++) {
            clients[i].friends[j] = -1;   // -1 significa vacio
        }
    }
}

// Enviar mensaje al cliente indicado a través de su pipe
void sendMessage(char username[], char message[]) {
    int i;

    i = get_id(username);
    write(clients[i].out_fd, message, strlen(message)+1);
}

// escribe lo que le sobra a token dentro de dst
void write_full(char *token, char dst[]) {
    char tmp[MSG_LEN] = "";

    while (token != NULL) {
        strcat(tmp, token);
        strcat(tmp, " ");
        token = strtok(NULL, " ");
    }

    strcpy(dst, tmp);
    dst[strlen(dst)-1] = 0;
}

// Agregar amigo a la lista de amigos del cliente c
void add_friend(struct client *c, char username[]) {
    int i, friend_id;

    friend_id = get_id(username);

    for (i=0; i<N; i++) {
        if (c->friends[i] == -1) {
            printf("agregando a %s con id %d en los amigos de %s\n", username, friend_id, c->username);
            c->friends[i] = friend_id;
            break;
        }
        else if(strcmp(clients[c->friends[i]].username, username) == 0) break;
    }
}

// Eliminar amigo de la lista de amigos del cliente c
void delete_friend(struct client *c, char username[]) {
    int i, friend_id;

    friend_id = get_id(username);

    for (i=0; i<N; i++) {
        if (c->friends[i] == friend_id) {
            c->friends[i] = -1;
            break;
        }
    }
}

// Retorna la posición del arreglo en donde se encuentra el usuario
// Retorna -1 si no lo encuentra
int get_id(char username[]) {
    int i;
    
    for (i=0; i<N; i++) 
        if (strcmp(clients[i].username, username) == 0) 
            return i;
    
    return -1;
}

// Registrar los datos y pipes del usuario
void login(char username[], int in_fd, int out_fd) {
    int i;

    for (i=0; i<N; i++) {
        if (strcmp(clients[i].username, "") == 0) {
            strcpy(clients[i].username, username);
            strcpy(clients[i].status, "");
            clients[i].in_fd = in_fd;
            clients[i].out_fd = out_fd;
            break;
        }
    }
}

// Eliminar al usuario de todas las listas de amigos y vaciar sus datos
void logout(char username[]) {
    int i, j, friend_id;
    
    i = get_id(username);
    strcpy(clients[i].username, "");
    strcpy(clients[i].status, "");
    close(clients[i].in_fd);
    close(clients[i].out_fd);
    clients[i].in_fd = 0;
    clients[i].out_fd = 0;
    
    for (j=0; j<N; j++) {
        friend_id = clients[i].friends[j];
        delete_friend(&clients[friend_id], clients[i].username);
        clients[i].friends[j] = -1;
    }
}

// Enviar un mensaje a los contactos que se encuentran hablando con el usuario
// indicandoles un cambio de estado del mismo (desconectado, custom)
void notify_friends(struct client c, char status[]) {
    char message[MSG_LEN] = "";
    int i;
    
    sprintf(message, "%s ha cambiado de estado a: %s", c.username, status);
    
    for (i=0; i<N; i++) {
        if (c.friends[i] != -1) {
            printf("Notificando a %s: %s\n", clients[c.friends[i]].username, message);
            sendMessage(clients[c.friends[i]].username, message);
        }
    }
}

void print_array(int arr[]) {int i; for (i=0; i<N; i++) printf("%d ", arr[i]); printf("\n"); }