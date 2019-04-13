#define _POSIX_C_SOURCE 200112L
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "common.h"
#include "server.h"

#define TEMPERATURA_LARGO_MAXIMO 100
#define CABECERA_lARGO_MAXIMO 27
#define PAQUETE_LARGO_MAXIMO 512 // 1/2 k
#define CLIENTES_ESCUCHAR 15

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
bool caracteres_a_lista(lista_t* lista, char *buffer, size_t largo){
    for (int i = 0; i < largo; ++i){
        char *caracter = malloc(sizeof(char));
        if (caracter == NULL){
            return false;
        }
        *caracter = buffer[i];
        bool seInserto;
        seInserto = lista_insertar_ultimo(lista, caracter); //OJO
        if (!seInserto){
            free(caracter);
            return false;
        }
    }
    return true;
}

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
bool buffer_a_lista(lista_t *lista, char* buffer, size_t largo){
    char *nuevaCadena = malloc(sizeof(char)*(largo+1)); // +\0
    if (nuevaCadena == NULL){
        return false;
    }
    snprintf(nuevaCadena, largo+1, "%s", buffer); //agrega solo el \0 creo
    bool todoOK = lista_insertar_ultimo(lista, (void*)nuevaCadena);
    if (!todoOK){
        free(nuevaCadena);
    }
    return todoOK;
}

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
lista_t* split(char *cadena, char *separador){
    size_t largoSeparador = strlen(separador);
    lista_t *listaSplit = malloc(sizeof(lista_t));
    if (listaSplit == NULL){
        free(listaSplit);
        return NULL;
    }
    lista_crear(listaSplit);
    bool todoOK;
    char *dirBuff = cadena;
    char *dirSep = strstr(dirBuff, separador); 
    while (dirSep != NULL){
        todoOK = buffer_a_lista(listaSplit, dirBuff, dirSep - dirBuff);
        if (!todoOK){
            lista_destruir(listaSplit, free);
            free(listaSplit);
            return NULL;
        }
        dirBuff = dirSep + largoSeparador;
        dirSep = strstr(dirBuff, separador);
    }
    todoOK = buffer_a_lista(listaSplit ,dirBuff, strlen(dirBuff));
    if (!todoOK){
        lista_destruir(listaSplit, free);
        free(listaSplit);
        return NULL;
    }
    return listaSplit;
}

/*
PRE: Recibe una lista (lista_t *) reservada en
memoria, cuyos elementos tambien estan reservados 
en memoria, y pueden ser liberados por medio la 
funcion free.
POST: Destruye la lista, liberando toda la memoria
mencionada.
*/
void free_split(lista_t *listaSplit){
    lista_destruir(listaSplit, free);
    free(listaSplit);
}

/*
PRE: Recibe un dato (void *) que en realidad es un 
visitante (visitante_t *), que esta reservado en
memoria. 
POST: Destruye al visitante y libera la memoria 
reservada
*/
void destruir_visitante_wrapper(void *datoVisitante){
    visitante_destruir((visitante_t*)datoVisitante);
    free(datoVisitante);
}

/*
PRE: Recibe dos datos (void *), donde el primero en 
realidad es un visitante (visitante_t *). El segundo
(void *) no se utiliza: puerde ser lo que sea. 
POST: Imprime el visitante y devuelve true.
*/
bool imprimir_visitante_wrapper(void *dato1, void*dato2){
    visitante_t *visitante = (visitante_t*)dato1;
    visitante_imprimir(visitante);
    return true;
}

/*
PRE: Recibe dos datos (void *) que son en realidad (char *).
POST: Devuelve true si los dos primeros caracteres de cada 
dato son iguales.
*/
bool son_mismo_caracter_wrapper(void *dato1, void *dato2){
    char caracter1 = *(char*)dato1;
    char caracter2 = *(char*)dato2;
    return caracter1 == caracter2;
}

/*
PRE: Recibe dos datos (void *), que en realidad, el primero es 
un visitante (visitante_t *), y el segundo es un nombre (char *).
POST: Devuelve true, si el visitante recibido tiene el nombre, 
tambien recibido.
*/
bool es_este_visitante_wrapper(void *datoVisitante, void* datoNombre){
    visitante_t *visitante = (visitante_t*)datoVisitante;
    char* nombre = (char*)datoNombre;
    return visitante_es_este(visitante, nombre);
}

