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
#include "server.h"
#define TEMPERATURA_LARGO_MAXIMO 100
#define CABECERA_lARGO_MAXIMO 27
#define PAQUETE_LARGO_MAXIMO 1024 // 1 k
#define CLIENTES_ESCUCHAR 15

/*
Inicializa una linea_t
Pre: Recibe una linea ya declarada (linea_t *)
Post: Devuelve true si se inicializo la linea con exito, 
false en caso contrario
*/
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

/*
Borra los caracteres agregados de la linea.
Pre: Recibe una linea (linea_t *) ya  inicializada.
Post: Borra la linea. Tras ejecutar esta linea, los proximos
caracteres a agregar, se escribiran desde el principio de la 
linea.
*/
void linea_borrar(linea_t *linea) {
    linea->caracteres[0] = 0;
    linea->largo = 0;
}

/*
Devuelve un arreglo de los caracteres que forman la linea.
Pre: Recibe una linea (linea_t *) ya inicializada.
Post: Devuelve  un arreglo de caracteres (char *) con todos 
los caracteres agregados hasta el momento; o NULL si hubo 
algun error. 
Queda a responsabilidad del usuario liberar la memoria 
reservada para el arreglo, por medio de la funcion free().
*/
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

/*
Destruye la linea.
Pre: Recibe una linea (linea_t *) ya inicializada.
Post: Destruye la linea.
*/
void linea_destruir(linea_t *linea) {
    free(linea->caracteres);
    return;
}

/*
Inicializa un recursoVisitado_t
Pre: Recibe un recurso visitado (recursoVisitado *) ya 
declarado, y su nombre (char *), terminado en \0.
Post: Devuelve true si logro inicializar el recurso
con exito, false en caso contrario.
*/
bool recursoVisitado_crear(recursoVisitado_t *recurso, char *nombre) {
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
Pre: Recibe un recurso visitado (recursoVisitado *) ya 
inicializado.
Post: Devuelve el nombre del recurso (char *). 
*/
char *recursoVisitado_ver_nombre(recursoVisitado_t *recurso) {
    return recurso->nombreRecurso;
} 

/*
Devuelve la cantidad de visitas.
Pre: Recibe un recurso visitado (recursoVisitado *) ya 
inicializado.
Post: Devuelve la cantidad de veces (int) que el recurso 
fue visitado.
*/
int recursoVisitado_ver_visitas(recursoVisitado_t *recurso) {
    return recurso->vecesVisitado;
}

/*
Visita el recurso.
Pre: Recibe un recurso visitado (recursoVisitado *) ya 
inicializado.
Post: Visita el recurso, aumentando la cantidad de visitas.
*/
void recursoVisitado_visitar(recursoVisitado_t *recurso) {
    recurso->vecesVisitado += 1;
}

/*
Destruye el recurso.
Pre: Recibe un recurso visitado (recursoVisitado_t *) ya 
inicializado y lo destruye.
Post: Destruye el recurso.
*/
void recursoVisitado_destruir(recursoVisitado_t *recurso) {
    free(recurso->nombreRecurso);
}

/*
Inicializa un recursosVector_t.
Pre: Recibe un vector de recursos visitados (recursosVector_t *) 
ya declarado.
Post: Devuelve true si logro inicializarlo con exito, false en
caso contrario.
*/
bool recursosVector_crear(recursosVector_t *vector) {
    vector->cantidad = 0;
    size_t cantidadInicial = 10;
    size_t memoriaReservar = sizeof(recursoVisitado_t *)*cantidadInicial;
    vector->recursos = malloc(memoriaReservar);
    if (vector->recursos == NULL){
        return false;
    }
    vector->_cantidadReservada = cantidadInicial;
    return true;
}

/*
Visita un recurso. 
Pre: Recibe un vector de recursos visitados (recursosVector_t *), y
el nombre de un recurso a visitar.
Post: Devuelve true si lo logro visitar con exito el recurso de 
nombre recibido, o false en caso contrario.
Nota: el recurso recibido se agrega al vector de forma automatica
si no esta presente. 
*/
bool recursosVector_visitar(recursosVector_t *vector, char *nombreRecurso) {
    size_t factorRedimensionar = 2;
    for (int i = 0; i < vector->cantidad; ++i) {
        recursoVisitado_t *recursoActual = vector->recursos[i];
        char *nombreRecursoActual; 
        nombreRecursoActual = recursoVisitado_ver_nombre(recursoActual);
        if (strcmp(nombreRecursoActual, nombreRecurso) == 0) {
            recursoVisitado_visitar(recursoActual);
            return true;
        }
    }
    // Entonces aun no lo agregamos
    if ((vector->cantidad + 1) >= (vector->_cantidadReservada)) {
        size_t nuevoCantidad = (vector->cantidad + 1) * factorRedimensionar;
        size_t memoriaReservar = sizeof(recursoVisitado_t *)*nuevoCantidad;
        recursoVisitado_t **nuevosRecursos;
        nuevosRecursos = realloc(vector->recursos, memoriaReservar);
        if (nuevosRecursos == NULL) {
            return false;
        }
        vector->recursos = nuevosRecursos;
        vector->_cantidadReservada = nuevoCantidad;
    }
    recursoVisitado_t *nuevoRecurso = malloc(sizeof(recursoVisitado_t));
    if (nuevoRecurso == NULL){
        return false;
    }
    bool seInicializo = recursoVisitado_crear(nuevoRecurso, nombreRecurso);
    if (seInicializo == false){
        free(nuevoRecurso);
        return false;
    }
    recursoVisitado_visitar(nuevoRecurso);
    vector->recursos[vector->cantidad] = nuevoRecurso;
    vector->cantidad += 1;
    return true;
}

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
void recursosVector_imprimir(recursosVector_t *vector) {
    fprintf(stdout, "# Estadisticas de visitantes\n\n");
    for (int i = 0; i < vector->cantidad; ++i) {
        recursoVisitado_t * recursoActual = vector->recursos[i];
        char *nombreActual = recursoVisitado_ver_nombre(recursoActual);
        int vecesVisitado = recursoVisitado_ver_visitas(recursoActual);
        fprintf(stdout, "* %s: %d\n", nombreActual, vecesVisitado);
    }
}

/*
Destruye el recursosVector_t.
Pre: Recibe un vector de recursos visitados (recursosVector_t *)
ya creado e inicializado.
Post: Destruye el vector recibido.
*/
void recursosVector_destruir(recursosVector_t *vector) {
    for (int i = 0; i<vector->cantidad; ++i) {
        recursoVisitado_destruir(vector->recursos[i]);
        free(vector->recursos[i]);
    }
    free(vector->recursos);
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
*/
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
        strncat(respuesta, cuerpo, largoTemplate+1);
    }
    return respuesta;
}

