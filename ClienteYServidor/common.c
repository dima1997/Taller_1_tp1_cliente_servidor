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
PRE: Recibe un socket (skt_t*)
POST: Inicializa el socket dejandolo en un estado
invalido hasta tanto no llamar a otros metodos del 
socket.
*/

void socket_crear(skt_t *skt) {
    skt->skt = -1;
}

/*
PRE: Recibe un socket ya creado.
POST: Destruye el socket.
*/

void socket_destruir(skt_t* skt) {
    if (skt->skt != -1) {
        shutdown(skt->skt, SHUT_RDWR);
        close(skt->skt);
    }
}


/*
Metodo para cliente.
PRE: Recibe los nombres (const char*) del host y 
puerto al que se desea conectar el socket.
*/
bool socket_conectar(skt_t* skt, const char* host, const char* puerto) {
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
Metodo para Servidor.
PRE: Recibe un socket (skt_t *) ya creado, y el
nombre (char *) de un puerto al cual enlazarlo.
POST: Configura al socket para que funcion de 
forma PASIVA, es decir, se lo enlaza al puerto de
nombre recibido. 
Devuelve true, si logro lo anterior, false en caso
contrario.
*/
bool socket_enlazar(skt_t *skt, const char *puerto){
    int estado = 0;
    struct addrinfo hints;
    struct addrinfo *direccion;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; //TCP
    hints.ai_flags = AI_PASSIVE; //AI_PASSIVE para servidor
    estado = getaddrinfo(NULL, puerto, &hints, &direccion);
    if (estado != 0) { 
        printf("Error in getaddrinfo: %s\n", gai_strerror(estado));
        return false;
    }
    struct addrinfo *dir = direccion;
    skt->skt = socket(dir->ai_family, dir->ai_socktype, dir->ai_protocol);
    if (skt->skt == -1) {
        printf("Error: %s\n", strerror(errno));
        freeaddrinfo(direccion);
        return false;
    }
    // Evita que le servidor falle al abrirlo y cerrarlo en poco tiempo
    int val = 1;
    estado = setsockopt(skt->skt, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    if (estado == -1) {
        printf("Error: %s\n", strerror(errno));
        close(skt->skt);
        freeaddrinfo(direccion);
        return false;
    }
    estado = bind(skt->skt, dir->ai_addr, dir->ai_addrlen);
    freeaddrinfo(direccion);
    if (estado == -1) {
        printf("Error: %s\n", strerror(errno));
        close(skt->skt);
        return false;
    }
    return true;
}

/*
Metodo para Servidor.
PRE: Recibe una socket (skt_t *) ya enlazado a algun puerto,
y la cantidad de sockets entrantes a escuchar.
POST: Pone a escuchar, al socket recibido, la cantidad de 
recibida de sockets entrantes.
Devuelve true, si logro lo anterior, false en caso contrario.
*/
bool socket_escuchar(skt_t *skt, size_t cuantosEscuchar){
   int estado = listen(skt->skt, cuantosEscuchar);
    if (estado == -1) {
        printf("Error: %s\n", strerror(errno));
        close(skt->skt);
        return false;
    }
    return true;    
}

/*
PRE: Recibe dos socket (socke_t *): el primero esta configurado 
como PASIVO (se ejecutaron metodos socket_enlazar y ..._escuchar),
y el segundo (ya creado) para ser configurado como ACTIVO. 
POST: Devuelve true si logro aceptar una nueva comunicacion y
configurar al socket ACTIVO para la misma, o false en caso 
contrario. 
*/
bool socket_aceptar(skt_t *sktPasivo, skt_t *sktActivo) {
    sktActivo->skt = accept(sktPasivo->skt, NULL, NULL);
    if (sktActivo->skt == -1) {
        printf("Error: %s\n", strerror(errno));
        return false;
    }
    return true; 
}


/*
PRE: Recibe un socket ya conectado (skt_t*), y
el modo en que se desea cerrar al socket:
SHUT_RD, SHUT_WR, SHUT_RDWR
POST: Cierra el socket. 
*/
void socket_cerrar_canal(skt_t* skt, int modoCierre) {
    shutdown(skt->skt, modoCierre);
}

/*
PRE: Recibe un socket ya conectado (socket)
POST: Cierra el socket, dejandolo en un estado
invalido.
*/
void socket_cerrar(skt_t* skt) {
    close(skt->skt);
    skt->skt = -1;
}

/*
PRE: Recibe un socket ya conectado (skt_t*), un 
mensaje a enviar, y su longitud.
POST: Devuelve true si logro enviar todo el mensaje, 
false en caso contrario, dado algun error.
*/
bool socket_enviar_todo(skt_t* skt, char* msj, int largo) {
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
PRE: Recibe un socket ya conectado (skt_t*), 
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
int socket_recibir_algo(skt_t *skt, char *buffer, int largo) {
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

