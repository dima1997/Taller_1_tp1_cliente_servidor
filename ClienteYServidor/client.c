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
#define PAQUETE_LARGO_MAXIMO 1024 // 1 k

/*
Pre: Recibe un socket ya contectado.
Post: Recibe un mensaje del servidor y lo imprime por
salida estandard. Devuevle true si logro recibir todo 
el mensaje, false en caso contrario.
*/
bool cliente_recibir_mensaje(int skt) {
    int estado = 0;
    bool hayErrorDeSocket = false;
    bool estaSocketRemotoCerrado = false;
    int bytesRecibidos = 0;
    char parteDelMensaje[PAQUETE_LARGO_MAXIMO];
    size_t largoMaximo = sizeof(PAQUETE_LARGO_MAXIMO);
    while (hayErrorDeSocket == false && estaSocketRemotoCerrado == false) {
        estado = recv(skt, &parteDelMensaje, largoMaximo - 1, MSG_NOSIGNAL); 
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
            parteDelMensaje[bytesRecibidos] = 0;
            printf("%s", parteDelMensaje);
            bytesRecibidos = 0; 
        }
    }
    return !(hayErrorDeSocket);
}
/*
Pre: Recibe un socket ya conectado, y un archivo de texto 
ya abierto de donde se leera el mensaje a enviar.
Post: Envia el mensaje en archivo recibido, a donde sea que 
este conectado el socket. Devuelve true si logro enviar todo
el mensaje; false en caso contrario dado algun error del
socket. 
*/
bool cliente_enviar_mensaje(int skt, FILE *archivo) {
    int estado = 0;
    bool hayErrorDeSocket = false;
    int bytesEnviados = 0;
    char parteDelMensaje[PAQUETE_LARGO_MAXIMO];
    size_t largoMensaje = sizeof(parteDelMensaje);
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
        parteDelMensaje[i] = caracter;
        if (i >= largoMensaje) {
            estado = send(skt, &parteDelMensaje, largoMensaje, MSG_NOSIGNAL);
            if (estado < 0) { 
                printf("Error: %s\n", strerror(errno));
                hayErrorDeSocket = true;
            } else if (estado == 0) { 
                // Bajo las hipotesis del TP, el que envia 
                // el mensaje deberia cerrar el canal
                hayErrorDeSocket = true;
            } else {
                bytesEnviados = estado;
                int bytesSinEnviar = largoMensaje - bytesEnviados;
                for (int j = 0; j < bytesSinEnviar ; ++j) {
                    int posicionByteSinEnviar; 
                    posicionByteSinEnviar = largoMensaje - bytesSinEnviar + j;
                    parteDelMensaje[j] = parteDelMensaje[posicionByteSinEnviar];
                }
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
        estado = send(skt, &parteDelMensaje, i, MSG_NOSIGNAL);
        if (estado < 0) { 
            printf("Error: %s\n", strerror(errno));
            hayErrorDeSocket = true;
        } else if (estado == 0) { 
            hayErrorDeSocket = true;
        } else {
            bytesEnviados = estado;
            int bytesSinEnviar = i - bytesEnviados;
            for (int j = 0; j < bytesSinEnviar ; ++j){
                int posicionByteSinEnviar = i - bytesSinEnviar + j;
                parteDelMensaje[j] = parteDelMensaje[posicionByteSinEnviar];
            }
            i = bytesSinEnviar;
        }
    }
    return !(hayErrorDeSocket);
}

/*
Pre: recibe un socket (int *), y los nombres del
host y el puerto al que se desea conectar.
Post: Devuelve true si logro pasar todo el proceso 
de conexion del socket con exito, false en caso 
contrario.
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
    int skt = 0;
    bool estamosConectados;
    estamosConectados = conectar_socket(&skt, nombreHost, nombrePuerto);
    if (estamosConectados == false) {
        return 1;
    }
    bool seEnvioPeticion = false;
    if (argc == 3) {
        seEnvioPeticion = cliente_enviar_mensaje(skt, stdin);
    } else {
        const char *nombreArchivoPeticion = argv[3]; 
        FILE *archivoPeticion;
        if ((archivoPeticion = fopen(nombreArchivoPeticion, "rt")) == NULL) {
            fprintf(stderr, "Archivo no encontrado.\n");
            return 1;
        }
        seEnvioPeticion = cliente_enviar_mensaje(skt, archivoPeticion);
        fclose(archivoPeticion);
    }
    if (seEnvioPeticion == false){
        shutdown(skt, SHUT_RDWR);
        close(skt);
        return 1;
    }
    shutdown(skt, SHUT_WR);

    bool seRecibioRespuesta;
    seRecibioRespuesta = cliente_recibir_mensaje(skt);
    shutdown(skt, SHUT_RDWR);
    close(skt);
    
    if (seRecibioRespuesta == false) {
        return 1;
    } else {
        return 0;
    }
}
