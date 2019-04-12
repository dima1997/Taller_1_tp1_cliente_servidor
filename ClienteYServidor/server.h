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
    socket_t skt;
    char *template;
} servidor_t;

typedef struct visitante {
    char *nombreRecurso;
    size_t vecesVisitado;
} visitante_t;

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

/*
PRE: Recibe una lista (lista_t *) ya creada, un buffer de datos
(void *), y el largo hasta donde agregar datos del buffer.
POST: Agrega los datos del buffer hasta el largo establecido.
Devuelve true si logro hacer lo anterior, o false en caso de 
algun error.
*/
bool lista_agregar(lista_t *lista, void **buffer, size_t largo);

/*
PRE: Recibe un vector (vector_t *) ya declarado, y el
largo del vector (size_t > 0).
POST: Devuelve true si logro inicializar al vector con
exito, false en caso contrario.
*/
bool vector_crear(vector_t *vector, size_t largo);

/*
PRE: Recibe un vector (vector_t *) ya creado.
POST: Devuelve el largo del vector.
*/
size_t vector_ver_largo(vector_t *vector);

/*
PRE: Recibe un vector (vector_t *) ya creado, y una funcion 
para destruir los datos que alamacena, o NULL en caso de no
ser necesario
POST: Destruye el vector.
*/
void vector_destruir(vector_t *vector);//, void destruir_dato(void *dato));

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
Wrapeer de visitante_destruir para lista enlazada
*/
void visitante_destruir_wrapper(void *visitante);

/*
PRE: Recibe una servidor ya declarado (servidor_t *), y el 
nombre del puerto del cual recibira peticiones.
POST: Devuelve true, si logro inicializar al servidor con 
exito, false en caso contrario.
*/
bool servidor_crear(servidor_t *servidor, const char *puerto, char *template);

/*
PRE: Recibe un servidor (servidor_t *) ya creado, y una funcion
para destruir el template, o NULL si no es necesario.
POST: Destruye el servidor.
*/
void servidor_destruir(servidor_t *servidor, void destruir_template(char*));

/*
PRE: Recibe un visitante (visitante_t *).
POST: Imprime las visitas de dicho visitante 
bajo la forma:
    *<nombre-visitante>: <cantidad de visitas>
*/
void imprimir_visitante(visitante_t *visitante);

bool imprimir_visitante_wrapper(void *dato1, void*dato2);

/*
PRE: Recibe un servidor (servidor_t *) ya creado.
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
void servidor_imprimir_visitas(servidor_t *servidor);

bool son_mismo_caracter_wrapper(void *dato1, void *dato2);

bool son_mismo_caracter(char *caracer1, char *caracter2);

/*
PRE: Recibe un servidor (servidor_t *) ya creado, un socket ACTIVO, 
configurado para comunicarse con otro socket entrante y la temperatura
(double) para insertar en el cuerpo del respuesta.
POST: Devuelve true si logro enviar el cuerpo de la respuesta, o false
en caso de algun error.
*/
bool servidor_enviar_cuerpo(servidor_t *svr, socket_t* skt, double temperatura);

/*
PRE: Recibe un servidor (servidor_t *) ya creado, un socket ACTIVO, que sirve para la
comunicacion con otro socket entrante, y una linea correspondiente a la primer linea
de una peticion recibida.
POST: Procesa la primer linea de una peticion y envia la primer parte de la respuesta
a la misma. 
Devuelve una de 3 valores de tipo peticion_t:
PETICION_VALIDA: la peticion recibida es valida.
PETICION_INVALIDA: la peticion recibida es invalida.
PETICION_ERROR: ocurrio un error. 
*/
peticion_t servidor_procesar_cabecera(servidor_t *svr, socket_t *skt, char *linea);

bool es_este_visitante(visitante_t* visitante, char* nombre);

bool es_este_visitante_wrapper(void *dato1, void* dato2);

bool visitar_si_es_este(visitante_t* visitante, char* nombre);

bool visitar_si_es_este(visitante_t* visitante, char* nombre);

/*
PRE: Recibe un servidor (servidor_t *) ya creado, y el nombre
de un visitante (char *). 
POST: Agrega una nueva visita de dicho visitante al servidor.
Devuelve true, si logro realizar lo anterior con exito, false
en caso contrario. 
*/
bool servidor_agregar_visita(servidor_t *servidor, char* nombre);

/*
PRE: Recibe un servidor (servidor_t) ya creado, y una linea 
(char *) correspondiente al una linea del cuerpo de una peticion
POST: Devuelve true si logro procesar la linea con exito, false
en caso de algun error.
*/
bool servidor_procesar_cuerpo(servidor_t *servidor, char* linea);

/*
PRE: Recibe un servidor (servidor_t *) ya creado, 
un socket (socket_t *) ACTIVO, es decir, un socket
configurado para comunicarse con otro socket 
entrante, una lista (lista_t*) de caracteres recibidos, 
correspondientes a una parte de una peticion.
POST: Procesa la lista recibida eliminando caracteres, 
hasta haber procesado todas las lineas de peticion que 
se encuentran en dicha parte.
Devuelve true si logro ejecutar lo anterior con exito,
false en caso contrario
*/
char* obtener_linea(lista_t *caracteres);

