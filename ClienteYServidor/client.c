// Cliente

/*
El cliente se ejecuta con la siguiente línea de comandos:
./client ​ <host> <port> ​ [ ​ <filename>​ ]
Donde ​ <host>​ y ​ <port>​ son la dirección IPv4 o ​ hostname ​ y el puerto o servicio donde el
servidor estará escuchando la conexión TCP.
<filename>​ es un argumento opcional que indica el ​ archivo de texto ​ con el requ
*/

//1ero: Abrir archivo e imprimirlo en pantalla
//#define _POSIX_C_SOURCE 200112L
#define _POSIX_C_SOURCE 200809L
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
/*
Pre: recibe un archivo de texto ya abierto.
Post: Devuelve una cadena de caracteres (terminada en \0) que 
contiene todos los carateres del archivo desde el inicio hasta
una linea vacia o fin de archivo; o NULL si hubo algun error 
durante la carga.
Queda a respondabilidad del usuario liberar la memoria reservada 
para la cadena, por medio de free
*/
char *cargar_archivo(FILE *archivo){
    size_t factorRedimension = 2;
    size_t largoTexto = 1000;
    size_t tamanioInicial = largoTexto * sizeof(char);
    char *texto = (char *)malloc(tamanioInicial);
    if (texto == NULL){
        return NULL;
    }

    char caracter;
    char caracterAnterior = '\0';
    size_t i = 0; 
    while ((caracter = getc(archivo)) != -1){ //eof
        if (caracter == '\n') {
            if (caracterAnterior == '\n' || caracterAnterior == '\0'){
                //Una linea vacia marca el final de la peticion
                //Para cuando archivoPeticion es stdin
                break;
            }
        }
        if (i >= largoTexto){
            size_t nuevoLargo = largoTexto*sizeof(char)*factorRedimension;
            char *nuevoTexto = realloc(texto, nuevoLargo);
            if (nuevoTexto == NULL) {
                free(texto);
                return NULL;
            }
            largoTexto = nuevoLargo;
        }
        texto[i] = caracter;
        ++i;
        caracterAnterior = caracter;
    }
    size_t largoFinal = i+1;
    char *textoFinal = realloc(texto, largoFinal);
    if (textoFinal == NULL) {
        free(texto);
        return NULL;
    }
    texto = textoFinal;
    texto[i] = '\0';
    return texto;
}


int main(int argc, const char* argv[]) {
    if (argc < 3 || argc > 4) {
        fprintf(stderr,"Uso:\n./client <direccion> <puerto> [<input>]\n");
        return 1;
    }
    const char *nombreHost = argv[1];
    const char *nombrePuerto = argv[2];
    char *peticion;
    if (argc == 3) {

        peticion = cargar_archivo(stdin);
            if (peticion == NULL){ 
            return 1;
        }
    } else {
        FILE *archivoPeticion;
        const char *nombreArchivoPeticion = argv[3]; 
        if ((archivoPeticion = fopen(nombreArchivoPeticion, "rt")) == NULL) {
            fprintf(stderr, "Archivo no encontrado.\n");
            return 1;
        }

        peticion = cargar_archivo(archivoPeticion);
        fclose(archivoPeticion);
        if (peticion == NULL){ 
            return 1;
        }
    }
    // Sockets
    int estado = 0;
    bool estamosConectados = false;
    struct addrinfo hints;
    struct addrinfo *direcciones, *ptr;
    int skt = 0;

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

    int bytesEnviados;
    size_t largoPeticion = strlen(peticion);
    bytesEnviados = enviar_mensaje(skt, peticion, largoPeticion); //buffer
    if (bytesEnviados < 0){
        shutdown(skt, SHUT_RDWR);
        close(skt);
        return 1;
    }

    shutdown(skt, SHUT_WR);
    free(peticion);

    char respuesta[MENSAJE_LARGO_MAXIMO];
    size_t largoMaximo = sizeof(respuesta);
    int bytesRecibidos;
    bytesRecibidos = recibir_mensaje(skt, respuesta, largoMaximo-1);
    shutdown(skt, SHUT_RDWR);
    close(skt);
    
    if (bytesRecibidos < 0) {
        return 1;   
    } else {
        return 0;
    }
}

