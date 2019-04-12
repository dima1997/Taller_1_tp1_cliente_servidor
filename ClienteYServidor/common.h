#ifndef COMMON_H
#define COMMON_H
#define _POSIX_C_SOURCE 200112L
#include <stdbool.h>

typedef struct Socket {
    int skt;
} socket_t;

/*
PRE: Recibe un socket (socket_t*)
POST: Inicializa el socket dejandolo en un estado
invalido hasta tanto no llamar a otros metodos del 
socket.
*/
void socket_crear(socket_t *skt);

/*
PRE: Recibe un socket ya creado.
POST: Destruye el socket.
*/
void socket_destruir(socket_t* skt);

/*
Metodo para cliente.
PRE: Recibe los nombres (const char*) del host y 
puerto al que se desea conectar el socket.
*/
bool socket_conectar(socket_t* skt, const char* host, const char* puerto);


/*
Metodo para Servidor.
PRE: Recibe un socket (socket_t *) ya creado, y el
nombre (char *) de un puerto al cual enlazarlo.
POST: Configura al socket para que funcion de 
forma PASIVA, es decir, se lo enlaza al puerto de
nombre recibido. 
Devuelve true, si logro lo anterior, false en caso
contrario.
*/
bool socket_enlazar(socket_t *skt, const char *puerto);

/*
Metodo para Servidor.
PRE: Recibe una socket (socket_t *) ya enlazado a algun puerto,
y la cantidad de sockets entrantes a escuchar.
POST: Pone a escuchar, al socket recibido, la cantidad de 
recibida de sockets entrantes.
Devuelve true, si logro lo anterior, false en caso contrario.
*/
bool socket_escuchar(socket_t *skt, size_t cuantosEscuchar);

/*
PRE: Recibe dos socket (socke_t *): el primero esta configurado 
como PASIVO (se ejecutaron metodos socket_enlazar y ..._escuchar),
y el segundo (ya creado) para ser configurado como ACTIVO. 
POST: Devuelve true si logro aceptar una nueva comunicacion y
configurar al socket ACTIVO para la misma, o false en caso 
contrario. 
*/
bool socket_aceptar(socket_t *sktPasivo, socket_t *sktActivo); 

/*
PRE: Recibe un socket ya conectado (socket_t*), y
el modo en que se desea cerrar al socket:
SHUT_RD, SHUT_WR, SHUT_RDWR
POST: Cierra el socket. 
*/
void socket_cerrar_canal(socket_t* skt, int modoCierre);

/*
PRE: Recibe un socket ya conectado (socket_t*)
POST: Cierra el socket, dejandolo en un estado
invalido.
*/
void socket_cerrar(socket_t* skt);

/*
PRE: Recibe un socket ya conectado (socket_t*), un 
mensaje a enviar, y su longitud.
POST: Devuelve true si logro enviar todo el mensaje, 
false en caso contrario, dado algun error.
*/
bool socket_enviar_todo(socket_t* skt, char* msj, int largo);

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
int socket_recibir_algo(socket_t* skt, char* buffer, int largo);
#endif // COMMON_H
