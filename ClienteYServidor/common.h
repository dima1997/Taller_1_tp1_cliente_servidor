#ifndef COMMON_H
#define COMMON_H

#define _POSIX_C_SOURCE 200112L
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

/*
Pre: Recibe un socket ya conectado: skt (int), y un puntero al 
mensaje que a enviar (char *).
Post: Devuelve true si logro enviar toda la peticion, false en caso 
contrario, dado un error en el socket o si el socket remoto fue cerrado.
*/
bool enviar_mensaje(int skt, char *mensaje, size_t largoMensaje);

/*
Pre: recibe un socket ya conectado: skt (int), y
recibe un puntero al buffer donde guardar el mensaje
recibido.
Post: Devuelve true si logro recibir todo el mensaje,
false en caso contrario, dado algun error de socket.
*/
bool recibir_mensaje(int skt, char *mensaje, size_t largoMaximoMensaje);

/*
 * Devuelve en un arreglo dinámico terminado en NULL con todos los subsegmentos
 * de ‘str’ separados por el carácter ‘sep’. Tanto el arreglo devuelto como las
 * cadenas que contiene son allocadas dinámicamente.
 *
 * Quien llama a la función toma responsabilidad de la memoria dinámica del
 * arreglo devuelto. La función devuelve NULL si falló alguna llamada a
 * malloc(), o si ‘sep’ es '\0'.
 */
char** split(const char* str, char sep);

/*
 * Libera un arreglo dinámico de cadenas, y todas las cadenas que contiene.
 */
void free_strv(char* strv[]);

/*
Pre: recibe un archivo de texto ya abierto.
Post: Devuelve una cadena de caracteres (terminada en \0) que 
contiene todos los carateres del archivo desde el inicio hasta
una linea vacia o fin de archivo; o NULL si hubo algun error 
durante la carga.
Queda a respondabilidad del usuario liberar la memoria reservada 
para la cadena, por medio de free
*/
char *cargar_archivo(FILE *archivo);

#endif // COMMON_H