/*
PRE: Recibe dos datos (void *), donde el primero en realidad
es un visitante (visitante_t *), y el segundo es un nombre
(char *).
Devuelve false, si se visito al visitante, true en caso 
contrario.
*/
bool visitar_si_es_este_wrapper(void *datoVisitante, void *datoNombre){
    visitante_t *visitante = (visitante_t*)datoVisitante;
    char *nombre = (char*)datoNombre;
    bool esEsteVisitante = visitante_es_este(visitante, nombre);
    if (esEsteVisitante){
        visitante_visitar(visitante);
    }
    return !esEsteVisitante;
}

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
char* obtener_linea_de_lista(lista_t *caracteres){
    if (!lista_esta(caracteres, son_mismo_caracter_wrapper, "\n")){
        return NULL;
    }
    vector_t vector;
    size_t largoVector = 30;
    bool todoOK;
    todoOK = vector_crear(&vector, largoVector);
    if (!todoOK){
        return NULL;
    }
    char *caracterActual = (char *)lista_borrar_primero(caracteres);
    size_t i = 0;
    while (*caracterActual != '\n'){
        todoOK = vector_autoinsertar(&vector, i, *caracterActual);
        free(caracterActual);
        if (!todoOK){
            vector_destruir(&vector);
            return NULL;
        }
        ++i;
        caracterActual = (char *)lista_borrar_primero(caracteres);
    }
    free(caracterActual);
    todoOK = vector_autoinsertar(&vector, i, '\0');
    if (!todoOK){
        vector_destruir(&vector);
        return NULL;
    }
    size_t largoLinea = i+1; // +\0
    char *linea = vector_obtener_cadena(&vector, largoLinea);
    vector_destruir(&vector);
    return linea;
}

/*
Pre: Recibe un archivo de texto ya abierto.
PRE: Recibe el nombre (char *) de un archivo de texto.
Post: Devuelve una cadena de caracteres (terminada en \0) que 
contiene todos los carateres del archivo desde el inicio hasta
una linea vacia o fin de archivo; o NULL si hubo algun error 
durante la carga.
Queda a respondabilidad del usuario liberar la memoria reservada 
para la cadena, por medio de free().
*/
char *cargar_archivo(const char *rutaArchivo) {
    FILE* archivo = fopen(rutaArchivo, "rt");
    if (archivo == NULL){
        return NULL;
    }
    size_t largoVector = PAQUETE_LARGO_MAXIMO;
    vector_t vector;
    bool todoOK;
    todoOK = vector_crear(&vector, largoVector);
    if (!todoOK){
        fclose(archivo);
        return NULL;
    }
    size_t i = 0; 
    char caracter;
    while ((caracter = getc(archivo)) != -1){ //eof
        todoOK = vector_autoinsertar(&vector, i, caracter);
        if (!todoOK){
            break;
        }
        ++i;
    }
    fclose(archivo);
    if (!todoOK){
        vector_destruir(&vector);
        return NULL;
    }
    todoOK = vector_autoinsertar(&vector, i, '\0');
    if (!todoOK){
        vector_destruir(&vector);
        return NULL;
    }
    size_t largoTexto = i + 1; 
    char *texto = vector_obtener_cadena(&vector, largoTexto);
    vector_destruir(&vector);
    return texto;
}

//tda lista

/*
PRE: Recibe una lista (lista_t *) ya declarada.
POST: Inicializa la lista.
*/
void lista_crear(lista_t *lista) {
    lista->primero = NULL;
    lista->ultimo = NULL;
    lista->largo = 0;
}

/*
PRE: Recibe una lista (lista_t *) ya creada, y una funcion
para destruir los elementos de la lista de tipo void y que 
reciba un dato void*, o NULL
POST: 
*/
void lista_destruir(lista_t *lista, void destruir_dato(void *dato)) {
    lista_nodo_t *actual = lista->primero;
    while (actual != NULL){
        lista_nodo_t *proximo = actual->proximo;
        if (destruir_dato) {
            destruir_dato(actual->dato);
        }
        free(actual);
        lista->largo -= 1;
        actual = proximo;
    }
}

