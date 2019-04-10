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
#include "common.h"

#define PAQUETE_LARGO_MAXIMO 512 // 1/2 k

/*
PRE: Recibe un cliente (cliente_t*) ya declarado, 
y los nombres del host y puerto al que se conecta.
POST: Devuelve true si logro inicializar al cliente 
recibido, false en caso contrario.
*/
bool cliente_crear(cliente_t *cliente, const char *host, const char *puerto){
    cliente->skt = malloc(sizeof(socket_t));
    if (cliente->skt == NULL) {
        return false;
    }
    socket_crear(cliente->skt);
    bool seConecto;
    seConecto = socket_conectar(cliente->skt, host, puerto);
    if (!seConecto) {
        free(cliente->skt);
        return false;
    }
    return true;
}

/*
PRE: Recibe un cliente (cliente_t*).
POST: Destruye al cliente.
*/
void cliente_destruir(cliente_t *cliente){
    if (cliente->skt != NULL) {
        socket_cerrar_canal(cliente->skt, SHUT_RDWR);
        socket_cerrar(cliente->skt);
        socket_destruir(cliente->skt);
        free(cliente->skt);
    }
}

/*
PRE: Recibe un cliente (cliente_t*) ya inicializado,
y el nombre de una ruta a un archivo con la peticion
http a enviar.
POST: Devuelve true si logro enviar la peticion a traves 
del puerto al que se conecta el cliente, false en caso
de la gun error. 
Si la ruta recibida es NULL, lee la peticion desde
la entrada estandard (stdin).
*/
bool cliente_enviar_peticion(cliente_t *cliente,const char *rutaPeticion) {
    FILE *fichero = stdin;
    if (rutaPeticion != NULL) {
        if ((fichero = fopen(rutaPeticion, "rt")) == NULL) {
            fprintf(stderr, "Archivo peticion no encontrado.\n");
            return false;
        }
    }
    bool hayError = false;
    char paquete[PAQUETE_LARGO_MAXIMO];
    while (!feof(fichero) && !hayError){
        int leidos;
        size_t largoTipo = sizeof(char);
        size_t largoBuffer = sizeof(paquete);
        leidos = fread((void*)&paquete, largoTipo, largoBuffer, fichero); 
        if (leidos) {
            bool seEnvio;
            seEnvio = socket_enviar_todo(cliente->skt, paquete, leidos);
            hayError = !seEnvio;
        }
    }
    if (rutaPeticion != NULL) {
        fclose(fichero);
    }
    socket_cerrar_canal(cliente->skt, SHUT_WR);
    return ! hayError;

}
/*
PRE: Recibe un cliente (cliente_t*) ya inicializado. 
POST: Recibe una respuesta desde el puerto al que esta
conectado, y la imprime por salida estandard.
Devuelve true, si se logro lo anterior, false en contrario,
dado algun error.
*/
bool cliente_recibir_respuesta(cliente_t *cliente){
    char respuesta[PAQUETE_LARGO_MAXIMO];
    int largo = PAQUETE_LARGO_MAXIMO;
    int recibidos;
    recibidos = socket_recibir_algo(cliente->skt, respuesta, largo);
    while (recibidos > 0){
        printf("%s", respuesta);
        recibidos = socket_recibir_algo(cliente->skt, respuesta, largo);
    }
    socket_cerrar_canal(cliente->skt, SHUT_RD);
    if (recibidos < 0) {
        return false;
    }
    return true;
}

int main(int argc, const char* argv[]) {
    if (argc < 3 || argc > 4) {
        fprintf(stderr,"Uso:\n./client <direccion> <puerto> [<input>]\n");
        return 1;
    }
    const char *nombreHost = argv[1];
    const char *nombrePuerto = argv[2];
    const char *rutaPeticion = NULL;
    if (argc == 4) {
        rutaPeticion = argv[3];
    }
    cliente_t cliente;
    bool todoOK;
    todoOK = cliente_crear(&cliente, nombreHost, nombrePuerto);
    if (!todoOK) {
        return 1;
    }
    todoOK = cliente_enviar_peticion(&cliente, rutaPeticion);
    if (!todoOK) {
        cliente_destruir(&cliente);
        return 1;
    }
    todoOK = cliente_recibir_respuesta(&cliente);
    cliente_destruir(&cliente);
    if (!todoOK){
        return 1;
    }
    return 0;
}
