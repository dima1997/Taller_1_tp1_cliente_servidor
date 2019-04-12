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
        lista->ultimo = nuevoNodo;
    } else {
        lista->ultimo->proximo = nuevoNodo;
    }
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
    lista->largo -= 0;
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
PRE: Recibe un vector (vector_t *) ya declarado, y el
largo del vector (size_t > 0).
POST: Devuelve true si logro inicializar al vector con
exito, false en caso contrario.
*/
bool vector_crear(vector_t *vector, size_t largo){
    if (largo == 0){
        return false;
    }
    vector->datos = malloc(sizeof(void*)*largo);
    if (vector->datos){
        return false;
    }
    for(int i = 0; i < largo; ++i){
        vector->datos[i] = NULL;
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
void vector_destruir(vector_t *vector, void destruir_dato(void *dato)){
    if (vector->datos == NULL){
        return;
    }
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
}

bool vector_redimensionar(vector_t *vector, size_t nuevoLargo){
    if (vector == NULL || nuevoLargo <= 0){
        return false;
    }
    size_t memoriaReservar = sizeof(void*)*nuevoLargo;
    void **nuevosDatos = realloc(vector->datos, memoriaReservar);
    if (nuevosDatos == NULL){
        return false;
    }
    vector->datos = nuevosDatos;
    for(int i = vector->largo; i < nuevoLargo; ++i){
        vector->datos[i] = NULL;
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
void *vector_ver(vector_t *vector, size_t posicion){
    if (vector->largo <= posicion){
        return NULL;
    }
    return vector->datos[posicion];
}

/*
PRE: Recibe un vector (vector_t *) ya creado y una posicion 
(size_t) del mismo, y un dato (void *) a insertar.
POST: Devuelve true si logro insertar el dato en la posicion
indicada, o false en caso de algun error, o fuera de rango.
*/
bool vector_insertar(vector_t *vector, size_t posicion, void *dato){
    if (vector->largo <= posicion){
        return false;
    }
    vector->datos[posicion] = dato;
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
    todoOK = socket_enlazar(&servidor->skt);
    if (!todoOk){
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
    lista_destruir(&servidor->visitantes, visitante_destruir);
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
    fprintf(stdout, "* %s: %d\n", nombre, cantidadVisitas);
}

bool imprimir_visitante_wrapper(void *dato1, void*dato2){
    visitante_t*vistante = (visitante_t*)dato1;
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
void servidor_imprimir_visitas(servidor_t servidor){
    fprintf(stdout,"#Estadisticas de visitantes:\n\n");
    lista_iterar(&servidor->visitantes, imprimir_visitante_wrapper, NULL)
}

bool son_mismo_caracter_wrapper(void *dato1, void *dato2){
    return son_mismo_caracter((char*)d1, (char*)d2);
}

bool son_mismo_caracter(char *caracer1, char *caracter2){
    return (*caracter1 == *caracter2)
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
    char *posSustituto = strstr(servidor->template, sustituto);
    size_t largo = posSustituto - (servidor->template);
    bool seEnvio;
    seEnvio = socket_enviar_todo(skt, servidor->template, largo)
    if (!seEnvio){
        return false;
    }
    seEnvio = socket_enviar_todo(skt, temperaturaBuffer, largoBuffer);
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
        return NULL;
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
    char *nombreVistante = visitante_ver_nombre(visitante1);
    return strcmp(nombreVistante, nombre) == 0;
}

bool es_este_visitante_wrapper(void *dato1, void* dato2){
    visitante_t *visitante = (visitante_t*)dato1;
    char* nombre = (char*)dato2;
    return es_este_visitante(visitante, nombre);
}

bool visitar_si_es_este(visitante_t* visitante, char* nombre){
    if (es_este_visitante(vistante, nombre)){
        visitante_visitar(vistante);
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
    lista_t *visitantes = &servidor->visitantes;
    if (lista_esta(visitantes, son_mismo_visitante_wrapper, nombre)){
        void *nombreWrapper = (void*)nombre;
        lista_iterar(visitantes, visitar_si_es_este_wrapper, nombreWrapper);
        return true;
    }
    visitante_t *nuevoVisitante = malloc(sizeof(visitante_t));
    if (nuevoVisitante == NULL){
        return false;
    }
    bool todoOK;
    todoOK = visitante_crear(nuevoVisitante, nombre);
    if (!seCreo){
        free(nuevoVisitante);
    }
    visitante_visitar(nuevoVisitante);
    todoOK = lista_insertar_ultimo(visitantes, (void*)nuevoVisitante);
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
    bool seCreo;
    seCreo = vector_crear(vector);
    if (!seCreo){
        return NULL;
    }
    char *caracterActual = lista_borrar_primero(lista);
    size_t i = 0;
    while (*caracterActual != '\n'){
        size_t largo = vector_ver_largo(&vector); 
        if (largo <= i){
            bool seRedimensiono;
            size_t nuevoLargo = largo * factorRedimensionar;
            seRedimensiono = vector_redimensionar(&vector, nuevoLargo);
            if (!seRedimensiono){
                vector_destruir(&vector, NULL);
                return NULL;
            }
        }
        vector_insertar(&vector, i, (void*)caracterActual); // OJO
        ++i;
    }
    char *linea = malloc(sizeof(char)*(i+1)); // +\0
    if (linea == NULL){
        vector_destruir(vector);
        return NULL;
    }
    linea[i] = 0;
    for (int j = 0; j < i; ++j){
        char* caracterActual = (char*)vector_ver(vector, i);
        linea[i]= *caracterActual;
    }
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
    lista_crear(caracteres);
    char paquete[PAQUETE_LARGO_MAXIMO];
    size_t largo = PAQUETE_LARGO_MAXIMO;
    int bytesRecibidos = 1; // > 0 //Para iniciar el ciclo
    while (bytesRecibidos > 0) {
        bytesRecibidos = socket_recibir_algo(&sktActivo, paquete, largo);
        bool todoOK;
        todoOK = lista_agregar(&caracteres, (void*)paquete, largo); // mmm...
        if (!todoOK){
            resultado = PETICION_ERROR;
            break;
        }
        while (lista_esta(parte, son_mismo_caracter_wrapper, "\n")){
            char *linea = obtener_linea(caracteres);
            if (numeroLinea <= 1){
                resultado = servidor_procesar_cabecera(servidor, sktActivo, caracteres);
                numeroLinea += 1;
            } else if (resultado == PETICION_VALIDA) {
                todoOK = servidor_procesar_cuerpo(servidor, caracteres);
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
    lista_destruir(lista, NULL);
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
    bool todoOK = true;
    while (!sensor.eof() && todoOK) {
        socket_t sktActivo;
        bool seAcepto;
        seAcepto socket_aceptar(servidor->skt, &sktActivo);
        if (!seAcepto) {
            todoOK = false;
            break;
        }
        int tipoPeticion;
        tipoPeticion = servidor_recibir_peticion(servidor, &sktActivo);
        if (tipoPeticion == PETICION_ERROR){
            todoOK = false;
        } else if (tipoPeticion == PETICION_VALIDA){
            uint16_t numeroLeido;
            size_t largoDato = sizeof(short int);
            fread(&numeroLeido,largoDato,1,sensor);
            numeroLeido = ntohs(numeroLeido);
            double temperaturaSensada = (numeroLeido - 2000.00)/100.00;
            bool seEnvio;
            seEnvio = servidor_enviar_cuerpo(servidor, temperaturaSensada);
            if (!seEnvio){
                todoOK = false;
            }
        }
        socket_destruir(&sktActivo);
    }
    fclose(sensor);
    return todoOK;
}

//implementar:
// tda: vector::crear, destruir, largo, redimensionar, obtener 
// tda: lista_enlazada::crear, destruir, insertar_ultimo, borrar primero, ver ultimo
// tda: servirdor:: crear, destruir, recibir_peticion, enviar_peticion (strstr(template), p + strstr( + len), strstr(p + len)) 

/*
Inicializa una linea_t
Pre: Recibe una linea ya declarada (linea_t *)
Post: Devuelve true si se inicializo la linea con exito, 
false en caso contrario

bool linea_crear(linea_t *linea){
    linea-> largo = 0;
    linea->_largoReservado = PAQUETE_LARGO_MAXIMO;
    linea->caracteres = malloc(sizeof(char)*PAQUETE_LARGO_MAXIMO);
    if (linea->caracteres == NULL) {
        return false;
    }
    linea->caracteres[0] = '\0';
    return true; 
}
*/
/*
Agrega caracteres a la linea
Pre: Recibe una cadena de caracteres (char *) terminada en \0.
Post: Agrega tantos caracteres de la cadena hasta encontrar un \n,
o hasta llegar al final de la misma.
Devuelve la cantidad de caracteres que sobraron desde el \n 
(inclusive).
Devuelve -1 en caso de no poder agregar ningun caracter, por causa 
de algun error.

int linea_agregar_caracteres(linea_t *linea, char *cadena) {
    const size_t factorRedimensionar = 2;
    size_t largoCadena = strlen(cadena);
    char *direccionSalto = strstr(cadena, "\n");
    size_t largoOcupar;
    size_t cantidadSobrante;
    if (direccionSalto != NULL){
        size_t i;
        for (i = 0; &cadena[i]!=direccionSalto; ++i) {}
        size_t posicionSalto = i;
        largoOcupar = linea->largo + posicionSalto;
        cantidadSobrante = largoCadena - posicionSalto; 
    } else {
        largoOcupar = linea->largo + largoCadena;
        cantidadSobrante = 0;
    }
    largoOcupar += 1; // + \0
    if (largoOcupar >= linea->_largoReservado){
        size_t nuevoLargoReservar = largoOcupar*factorRedimensionar;
        size_t memoriaReservar = sizeof(char)*nuevoLargoReservar;
        char *nuevosCaracteres = realloc(linea->caracteres, memoriaReservar); 
        if (nuevosCaracteres == NULL) {
            return -1;
        }
        linea->caracteres = nuevosCaracteres;
        linea->_largoReservado = nuevoLargoReservar;
    } 
    size_t posicionHastaCopiar = largoCadena - cantidadSobrante;
    size_t posicionInicial = linea->largo;
    size_t i;
    for (i = 0; i < posicionHastaCopiar; ++i){
        linea->caracteres[posicionInicial + i] = cadena[i];
    }
    linea->caracteres[posicionInicial + i] = 0;
    linea->largo = posicionInicial + i; 
    return cantidadSobrante; 
}
*/
/*
Borra los caracteres agregados de la linea.
Pre: Recibe una linea (linea_t *) ya  inicializada.
Post: Borra la linea. Tras ejecutar esta linea, los proximos
caracteres a agregar, se escribiran desde el principio de la 
linea.

void linea_borrar(linea_t *linea) {
    linea->caracteres[0] = 0;
    linea->largo = 0;
}
*/
/*
Devuelve un arreglo de los caracteres que forman la linea.
Pre: Recibe una linea (linea_t *) ya inicializada.
Post: Devuelve  un arreglo de caracteres (char *) con todos 
los caracteres agregados hasta el momento; o NULL si hubo 
algun error. 
Queda a responsabilidad del usuario liberar la memoria 
reservada para el arreglo, por medio de la funcion free().

char *linea_ver_caracteres(linea_t *linea) {
    size_t largoCopia = linea->largo + 1; // +\0
    size_t memoriaReservar = sizeof(char) * largoCopia; 
    char *copiaCaracteres = malloc(memoriaReservar);
    if (copiaCaracteres == NULL) {
        return NULL;
    }
    snprintf(copiaCaracteres, largoCopia, "%s", linea->caracteres);
    return copiaCaracteres;
}
*/
/*
Destruye la linea.
Pre: Recibe una linea (linea_t *) ya inicializada.
Post: Destruye la linea.

void linea_destruir(linea_t *linea) {
    free(linea->caracteres);
    return;
}
*/
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
Inicializa un recursosVector_t.
Pre: Recibe un vector de recursos visitados (recursosVector_t *) 
ya declarado.
Post: Devuelve true si logro inicializarlo con exito, false en
caso contrario.

bool recursosVector_crear(recursosVector_t *vector) {
    vector->cantidad = 0;
    size_t cantidadInicial = 10;
    size_t memoriaReservar = sizeof(visitante_t *)*cantidadInicial;
    vector->recursos = malloc(memoriaReservar);
    if (vector->recursos == NULL){
        return false;
    }
    vector->_cantidadReservada = cantidadInicial;
    return true;
}
*/
/*
Visita un recurso. 
Pre: Recibe un vector de recursos visitados (recursosVector_t *), y
el nombre de un recurso a visitar.
Post: Devuelve true si lo logro visitar con exito el recurso de 
nombre recibido, o false en caso contrario.
Nota: el recurso recibido se agrega al vector de forma automatica
si no esta presente. 

bool recursosVector_visitar(recursosVector_t *vector, char *nombreRecurso) {
    size_t factorRedimensionar = 2;
    for (int i = 0; i < vector->cantidad; ++i) {
        visitante_t *recursoActual = vector->recursos[i];
        char *nombreRecursoActual; 
        nombreRecursoActual = visitante_ver_nombre(recursoActual);
        if (strcmp(nombreRecursoActual, nombreRecurso) == 0) {
            visitante_visitar(recursoActual);
            return true;
        }
    }
    // Entonces aun no lo agregamos
    if ((vector->cantidad + 1) >= (vector->_cantidadReservada)) {
        size_t nuevoCantidad = (vector->cantidad + 1) * factorRedimensionar;
        size_t memoriaReservar = sizeof(visitante_t *)*nuevoCantidad;
        visitante_t **nuevosRecursos;
        nuevosRecursos = realloc(vector->recursos, memoriaReservar);
        if (nuevosRecursos == NULL) {
            return false;
        }
        vector->recursos = nuevosRecursos;
        vector->_cantidadReservada = nuevoCantidad;
    }
    visitante_t *nuevoRecurso = malloc(sizeof(visitante_t));
    if (nuevoRecurso == NULL){
        return false;
    }
    bool seInicializo = visitante_crear(nuevoRecurso, nombreRecurso);
    if (seInicializo == false){
        free(nuevoRecurso);
        return false;
    }
    visitante_visitar(nuevoRecurso);
    vector->recursos[vector->cantidad] = nuevoRecurso;
    vector->cantidad += 1;
    return true;
}
*/
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

void recursosVector_imprimir(recursosVector_t *vector) {
    fprintf(stdout, "# Estadisticas de visitantes\n\n");
    for (int i = 0; i < vector->cantidad; ++i) {
        visitante_t * recursoActual = vector->recursos[i];
        char *nombreActual = visitante_ver_nombre(recursoActual);
        int vecesVisitado = visitante_ver_visitas(recursoActual);
        fprintf(stdout, "* %s: %d\n", nombreActual, vecesVisitado);
    }
}
/*
/*
Destruye el recursosVector_t.
Pre: Recibe un vector de recursos visitados (recursosVector_t *)
ya creado e inicializado.
Post: Destruye el vector recibido.

void recursosVector_destruir(recursosVector_t *vector) {
    for (int i = 0; i<vector->cantidad; ++i) {
        visitante_destruir(vector->recursos[i]);
        free(vector->recursos[i]);
    }
    free(vector->recursos);
}
*/
/*
Pre: Recibe dos cadena de caracteres (char *): un texto, 
y un separador para del cual splitear el texto. El separador
debe pertencer al texto.
Post: Devuelve un arreglo de cadenas, terminada en NULL
(char **), o NULL si ocurrio algun error.
Queda a responsabilidad del usuario liberar la memoria
reservada por de la funcion free_split().
*/
char **split(const char *texto, char* separador) {
    size_t largoSeparador = strlen(separador);
    size_t largoBuffer = strlen(texto) + 1; //+\0
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
Pre: Recibe la primer linea de la peticion a responder 
(char *) terminada en '\0'.
Post: Devuelve una cadena de caractares con la respuesta 
a la peticion cuya primer linea recibidmos; o NULL si 
hubo algun error.
Queda a responsabilidad del usuario liberar dicha memoria
reservada, por medio de la funcion free().

char *responder_peticion(const char *primerLinea, char *cuerpo) { 
    char *status = "HTTP/1.1 %s %s\n\n\0";
    bool usarTemplate = false;
    size_t i;
    char **argumentosMetodo = split(primerLinea, " ");
    if (argumentosMetodo == NULL) {
        return NULL;
    }
    char *metodo = argumentosMetodo[0];
    for (i=0; argumentosMetodo[i]!=NULL; ++i) {}
    int cantidadArgumentos = i;
    bool cantArgumentosCorrecta = (cantidadArgumentos == 3);
    bool metodoCorrecto = (strcmp(metodo, "GET")==0);
    char cabecera[CABECERA_lARGO_MAXIMO];
    size_t largoCabecera = sizeof(cabecera);
    if (!(cantArgumentosCorrecta && metodoCorrecto)) {
        snprintf(cabecera, largoCabecera, status, "400", "Bad request");
    } else if (strcmp(argumentosMetodo[1], "/sensor")!=0) { //recurso
        snprintf(cabecera, largoCabecera, status, "404", "Not found");
    } else {
        snprintf(cabecera, largoCabecera, status, "200", "OK");
        usarTemplate = true;
    }
    free_split(argumentosMetodo); 
    largoCabecera = strlen(cabecera);
    size_t largoTemplate = strlen(cuerpo);
    size_t memoriaReservar;
    if (usarTemplate == true) {
        memoriaReservar = sizeof(char)*(largoTemplate + largoCabecera + 1);
        // + \0
    } else {
        memoriaReservar = sizeof(char)*(largoCabecera + 1); 
        // + \0
    }
    char *respuesta = (char *)malloc(memoriaReservar);
    if (respuesta == NULL) {
        return NULL;
    }
    respuesta[0] = '\0';
    snprintf(respuesta, largoCabecera + 1, "%s", cabecera);
    if (usarTemplate == true){
        strncat(respuesta, cuerpo, largoTemplate+1); //overflow
    }
    return respuesta;
}
*/
/*
Pre: Recibe una cadena de caracteres (char *) con la siguiente 
linea del cuerpo de la peticion a procesar, y un vector con los
visitantes de peticiones anteriores (recursosVector_t *).
Post: Devuelve true si logro procesa la linea recibida con exito, 
actualizando la informacion del vector de visitas recibido; o
false en caso contrario.

bool procesar_lineas_cuerpo(char *linea, recursosVector_t *visitantes) {
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
        char *nombreRecurso = campos[1];
        bool seVisito;
        seVisito = recursosVector_visitar(visitantes, nombreRecurso); 
        free_split(campos); 
        return seVisito;
    }
    free_split(campos); 
    return true;
}
*/
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

bool procesar_cuerpo(char *cuerpo, linea_t *linea, recursosVector_t *vector) {
    char resto[PAQUETE_LARGO_MAXIMO];
    snprintf(resto, PAQUETE_LARGO_MAXIMO, "%s", cuerpo);
    
    int largoAnterior = strlen(cuerpo);
    int largoResto = linea_agregar_caracteres(linea, resto);
    if (largoResto == 0) {    
        return true;
        }
    if (largoResto < 0) {
        return false;
    }
    while (largoResto > 0) {
        char *siguienteLinea = linea_ver_caracteres(linea);
        if (siguienteLinea == NULL) {
            return false;
        }
        bool seProceso;
        seProceso = procesar_lineas_cuerpo(siguienteLinea, vector); 
        free(siguienteLinea);
        if (seProceso == false) {
            return false;
        }
        linea_borrar(linea);
        int posicionSobrante = largoAnterior - largoResto;
        int i;
        for (i = 1; i< largoResto; ++i) {
            resto[i-1] = resto[posicionSobrante + i];
        }
        resto[i-1] = 0;
        largoAnterior = largoResto -1; 
        largoResto = linea_agregar_caracteres(linea, resto);
        if (largoResto == 0) {
            return true;
        }
        if (largoResto < 0) {
            return false;
        }
    }
    return (largoResto == 0);
}
*/
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

char *procesar_peticion(int *skt, char* template, recursosVector_t *visitantes){
    int estado = 0;
    bool estaSocketRemotoCerrado = false;
    int bytesRecibidos = 0;
    char paquete[PAQUETE_LARGO_MAXIMO];
    size_t largoMaximo = sizeof(paquete);
    linea_t linea;
    bool seCreoLinea = linea_crear(&linea);
    if (seCreoLinea == false) {
        return NULL;
    }
    bool hayError = false;
    char *respuesta = NULL; 
    size_t numeroLinea = 0; // primer linea: cabecera
    // tda socket
    while (estaSocketRemotoCerrado == false) {
        estado = recv(*skt, &paquete, largoMaximo - 1, MSG_NOSIGNAL);
        if (estado < 0) {
            printf("Error: %s\n", strerror(errno));
            hayError = true;
            break;
        } else if (estado == 0) {
            estaSocketRemotoCerrado = true;
            continue;
        }
        bytesRecibidos = estado; 
        paquete[bytesRecibidos] = 0;
    //tda socket

        if (numeroLinea <= 0) {
            int cupoSobrante;
            cupoSobrante = linea_agregar_caracteres(&linea, paquete);
            if (cupoSobrante < 0) {
            	hayError = true;
            	break;
            }
            if (cupoSobrante > 0) {
                char *primerLinea = linea_ver_caracteres(&linea);
                if (primerLinea == NULL) {
                	hayError = true;
                	break;
                }
                respuesta = responder_peticion(primerLinea, template);
                free(primerLinea);
                if (respuesta == NULL) {
                	hayError = true;
                	break;
                }
                if (!(strlen(respuesta)>CABECERA_lARGO_MAXIMO)) {
                    break;
                }
                numeroLinea += 1;
                linea_borrar(&linea);
                int posicionSobrante = bytesRecibidos - cupoSobrante;
                int i;
                for (i = 1; i< cupoSobrante; ++i) {
                    paquete[i-1] = paquete[posicionSobrante + i];
                }
                paquete[i-1] = 0;
            }
        }   
        bool seProceso;
        seProceso = procesar_cuerpo(paquete, &linea, visitantes);
        if (seProceso == false) {
        	hayError = true;
        	break;
        }
    }
    linea_destruir(&linea);
    if (hayError == true && respuesta != NULL) {
    	free(respuesta);
    	respuesta = NULL;
    }
    return respuesta;
}
*/
/*
Pre: Recibe un socket ya conectado: skt (int *), y la 
respuesta que a enviar (char *).
Post: Devuelve true si logro enviar toda la respuesta, 
false en caso contrario, dado un error en el socket.

bool enviar_respuesta(int *skt, char *mensaje, size_t largoMensaje) {
    int estado = 0;
    bool hayErrorDeSocket = false;
    int bytesEnviados = 0;
    while (bytesEnviados < largoMensaje && hayErrorDeSocket == false) {
        size_t largoMaximo = largoMensaje - bytesEnviados;
        estado = send(*skt, &mensaje[bytesEnviados], largoMaximo, MSG_NOSIGNAL);
        if (estado < 0) { 
            printf("Error: %s\n", strerror(errno));
            hayErrorDeSocket = true;
        } else if (estado == 0) { 
            hayErrorDeSocket = true;
        } else {
            bytesEnviados += estado;
        }
    }
    return !(hayErrorDeSocket);
}
*/
/*
Pre:  Recibe un socket (int *), y el nombre del puerto 
(char *) al cual conectarse.
Post: Devuelve true si logro conectar el socket al puerto
recibido y setearlo para funcionar de socket pasivo; false 
en caso contrario.

bool conectar_socket_pasivo(int *skt, const char *puerto) {
	int estado = 0;
    struct addrinfo hints;
    struct addrinfo *direccion;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; //TCP
    hints.ai_flags = AI_PASSIVE; //AI_PASSIVE para servidor
    estado = getaddrinfo(NULL, puerto, &hints, &direccion);
    if (estado != 0) { 
        printf("Error in getaddrinfo: %s\n", gai_strerror(estado));
        return false;
    }
    struct addrinfo *dir = direccion;
    *skt = socket(dir->ai_family, dir->ai_socktype, dir->ai_protocol);
    if (*skt == -1) {
        printf("Error: %s\n", strerror(errno));
        freeaddrinfo(direccion);
        return false;
    }
    // Evita que le servidor falle al abrirlo y cerrarlo en poco tiempo
    int val = 1;
    estado = setsockopt(*skt, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    if (estado == -1) {
        printf("Error: %s\n", strerror(errno));
        close(*skt);
        freeaddrinfo(direccion);
        return false;
    }
    estado = bind(*skt, dir->ai_addr, dir->ai_addrlen);
    freeaddrinfo(direccion);
    if (estado == -1) {
        printf("Error: %s\n", strerror(errno));
        close(*skt);
        return false;
    }
    estado = listen(*skt, CLIENTES_ESCUCHAR);
    if (estado == -1) {
        printf("Error: %s\n", strerror(errno));
        close(*skt);
        return false;
    }
    return true;
}
*/
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
    size_t tamanioInicial = largoTexto * sizeof(char);
    char *texto = (char *)malloc(tamanioInicial);
    if (texto == NULL){
        return NULL;
    }
    char caracter;
    char caracterAnterior = '\0';
    size_t i = 0; 
    while ((caracter = getc(archivo)) != -1){ //eof
        if (caracter == '\n') {
            if (caracterAnterior == '\n' || caracterAnterior == '\0'){
                //Una linea vacia marca el final de la peticion
                //Para cuando archivo es stdin
                break;
            }
        }
        if (i >= largoTexto){
            size_t nuevoLargo = largoTexto*sizeof(char)*factorRedimension;
            char *nuevoTexto = realloc(texto, nuevoLargo);
            if (nuevoTexto == NULL) {
                free(texto);
                return NULL;
            }
            largoTexto = nuevoLargo;
        }
        texto[i] = caracter;
        ++i;
        caracterAnterior = caracter;
    }
    size_t largoFinal = i+1;
    char *textoFinal = realloc(texto, largoFinal);
    if (textoFinal == NULL) {
        free(texto);
        return NULL;
    }
    texto = textoFinal;
    texto[i] = '\0';
    return texto;
}

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

char **cargar_template(const char *ruta) {
	FILE *archivoTemplate; 
    if ((archivoTemplate = fopen(ruta, "rt")) == NULL) {
        fprintf(stderr, "Archivo template no encontrado.\n");
        return NULL;
    }
    char *template = cargar_archivo(archivoTemplate);
    fclose(archivoTemplate); 
    if (template == NULL) {
        fprintf(stderr, "Error al cargar template.\n");
        return NULL;
    }
    char **partesTemplate = split(template, "{{datos}}");
    free(template);
    if (partesTemplate == NULL){
        fprintf(stderr, "Error al procesar template.\n");
        return NULL;
    }
    return partesTemplate;
}
*/
/*
Pre: Recibe las dos partes del template (char **), y 
la temperatura (double) a meter entre dichas partes.
Post: Devuelve el cuerpo de la respuesta (char *), que
viene a ser las dos partes del template unidas por medio
de la temperatura recibida; o NULL si ocurrio algun error.
Queda a responsabilidad del usuario liberar la memoria
reservada por medio de la funcion free(); 

char *contruir_cuerpo(char **partesTemplate, double temperatura) {
	char buffer[TEMPERATURA_LARGO_MAXIMO];
    size_t largoBuffer = sizeof(buffer);
    snprintf(buffer, largoBuffer, "%.2f", temperatura);
    size_t largoPrimera = strlen(partesTemplate[0]);
    size_t largoSegunda = strlen(partesTemplate[1]);
    size_t memoriaReservar; 
    size_t largoCuerpo = largoPrimera + largoSegunda + largoBuffer + 1;
    // + \0
    memoriaReservar = sizeof(char)*largoCuerpo; 
    char *cuerpo = malloc(memoriaReservar);
    if (cuerpo == NULL) {
        return NULL;
    }
    cuerpo[0] = '\0';
    size_t largoSiguiente = largoPrimera + 1;
    strncat(cuerpo, partesTemplate[0], largoSiguiente);
    largoSiguiente += largoBuffer;
    strncat(cuerpo, buffer, largoSiguiente);
    largoSiguiente += largoSegunda;
    strncat(cuerpo, partesTemplate[1], largoSiguiente);
    return cuerpo;
}
*/
int main(int argc, const char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso:\n./server <puerto> <input> [<template>]\n");
        return 1;
    }
   	const char *nombrePuerto = argv[1];
    const char *sensorBinario = argv[2];
    const char *rutaTemplate = argv[3];
    char *template = cargar_archivo(rutaTemplate);
    if (template == NULL){
        return 1;
    }
    servidor_t servidor;
    bool todoOK;
    todoOK = servidor_crear(servidor, nombrePuerto, template);
    if (!todoOK){
        free(template);
        return 1;
    }
    todoOK = servidor_aceptar_clientes(servidor, sensorBinario);
    free(template);
    if (!todoOK){
        return 1;
    }
    servidor_imprimir_visitas(servidor);
    servidor_destruir(servidor);
    return 0;
    /*
    //Conectamos socket pasivo.
    int sktPasivo, sktActivo = 0;
	bool seConecto;
	seConecto = conectar_socket_pasivo(&sktPasivo, nombrePuerto);
	if (seConecto == false) {
		return 1;
	}
    //Procesamos template.
    char **partesTemplate = cargar_template(rutaTemplate);
    if (partesTemplate == NULL) {
    	close(sktPasivo);
    	return 1;
    }
    //Abrimos binario.
    FILE *archivoBinario; 
    if ((archivoBinario = fopen(sensorBinario, "rb")) == NULL) {
        fprintf(stderr, "Archivo binario no encontrado.\n");
        close(sktPasivo);
        free_split(partesTemplate); 
        return 1;
    }
    //Inicializamos vector de recursos visitados.
    recursosVector_t visitantes;
    bool seInicializo = recursosVector_crear(&visitantes);
    if (seInicializo == false) {
        close(sktPasivo);
        free_split(partesTemplate); 
        return 1;
    }
    //Comezamos a procesar clientes
    uint16_t numeroLeido; 
    bool hayError = false;
    int cantidadLeidos;
    cantidadLeidos = fread(&numeroLeido, sizeof(short int),1,archivoBinario);
    numeroLeido = ntohs(numeroLeido);
    double temperaturaSensada = (numeroLeido - 2000.00)/100.00;
    char *cuerpo = contruir_cuerpo(partesTemplate, temperaturaSensada);
    if (cuerpo == NULL) {
        hayError = true;
    }
    while (cantidadLeidos > 0 && hayError == false) {  
        sktActivo = accept(sktPasivo, NULL, NULL);
        if (sktActivo == -1) {
            printf("Error: %s\n", strerror(errno));
            hayError = true;
        } else {
            char *respuesta;
            respuesta = procesar_peticion(&sktActivo, cuerpo, &visitantes); 
            if (respuesta == NULL){
                shutdown(sktActivo, SHUT_RDWR);
                close(sktActivo);
                break;
            }
            size_t largoRespuesta = strlen(respuesta);
            bool seEnvio;
            seEnvio =  enviar_respuesta(&sktActivo, respuesta, largoRespuesta);
            shutdown(sktActivo, SHUT_RDWR);
            close(sktActivo);
            free(respuesta);
            if (seEnvio == false) {
                break;
            }
            if (largoRespuesta > CABECERA_lARGO_MAXIMO) {
                free(cuerpo);
                size_t largoDato = sizeof(short int);
                cantidadLeidos = fread(&numeroLeido,largoDato,1,archivoBinario);
                numeroLeido = ntohs(numeroLeido);
                double temperaturaSensada = (numeroLeido - 2000.00)/100.00;
                cuerpo = contruir_cuerpo(partesTemplate, temperaturaSensada);
                if (cuerpo == NULL) {
                    hayError = true;
                }
            }
    	}
    }
    free(cuerpo);
    fclose(archivoBinario);
    free_split(partesTemplate); 
    shutdown(sktPasivo, SHUT_RDWR);
    close(sktPasivo);
    */
    /*
    if (hayError == true) {
        recursosVector_destruir(&visitantes);
       return 1;
    } else { 
        recursosVector_imprimir(&visitantes);
        recursosVector_destruir(&visitantes);
        return 0;
    }
    */
}