/*
PRE: Recibe un servidor (servidor_t *) ya creado, y un socket (socket_t *) ACTIVO,
es decir, configurado para comunicarse con socket entrantes, y recibir peticiones.
POST: Recibe y procesa una peticion. Devuelve uno de 3 valores del tipo peticion_t:
PETICION_VALIDA: la peticion recibida es valida.
PETICION_INVALIDA:  la peticion recibida es invalida.
PETICION_ERROR: ocurrio un error. 
*/
peticion_t servidor_recibir_peticion(servidor_t *servidor, socket_t *sktActivo);

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
bool servidor_aceptar_clientes(servidor_t *servidor, const char *nombreBin);


/*
typedef struct linea {
    char *caracteres;
    size_t largo;
    size_t _largoReservado;
} linea_t;
*/
/*
Inicializa una linea_t
Pre: Recibe una linea ya declarada (linea_t *)
Post: Devuelve true si se inicializo la linea con exito, 
false en caso contrario
*/
//bool linea_crear(linea_t *linea);

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
//int linea_agregar_caracteres(linea_t *linea, char *cadena);

/*
Borra los caracteres agregados de la linea.
Pre: Recibe una linea (linea_t *) ya  inicializada.
Post: Borra la linea. Tras ejecutar esta linea, los proximos
caracteres a agregar, se escribiran desde el principio de la 
linea.
*/
//void linea_borrar(linea_t *linea);

/*
Devuelve un arreglo de los caracteres que forman la linea.
Pre: Recibe una linea (linea_t *) ya inicializada.
Post: Devuelve  un arreglo de caracteres (char *) con todos 
los caracteres agregados hasta el momento; o NULL si hubo 
algun error. 
Queda a responsabilidad del usuario liberar la memoria 
reservada para el arreglo, por medio de la funcion free().
*/
//char *linea_ver_caracteres(linea_t *linea);

/*
Destruye la linea.
Pre: Recibe una linea (linea_t *) ya inicializada.
Post: Destruye la linea.
*/
//void linea_destruir(linea_t *linea);

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
typedef struct recursosVector {
    visitante_t **recursos;
    size_t _cantidadReservada; 
    size_t cantidad;
} recursosVector_t;
*/
/*
Inicializa un recursosVector_t.
Pre: Recibe un vector de recursos visitados (recursosVector_t *) 
ya declarado.
Post: Devuelve true si logro inicializarlo con exito, false en
caso contrario.
*/
//bool recursosVector_crear(recursosVector_t *vector);

/*
Visita un recurso. 
Pre: Recibe un vector de recursos visitados (recursosVector_t *), y
el nombre de un recurso a visitar.
Post: Devuelve true si lo logro visitar con exito el recurso de 
nombre recibido, o false en caso contrario.
Nota: el recurso recibido se agrega al vector de forma automatica
si no esta presente. 
*/
//bool recursosVector_visitar(recursosVector_t *vector, char *nombreRecurso);

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
//void recursosVector_imprimir(recursosVector_t *vector);

/*
Destruye el recursosVector_t.
Pre: Recibe un vector de recursos visitados (recursosVector_t *)
ya creado e inicializado.
Post: Destruye el vector recibido.
*/
//void recursosVector_destruir(recursosVector_t *vector);

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
//char *responder_peticion(const char *primerLinea, char *cuerpo);

/*
Pre: Recibe una cadena de caracteres (char *) con la siguiente 
linea del cuerpo de la peticion a procesar, y un vector con los
visitantes de peticiones anteriores (recursosVector_t *).
Post: Devuelve true si logro procesa la linea recibida con exito, 
actualizando la informacion del vector de visitas recibido; o
false en caso contrario.
*/
//bool procesar_lineas_cuerpo(char *linea, recursosVector_t *visitantes);

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
//bool procesar_cuerpo(char *cuerpo, linea_t *linea, recursosVector_t *vector);

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
//char *procesar_peticion(int *skt, char* template, recursosVector_t *visitantes);

/*
Pre: Recibe un socket ya conectado: skt (int *), y la 
respuesta que a enviar (char *).
Post: Devuelve true si logro enviar toda la respuesta, 
false en caso contrario, dado un error en el socket.
*/
//bool enviar_respuesta(int *skt, char *mensaje, size_t largoMensaje);

/*
Pre:  Recibe un socket (int *), y el nombre del puerto 
(char *) al cual conectarse.
Post: Devuelve true si logro conectar el socket al puerto
recibido y setearlo para funcionar de socket pasivo; false 
en caso contrario.
*/
//bool conectar_socket_pasivo(int *skt, const char *puerto);

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
//char **cargar_template(const char *ruta);

/*
Pre: Recibe las dos partes del template (char **), y 
la temperatura (double) a meter entre dichas partes.
Post: Devuelve el cuerpo de la respuesta (char *), que
viene a ser las dos partes del template unidas por medio
de la temperatura recibida; o NULL si ocurrio algun error.
Queda a responsabilidad del usuario liberar la memoria
reservada por medio de la funcion free(); 
*/
//char *contruir_cuerpo(char **partesTemplate, double temperatura);

#endif // SERVER_H
