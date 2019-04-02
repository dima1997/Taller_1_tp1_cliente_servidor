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
#include "client.h"
#define PAQUETE_LARGO_MAXIMO 1024 // 1 k

/*
Pre: Recibe un socket (int *) ya contectado.
Post: Recibe una respuesta del servidor y la imprime por
salida estandard. Devuelve true si logro recibir todo 
el mensaje, false en caso contrario.
*/
bool recibir_respuesta(int *skt) {
    int estado = 0;
    bool hayErrorDeSocket = false;
    bool estaSocketRemotoCerrado = false;
    int bytesRecibidos = 0;
    char parteRespuesta[PAQUETE_LARGO_MAXIMO];
    size_t largoMaximo = sizeof(PAQUETE_LARGO_MAXIMO);
    while (hayErrorDeSocket == false && estaSocketRemotoCerrado == false) {
        estado = recv(*skt, &parteRespuesta, largoMaximo - 1, MSG_NOSIGNAL); 
        // - \0
        if (estado < 0) {
            printf("Error: %s\n", strerror(errno));
            hayErrorDeSocket = true;
        } else if (estado == 0) {
            // Cerro el canal, entonces, por hipotesis del TP, 
            // el servidor termino de enviarnos el mensaje.
            estaSocketRemotoCerrado = true;
        } else {
            bytesRecibidos = estado; 
            parteRespuesta[bytesRecibidos] = 0;
            printf("%s", parteRespuesta);
            bytesRecibidos = 0; 
        }
    }
    return !(hayErrorDeSocket);
}

/*
Pre: Recibe un socket (int *) ya conectado, la parte del
mensaje (char *) que se intenta enviar, y el largo de 
dicha parte.
Post: Intenta enviar la parte del mensaje recibida. Devuelve 
la cantidad de bytes que no se lograron enviar. Si esta 
cantidad es -1 o menor, significa que hubo un error.
Ademas modifica la parte recibida, dejando en ella solo los 
caracteres correspondientes a los bytes sin enviar. 
*/
int enviar_parte(int *skt, char* parte, int largo) {
    int estado = 0;
    estado = send(*skt, parte, largo, MSG_NOSIGNAL);
    int bytesSinEnviar = 0;
    if (estado < 0) { 
        printf("Error: %s\n", strerror(errno));
        bytesSinEnviar = -1;
    } else if (estado == 0) { 
        // Bajo las hipotesis del TP, el que envia 
        // el mensaje deberia cerrar el canal
        bytesSinEnviar = -1;
    } else {
        int bytesEnviados = estado;
        bytesSinEnviar = largo - bytesEnviados;
        for (int j = 0; j < bytesSinEnviar ; ++j) {
            int posicionByteSinEnviar; 
            posicionByteSinEnviar = largo - bytesSinEnviar + j;
            parte[j] = parte[posicionByteSinEnviar];
        }
    }
    return bytesSinEnviar;
}

/*
Pre: Recibe un socket (int *) ya conectado, y un archivo (FILE *)
de texto ya abierto de donde se leera la peticion a enviar.
Post: Envia la peticion del archivo recibido, a donde sea que 
este conectado el socket. Devuelve true si logro enviar todo
el mensaje; false en caso contrario dado algun error del
socket. 
*/
bool enviar_peticion(int *skt, FILE *archivo) {
    bool hayErrorDeSocket = false;
    char partePeticion[PAQUETE_LARGO_MAXIMO];
    size_t largoParte = sizeof(partePeticion);
    char caracter;
    char caracterAnterior = '\0';
    int i = 0; 
    while ((caracter = getc(archivo)) != -1 && hayErrorDeSocket == false) {
        if (caracter == '\n') {
            if (caracterAnterior == '\n' || caracterAnterior == '\0'){
                //Una linea vacia marca el final de la peticion
                //Para cuando archivoPeticion es stdin
                break;
            }
        }
        partePeticion[i] = caracter;
        if (i >= largoParte) {
            int bytesSinEnviar = enviar_parte(skt, partePeticion, largoParte);
            if (bytesSinEnviar < 0){
                hayErrorDeSocket = true;
            } else {
                i = bytesSinEnviar;
            }
        }
        ++i;
        caracterAnterior = caracter;
    }
    if (hayErrorDeSocket) {
        return false;
    }
    while (i > 0 && hayErrorDeSocket == false) {
        int bytesSinEnviar = enviar_parte(skt, partePeticion, i);
        if (bytesSinEnviar < 0){
                hayErrorDeSocket = true;
        } else {
            i = bytesSinEnviar;
        }
    }
    return !(hayErrorDeSocket);
}

/*
Pre: Recibe un socket (int *), y los nombres (char *) 
del host y puerto al que se desea conectar.
Post: Devuelve true si logro se logro conectarse con exito, 
false en caso contrario.
*/
bool conectar_socket(int *skt, const char* host, const char* puerto) {
    int estado = 0;
    bool estamosConectados = false;
    struct addrinfo hints;
    struct addrinfo *direcciones, *sig; //siguiente
    *skt = 0;
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
        *skt = socket(sig->ai_family, sig->ai_socktype, sig->ai_protocol);
        if (*skt == -1) {
            printf("Error: %s\n",strerror(errno));
        } else {
            estado = connect(*skt, sig->ai_addr, sig->ai_addrlen);
            if (estado == -1) {
                printf("Error: %s\n", strerror(errno));
                close(*skt);
            }
            estamosConectados = (estado != -1);
        }
        sig = sig->ai_next;
    }
    freeaddrinfo(direcciones);
    return estamosConectados;
}

int main(int argc, const char* argv[]) {
    if (argc < 3 || argc > 4) {
        fprintf(stderr,"Uso:\n./client <direccion> <puerto> [<input>]\n");
        return 1;
    }
    const char *nombreHost = argv[1];
    const char *nombrePuerto = argv[2];
    // Conectamos socket
    int skt = 0;
    bool estamosConectados;
    estamosConectados = conectar_socket(&skt, nombreHost, nombrePuerto);
    if (estamosConectados == false) {
        return 1;
    }
    // Enviamos peticion
    bool seEnvioPeticion = false;
    if (argc == 3) {
        seEnvioPeticion = enviar_peticion(&skt, stdin);
    } else {
        const char *nombreArchivoPeticion = argv[3]; 
        FILE *archivoPeticion;
        if ((archivoPeticion = fopen(nombreArchivoPeticion, "rt")) == NULL) {
            fprintf(stderr, "Archivo no encontrado.\n");
            return 1;
        }
        seEnvioPeticion = enviar_peticion(&skt, archivoPeticion);
        fclose(archivoPeticion);
    }
    if (seEnvioPeticion == false){
        shutdown(skt, SHUT_RDWR);
        close(skt);
        return 1;
    }
    shutdown(skt, SHUT_WR);

    // Recibimos respuesta
    bool seRecibioRespuesta;
    seRecibioRespuesta = recibir_respuesta(&skt);
    shutdown(skt, SHUT_RDWR);
    close(skt);
    
    if (seRecibioRespuesta == false) {
        return 1;
    } else {
        return 0;
    }
}