/*
PRE: Recibe una lista (lista_t *) ya creada, y un dato 
(void *) para insertar en la misma.
POST: Devuelve true si logro insertar el elemento en la
ultima posicion de la lista, false en caso de no poder 
insertarlo.
*/
bool lista_insertar_ultimo(lista_t *lista, void* dato){
    lista_nodo_t *nuevoNodo = malloc(sizeof(lista_nodo_t));
    if (nuevoNodo == NULL) {
        return false;
    }
    nuevoNodo->dato = dato;
    nuevoNodo->proximo = NULL;
    if (lista->primero == NULL){
        lista->primero = nuevoNodo;
    } else {
        lista->ultimo->proximo = nuevoNodo;
    }
    lista->ultimo = nuevoNodo;
    lista->largo += 1;
    return true;
} 

/*
PRE: Recibe una lista (lista_t *) ya creada.
POST: Toma el primer dato de la lista, lo elimina de la misma, 
y lo devuelve (void *). Devuelve NULL, si la lista esta vacia.
*/
void *lista_borrar_primero(lista_t *lista){
    if (lista == NULL || lista->primero == NULL){
        return NULL;
    }
    void *dato = lista->primero->dato;
    lista_nodo_t *proximo = lista->primero->proximo; 
    free(lista->primero);
    lista->primero = proximo;
    if (lista->largo == 1){
        lista->ultimo = lista->primero;
    }
    lista->largo -= 1;
    return dato;
}

/*
PRE: Recibe una lista (lista_t *) ya creada.
POST: Devuelve la cantidad de elementos en la misma.
*/
size_t lista_ver_largo(lista_t *lista){
    if (lista == NULL) {
        return 0;
    }
    return lista->largo;
}

/*
PRE: Recibe una lista (lista_t*) ya creada, una funcion
bool visitar(void *dato, void *extra), y un dato extra
(void *).
POST: Itera la lista recibida, ejecutando la funcion recibida
sobre cada uno de los elementos de la lista, hasta que la misma 
devuelva false. Si la funcion es NULL, no itera. 
*/
void lista_iterar(lista_t* lista, bool visitar(void *d, void* e), void* extra){
    if (lista == NULL || visitar == NULL){
        return;
    }
    lista_nodo_t *actual = lista->primero;
    while (actual && visitar(actual->dato, extra)){
        actual = actual->proximo;
    }
    return;
}

/*
PRE: Recibe una lista (lista_t *) ya creada, 
una funcion para comparar elementos en la lista,
y el elemento contra el que se quiera comparar.
POST: Devuelve true, alguno de los datos de la 
es igual es igual al elemento recibido, false en 
caso contrario o de un error.
*/
bool lista_esta(lista_t* lista, bool son_iguales(void* d1, void*d2 ), void* d2){
    if (lista == NULL || son_iguales == NULL){
        return false;
    }
    lista_nodo_t *actual = lista->primero;
    while (actual){
        if (son_iguales(actual->dato, d2)){
            return true;
        }
        actual = actual->proximo;
    }
    return false;
}

//tda vector

/*
PRE: Recibe un vector (vector_t *) ya declarado, y el
largo del vector (size_t > 0).
POST: Devuelve true si logro inicializar al vector con
exito, false en caso contrario.
*/
bool vector_crear(vector_t *vector, size_t largo){
    if (largo == 0){
        return false;
    }
    vector->datos = malloc(sizeof(char)*largo);
    if (vector->datos == NULL){
        return false;
    }
    
    for(int i = 0; i < largo; ++i){
        vector->datos[i] = 0;
    }
    
    vector->largo = largo;
    return true;
}

/*
PRE: Recibe un vector (vector_t *) ya creado, y una funcion 
para destruir los datos que alamacena, o NULL en caso de no
ser necesario
POST: Destruye el vector.
*/
void vector_destruir(vector_t *vector){
    if (vector->datos == NULL){
        return;
    }
    free(vector->datos);
}

/*
PRE: Recibe un vector (vector_t *) ya creado.
POST: Devuelve el largo del vector (size_t).
*/
size_t vector_ver_largo(vector_t *vector){
    return vector->largo;
}


