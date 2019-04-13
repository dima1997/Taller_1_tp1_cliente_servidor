#ifndef SERVER_H
#define SERVER_H
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef enum {
    PETICION_VALIDA,
    PETICION_INVALIDA,
    PETICION_ERROR,
} peticion_t;

typedef struct Nodo {
    void *dato;
    struct Nodo *proximo;
} lista_nodo_t;


typedef struct ListaEnlazada {
    lista_nodo_t *primero;
    lista_nodo_t *ultimo;
    size_t largo;
} lista_t;

typedef struct Vector{
    char *datos;
    size_t largo;
} vector_t;

typedef struct servidor{
    lista_t visitas;
    skt_t skt;
    char *template;
} svr_t;

typedef struct visitante {
    char *nombreRecurso;
    size_t vecesVisitado;
} visitante_t;

// Funciones Auxiliares:

/*
PRE: Recibe una lista (lista_t *) ya creada, un buffer (char *)
de caracteres (char) y un largo (size_t).
POST: Reserva memoria para punteros a cada caracter del buffer 
hasta el largo dado, y los agrega uno a uno a la lista desde el
inicio del buffer hasta el largo recibido.
Devuelve true si logro lo anterior, false en caso contrario.
Queda a responsabilidad del usuario liberar la memoria reservada
para cada caracter mediante la funcion free().
*/
bool caracteres_a_lista(lista_t* lista, char *buffer, size_t largo);

/*
PRE: Recibe una lista (lista_t*), un buffer de caracteres (char *),
y el largo (size_t) hasta donde copiar del buffer.
POST: Crea un nueva cadena de caracteres almacenada en memoria, con
los caracteres del buffer hasta el largo recibido, terminada en \0, 
y lo agrega al final de la lista recibida.
Devuelve true si logro realizar lo anterior, false en caso contrario.
Queda a responsabilidad del usuario liberar la memoria reservada con
la funcion free().
*/
bool buffer_a_lista(lista_t *lista, char* buffer, size_t largo);

/*
PRE: Recibe dos cadenas de caraceres (char *), terminanda 
en '\0': la primera la cadena a partir en partes, y la 
segunda el separador por el cual se partira a la cadena 
anterior.
POST: Devuelve una lista (lista_t*) reservada en memoria
cuyos elementos son las subpartes del la cadena recibida
(char *), guardados como (void *), tambien reservadas en 
memoria; o NULL en caso de algun error.
Queda a responsabilidad del usuario liberar la memoria 
reservada mediante la funcion free_split2. 
*/
lista_t* split(char *cadena, char *separador);

/*
PRE: Recibe una lista (lista_t *) reservada en
memoria, cuyos elementos tambien estan reservados 
en memoria, y pueden ser liberados por medio la 
funcion free.
POST: Destruye la lista, liberando toda la memoria
mencionada.
*/
void free_split(lista_t *listaSplit);

/*
PRE: Recibe un dato (void *) que en realidad es un 
visitante (visitante_t *), que esta reservado en
memoria. 
POST: Destruye al visitante y libera la memoria 
reservada
*/
void destruir_visitante_wrapper(void *datoVisitante);

/*
PRE: Recibe dos datos (void *), donde el primero en 
realidad es un visitante (visitante_t *). El segundo
(void *) no se utiliza: puerde ser lo que sea. 
POST: Imprime el visitante y devuelve true.
*/
bool imprimir_visitante_wrapper(void *dato1, void*dato2);

/*
PRE: Recibe dos datos (void *) que son en realidad (char *).
POST: Devuelve true si los dos primeros caracteres de cada 
dato son iguales.
*/
bool son_mismo_caracter_wrapper(void *dato1, void *dato2);

/*
PRE: Recibe dos datos (void *), que en realidad, el primero es 
un visitante (visitante_t *), y el segundo es un nombre (char *).
POST: Devuelve true, si el visitante recibido tiene el nombre, 
tambien recibido.
*/
bool es_este_visitante_wrapper(void *datoVisitante, void* datoNombre);

/*
PRE: Recibe dos datos (void *), donde el primero en realidad
es un visitante (visitante_t *), y el segundo es un nombre
(char *).
Devuelve false, si se visito al visitante, true en caso 
contrario.
*/
bool visitar_si_es_este_wrapper(void *dato1, void *dato2);

