#ifndef CLIENT_H
#define CLIENT_H
#include <stdbool.h>

/*
Pre: Recibe un socket (int *) ya contectado.
Post: Recibe una respuesta del servidor y la imprime por
salida estandard. Devuelve true si logro recibir todo 
el mensaje, false en caso contrario.
*/
bool recibir_respuesta(int *skt);

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
int enviar_parte(int *skt, char* parte, int largo);

/*
Pre: Recibe un socket (int *) ya conectado, y un archivo (FILE *)
de texto ya abierto de donde se leera la peticion a enviar.
Post: Envia la peticion del archivo recibido, a donde sea que 
este conectado el socket. Devuelve true si logro enviar todo
el mensaje; false en caso contrario dado algun error del
socket. 
*/
bool enviar_peticion(int *skt, FILE *archivo);

/*
Pre: Recibe un socket (int *), y los nombres (char *) 
del host y puerto al que se desea conectar.
Post: Devuelve true si logro se logro conectarse con exito, 
false en caso contrario.
*/
bool conectar_socket(int *skt, const char* host, const char* puerto);

#endif // CLIENT_H