/*
PRE: Recibe un vector (vector_t*) ya creado, y nuevo largo 
(size_t) para redimensionar al vector.
POST: Devuelve true, si logro redimensionar al vector, false
en caso contrario.
Si el vector se agranda, las nuevas posicion quedan 
inicializadas en '\0'.
*/
bool vector_redimensionar(vector_t *vector, size_t nuevoLargo){
    if (vector == NULL || nuevoLargo <= 0){
        return false;
    }
    size_t memoriaReservar = sizeof(char)*nuevoLargo;
    char *nuevosDatos = realloc(vector->datos, memoriaReservar);
    if (nuevosDatos == NULL){
        return false;
    }
    vector->datos = nuevosDatos;  
    for(int i = vector->largo; i < nuevoLargo; ++i){
        vector->datos[i] = 0;
    }
    vector->largo = nuevoLargo;
    return true;
}

/*
PRE: Recibe un vector (vector_t *) ya creado y una posicion 
(size_t) del mismo.
POST: Devuelve el dato (void *) asociado al mismo, o NULL
si se fue de rango.
*/
char *vector_ver(vector_t *vector, size_t posicion){
    if (vector->largo <= posicion){
        return NULL;
    }
    return &vector->datos[posicion];
}

/*
PRE: Recibe un vector (vector_t *) ya creado y una posicion 
(size_t) del mismo, y un dato (void *) a insertar.
POST: Devuelve true si logro insertar el dato en la posicion
indicada, o false en caso de algun error, o fuera de rango.
*/
bool vector_insertar(vector_t *vector, size_t posicion, char dato){
    if (vector->largo <= posicion){
        return false;
    }
    vector->datos[posicion] = dato;
    return true;
}

/*
PRE: Recibe un vector (vector_t *) ya creado, una posicion (size_t)
y un caracter (char) a insertar.
POST: Inserta el caracter en la posicion indicada. 
Si la posicion sobrepasa el largo de vector, se autoredimensiona
para lograr la insercion.
Devuelve true, si logro lo anterio, o false en caso contrario. 
*/
bool vector_autoinsertar(vector_t *vector, size_t posicion, char caracter){
    size_t factorRedimensionar = 2;
    size_t largoVector = vector_ver_largo(vector);
    if (largoVector > posicion){
        return vector_insertar(vector, posicion, caracter);
    }
    size_t nuevoLargo = largoVector * factorRedimensionar;
    bool seRedimensiono = vector_redimensionar(vector, nuevoLargo);
    if (!seRedimensiono){
        return false;
    }
    return vector_insertar(vector, posicion, caracter);
}

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
char *vector_obtener_cadena(vector_t *vector, size_t largo){
    if (vector_ver_largo(vector) < largo || largo == 0){
        return NULL;
    }
    char *cadena = malloc(sizeof(char)*largo);
    if (cadena == NULL){
        return NULL;
    }
    for(int i = 0; i < largo; ++i){
        cadena[i] = *(vector_ver(vector, i));
    }
    return cadena;
}

//tda visitante

/*
Inicializa un visitante_t
Pre: Recibe un recurso visitado (visitante *) ya 
declarado, y su nombre (char *), terminado en \0.
Post: Devuelve true si logro inicializar el recurso
con exito, false en caso contrario.
*/
bool visitante_crear(visitante_t *recurso, char *nombre) {
    size_t largoNombre = strlen(nombre) +1; // + \0 
    recurso->vecesVisitado = 0;
    size_t memoriaReservar = sizeof(char) * (largoNombre);  
    recurso->nombreRecurso = malloc(memoriaReservar);
    if (recurso->nombreRecurso == NULL) {
        return false;
    }
    snprintf(recurso->nombreRecurso, largoNombre, "%s", nombre);
    return true;
}

/*
Destruye el recurso.
Pre: Recibe un recurso visitado (visitante_t *) ya 
inicializado y lo destruye.
Post: Destruye el recurso.
*/
void visitante_destruir(visitante_t *recurso) {
    free(recurso->nombreRecurso);
}

/*
Devuelve el nombre del recurso.
Pre: Recibe un recurso visitado (visitante *) ya 
inicializado.
Post: Devuelve el nombre del recurso (char *). 
*/
char *visitante_ver_nombre(visitante_t *recurso) {
    return recurso->nombreRecurso;
} 

/*
Devuelve la cantidad de visitas.
Pre: Recibe un recurso visitado (visitante *) ya 
inicializado.
Post: Devuelve la cantidad de veces (int) que el recurso 
fue visitado.
*/
int visitante_ver_visitas(visitante_t *recurso) {
    return recurso->vecesVisitado;
}

