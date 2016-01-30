#include <sys/select.h>         // select
#include <sys/stat.h>           // mkfifo
#include <time.h>               // sleep
#include <fcntl.h>              // O_NONBLOCK, etc
#include <stdio.h>              // printf
#include <stdlib.h>             // malloc, free
#include <stdbool.h>            // bool, true, false
#include <unistd.h>             // unlink
#include <string.h>             // strlen
#include "../commons.h"      // error messages

#define MSG_LEN 500
#define BASIC_PERMISSIONS 0666
#define N 100


/* @Nombre: Quien
 * @Funcion: Esta orden muestra una lista de los usuarios conectados al
 * 			 servidor y el estado de cada uno.
			 La lista desplegada por la orden quien se mostrará en la ventana
			 de conversación.

 * @Entrada: Ninguna
 * @Salida:  Imprime en pantalla
 */

void whoServer(ClientList* clientlist,int pipeId)

{
	int i;
	char* buffer;
	int bufferSize;

	// Calculamos el tamaño del buffer
	for (i = 0; i < clientlist ->size; i = i + 1)
	{
		bufferSize = bufferSize + strlen(clientlist->client[i].nombre);

		bufferSize = bufferSize + strlen("-");

		bufferSize = bufferSize + strlen(clientlist->client[i].estado);

		// Si no estamos en el ultimo usuario, agregamos un separador
		if (i < clientlist ->size -1)
		{
			bufferSize = bufferSize + strlen("|");
		}
	}

	printf("buffer = %d",bufferSize);

	// Reservamos la memoria para el buffer

	buffer = (char *) malloc(bufferSize);

	// Vamos creando el mensaje
	for (i = 0; i < clientlist ->size; i = i + 1)
	{
		sprintf(buffer,"%s%s-%s",buffer,clientlist->client[i].nombre,clientlist->client[i].estado);

		// Si no estamos en el ultimo usuario, agregamos un separador
		if (i < clientlist ->size -1)
		{
			sprintf(buffer,"%s|",buffer);
		}
	}


	printf("buffer = %s \n \n",buffer);

}

/* @Nombre: Escribir
 * @Funcion: Esta orden toma el nombre de un usuario conectado como argumento
 * 			 e indica que se quiere conversar con él. Los mensajes enviados
 * 			 después de ejecutar la orden serán dirigidos a ese usuario.
 *
 * 			 Si se vuelve a ejecutar la orden escribir con otro nombre de
 * 			 usuario, los mensajes ahora serán dirigidos al nuevo usuario.
 *
 * 			 De este modo, será posible mantener varias conversaciones con
 * 			 distintos usuarios en la misma pantalla.
 *
 * @Entrada: Cliente cliente: cliente que escribe el mensaje
 * @Entrada: Cliente clienteAEscribir: cliente al que se le escribe el mensaje
 * @Salida:  Imprime en pantalla
 */

void writeToServer(Client* client,Client* clientToWrite, char* message)

{
	/*
	 * Defino un constructor para esta clase
		#define INIT_MESSAGE(new,text,sender,reciever)
		Message new = {.text = text, .sender =sender, .reciever = reciever}
	 */

	//INIT_MESSAGE(newMessage,message,client,clientToWrite);
}

/* @Nombre: Estoy
 * @Funcion: Esta orden cambia el estado del usuario.
 * Si un cliente está conversando con determinado usuario
 * (es decir, si la última orden escribir ejecutada en el cliente fue a
 * ese usuario), y este usuario cambia de estado, se debe mostrar
 * una notificación en el cliente con el nuevo estado del usuario.

 * @Entrada: Cliente cliente: cliente que actualiza su estado
 * @Entrada: char* estado: el estado nuevo del cliente
 * @Salida:  Imprime en pantalla
 */

void statusServer(Client* client, char* estado)

