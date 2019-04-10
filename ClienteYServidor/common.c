#define _POSIX_C_SOURCE 200112L
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "common.h"

/*
PRE: Recibe un socket (socket_t*)
POST: Inicializa el socket dejandolo en un estado
invalido hasta tanto no llamar a otros metodos del 
socket.
*/

void socket_crear(socket_t *skt) {
    skt->skt = -1;
}


/*
PRE: Recibe un socket ya creado.
POST: Destruye el socket.
*/

void socket_destruir(socket_t* skt) {
    //No hace nada.
}


/*
Metodo para cliente.
PRE: Recibe los nombres (const char*) del host y 
puerto al que se desea conectar el socket.
*/
bool socket_conectar(socket_t* skt, const char* host, const char* puerto) {
    int estado = 0;
    bool estamosConectados = false;
    struct addrinfo hints;
    struct addrinfo *direcciones, *sig; //siguiente
    skt->skt = 0;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; //TCP
    hints.ai_flags = 0;
    estado = getaddrinfo(host, puerto, &hints, &direcciones);
    if (estado != 0) {
        printf("Error in getaddrinfo: %s\n", gai_strerror(estado));
        return false;
    }
    sig = direcciones;
    while (sig != NULL) {
        skt->skt = socket(sig->ai_family, sig->ai_socktype, sig->ai_protocol);
        if (skt->skt == -1) {
            printf("Error: %s\n",strerror(errno));
        } else {
            estado = connect(skt->skt, sig->ai_addr, sig->ai_addrlen);
            if (estado == -1) {
                printf("Error: %s\n", strerror(errno));
                close(skt->skt);
            }
            estamosConectados = (estado != -1);
        }
        sig = sig->ai_next;
    }
    freeaddrinfo(direcciones);
    return estamosConectados;
}

/*
PRE: Recibe un socket ya conectado (socket_t*), y
el modo en que se desea cerrar al socket:
SHUT_RD, SHUT_WR, SHUT_RDWR
POST: Cierra el socket. 
*/
void socket_cerrar_canal(socket_t* skt, int modoCierre) {
    shutdown(skt->skt, modoCierre);
}

/*
PRE: Recibe un socket ya conectado (socket)
POST: Cierra el socket, dejandolo en un estado
invalido.
*/
void socket_cerrar(socket_t* skt) {
    close(skt->skt);
    skt->skt = -1;
}

/*
PRE: Recibe un socket ya conectado (socket_t*), un 
mensaje a enviar, y su longitud.
POST: Devuelve true si logro enviar todo el mensaje, 
false en caso contrario, dado algun error.
*/
bool socket_enviar_todo(socket_t* skt, char* msj, int largo) {
    int estado = 0;
    int bytesEnviados = 0;
    bool hayError = false;
    while (largo > bytesEnviados && ! hayError){
        int i = bytesEnviados;
        int largoMaximo = largo - bytesEnviados;
        estado = send(skt->skt, &msj[i], largoMaximo, MSG_NOSIGNAL);
        if (estado < 0) {
            printf("Error: %s\n", strerror(errno));
            hayError = true;
        } else if (estado == 0) {
            printf("Error: el canal de escritura fue cerrado\n");
            hayError = true;
        } else {
            bytesEnviados = estado;
        }
    }
    return ! hayError;
}

/*
PRE: Recibe un socket ya conectado (socket_t*), 
un buffer (char*) donde guardar los bytes 
recibidos y el largo (int) del mismo.
POST: Devuelve la cantidad de bytes recibidos.
Si esta cantidad es negativa, entonces significa 
que hubo algun error de socket. Si la cantidad es 
igual a cero, significa que ya no queda nada mas 
que recibir.  
Nota: Se llena a lo sumo largo -1 del buffer,
pues el ultimo caracter despues del ultimo byte
recibido se completa con un \0, para indicar donde 
terminan estos ultimos.
*/
int socket_recibir_algo(socket_t *skt, char *buffer, int largo) {
    int estado = 0;
    bool hayError = false;
    bool seguirRecibiendo = true;
    int bytesRecibidos = 0;
    while (largo > bytesRecibidos && ! hayError && seguirRecibiendo) {
        int i = bytesRecibidos;
        int largoMaximo = largo - bytesRecibidos; 
        estado = recv(skt->skt, &buffer[i], largoMaximo-1, MSG_NOSIGNAL);
        if (estado < 0) {
            printf("Error: %s\n", strerror(errno));
            hayError = true;
        } else if (estado == 0) {
            seguirRecibiendo = false;
        } else {
            bytesRecibidos += estado; 
            buffer[bytesRecibidos] = 0;
        }
    }
    if (hayError) {
        return -1;
    }
    return bytesRecibidos;
}