/*
Visita el recurso.
Pre: Recibe un recurso visitado (visitante *) ya 
inicializado.
Post: Visita el recurso, aumentando la cantidad de visitas.
*/
void visitante_visitar(visitante_t *recurso) {
    recurso->vecesVisitado += 1;
}


/*
PRE: Recibe un visitante (visitante_t *).
POST: Imprime un representacion del visitante 
bajo la forma:
    *<nombre-visitante>: <cantidad de visitas>
*/
void visitante_imprimir(visitante_t *visitante){
    char *nombre = visitante_ver_nombre(visitante);
    size_t cantidadVisitas = visitante_ver_visitas(visitante);
    fprintf(stdout, "* %s: %d\n", nombre, (int)cantidadVisitas);
}

/*
PRE: Recibe un visitante (visitante_t *) y un nombre (char *).
POST: Devuelve true si el visitante recibido tiene el nombre
recibido, false en caso contrario. 
*/
bool visitante_es_este(visitante_t* visitante, char* nombre){
    char *nombreVistante = visitante_ver_nombre(visitante);
    return strcmp(nombreVistante, nombre) == 0;
}

//tda servidor

/*
PRE: Recibe una servidor ya declarado (svr_t *), y el 
nombre del puerto del cual recibira peticiones.
POST: Devuelve true, si logro inicializar al servidor con 
exito, false en caso contrario.
*/
bool servidor_crear(svr_t *servidor, const char *puerto,const char *ruta){
    socket_crear(&servidor->skt);
    bool todoOK;
    todoOK = socket_enlazar(&servidor->skt, puerto);
    if (!todoOK){
        socket_destruir(&servidor->skt);
        return false;
    }
    todoOK = socket_escuchar(&servidor->skt, CLIENTES_ESCUCHAR);
    if (!todoOK){
        socket_destruir(&servidor->skt);
        return false;
    }
    lista_crear(&servidor->visitas);
    servidor->template = cargar_archivo(ruta);
    if (servidor->template == NULL){
        lista_destruir(&servidor->visitas, destruir_visitante_wrapper);
        socket_destruir(&servidor->skt);
        return false;
    }
    return true;
}
/*
PRE: Recibe un servidor (svr_t *) ya creado, y una funcion
para destruir el template, o NULL si no es necesario.
POST: Destruye el servidor.
*/
void servidor_destruir(svr_t *servidor){ //, void destruir_template(char*)){
    lista_destruir(&servidor->visitas, destruir_visitante_wrapper);
    socket_destruir(&servidor->skt);
    free(servidor->template);
}

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
void servidor_imprimir_visitas(svr_t *servidor){
    fprintf(stdout,"#Estadisticas de visitantes:\n\n");
    lista_iterar(&servidor->visitas, imprimir_visitante_wrapper, NULL);
}