/*
Pre: Recibe una cadena de caracteres (char *) con la siguiente 
linea del cuerpo de la peticion a procesar, y un vector con los
visitantes de peticiones anteriores (recursosVector_t *).
Post: Devuelve true si logro procesa la linea recibida con exito, 
actualizando la informacion del vector de visitas recibido; o
false en caso contrario.
*/
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

/*
Pre: Recibe un socket ya conectado: skt (int *), y la 
respuesta que a enviar (char *).
Post: Devuelve true si logro enviar toda la respuesta, 
false en caso contrario, dado un error en el socket.
*/
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
/*
Pre:  Recibe un socket (int *), y el nombre del puerto 
(char *) al cual conectarse.
Post: Devuelve true si logro conectar el socket al puerto
recibido y setearlo para funcionar de socket pasivo; false 
en caso contrario.
*/
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
    //------------------------------------------------------
    // Adaptado de:
    // https://github.com/Taller-de-Programacion/clases/blob/master/sockets/src/echoserver.c

    // Evita que le servidor falle al abrirlo y cerrarlo en poco tiempo

    // Activamos la opcion de Reusar la Direccion en caso de que esta
    // no este disponible por un TIME_WAIT
    int val = 1;
    estado = setsockopt(*skt, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    if (estado == -1) {
        printf("Error: %s\n", strerror(errno));
        close(*skt);
        freeaddrinfo(direccion);
        return false;
    }
    //------------------------------------------------------
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
*/
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

/*
Pre: Recibe las dos partes del template (char **), y 
la temperatura (double) a meter entre dichas partes.
Post: Devuelve el cuerpo de la respuesta (char *), que
viene a ser las dos partes del template unidas por medio
de la temperatura recibida; o NULL si ocurrio algun error.
Queda a responsabilidad del usuario liberar la memoria
reservada por medio de la funcion free(); 
*/
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

int main(int argc, const char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso:\n./server <puerto> <input> [<template>]\n");
        return 1;
    }
   	const char *nombrePuerto = argv[1];
    const char *sensorBinario = argv[2];
    const char *rutaTemplate = argv[3];
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
    while (cantidadLeidos > 0 && hayError == false) {  
        sktActivo = accept(sktPasivo, NULL, NULL);
        if (sktActivo == -1) {
            printf("Error: %s\n", strerror(errno));
            hayError = true;
        } else {
            numeroLeido = ntohs(numeroLeido);
            double temperaturaSensada = (numeroLeido - 2000.00)/100.00;
            char *cuerpo = contruir_cuerpo(partesTemplate, temperaturaSensada);
            if (cuerpo == NULL){
            	shutdown(sktActivo, SHUT_RDWR);
                close(sktActivo);
                break;
            }
            char *respuesta;
            respuesta = procesar_peticion(&sktActivo, cuerpo, &visitantes); 
            free(cuerpo);
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
            //Lo invierto de vuelta, pues se volvera a invertir luego
            //si no entro en el if
            numeroLeido = ntohs(numeroLeido);
            if (largoRespuesta > CABECERA_lARGO_MAXIMO) {
                size_t largoDato = sizeof(short int);
                cantidadLeidos = fread(&numeroLeido,largoDato,1,archivoBinario);
            }
    	}
    }
    fclose(archivoBinario);
    free_split(partesTemplate); 
    shutdown(sktPasivo, SHUT_RDWR);
    close(sktPasivo);
    if (hayError == true) {
        recursosVector_destruir(&visitantes);
       return 1;
    } else { 
        recursosVector_imprimir(&visitantes);
        recursosVector_destruir(&visitantes);
        return 0;
    }
}
