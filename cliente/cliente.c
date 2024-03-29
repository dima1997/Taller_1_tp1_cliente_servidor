// Cliente

/*
El cliente se ejecuta con la siguiente línea de comandos:
./client ​ <host> <port> ​ [ ​ <filename>​ ]
Donde ​ <host>​ y ​ <port>​ son la dirección IPv4 o ​ hostname ​ y el puerto o servicio donde el
servidor estará escuchando la conexión TCP.
<filename>​ es un argumento opcional que indica el ​ archivo de texto ​ con el requ
*/

//1ero: Abrir archivo e imprimirlo en pantalla
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

#define MENSAJE_LARGO_MAXIMO 1000


/*
Pre: recibe un archivo (FILE*)ya abierto de donde leer la peticion.
Post: devuelve un puntero a un vector dinamico de chars (vector_t*) 
almacenado de memoria dinamica, que puede ser liberada por medio de
vector_destruir(<vector>).
El vector tiene un tamanio igual a la cantidad de caracteres que 
almacena (ni mas, ni menos).
Si hubo algun problema al cargar la peticion, devuelve NULL.
*/
vector_t *cargar_peticion(FILE* archivoPeticion){
    size_t largoPeticion = 200;
    vector_t *peticion = vector_crear(largoPeticion);

    char caracter;
    char caracterAnterior = '\0';
    size_t i = 0; 
    while ((caracter = getc(archivoPeticion)) != -1){ //eof
        if (caracter == '\n') {
            if (caracterAnterior == '\n' || caracterAnterior == '\0'){
                //Una linea vacia marca el final de la peticion
                //Para cuando archivoPeticion es stdin
                break;
            }
        }
        if (i >= largoPeticion){
            size_t nuevoLargo = largoPeticion*2;
            bool seRedimensiono = vector_redimensionar(peticion, nuevoLargo);
            if (!seRedimensiono) {
                vector_destruir(peticion);
                return NULL;
            }
            largoPeticion = nuevoLargo;
        }
        vector_guardar(peticion, i, caracter);
        ++i;
        caracterAnterior = caracter;
    }
    size_t largoFinal = i;
    bool seRedimensiono = vector_redimensionar(peticion, largoFinal);
    if (!seRedimensiono) {
        vector_destruir(peticion);
        return NULL;
        }
    return peticion;
    }

int main(int argc, const char* argv[]) {
    if (argc < 3 || argc > 4) {
        fprintf(stderr,"Uso:\n./client <direccion> <puerto> [<input>]\n");
        return 1;
    }
    const char *nombreHost = argv[1];
    const char *nombrePuerto = argv[2];
    vector_t *peticionVector;
    if (argc == 3) {
        peticionVector = cargar_peticion(stdin);
            if (peticionVector == NULL){
            return 1;
        }
    } else {
        FILE *archivoPeticion;
        const char *nombreArchivoPeticion = argv[3]; 
        if ((archivoPeticion = fopen(nombreArchivoPeticion, "rt")) == NULL) {
            fprintf(stderr, "Archivo no encontrado.\n");
            return 1;
        }
        peticionVector = cargar_peticion(archivoPeticion);
        fclose(archivoPeticion);
        if (peticionVector == NULL){
            return 1;
        }
    }
    // Sockets
    int estado = 0;
    bool estamosConectados = false;
    struct addrinfo hints;
    struct addrinfo *direcciones, *ptr;
    int skt = 0;
    estado = getaddrinfo(nombreHost, nombrePuerto, &hints, &direcciones);
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;     //IPv4
    hints.ai_socktype = SOCK_STREAM;  //TCP
    hints.ai_flags = 0;
    estado = getaddrinfo(nombreHost, nombrePuerto, &hints, &direcciones);
    if (estado != 0) { 
        printf("Error in getaddrinfo: %s\n", gai_strerror(estado));
        return 1;
    }
    for (ptr = direcciones; ptr != NULL && estamosConectados == false; ptr = ptr->ai_next) {
        skt = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (skt == -1) {
            printf("Error: %s\n", strerror(errno));
        } else {
        estado = connect(skt, ptr->ai_addr, ptr->ai_addrlen);
        if (estado == -1) {
            printf("Error: %s\n", strerror(errno));
            close(skt);
            }
        estamosConectados = (estado != -1); 
        }
    }
    freeaddrinfo(direcciones);
    if (estamosConectados == false) {
        return 1; 
    }
    //----------------------------------------------------------------
    // cargamos en un buffer el la peticion a enviar
    size_t largoPeticion = vector_obtener_tamanio(peticionVector);
    char buffer[MENSAJE_LARGO_MAXIMO];
    for (int i = 0; i < largoPeticion; ++i) {
        vector_obtener(peticionVector, i, &buffer[i]);
    }
    vector_destruir(peticionVector);
    //----------------------------------------------------------------
    int bytesEnviados;
    bytesEnviados = enviar_mensaje(skt, buffer, largoPeticion);
    if (bytesEnviados < largoPeticion){
        shutdown(skt, SHUT_RDWR);
        close(skt);
        return 1;
    }

    shutdown(skt, SHUT_WR);
    
    size_t largoMaximo = MENSAJE_LARGO_MAXIMO;
    int bytesRecibidos;
    bytesRecibidos = recibir_mensaje(skt, buffer, largoMaximo-1);
    shutdown(skt, SHUT_RDWR);
    close(skt);
    
    if (bytesRecibidos < 0) {
        return 1;   
    } else {
        buffer[bytesRecibidos] = "\0";
        printf("%s\n", buffer);
        return 0;
    }
}