/*
PRE: Recibe un servidor (svr_t *) ya creado, un socket ACTIVO, 
configurado para comunicarse con otro socket entrante y la temperatura
(double) para insertar en el cuerpo del respuesta.
POST: Devuelve true si logro enviar el cuerpo de la respuesta, o false
en caso de algun error.
*/
bool servidor_enviar_cuerpo(svr_t *svr, skt_t* skt, double temperatura){
    char temperaturaBuffer[TEMPERATURA_LARGO_MAXIMO];
    size_t largoBuffer = TEMPERATURA_LARGO_MAXIMO;
    snprintf(temperaturaBuffer, largoBuffer, "%.2f", temperatura);
    char *sustituto = "{{datos}}";
    char *posSustituto = strstr(svr->template, sustituto);
    size_t largo = posSustituto - (svr->template);
    bool seEnvio;
    seEnvio = socket_enviar_todo(skt, svr->template, largo);
    if (!seEnvio){
        return false;
    }
    largoBuffer = strlen(temperaturaBuffer); 
    seEnvio = socket_enviar_todo(skt, temperaturaBuffer,largoBuffer); 
    if (!seEnvio){
        return false;
    }
    size_t largoSustituto = strlen(sustituto);
    largo = strlen(posSustituto + largoSustituto);
    return socket_enviar_todo(skt, posSustituto + largoSustituto, largo);
}

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
peticion_t servidor_procesar_cabecera(svr_t *svr, skt_t *skt, char *linea){
    peticion_t resultado;
    char *status = "HTTP/1.1 %s %s\n\n\0";
    lista_t *argumentosMetodo = split(linea, " ");
    if (argumentosMetodo == NULL) {
        return PETICION_ERROR;
    }
    size_t cantidadArgumentos = lista_ver_largo(argumentosMetodo);
    char *metodo = (char *)lista_borrar_primero(argumentosMetodo);
    bool cantArgumentosCorrecta = (cantidadArgumentos == 3);
    bool metodoCorrecto = (strcmp(metodo, "GET")==0);
    free(metodo);
    char cabecera[CABECERA_lARGO_MAXIMO];
    size_t largoCabecera = sizeof(cabecera);
    if (!(cantArgumentosCorrecta && metodoCorrecto)) {
        snprintf(cabecera, largoCabecera, status, "400", "Bad request");
        resultado = PETICION_INVALIDA;
    } else {
        char *recurso = (char *)lista_borrar_primero(argumentosMetodo);
        if (strcmp(recurso, "/sensor")!=0) { 
            snprintf(cabecera, largoCabecera, status, "404", "Not found");
            resultado = PETICION_INVALIDA;
        } else {
            snprintf(cabecera, largoCabecera, status, "200", "OK");
            resultado = PETICION_VALIDA;
        }
        free(recurso);
    }
    free_split(argumentosMetodo);
    largoCabecera = strlen(cabecera);
    bool seEnvio;
    seEnvio = socket_enviar_todo(skt, cabecera, largoCabecera);
    if (!seEnvio){
        resultado = PETICION_ERROR;
    }
    return resultado;
}

/*
PRE: Recibe un servidor (svr_t *) ya creado, y el nombre
de un visitante (char *). 
POST: Agrega una nueva visita de dicho visitante al servidor.
Devuelve true, si logro realizar lo anterior con exito, false
en caso contrario. 
*/
bool servidor_agregar_visita(svr_t *servidor, char* nombre){
    lista_t *visitas = &servidor->visitas;
    if (lista_esta(visitas, es_este_visitante_wrapper, nombre)){
        void *nombreWrapper = (void*)nombre;
        lista_iterar(visitas, visitar_si_es_este_wrapper, nombreWrapper);
        return true;
    }
    visitante_t *nuevoVisitante = malloc(sizeof(visitante_t));
    if (nuevoVisitante == NULL){
        return false;
    }
    bool todoOK;
    todoOK = visitante_crear(nuevoVisitante, nombre);
    if (!todoOK){
        free(nuevoVisitante);
    }
    visitante_visitar(nuevoVisitante);
    todoOK = lista_insertar_ultimo(visitas, (void*)nuevoVisitante);
    if (!todoOK){
        visitante_destruir(nuevoVisitante);
        free(nuevoVisitante);
    }
    return todoOK;
}

/*
PRE: Recibe un servidor (svr_t) ya creado, y una linea 
(char *) correspondiente al una linea del cuerpo de una peticion
POST: Devuelve true si logro procesar la linea con exito, false
en caso de algun error.
*/
bool servidor_procesar_cuerpo(svr_t *servidor, char* linea){
    lista_t *campos = split(linea, ": ");
    if (campos == NULL) {
        return false;
    }
    size_t cantidadCampos = lista_ver_largo(campos);
    if (cantidadCampos != 2) {
        // Esto no ha de pasar, pero tomamos como supuesto
        // que simplemente se ignora la linea.
        free_split(campos); 
        return true; 
    }
    char *nombreCampo = (char *)lista_borrar_primero(campos);
    char *nombreVisitante = (char *)lista_borrar_primero(campos);
    free_split(campos);
    bool esCampoAgent = strcmp(nombreCampo, "User-Agent")==0;
    free(nombreCampo);
    if (esCampoAgent){
        bool seVisito;
        seVisito = servidor_agregar_visita(servidor, nombreVisitante);
        free(nombreVisitante);
        return seVisito;
    }
    free(nombreVisitante);
    return true;
}

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
peticion_t servidor_proc_carac(svr_t *svr, skt_t *skt, lista_t* lCarac, int *n){
    peticion_t resultado = PETICION_VALIDA;
    bool todoOK = true;
    bool estaSalto;
    estaSalto = lista_esta(lCarac, son_mismo_caracter_wrapper, "\n");
    bool resultadoValido = (resultado == PETICION_VALIDA); 
    while (estaSalto && resultadoValido){
        char *linea = obtener_linea_de_lista(lCarac);
        if (*n <= 1){
            resultado = servidor_procesar_cabecera(svr, skt, linea);
            if (resultado == PETICION_ERROR){
                todoOK = false;
            }
            *n += 1;
            resultadoValido = (resultado == PETICION_VALIDA);
        } else {  
            todoOK = servidor_procesar_cuerpo(svr, linea);
        }
        free(linea);
        if (!todoOK){
            resultado = PETICION_ERROR;
            break;
        }
        estaSalto = lista_esta(lCarac, son_mismo_caracter_wrapper, "\n");
    }
    return resultado;
}