/*
PRE: Recibe una lista (lista_t *) cuyos elementos sean
punteros a caracteres (char *), guardados como (void*), 
almacenados en memoria.
POST: Devuelve una cadena de caracteres reservada en 
memoria, con caracteres quitados desde el principio de
la lista hasta encontrarse con un \n. Este ultimo lo
elimina de la lista, pero no lo incluye en la cadena 
final. La cadena final termina en '\0'. O NULL, si 
ocurrio algun error o no hay '\n' en la lista.
Queda a responsabilidad del usuario liberar la memoria
reservada. 
*/
char* obtener_linea_de_lista(lista_t *caracteres);

/*
Pre: Recibe un archivo de texto ya abierto.
Post: Devuelve una cadena de caracteres (terminada en \0) que 
contiene todos los carateres del archivo desde el inicio hasta
una linea vacia o fin de archivo; o NULL si hubo algun error 
durante la carga.
Queda a respondabilidad del usuario liberar la memoria reservada 
para la cadena, por medio de free().
*/
char *cargar_archivo(const char *rutaArchivo);

//tda lista

/*
PRE: Recibe una lista (lista_t *) ya declarada.
POST: Inicializa la lista.
*/
void lista_crear(lista_t *lista);

/*
PRE: Recibe una lista (lista_t *) ya creada, y una funcion
para destruir los elementos de la lista de tipo void y que 
reciba un dato void*, o NULL
POST: 
*/
void lista_destruir(lista_t *lista, void destruir_dato(void *dato));

/*
PRE: Recibe una lista (lista_t *) ya creada, y un dato 
(void *) para insertar en la misma.
POST: Devuelve true si logro insertar el elemento en la
ultima posicion de la lista, false en caso de no poder 
insertarlo.
*/
bool lista_insertar_ultimo(lista_t *lista, void* dato);

/*
PRE: Recibe una lista (lista_t *) ya creada.
POST: Toma el primer dato de la lista, lo elimina de la misma, 
y lo devuelve (void *). Devuelve NULL, si la lista esta vacia.
*/
void *lista_borrar_primero(lista_t *lista);

/*
PRE: Recibe una lista (lista_t *) ya creada.
POST: Devuelve la cantidad de elementos en la misma.
*/
size_t lista_ver_largo(lista_t *lista);

/*
PRE: Recibe una lista (lista_t*) ya creada, una funcion
bool visitar(void *dato, void *extra), y un dato extra
(void *).
POST: Itera la lista recibida, ejecutando la funcion recibida
sobre cada uno de los elementos de la lista, hasta que la misma 
devuelva false. Si la funcion es NULL, no itera. 
*/
void lista_iterar(lista_t* lista, bool visitar(void *d, void* e), void* extra);

/*
PRE: Recibe una lista (lista_t *) ya creada, 
una funcion para comparar elementos en la lista,
y el elemento contra el que se quiera comparar.
POST: Devuelve true, alguno de los datos de la 
es igual es igual al elemento recibido, false en 
caso contrario o de un error.
*/
bool lista_esta(lista_t* lista, bool son_iguales(void* d1, void*d2 ), void* d2);

//tda vector

/*
PRE: Recibe un vector (vector_t *) ya declarado, y el
largo del vector (size_t > 0).
POST: Devuelve true si logro inicializar al vector con
exito, false en caso contrario.
*/
bool vector_crear(vector_t *vector, size_t largo);

/*
PRE: Recibe un vector (vector_t *) ya creado, y una funcion 
para destruir los datos que alamacena, o NULL en caso de no
ser necesario
POST: Destruye el vector.
*/
void vector_destruir(vector_t *vector);

/*
PRE: Recibe un vector (vector_t *) ya creado.
POST: Devuelve el largo del vector.
*/
size_t vector_ver_largo(vector_t *vector);

/*
PRE: Recibe un vector (vector_t*) ya creado, y nuevo largo 
(size_t) para redimensionar al vector.
POST: Devuelve true, si logro redimensionar al vector, false
en caso contrario.
Si el vector se agranda, las nuevas posicion quedan 
inicializadas en '\0'.
*/
bool vector_redimensionar(vector_t *vector, size_t nuevoLargo);

