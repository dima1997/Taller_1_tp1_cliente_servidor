#ifndef COMMON_H
#define COMMON_H

#define _POSIX_C_SOURCE 200809L
//#define _POSIX_C_SOURCE 200112L
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

/*******************************************************************
 *                DEFINICION DE LOS TIPOS DE DATOS
 ******************************************************************/

typedef struct vector {
    size_t tam;
    char* datos;
} vector_t;


/*******************************************************************
 *                    PRIMITIVAS DEL VECTOR
 ******************************************************************/

// Crea un vector de tamaño tam
// Post: vector es una vector vacío de tamaño tam
vector_t* vector_crear(size_t tam);

// Destruye el vector
// Pre: el vector fue creado
// Post: se eliminaron todos los elementos del vector
void vector_destruir(vector_t* vector);

// Cambia el tamaño del vector
// Pre: el vector fue creado
// Post: el vector cambió de tamaño a nuevo_tam y devuelve true
// o el vector queda intacto y devuelve false si no se pudo cambiar el tamaño
// a nuevo_tam
bool vector_redimensionar(vector_t* vector, size_t nuevo_tam);

// Almacena en valor el dato guardado en la posición pos del vector
// Pre: el vector fue creado
// Post: se almacenó en valor el dato en la posición pos. Devuelve false si la
// posición es inválida (fuera del rango del vector, que va de 0 a tamaño-1)
bool vector_obtener(vector_t* vector, size_t pos, char* valor);

// Almacena el valor en la posición pos
// Pre: el vector fue creado
// Post: se almacenó el valor en la posición pos. Devuelve false si la posición
// es inválida (fuera del rango del vector, que va de 0 a tamaño-1) y true si
// se guardó el valor con éxito.
bool vector_guardar(vector_t* vector, size_t pos, char valor);

// Devuelve el tamaño del vector
// Pre: el vector fue creado
size_t vector_obtener_tamanio(vector_t* vector);

// Pre: Recibe un puntero a vector dinamico de chars (vector_t).
// Post: imprime por salida estandard (stdout) el contenido del 
// vector.
void vector_imprimir(vector_t *vector);

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
#endif // COMMON_H