/*
PRE: Recibe un servidor (svr_t *) ya creado, y un socket (skt_t *) ACTIVO,
es decir, configurado para comunicarse con socket entrantes, y recibir peticiones.
POST: Recibe y procesa una peticion. Devuelve uno de 3 valores del tipo peticion_t:
PETICION_VALIDA: la peticion recibida es valida.
PETICION_INVALIDA:  la peticion recibida es invalida.
PETICION_ERROR: ocurrio un error. 
*/
peticion_t servidor_recibir_peticion(svr_t *svr, skt_t *sktActivo){
    peticion_t resultado = PETICION_VALIDA;
    lista_t caracteres;
    lista_crear(&caracteres);
    char paquete[PAQUETE_LARGO_MAXIMO];
    size_t largo = PAQUETE_LARGO_MAXIMO;
    int bytesRecibidos = 1; // > 0 //Para iniciar el ciclo
    int nLinea = 1;
    bool todoOK = true;
    while (bytesRecibidos > 0 && resultado != PETICION_ERROR) {
        bytesRecibidos = socket_recibir_algo(sktActivo, paquete, largo);
        if (bytesRecibidos < 0){
            todoOK = false;
            continue;
        }
        todoOK = caracteres_a_lista(&caracteres, paquete, bytesRecibidos);
        if (!todoOK){
            resultado = PETICION_ERROR;
            continue;
        }
        if (resultado != PETICION_VALIDA){
            continue;
        }
        resultado = servidor_proc_carac(svr, sktActivo, &caracteres, &nLinea);   
    }
    lista_destruir(&caracteres, free);
    return resultado;
}

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
bool servidor_aceptar_clientes(svr_t *servidor, const char *nombreBin){
    FILE *sensor; 
    if ((sensor = fopen(nombreBin, "rb")) == NULL) {
        fprintf(stderr, "Archivo binario no encontrado.\n");
        return false;
    }
    uint16_t numeroLeido;
    size_t largoDato = sizeof(short int);
    fread(&numeroLeido,largoDato,1,sensor);
    bool todoOK = true;
    while (!feof(sensor) && todoOK) {
        skt_t sktActivo;
        todoOK = socket_aceptar(&servidor->skt, &sktActivo);
        if (!todoOK) {
            break;
        }
        peticion_t resultado;
        resultado = servidor_recibir_peticion(servidor, &sktActivo);
        if (resultado == PETICION_ERROR){
            todoOK = false;
        } else if (resultado == PETICION_VALIDA){
            numeroLeido = ntohs(numeroLeido);
            double temperatura = (numeroLeido - 2000.00)/100.00;
            bool seEnvio;
            seEnvio = servidor_enviar_cuerpo(servidor, &sktActivo, temperatura);
            if (!seEnvio){
                todoOK = false;
            }
            fread(&numeroLeido,largoDato,1,sensor);
        }
        socket_destruir(&sktActivo);
    }
    fclose(sensor);
    return todoOK;
}

int main(int argc, const char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso:\n./server <puerto> <input> [<template>]\n");
        return 1;
    }
   	const char *nombrePuerto = argv[1];
    const char *sensorBinario = argv[2];
    const char *rutaTemplate = argv[3];
    svr_t servidor;
    bool todoOK;
    todoOK = servidor_crear(&servidor, nombrePuerto, rutaTemplate);
    if (!todoOK){
        return 1;
    }
    todoOK = servidor_aceptar_clientes(&servidor, sensorBinario);
    if (!todoOK){
        return 1;
    }
    servidor_imprimir_visitas(&servidor);
    servidor_destruir(&servidor);
    return 0;
}