/*
PRE: Recibe un vector (vector_t *) ya creado y una posicion 
(size_t) del mismo.
POST: Devuelve el dato (void *) asociado al mismo, o NULL
si se fue de rango.
*/
char *vector_ver(vector_t *vector, size_t posicion);

/*
PRE: Recibe un vector (vector_t *) ya creado y una posicion 
(size_t) del mismo, y un dato (void *) a insertar.
POST: Devuelve true si logro insertar el dato en la posicion
indicada, o false en caso de algun error, o fuera de rango.
*/
bool vector_insertar(vector_t *vector, size_t posicion, char dato);

/*
PRE: Recibe un vector (vector_t *) ya creado, una posicion (size_t)
y un caracter (char) a insertar.
POST: Inserta el caracter en la posicion indicada. 
Si la posicion sobrepasa el largo de vector, se autoredimensiona
para lograr la insercion.
Devuelve true, si logro lo anterio, o false en caso contrario. 
*/
bool vector_autoinsertar(vector_t *vector, size_t posicion, char caracter);

/*
PRE: Recibe un vector (vector_t *) ya creado, y un largo 
(size_t) correspondiente a la cantidad de caracteres que
se desea convertir en cadena desde el inicio del vector.
POST: Devuelve una cadena de caracteres del largo recibido
con los caracteres desde el inicio del vector hasta completar
dicho largo, o NULL en caso de algun error (largo mayor a 
largo del vector, o largo nulo). 
Queda a responsabilidad del usuario liberar la memoria 
reservada.
*/
char *vector_obtener_cadena(vector_t *vector, size_t largo);

//tda visitante

/*
Inicializa un visitante_t
Pre: Recibe un recurso visitado (visitante *) ya 
declarado, y su nombre (char *), terminado en \0.
Post: Devuelve true si logro inicializar el recurso
con exito, false en caso contrario.
*/
bool visitante_crear(visitante_t *visitante, char *nombre);

/*
Devuelve el nombre del recurso.
Pre: Recibe un recurso visitado (visitante *) ya 
inicializado.
Post: Devuelve el nombre del recurso (char *). 
*/
char *visitante_ver_nombre(visitante_t *visitante);

/*
Devuelve la cantidad de visitas.
Pre: Recibe un recurso visitado (visitante *) ya 
inicializado.
Post: Devuelve la cantidad de veces (int) que el recurso 
fue visitado.
*/
int visitante_ver_visitas(visitante_t *visitante);

/*
Visita el recurso.
Pre: Recibe un recurso visitado (visitante *) ya 
inicializado.
Post: Visita el recurso, aumentando la cantidad de visitas.
*/
void visitante_visitar(visitante_t *visitante);

/*
Destruye el recurso.
Pre: Recibe un recurso visitado (visitante_t *) ya 
inicializado y lo destruye.
Post: Destruye el recurso.
*/
void visitante_destruir(visitante_t *visitante);

/*
PRE: Recibe un visitante (visitante_t *).
POST: Imprime un representacion del visitante 
bajo la forma:
    *<nombre-visitante>: <cantidad de visitas>
*/
void visitante_imprimir(visitante_t *visitante);

/*
PRE: Recibe un visitante (visitante_t *) y un nombre (char *).
POST: Devuelve true si el visitante recibido tiene el nombre
recibido, false en caso contrario. 
*/
bool visitante_es_este(visitante_t* visitante, char* nombre);

//tda servidor

/*
PRE: Recibe una servidor ya declarado (svr_t *), y el 
nombre del puerto del cual recibira peticiones.
POST: Devuelve true, si logro inicializar al servidor con 
exito, false en caso contrario.
*/
bool servidor_crear(svr_t *servidor, const char *puerto,const char *ruta);

/*
PRE: Recibe un servidor (svr_t *) ya creado, y una funcion
para destruir el template, o NULL si no es necesario.
POST: Destruye el servidor.
*/
void servidor_destruir(svr_t *servidor);//, void destruir_template(char*));

