#ifndef SERVER_H
#define SERVER_H
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct linea {
    char *caracteres;
    size_t largo;
    size_t _largoReservado;
} linea_t;

/*
Inicializa una linea_t
Pre: Recibe una linea ya declarada (linea_t *)
Post: Devuelve true si se inicializo la linea con exito, 
false en caso contrario
*/
bool linea_crear(linea_t *linea);

/*
Agrega caracteres a la linea
Pre: Recibe una cadena de caracteres (char *) terminada en \0.
Post: Agrega tantos caracteres de la cadena hasta encontrar un \n,
o hasta llegar al final de la misma.
Devuelve la cantidad de caracteres que sobraron desde el \n 
(inclusive).
Devuelve -1 en caso de no poder agregar ningun caracter, por causa 
de algun error.
*/
int linea_agregar_caracteres(linea_t *linea, char *cadena);

/*
Borra los caracteres agregados de la linea.
Pre: Recibe una linea (linea_t *) ya  inicializada.
Post: Borra la linea. Tras ejecutar esta linea, los proximos
caracteres a agregar, se escribiran desde el principio de la 
linea.
*/
void linea_borrar(linea_t *linea);

/*
Devuelve un arreglo de los caracteres que forman la linea.
Pre: Recibe una linea (linea_t *) ya inicializada.
Post: Devuelve  un arreglo de caracteres (char *) con todos 
los caracteres agregados hasta el momento; o NULL si hubo 
algun error. 
Queda a responsabilidad del usuario liberar la memoria 
reservada para el arreglo, por medio de la funcion free().
*/
char *linea_ver_caracteres(linea_t *linea);

/*
Destruye la linea.
Pre: Recibe una linea (linea_t *) ya inicializada.
Post: Destruye la linea.
*/
void linea_destruir(linea_t *linea);

typedef struct recursoVisitado {
    char *nombreRecurso;
    size_t vecesVisitado;
} recursoVisitado_t;

/*
Inicializa un recursoVisitado_t
Pre: Recibe un recurso visitado (recursoVisitado *) ya 
declarado, y su nombre (char *), terminado en \0.
Post: Devuelve true si logro inicializar el recurso
con exito, false en caso contrario.
*/
bool recursoVisitado_crear(recursoVisitado_t *recurso, char *nombre);

/*
Devuelve el nombre del recurso.
Pre: Recibe un recurso visitado (recursoVisitado *) ya 
inicializado.
Post: Devuelve el nombre del recurso (char *). 
*/
char *recursoVisitado_ver_nombre(recursoVisitado_t *recurso);

/*
Devuelve la cantidad de visitas.
Pre: Recibe un recurso visitado (recursoVisitado *) ya 
inicializado.
Post: Devuelve la cantidad de veces (int) que el recurso 
fue visitado.
*/
int recursoVisitado_ver_visitas(recursoVisitado_t *recurso);

/*
Visita el recurso.
Pre: Recibe un recurso visitado (recursoVisitado *) ya 
inicializado.
Post: Visita el recurso, aumentando la cantidad de visitas.
*/
void recursoVisitado_visitar(recursoVisitado_t *recurso);

/*
Destruye el recurso.
Pre: Recibe un recurso visitado (recursoVisitado_t *) ya 
inicializado y lo destruye.
Post: Destruye el recurso.
*/
void recursoVisitado_destruir(recursoVisitado_t *recurso);

typedef struct recursosVector {
    recursoVisitado_t **recursos;
    size_t _cantidadReservada; 
    size_t cantidad;
} recursosVector_t;

/*
Inicializa un recursosVector_t.
Pre: Recibe un vector de recursos visitados (recursosVector_t *) 
ya declarado.
Post: Devuelve true si logro inicializarlo con exito, false en
caso contrario.
*/
bool recursosVector_crear(recursosVector_t *vector);

/*
Visita un recurso. 
Pre: Recibe un vector de recursos visitados (recursosVector_t *), y
el nombre de un recurso a visitar.
Post: Devuelve true si lo logro visitar con exito el recurso de 
nombre recibido, o false en caso contrario.
Nota: el recurso recibido se agrega al vector de forma automatica
si no esta presente. 
*/
bool recursosVector_visitar(recursosVector_t *vector, char *nombreRecurso);

/*
Pre: Recibe un vector de recursos visitados (recursosVector_t *)
ya creado e inicializado.
Post: Imprime por salida estandar la estadistica de los recursos
visitados, con el siguiente formato:
# Estadisticas de visitantes:

* <nombre-visitante-1>: <cantidad de visitas>
* <nombre-visitante-2>: <cantidad de visitas>
  .
  .
  .
* <nombre-visitante-n>: <cantidad de visitas> 
*/
void recursosVector_imprimir(recursosVector_t *vector);

/*
Destruye el recursosVector_t.
Pre: Recibe un vector de recursos visitados (recursosVector_t *)
ya creado e inicializado.
Post: Destruye el vector recibido.
*/
void recursosVector_destruir(recursosVector_t *vector);

