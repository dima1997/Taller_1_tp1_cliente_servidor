#ifndef CLIENT_H
#define CLIENT_H
#include <stdbool.h>
#include "common.h"

typedef struct Cliente{
    socket_t* skt;
} cliente_t;

/*
PRE: Recibe un cliente (cliente_t*) ya declarado, 
y los nombres del host y puerto al que se conecta.
POST: Devuelve true si logro inicializar al cliente 
recibido, false en caso contrario.
*/
bool cliente_crear(cliente_t *cliente, const char *host, const char *puerto);

/*
PRE: Recibe un cliente (cliente_t*).
POST: Destruye al cliente.
*/
void cliente_destruir(cliente_t *cliente);

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
bool cliente_enviar_peticion(cliente_t *cliente,const char *rutaPeticion);

/*
PRE: Recibe un cliente (cliente_t*) ya inicializado. 
POST: Recibe una respuesta desde el puerto al que esta
conectado, y la imprime por salida estandard.
Devuelve true, si se logro lo anterior, false en contrario,
dado algun error.
*/
bool cliente_recibir_respuesta(cliente_t *cliente);

#endif // CLIENT_H