/*
PRE: Recibe un servidor (svr_t *) ya creado.
POST: Imprime las visitas que haya recibido hasta el 
momento, bajo el siguiente formato:
# Estadisticas de visitantes:

* <nombre-visitante-1>: <cantidad de visitas>
* <nombre-visitante-2>: <cantidad de visitas>
  .
  .
  .
* <nombre-visitante-n>: <cantidad de visitas> 
*/
void servidor_imprimir_visitas(svr_t *servidor);

/*
PRE: Recibe un servidor (svr_t *) ya creado, un socket ACTIVO, 
configurado para comunicarse con otro socket entrante y la temperatura
(double) para insertar en el cuerpo del respuesta.
POST: Devuelve true si logro enviar el cuerpo de la respuesta, o false
en caso de algun error.
*/
bool servidor_enviar_cuerpo(svr_t *svr, skt_t* skt, double temperatura);

/*
PRE: Recibe un servidor (svr_t *) ya creado, un socket ACTIVO, que sirve para la
comunicacion con otro socket entrante, y una linea correspondiente a la primer linea
de una peticion recibida.
POST: Procesa la primer linea de una peticion y envia la primer parte de la respuesta
a la misma. 
Devuelve una de 3 valores de tipo peticion_t:
PETICION_VALIDA: la peticion recibida es valida.
PETICION_INVALIDA: la peticion recibida es invalida.
PETICION_ERROR: ocurrio un error. 
*/
peticion_t servidor_procesar_cabecera(svr_t *svr, skt_t *skt, char *linea);

/*
PRE: Recibe un servidor (svr_t *) ya creado, y el nombre
de un visitante (char *). 
POST: Agrega una nueva visita de dicho visitante al servidor.
Devuelve true, si logro realizar lo anterior con exito, false
en caso contrario. 
*/
bool servidor_agregar_visita(svr_t *servidor, char* nombre);

/*
PRE: Recibe un servidor (svr_t) ya creado, y una linea 
(char *) correspondiente al una linea del cuerpo de una peticion
POST: Devuelve true si logro procesar la linea con exito, false
en caso de algun error.
*/
bool servidor_procesar_cuerpo(svr_t *servidor, char* linea);

/*
PRE: Recibe un servidor (svr_t *) ya creado, un socket (skt_t *) 
ACTIVO, configurado para comunicarse con otro socket entrante,
una lista cuyos elementos son los caracteres de una parte de 
un peticion, y un numero entero n (int *) que representa la linea de la
peticion que aun no se proceso.
POST: Procesa lo lista de caracteres recibida, quitando conjuntos 
de estos que formen una linea, actualiza el contador de linea n, y 
procesando la linea, ya sea enviando al socket entrante la primer parte
de una respuesta, o guardando informacion sobre la peticion en el servidor.
Devuelve uno de 3 valores de tipo peticion_t:
PETICION_VALIDA: la peticion recibida es valida.
PETICION_INVALIDA:  la peticion recibida es invalida.
PETICION_ERROR: ocurrio un error. 
*/
peticion_t servidor_proc_carac(svr_t *svr, skt_t *skt, lista_t* lCarac, int *n)

/*
PRE: Recibe un servidor (svr_t *) ya creado, y un socket (skt_t *) ACTIVO,
es decir, configurado para comunicarse con socket entrantes, y recibir peticiones.
POST: Recibe y procesa una peticion. Devuelve uno de 3 valores del tipo peticion_t:
PETICION_VALIDA: la peticion recibida es valida.
PETICION_INVALIDA:  la peticion recibida es invalida.
PETICION_ERROR: ocurrio un error. 
*/
peticion_t servidor_recibir_peticion(svr_t *servidor, skt_t *sktActivo);

/*
PRE: Recibe el nombre (char *) de un sensor binario (un archivo binario) 
de donde extraer datos para responder a las peticiones de clientes 
entrantes.
POST: Recibe, acepta y procesa peticiones de clientes, mientras no se 
hayan agotado los dato del archivo binario correspondiente al nombre
recibido.
Devuelve true si logro realizar lo anterior con exito, false en caso
de algun error.
*/
bool servidor_aceptar_clientes(svr_t *servidor, const char *nombreBin);

#endif // SERVER_H