/*
Pre: Recibe dos cadena de caracteres (char *): un texto, 
y un separador para del cual splitear el texto. El separador
debe pertencer al texto.
Post: Devuelve un arreglo de cadenas, terminada en NULL
(char **), o NULL si ocurrio algun error.
Queda a responsabilidad del usuario liberar la memoria
reservada por de la funcion free_split().
*/
char **split(const char *texto, char* separador);

/*
Pre: Recibe un arreglo de punteros reservado en memoria 
dinamica, terminado en NULL. Donde, ademas, cada puntero 
del arreglo apunta a un otro bloque de memoria dinamica
reservada.
Cada uno de estos bloques individuales de memoria deben
poder ser liberados con la funcion free().
Post: libera toda la memoria reservada.
*/
void free_split(char **arreglo);

/*
Pre: Recibe la primer linea de la peticion a responder 
(char *) terminada en '\0'.
Post: Devuelve una cadena de caractares con la respuesta 
a la peticion cuya primer linea recibidmos; o NULL si 
hubo algun error.
Queda a responsabilidad del usuario liberar dicha memoria
reservada, por medio de la funcion free().
*/
char *responder_peticion(const char *primerLinea, char *cuerpo);

/*
Pre: Recibe una cadena de caracteres (char *) con la siguiente 
linea del cuerpo de la peticion a procesar, y un vector con los
visitantes de peticiones anteriores (recursosVector_t *).
Post: Devuelve true si logro procesa la linea recibida con exito, 
actualizando la informacion del vector de visitas recibido; o
false en caso contrario.
*/
bool procesar_lineas_cuerpo(char *linea, recursosVector_t *visitantes);

/*
Pre: Recibe una cadena de caracteres (char *) correspondiente a
una parte del cuerpo de la peticion cargada. Tambien recibe
una linea ya inicializada con caracteres del cuerpo que preceden 
al recibido y que sabemos que aun no se encontro un \n entre ellos.
Y, por ultimo, recibe un vector de recursos visitados 
(recursosVisitados_t *) con las visitas de otras peticiones 
anteriores.
Post: Devuelve true si logra procesar la parte del cuerpo recibida,
false en caso contrario.
*/
bool procesar_cuerpo(char *cuerpo, linea_t *linea, recursosVector_t *vector);

/*
Pre: Recibe un socket (int *) ya conectado, el template (char *) ya
armado, es decir, el cuerpo de la respuesta a la peticion actual, si 
es que este es valida. Tambien recibe un vector de recursos visitados
con las visitas a peticiones anteriores. 
Post: Recibe y procesa una peticion del cliente. Devuelve la respuesta 
a la misma (char *), modificando,o  NULL si hubo algun error con el 
socket.
Actualiza los recursos visitados.
Queda a responsabilidad del usuario liberar la memoria
reservada para la linea, por medio de la funcion free().
*/
char *procesar_peticion(int *skt, char* template, recursosVector_t *visitantes);

/*
Pre: Recibe un socket ya conectado: skt (int *), y la 
respuesta que a enviar (char *).
Post: Devuelve true si logro enviar toda la respuesta, 
false en caso contrario, dado un error en el socket.
*/
bool enviar_respuesta(int *skt, char *mensaje, size_t largoMensaje);

/*
Pre:  Recibe un socket (int *), y el nombre del puerto 
(char *) al cual conectarse.
Post: Devuelve true si logro conectar el socket al puerto
recibido y setearlo para funcionar de socket pasivo; false 
en caso contrario.
*/
bool conectar_socket_pasivo(int *skt, const char *puerto);

/*
Pre: Recibe un archivo de texto ya abierto.
Post: Devuelve una cadena de caracteres (terminada en \0) que 
contiene todos los carateres del archivo desde el inicio hasta
una linea vacia o fin de archivo; o NULL si hubo algun error 
durante la carga.
Queda a respondabilidad del usuario liberar la memoria reservada 
para la cadena, por medio de free().
*/
char *cargar_archivo(FILE *archivo);

/*
Pre: Recibe la ruta del archivo template a cargar,
que en algun momento del mismo tiene la cadena de 
caracteres: "{{datos}}".
Post: Devuelve un arreglo de cadenas de caracteres,
con las dos partes del texto template, antes y 
despues de la cadena: "{{datos}}"; o NULL si ocurrio
algun problema.
Queda a responsabilidad del usuario liberar la memoria 
dinamica reservada para el arreglo, por medio de la 
funcion: void free_split(void **).
*/
char **cargar_template(const char *ruta);

/*
Pre: Recibe las dos partes del template (char **), y 
la temperatura (double) a meter entre dichas partes.
Post: Devuelve el cuerpo de la respuesta (char *), que
viene a ser las dos partes del template unidas por medio
de la temperatura recibida; o NULL si ocurrio algun error.
Queda a responsabilidad del usuario liberar la memoria
reservada por medio de la funcion free(); 
*/
char *contruir_cuerpo(char **partesTemplate, double temperatura);

#endif // SERVER_H
