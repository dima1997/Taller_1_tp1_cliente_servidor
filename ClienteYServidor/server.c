// Servidor
#define _POSIX_C_SOURCE 200112L
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include <arpa/inet.h>

#define TEMPERATURA_LARGO_MAXIMO 100
#define MENSAJE_LARGO_MAXIMO 1000
#define CABECERA_lARGO_MAXIMO 27
#define PAQUETE_LARGO_MAXIMO 1024 // 1 k

typedef struct linea {
    char *caracteres;
    size_t largo;
    size_t _largoReservado;
} linea_t ;

/*
Inicializa una Linea
Pre: recibe una linea ya creada (linea_t *)
Post: devuelve true si se inicializo la linea con exito, 
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
Pre: Recibe una cadena de caracteres (char *) terminada en \0.
Post: Agrega tantos caracteres de la cadena hasta encontrar un \n,
o hasta llegar al final de la misma.
Devuelve la cantidad de caracteres que sobraron desde el \n.
Devuelve -1 en caso de no poder agregar ningun caracter, por falta
de memoria.
*/
int linea_agregar_caracteres(linea_t *linea, char *cadena) {
    const size_t factorRedimensionar = 2;
    size_t largoCadena = strlen(cadena);
    char *direccionSalto = strstr(cadena, "\n");
    size_t largoOcupar;
    size_t cantidadSobrante;
    if (direccionSalto != NULL){
        size_t i;
        for (i = 0; &cadena[i]!=direccionSalto;++i) {}
        size_t posicionSalto = i;
        largoOcupar = linea->largo + posicionSalto;
        cantidadSobrante = largoCadena - posicionSalto; 
        // hola\nPedro => cant_cupoSobrante = 10 - 4 = 6 :) 
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
Pre: Recibe una linea (linea_t *) ya creada e inicializada.
Post: Borra la linea. Tras ejecutar esta linea, los proximos
caracteres a agregar, es escribiran desde el principio de la 
linea.
*/
void linea_borrar(linea_t *linea) {
    linea->caracteres[0] = 0;
    linea->largo = 0;
}
/*
Pre: Recibe una linea (linea_t *) ya creada e inicializada.
Post: Devuelve la un arreglo de caracteres constante
(const char *) con todos los caracteres agregados hasta
el momento; o NULL si hubo algun error. 
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
Pre: recibe una linea ya creada e inicializada.
Post: destruye la linea.
*/
void linea_destruir(linea_t *linea) {
    free(linea->caracteres);
    return;
}

/*
Pre: Recibe dos cadenas de caracteres terminadas en \0: 
un texto (char *) y una palabra (char *) que este en el 
primero.
Post: Devuelve un arreglo dinamico de dos cadenas de 
caracteres que corresponden a la parte del texto que 
viene antes de la palabra recibida (primera cadena) y a 
la que viene despues de la misma (segunda cadena). El
arreglo termina en NULL. Si ocurrio algun error devuelve
NULL. 
Queda a responsabilidad del usuario liberar la memoria 
reservada para cada cadena y para el arreglo en si, por
medio de la funcion free().
*/
char **partir_texto(char *texto, char *palabra) {
    char *direccionPalabra = strstr(texto, palabra);
    if (direccionPalabra == NULL) {
        return NULL;
    }
    size_t i; 
    for (i = 0; &texto[i] != direccionPalabra; ++i) {}
    size_t posicionPalabra = i;
    size_t memoriaReservar = sizeof(char)*(posicionPalabra+1);
    char *primerParte = malloc(memoriaReservar);
    if (primerParte == NULL) {
        return NULL;
    } 
    strncpy(primerParte, texto, posicionPalabra);
    primerParte[posicionPalabra] = '\0';
    size_t largoTexto = strlen(texto);
    size_t largoPalabra = strlen(palabra);
    size_t largoSegundaParte = largoTexto - largoPalabra - posicionPalabra;
    memoriaReservar = sizeof(char)*(largoSegundaParte+1);
    char *segundaParte = malloc(memoriaReservar);
    if (segundaParte == NULL) {
        free(primerParte);
        return NULL;
    }
    size_t posicionSegundaParte;
    posicionSegundaParte = posicionPalabra + largoPalabra;
    for (i = posicionSegundaParte; i<largoTexto; ++i){
        segundaParte[i - posicionSegundaParte] = texto[i];
    }
    segundaParte[largoSegundaParte] = '\0';
    char **ambasPartes = malloc(sizeof(char*)*3); // 2 partes + 1 \0
    if (ambasPartes == NULL){
        free(segundaParte);
        free(primerParte);
        return NULL;
    }
    ambasPartes[0] = primerParte;
    ambasPartes[1] = segundaParte;
    ambasPartes[2] = NULL;
    return ambasPartes;

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
void free_arreglo_punteros(void **arreglo) {
    if (arreglo == NULL) {
        return;
    }
    for (int i = 0; arreglo[i] != NULL; ++i){
        free(arreglo[i]);
    }
    free(arreglo);
}

/*
Pre: recibe la primer linea de la peticion a responder 
(char *) terminada en '\0'.
Post: devuelve una cadena de caractares guardados en memoria 
reservada, con la respuesta a la peticion recibida; o NULL si 
hubo algun error.
Queda a responsabilidad del usuario liberar dicha memoria
reservada.
*/
char *responder_peticion(const char *primerLinea, char *cuerpo) { //, bool *seUsoTemplate
    char *status = "HTTP/1.1 %s %s\n\n\0";
    bool usarTemplate = false;
    size_t i;
    char **argumentosMetodo = split(primerLinea, ' ');
    if (argumentosMetodo == NULL) {
        return NULL;
    }
    char *metodo = argumentosMetodo[0];
    char *recurso = argumentosMetodo[1]; // ESTO EXPLOTARA ?
    for (i=0; argumentosMetodo[i]!=NULL; ++i) {}
    int cantidadArgumentos = i;
    bool cantArgumentosCorrecta = (cantidadArgumentos == 3);
    bool metodoCorrecto = (strcmp(metodo, "GET")==0);
    char cabecera[CABECERA_lARGO_MAXIMO];
    size_t largoCabecera = sizeof(cabecera);
    if (!(cantArgumentosCorrecta && metodoCorrecto)) {
        snprintf(cabecera, largoCabecera, status, "400", "Bad request");
    } else if (strcmp(recurso, "/sensor")!=0) {
        snprintf(cabecera, largoCabecera, status, "404", "Not found");
    } else {
        snprintf(cabecera, largoCabecera, status, "200", "OK");
        usarTemplate = true;
    }
    free_strv(argumentosMetodo);
    largoCabecera = strlen(cabecera);
    size_t largoTemplate = strlen(cuerpo);
    size_t memoriaReservar;
    if (usarTemplate == true) {
        memoriaReservar = sizeof(char)*(largoTemplate + largoCabecera + 2); // + \n + \0
    } else {
        memoriaReservar = sizeof(char)*(largoCabecera + 1); // + \0
    }
    char *respuesta = (char *)malloc(memoriaReservar);
    if (respuesta == NULL) {
        return NULL;
    }
    respuesta[0] = '\0';
    strcat(respuesta, cabecera);
    if (usarTemplate == true){
        strcat(respuesta, cuerpo);
        strcat(respuesta, "\n");
    }
    return respuesta;
}

typedef struct recursoVisitado {
    char *nombreRecurso;
    size_t vecesVisitado;
} recursoVisitado_t;

/*
Pre: Recibe un recurso visitado (recursoVisitado *) ya creado, y su 
nombre (char *), terminado en \0.
Post: Inicializa el recurso. Devuelve true si logro inicializarlo
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
Pre: Recibe un recurso visitado (recursoVisitado *) ya 
creado e inicializa.
Post: Devuelve el nombre del recurso (char *). 
*/
char *recursoVisitado_ver_nombre(recursoVisitado_t *recurso) {
    return recurso->nombreRecurso;
} 

/*
Pre: Recibe un recurso visitado (recursoVisitado *) ya 
creado e inicializado.
Post: Devuelve la cantidad de veces (int) que el recurso 
fue visitado.
*/
int recursoVisitado_ver_visitas(recursoVisitado_t *recurso){
    return recurso->vecesVisitado;
}

/*
Pre: Recibe un recurso visitado (recursoVisitado *) ya 
creado  e inicializado.
Post: Lo visita.
*/
void recursoVisitado_visitar(recursoVisitado_t *recurso) {
    recurso->vecesVisitado += 1;
}

/*
Pre: recibe un recurso visitado (recursoVisitado_t *) ya creado
e inicializado y lo destruye.
*/
void recursoVisitado_destruir(recursoVisitado_t *recurso) {
    free(recurso->nombreRecurso);
}

typedef struct recursosVector {
    recursoVisitado_t **recursos;
    size_t _cantidadReservada; 
    size_t cantidad;
} recursosVector_t;

/*
Pre: Recibe un vector de recursos visitados (recursosVector_t *) 
ya creado.
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
Pre: Recibe un vector de recursos visitados (recursosVector_t *), y
el nombre de un recurso a visitar.
Post: Visita el recurso de nombre recibido. Devuelve true si lo logro 
visitar con exito, o false en caso contrario.
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
        fprintf(stdout, "%s: %d\n", nombreActual, vecesVisitado);
    }
}

/*
Pre: Recibe una cadena de caracteres (char *) con la siguiente 
linea del cuerpo de la peticion a procesar, y un vector con los
visitantes de peticiones anteriores.
Post: procesa la linea recibida, actualizando la informacion
del vector de visitas recibido.
*/
bool procesar_cuerpo_peticion(char *linea, recursosVector_t *visitantes) {
    
    char **campos = partir_texto(linea, ": ");

    if (campos == NULL) {
        return false;
    }
    size_t i;
    for (i = 0; campos[i]!=NULL; ++i) {}
    size_t cantidadCampos = i; 
    if (cantidadCampos != 2) {
        // Esto no ha de pasar, pero tomamos como supuesto
        // que simplemente se ignora la linea.
        free_arreglo_punteros((void **)campos);
        return true; 
    }
    char *nombreCampo = campos[0];
    if (strcmp(nombreCampo, "User-Agent")==0){
        char *nombreRecurso = campos[1];
        bool seVisito;
        seVisito = recursosVector_visitar(visitantes, nombreRecurso); 
        free_arreglo_punteros((void **)campos);
        return seVisito;
    }
    free_arreglo_punteros((void **)campos);
    return true;
}

/*
Pre: Recibe el resto de caracteres de la peticion que no 
agrega linea_agregar_caracteres en linea (linea_t *) quien
tambien lo recibe, y al vector de recursos visitados por
peticiones anteriores.
Post: procesa el resto de los caracteres recibidos.
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
        seProceso = procesar_cuerpo_peticion(siguienteLinea, vector); 
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
Pre: Recibe un socket ya conectado, el template para
        restoSinSalto[i-1] = 0;
        largoResto = linea_agregar_caracteres(linea, restoSinSalto);
la posible respuesta actual, la direccion de un booleano
que indique se se utiliza el template, y la lista de sitios
visitados.
Post: Recibe y procesa una peticion del cliente. 
Devuelve la respuesta a la peticion http (char *), modificando
el valor de haber utilizado o no el template recibido;
o NULL si hubo algun error con el socket.
Actualiza los sitios visitados.
Queda a responsabilidad del usuario liberar la memoria
reservada para la linea, por medio de la funcion free.
*/
char *procesar_peticion(int skt, char* template, recursosVector_t *visitantes) {
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
    char *respuesta;
    size_t numeroLinea = 0; // primer linea: cabecera
    while (estaSocketRemotoCerrado == false) {
        estado = recv(skt, &paquete, largoMaximo - 1, MSG_NOSIGNAL);

        if (estado < 0) {
            printf("Error: %s\n", strerror(errno));
            linea_destruir(&linea);
            return NULL;
        }
        else if (estado == 0) {
            estaSocketRemotoCerrado = true;
            continue;
        }
        bytesRecibidos = estado; 
        paquete[bytesRecibidos] = 0;
        if (numeroLinea <= 0) {
            int cupoSobrante;
            cupoSobrante = linea_agregar_caracteres(&linea, paquete);
            if (cupoSobrante < 0) {
                linea_destruir(&linea);
                return NULL;
            }
            if (cupoSobrante > 0) {
                char *primerLinea = linea_ver_caracteres(&linea);
                if (primerLinea == NULL) {
                    linea_destruir(&linea);
                    return NULL;
                }
                respuesta = responder_peticion(primerLinea, template);
                free(primerLinea);
                if (respuesta == NULL) {
                    linea_destruir(&linea);
                    return NULL;
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
        if (seProceso == false){
            free(respuesta);
            linea_destruir(&linea);
            return NULL;
        }
    }
    linea_destruir(&linea);
    return respuesta;
}

int main(int argc, const char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso:\n./server <puerto> <input> [<template>]\n");
        return 1;
    }
   	const char *nombrePuerto = argv[1];
    const char *sensorBinario = argv[2];
    const char *rutaTemplate = argv[3];
    FILE *archivoTemplate; 
    if ((archivoTemplate = fopen(rutaTemplate, "rt")) == NULL) {
        fprintf(stderr, "Archivo template no encontrado.\n");
        return 1;
    }
    char *template = cargar_archivo(archivoTemplate);
    fclose(archivoTemplate); 
    if (template == NULL) {
        fprintf(stderr, "Error al cargar template.\n");
        return 1;
    }
    char **partesTemplate = partir_texto(template,"{{datos}}");
    free(template);
    if (partesTemplate == NULL){
        fprintf(stderr, "Error al procesar template.\n");
        return 1;
    }
    //Sockets
    int estado = 0;
    bool seguirEjecutando = true;
    bool hayErrorDeAceptacion = false;
    
    struct addrinfo hints;
    struct addrinfo *ptr;

    int sktPasivo, sktActivo = 0;
    int val;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;       // IPv4 (or AF_INET6 for IPv6)
    hints.ai_socktype = SOCK_STREAM; // TCP  (or SOCK_DGRAM for UDP)
    hints.ai_flags = AI_PASSIVE; // AI_PASSIVE for server
    
    estado = getaddrinfo(NULL, nombrePuerto, &hints, &ptr);

    if (estado != 0) { 
        printf("Error in getaddrinfo: %s\n", gai_strerror(estado));
        free_arreglo_punteros((void *)partesTemplate);
        return 1;
    }

    sktPasivo = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

    if (sktPasivo == -1) {
        printf("Error: %s\n", strerror(errno));
        freeaddrinfo(ptr);
        free_arreglo_punteros((void *)partesTemplate);
        return 1;
    }
    //---------------------------------------------------------------------------------------
    // Copiado de:
    // https://github.com/Taller-de-Programacion/clases/blob/master/sockets/src/echoserver.c

    // Evita que le servidor falle al abrilo y cerrarlo en poco tiemp

    // Activamos la opcion de Reusar la Direccion en caso de que esta
    // no este disponible por un TIME_WAIT
    val = 1;
    estado = setsockopt(sktPasivo, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    if (estado == -1) {
        printf("Error: %s\n", strerror(errno));
        close(sktPasivo);
        freeaddrinfo(ptr);
        free_arreglo_punteros((void *)partesTemplate);
        return 1;
    }
    //---------------------------------------------------------------------------------------
    estado = bind(sktPasivo, ptr->ai_addr, ptr->ai_addrlen);
    if (estado == -1) {
        printf("Error: %s\n", strerror(errno));
        close(sktPasivo);
        freeaddrinfo(ptr);
        free_arreglo_punteros((void *)partesTemplate);
        return 1;
    }
    freeaddrinfo(ptr);

    estado = listen(sktPasivo, 15); // definir constante !!!
    if (estado == -1) {
        printf("Error: %s\n", strerror(errno));
        close(sktPasivo);
        free_arreglo_punteros((void *)partesTemplate);
        return 1;
    }

    FILE *archivoBinario; 
    if ((archivoBinario = fopen(sensorBinario, "rb")) == NULL) {
        fprintf(stderr, "Archivo binario no encontrado.\n");
        close(sktPasivo);
        free_arreglo_punteros((void *)partesTemplate);
        return 1;
    }
    recursosVector_t visitantes;
    bool seInicializo = recursosVector_crear(&visitantes);
    if (seInicializo == false) {
        close(sktPasivo);
        free_arreglo_punteros((void *)partesTemplate);
        return 1;
    }

    short int numeroLeido; 
    int cantidadLeidos;
    cantidadLeidos = fread(&numeroLeido, sizeof(short int),1,archivoBinario);
        
    while (cantidadLeidos > 0 && seguirEjecutando ) { 
        sktActivo = accept(sktPasivo, NULL, NULL);   // aceptamos un cliente
        if (sktActivo == -1) {
            printf("Error: %s\n", strerror(errno));
            printf("VIENE DE AQUI\n");
            seguirEjecutando = false;
            hayErrorDeAceptacion = false;
        } else {
            double temperaturaSensada = (numeroLeido - 2000)/100;
            char temperatura[TEMPERATURA_LARGO_MAXIMO];
            size_t largoBuffer = sizeof(temperatura);
            snprintf(temperatura, largoBuffer, "%.2f", temperaturaSensada);
            size_t largoPrimera = strlen(partesTemplate[0]);
            size_t largoSegunda = strlen(partesTemplate[1]);
            size_t memoriaReservar; 
            memoriaReservar = largoPrimera + largoSegunda + largoBuffer + 1; // + \0
            char *cuerpo = malloc(memoriaReservar);
            if (cuerpo == NULL) {
                shutdown(sktActivo, SHUT_RDWR);
                close(sktActivo);
                break;
            }
            cuerpo[0] = '\0';
            strcat(cuerpo, partesTemplate[0]);
            strcat(cuerpo, temperatura);
            strcat(cuerpo, partesTemplate[1]);
            char *respuesta;
            respuesta = procesar_peticion(sktActivo, cuerpo, &visitantes);            
            free(cuerpo);
            if (respuesta == NULL){
                shutdown(sktActivo, SHUT_RDWR);
                close(sktActivo);
                break;
            }
            size_t largoRespuesta = strlen(respuesta);
            enviar_mensaje(sktActivo, respuesta, largoRespuesta);
            shutdown(sktActivo, SHUT_RDWR);
            close(sktActivo);
            free(respuesta);
            if (largoRespuesta > CABECERA_lARGO_MAXIMO){
                cantidadLeidos = fread(&numeroLeido, sizeof(short int),1,archivoBinario);
            }
    	}
    }
    fclose(archivoBinario);
    free_arreglo_punteros((void **)partesTemplate);
    shutdown(sktPasivo, SHUT_RDWR);
    close(sktPasivo);

    if (hayErrorDeAceptacion) {
        recursosVector_destruir(&visitantes);
       return 1;
    } else { 
        recursosVector_imprimir(&visitantes);
        recursosVector_destruir(&visitantes);
        return 0;
    }
}