{

	// Recibo la data del buffer



	// Si ya existe un estado anterior libero la memoria

	if (client -> estado != NULL)
	{
		// Liberamos la memoria utilizada por el
		free(client -> estado);
	}
	// Reservamos la memoria para el nuevo estado
	client -> estado = (char *) malloc(strlen(estado));

	// Obtenemos el estado del cliente dado y lo actualizamos
	strcpy(client -> estado, estado);

}

/* @Nombre: Salir
 * @Funcion: Esta opción cierra el programa cliente y le notifica al servidor
 * que el usuario se desconectó. Se mostrará un mensaje indicando que el
 * usuario se desconectó en los clientes que estén conversando con él.
 * @Entrada: Cliente cliente que efectua la salida
 * @Salida:  Imprime en pantalla
 */

void logOutServer(Client client)

{
	printf(LogOutMessage,"%s");
	exit(0);
}



int main() {
    struct timeval tv;          // estructura para indicar el timeout de select
    int n;                      // 1 + file descriptor mas alto
    int rv;                     // return value del select, sirve para saber si
                                // retornó algo útil (>0),
                                // si hubo error (-1) o si hubo timeout (0)
    char message[MSG_LEN];      // string de datos enviados desde los clientes
                                // al servidor y viceversa
    int fifo;
    fd_set fdset;               // crear set de pipes nominales
    char in_file_name[11],      // nombre de los pipes de entrada
         out_file_name[11];

    printf("Iniciando servidor!\n");

    // Probando whoServer

    printf("Probando whoServer! \n \n");
    int pruebaIterador;
    INIT_CLIENTLIST(listaPrueba);
    INIT_CLIENT(cliente1,"Pepe");
    INIT_CLIENT(cliente2,"Francisco");

    ClientList* clientListPointer = &listaPrueba;
    addNewClient(clientListPointer, cliente1);
    addNewClient(clientListPointer, cliente2);
    whoServer(&listaPrueba,0);

    // Probando whoServer

    fifo = open_fifo("servidor");
    FD_ZERO(&fdset);            // limpiar el set de pipes nominales

    Client c1;

    // EN CONSTRUCCION

    while (true) {
        printf("escuchando conexiones...\n");

        FD_SET(fifo, &fdset);           // agregar fifo al set de pipes
        n = 100;                        // sustituir por: fd mas alto +1

        // agregar los pipes de todos los usuarios al set de pipes
        // EN CONSTRUCCION

        tv.tv_sec = 1;                  // 1 seg de timeout con 0 microsegs
        tv.tv_usec = 0;

        rv = select(n, &fdset, NULL, NULL, &tv);

        if (rv == -1) {                 // error chequeando pipes
            perror((getErrorMessage(selectError,__LINE__,__FILE__)));
        }
        else if (rv > 0) {              // existen archivos con datos para leer

            // si existe una nueva solicitud de conexion en el pipe 'fifo'
            if(FD_ISSET(fifo, &fdset)) {
                printf("fifo ready to read!");
                // agregar informacion del nuevo usuario al lista
                // leer del pipe fifo los id's de los pipes del cliente
                read(fifo, message, MSG_LEN);
                sscanf(message, "%s %s", out_file_name, in_file_name);
                printf("%s %s\n", in_file_name, out_file_name);
                close(fifo);
                fifo = open_fifo("servidor");

                char mensaje[] = "El servidor esta enviando datos...";
                printf("%s -- %d\n", mensaje, strlen(mensaje));
                c1.in_fd = open(in_file_name, O_RDONLY | O_NONBLOCK);
                c1.out_fd = open(out_file_name, O_WRONLY | O_NONBLOCK);
                write(c1.out_fd, mensaje, strlen(mensaje)+1);
            }
        }
    
        
        sleep(1);
    }
}


    //     revisar el pipe 'conexiones_nuevas'
    //     si existe una nueva solicitud de conexion:
    //         leer del pipe 'conexiones_nuevas' los id's de los pipes del cliente
    //         agregar informacion del nuevo usuario al diccionario
