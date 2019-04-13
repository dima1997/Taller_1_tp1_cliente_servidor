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
/*
PRE: Recibe una lista (lista_t *) ya creada, un buffer de datos
(void *), y el largo hasta donde agregar datos del buffer.
POST: Agrega los datos del buffer hasta el largo establecido.
Devuelve true si logro hacer lo anterior, o false en caso de 
algun error.
*/
bool lista_agregar(lista_t *lista, void **buffer, size_t largo){
    for (int i = 0; i < largo; ++i){
        bool seInserto;
        seInserto = lista_insertar_ultimo(lista, buffer[i]); //OJO
        if (!seInserto){
            return false;
        }
    }
    return true;
}
/*
PRE: ...
POST: ...
*/
bool agregar_a_lista(lista_t* lista, char *buffer, size_t largo){
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
PRE: Recibe un vector (vector_t *) ya creado.
POST: Devuelve el largo del vector.
*/
size_t vector_ver_largo(vector_t *vector){
    return vector->largo;
}

/*
PRE: Recibe un vector (vector_t *) ya creado, y una funcion 
para destruir los datos que alamacena, o NULL en caso de no
ser necesario
POST: Destruye el vector.
*/
void vector_destruir(vector_t *vector){//, void destruir_dato(void *dato)){
    if (vector->datos == NULL){
        return;
    }
    /*
    size_t largo = vector_ver_largo(vector);
    if (destruir_dato == NULL){
        free(vector->datos);
        return;
    }
    for(int i = 0; i < largo; ++i){
        void *datoActual = vector->datos[i];
        if (datoActual != NULL){
            destruir_dato(datoActual);
        }
    }
    */
    free(vector->datos);
}

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

/*
Wrapeer de visitante_destruir para lista enlazada
*/
void visitante_destruir_wrapper(void *visitante){
    visitante_destruir((visitante_t*)visitante);
    free(visitante);
}

/*
PRE: Recibe una servidor ya declarado (servidor_t *), y el 
nombre del puerto del cual recibira peticiones.
POST: Devuelve true, si logro inicializar al servidor con 
exito, false en caso contrario.
*/
bool servidor_crear(servidor_t *servidor, const char *puerto, char *template){
    lista_crear(&servidor->visitas);
    socket_crear(&servidor->skt);
    bool todoOK;
    todoOK = socket_enlazar(&servidor->skt, puerto);
    if (!todoOK){
        lista_destruir(&servidor->visitas, visitante_destruir_wrapper);
        socket_destruir(&servidor->skt);
    }
    todoOK = socket_escuchar(&servidor->skt, CLIENTES_ESCUCHAR);
    if (!todoOK){
        lista_destruir(&servidor->visitas, visitante_destruir_wrapper);
        socket_destruir(&servidor->skt);
    }
    servidor->template = template;
    return true;
}
/*
PRE: Recibe un servidor (servidor_t *) ya creado, y una funcion
para destruir el template, o NULL si no es necesario.
POST: Destruye el servidor.
*/
void servidor_destruir(servidor_t *servidor, void destruir_template(char*)){
    lista_destruir(&servidor->visitas, visitante_destruir_wrapper);
    socket_destruir(&servidor->skt);
    if (destruir_template){
        destruir_template(servidor->template);
    }
}

/*
PRE: Recibe un visitante (visitante_t *).
POST: Imprime las visitas de dicho visitante 
bajo la forma:
    *<nombre-visitante>: <cantidad de visitas>
*/
void imprimir_visitante(visitante_t *visitante){
    char *nombre = visitante_ver_nombre(visitante);
    size_t cantidadVisitas = visitante_ver_visitas(visitante);
    fprintf(stdout, "* %s: %d\n", nombre, (int)cantidadVisitas);
}

bool imprimir_visitante_wrapper(void *dato1, void*dato2){
    visitante_t *visitante = (visitante_t*)dato1;
    imprimir_visitante(visitante);
    return true;
}

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
void servidor_imprimir_visitas(servidor_t *servidor){
    fprintf(stdout,"#Estadisticas de visitantes:\n\n");
    lista_iterar(&servidor->visitas, imprimir_visitante_wrapper, NULL);
}

bool son_mismo_caracter_wrapper(void *dato1, void *dato2){
    return son_mismo_caracter((char*)dato1, (char*)dato2);
}

bool son_mismo_caracter(char *caracter1, char *caracter2){
    return (*caracter1 == *caracter2);
}

/*
PRE: Recibe un servidor (servidor_t *) ya creado, un socket ACTIVO, 
configurado para comunicarse con otro socket entrante y la temperatura
(double) para insertar en el cuerpo del respuesta.
POST: Devuelve true si logro enviar el cuerpo de la respuesta, o false
en caso de algun error.
*/
bool servidor_enviar_cuerpo(servidor_t *svr, socket_t* skt, double temperatura){
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
peticion_t servidor_procesar_cabecera(servidor_t *svr, socket_t *skt, char *linea){
    peticion_t resultado;
    char *status = "HTTP/1.1 %s %s\n\n\0";
    size_t i;
    char **argumentosMetodo = split(linea, " ");
    if (argumentosMetodo == NULL) {
        return PETICION_ERROR;
    }
    char *metodo = argumentosMetodo[0];
    for (i=0; argumentosMetodo[i]!=NULL; ++i) {}
    size_t cantidadArgumentos = i;
    bool cantArgumentosCorrecta = (cantidadArgumentos == 3);
    bool metodoCorrecto = (strcmp(metodo, "GET")==0);
    char cabecera[CABECERA_lARGO_MAXIMO];
    size_t largoCabecera = sizeof(cabecera);
    if (!(cantArgumentosCorrecta && metodoCorrecto)) {
        snprintf(cabecera, largoCabecera, status, "400", "Bad request");
        resultado = PETICION_INVALIDA;
    } else if (strcmp(argumentosMetodo[1], "/sensor")!=0) { //recurso
        snprintf(cabecera, largoCabecera, status, "404", "Not found");
        resultado = PETICION_INVALIDA;
    } else {
        snprintf(cabecera, largoCabecera, status, "200", "OK");
        resultado = PETICION_VALIDA;
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

bool es_este_visitante(visitante_t* visitante, char* nombre){
    char *nombreVistante = visitante_ver_nombre(visitante);
    return strcmp(nombreVistante, nombre) == 0;
}

bool es_este_visitante_wrapper(void *dato1, void* dato2){
    visitante_t *visitante = (visitante_t*)dato1;
    char* nombre = (char*)dato2;
    return es_este_visitante(visitante, nombre);
}

bool visitar_si_es_este(visitante_t* visitante, char* nombre){
    if (es_este_visitante(visitante, nombre)){
        visitante_visitar(visitante);
        return true;
    }
    return false;
}

bool visitar_si_es_este_wrapper(void *dato1, void *dato2){
    visitante_t *visitante = (visitante_t*)dato1;
    char *nombre = (char*)dato2;
    return !visitar_si_es_este(visitante, nombre);
}

/*
PRE: Recibe un servidor (servidor_t *) ya creado, y el nombre
de un visitante (char *). 
POST: Agrega una nueva visita de dicho visitante al servidor.
Devuelve true, si logro realizar lo anterior con exito, false
en caso contrario. 
*/
bool servidor_agregar_visita(servidor_t *servidor, char* nombre){
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
PRE: Recibe un servidor (servidor_t) ya creado, y una linea 
(char *) correspondiente al una linea del cuerpo de una peticion
POST: Devuelve true si logro procesar la linea con exito, false
en caso de algun error.
*/
bool servidor_procesar_cuerpo(servidor_t *servidor, char* linea){
    char **campos = split(linea, ": ");
    if (campos == NULL) {
        return false;
    }
    size_t i;
    for (i = 0; campos[i]!=NULL; ++i) {}
    size_t cantidadCampos = i; 
    if (cantidadCampos != 2) {
        // Esto no ha de pasar, pero tomamos como supuesto
        // que simplemente se ignora la linea.
        free_split(campos); 
        return true; 
    }
    char *nombreCampo = campos[0];
    if (strcmp(nombreCampo, "User-Agent")==0){
        char *nombreVistante = campos[1];
        bool seVisito;
        seVisito = servidor_agregar_visita(servidor, nombreVistante);
        free_split(campos); 
        return seVisito;
    }
    free_split(campos); 
    return true;
}
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
char* obtener_linea(lista_t *caracteres){
    size_t factorRedimensionar = 2;
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
        todoOK = vector_insertar(&vector, i, *caracterActual);
        free(caracterActual);
        ++i;
        if (!todoOK){
            vector_destruir(&vector);
            return NULL;
        }
        largoVector = vector_ver_largo(&vector); 
        if (largoVector <= i){
            size_t nuevoLargo = largoVector * factorRedimensionar;
            todoOK = vector_redimensionar(&vector, nuevoLargo);
            if (!todoOK){
                vector_destruir(&vector);
                return NULL;
            }
        }
        caracterActual = (char *)lista_borrar_primero(caracteres);
    }
    free(caracterActual);
    todoOK = vector_insertar(&vector, i, '\0');
    if (!todoOK){
        vector_destruir(&vector);
        return NULL;
    }
    size_t largoLinea = i+1;
    char *linea = vector_obtener_cadena(&vector, largoLinea);
    vector_destruir(&vector);
    return linea;

}

/*
PRE: Recibe un servidor (servidor_t *) ya creado, y un socket (socket_t *) ACTIVO,
es decir, configurado para comunicarse con socket entrantes, y recibir peticiones.
POST: Recibe y procesa una peticion. Devuelve uno de 3 valores del tipo peticion_t:
PETICION_VALIDA: la peticion recibida es valida.
PETICION_INVALIDA:  la peticion recibida es invalida.
PETICION_ERROR: ocurrio un error. 
*/
peticion_t servidor_recibir_peticion(servidor_t *servidor, socket_t *sktActivo){
    peticion_t resultado = PETICION_VALIDA;
    lista_t caracteres;
    lista_crear(&caracteres);
    char paquete[PAQUETE_LARGO_MAXIMO];
    size_t largo = PAQUETE_LARGO_MAXIMO;
    int bytesRecibidos = 1; // > 0 //Para iniciar el ciclo
    size_t numeroLinea = 1;
    while (bytesRecibidos > 0) {
        bytesRecibidos = socket_recibir_algo(sktActivo, paquete, largo);
        bool todoOK;
        todoOK = agregar_a_lista(&caracteres, paquete, bytesRecibidos);
        if (!todoOK){
            resultado = PETICION_ERROR;
            break;
        }
        while (lista_esta(&caracteres, son_mismo_caracter_wrapper, "\n")){
            char *linea = obtener_linea(&caracteres);
            if (numeroLinea <= 1){
                resultado = servidor_procesar_cabecera(servidor, sktActivo, linea);
                numeroLinea += 1;
            } else if (resultado == PETICION_VALIDA) {
                todoOK = servidor_procesar_cuerpo(servidor, linea);
            }
            free(linea);
            if (!todoOK){
                break;
            }
        }
        if (!todoOK){
            resultado = PETICION_ERROR;
            break;
        }
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
bool servidor_aceptar_clientes(servidor_t *servidor, const char *nombreBin){
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
        socket_t sktActivo;
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
Destruye el recurso.
Pre: Recibe un recurso visitado (visitante_t *) ya 
inicializado y lo destruye.
Post: Destruye el recurso.
*/
void visitante_destruir(visitante_t *recurso) {
    free(recurso->nombreRecurso);
}

/*
Pre: Recibe dos cadena de caracteres (char *): un texto, 
y un separador para del cual splitear el texto. El separador
debe pertencer al texto.
Post: Devuelve un arreglo de cadenas, terminada en NULL
(char **), o NULL si ocurrio algun error.
Queda a responsabilidad del usuario liberar la memoria
reservada por de la funcion free_split().
*/

//dir1 = texto
//dir2 = strstr(dir1, separador);
//agregar_al_arreglo(dir1, dir2-dir1)
//dir1 = dir2+largo(separador)

bool _agregar_a_arreglo(lista_t *lista, char* buffer, size_t largo){
    char *nuevaCadena = malloc(sizeof(char)*(largo+1)); // +\0
    if (nuevaCadena == NULL){
        return false;
    }
    snprintf(nuevaCadena, largo, "%s", buffer);
    //AQUI ME QUEDEEEEEEE
    //USAR LISTA_T SERA MAS FACIL, 
    //CONTINUAR

}

char **split(const char *texto, char* separador) {
    size_t largoSeparador = strlen(separador);
    //size_t largoVector = strlen(texto) + 1; //+\0
    size_t largoBuffer = strlen(texto) + 1; //+\0
    //vector_t vector;
    char *bufferAuxiliar = malloc(sizeof(char)*(largoBuffer)); 
    if (bufferAuxiliar == NULL){
        return NULL;
    }

    snprintf(bufferAuxiliar, largoBuffer, "%s", texto);
    size_t factorRedimensionar = 2;
    size_t largoReserva = 10;
    size_t memoriaReservar = sizeof(char *)*largoReserva; 
    char **arregloPartes = malloc(memoriaReservar);
    if (arregloPartes == NULL) {
        free(bufferAuxiliar);
        return NULL;
    }
    size_t i = 0;
    char *direccionSeparador = strstr(bufferAuxiliar, separador);
    while (direccionSeparador!=NULL){
        if (i >= largoReserva) {
            char **nuevoArreglo;
            size_t nuevoLargoReservar = largoReserva * factorRedimensionar;
            size_t memoriaReservar = sizeof(char *)*nuevoLargoReservar;
            nuevoArreglo = realloc(arregloPartes, memoriaReservar);
            if (nuevoArreglo == NULL) {
                for (int j = 0; j<i; ++j){
                    free(arregloPartes[j]);
                }
                free(arregloPartes);
                free(bufferAuxiliar);
                return NULL;
            }
            arregloPartes = nuevoArreglo;
            largoReserva = nuevoLargoReservar;
        }
        size_t k;
        for (k = 0; &bufferAuxiliar[k]!=direccionSeparador; ++k) {}
        size_t posicionSeparador = k;
        size_t memoriaReservar = sizeof(char)*(posicionSeparador + 1);
        char *parte = malloc(memoriaReservar);
        if (parte == NULL){
            for (int j = 0; j<i; ++j){
                free(arregloPartes[j]);
            }
            free(arregloPartes);
            free(bufferAuxiliar);
        }
        for (k = 0; k<posicionSeparador; ++k){
            parte[k] = bufferAuxiliar[k];
        }
        parte[k] = 0;
        arregloPartes[i] = parte;
        ++i;
        size_t posicionInicial = k + largoSeparador;
        for (k = posicionInicial; k<largoBuffer; ++k) {
            bufferAuxiliar[k-posicionInicial] = bufferAuxiliar[k];
        }
        largoBuffer = k-posicionInicial;
        bufferAuxiliar[largoBuffer] = 0;
        direccionSeparador = strstr(bufferAuxiliar, separador);
    }
    char **arregloFinal;
    size_t largoReservarFinal = i+2; // + ultima parte + NULL
    memoriaReservar = sizeof(char *)*largoReservarFinal;
    arregloFinal = realloc(arregloPartes, memoriaReservar);
    if (arregloFinal == NULL) {
        for (int j = 0; j<i; ++j){
            free(arregloPartes[j]);
        }
        free(arregloPartes);
        free(bufferAuxiliar);
        return NULL;
    }
    arregloPartes = arregloFinal;
    largoReserva = largoReservarFinal;
    size_t largoUltimaParte = largoBuffer;// +1 + \0
    memoriaReservar = sizeof(char)*largoUltimaParte;
    char *ultimaParte = malloc(memoriaReservar);
    if (ultimaParte == NULL){
        for (int j = 0; j<i; ++j){
            free(arregloPartes[j]);
        }
        free(arregloPartes);
        free(bufferAuxiliar);
        return NULL;
    } 
    for (int k = 0; k<largoBuffer; ++k) {
        ultimaParte[k] = bufferAuxiliar[k];
    }
    arregloPartes[i] = ultimaParte;
    free(bufferAuxiliar);
    arregloPartes[i+1] = NULL;
    return arregloPartes;
}

/*
Pre: Recibe un arreglo de punteros reservado en memoria 
dinamica, terminado en NULL. Donde, ademas, cada puntero 
del arreglo apunta a un otro bloque de memoria dinamica
reservada.
Cada uno de estos bloques individuales de memoria deben
poder ser liberados con la funcion free().
Post: libera toda la memoria reservada.
*/
void free_split(char **arreglo) {
    if (arreglo == NULL) {
        return;
    }
    for (int i = 0; arreglo[i] != NULL; ++i){
        free(arreglo[i]);
    }
    free(arreglo);
}

/*
Pre: Recibe un archivo de texto ya abierto.
Post: Devuelve una cadena de caracteres (terminada en \0) que 
contiene todos los carateres del archivo desde el inicio hasta
una linea vacia o fin de archivo; o NULL si hubo algun error 
durante la carga.
Queda a respondabilidad del usuario liberar la memoria reservada 
para la cadena, por medio de free().
*/
char *cargar_archivo(FILE *archivo) {
    size_t factorRedimension = 2;
    size_t largoTexto = 1000;
    vector_t vector;
    bool todoOK;
    todoOK = vector_crear(&vector, largoTexto);
    if (!todoOK){
        return NULL;
    }
    size_t i = 0; 
    char caracter;
    while ((caracter = getc(archivo)) != -1){ //eof
        todoOK = vector_insertar(&vector, i , caracter);
        if (!todoOK){
            vector_destruir(&vector);
            return NULL;
        }
        ++i;
        largoTexto = vector_ver_largo(&vector);
        if (largoTexto <= i){
            size_t nuevoLargo = largoTexto * factorRedimension;
            todoOK = vector_redimensionar(&vector, nuevoLargo);
            if (!todoOK){
                vector_destruir(&vector);
                return NULL;
            }
        }
    }
    vector_insertar(&vector, i , '\0');
    largoTexto = i + 1; //Largo final
    char *texto = vector_obtener_cadena(&vector, largoTexto);
    vector_destruir(&vector);
    return texto;
}

int main(int argc, const char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso:\n./server <puerto> <input> [<template>]\n");
        return 1;
    }
   	const char *nombrePuerto = argv[1];
    const char *sensorBinario = argv[2];
    const char *rutaTemplate = argv[3];
    FILE* templateFichero = fopen(rutaTemplate, "rt");
    if (templateFichero == NULL){
        return 1;
    }
    char *template = cargar_archivo(templateFichero);
    fclose(templateFichero);
    if (template == NULL){
        return 1;
    }
    servidor_t servidor;
    bool todoOK;
    todoOK = servidor_crear(&servidor, nombrePuerto, template);
    if (!todoOK){
        free(template);
        return 1;
    }
    todoOK = servidor_aceptar_clientes(&servidor, sensorBinario);
    free(template);
    if (!todoOK){
        return 1;
    }
    servidor_imprimir_visitas(&servidor);
    servidor_destruir(&servidor, NULL);
    return 0;
}
