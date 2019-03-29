// Servidor

/*
El servidor abre un archivo binario que simula ser un sensor. Luego escucha conexiones en un
puerto, y, mientras haya datos para leer en el archivo binario, realiza lo siguiente:
	+Se leen los datos del sensor.
	+Acepta un cliente y recibe un ​ request ​ HTTP.
	+Verifica que el método utilizado sea del tipo "GET" y el recurso sea "/sensor". Si el
	método no es "GET", la respuesta será un error de tipo "400 Bad request", y si el
	recurso no es "/sensor", la respuesta será un error de tipo "404 Not found". Si el método
	y recurso son válidos, la respuesta es de tipo "200 OK".
	+Lee la cabecera del ​ request , ​ si encuentra la clave "User-Agent", lee su valor y cuenta
	una visita de este agente. La cabecera finaliza con una línea en blanco.
	+Una vez que procesó la cabecera, le envía el cuerpo del mensaje:
○ Si el ​ request ​ es válido, el cuerpo del mensaje es el contenido del archivo
template, reemplazando el marcador ​ {{datos}}​ por los datos del sensor (ver
más adelante).
○ Si el ​ request ​ es inválido, no habrá cuerpo del mensaje.
Si el request fue válido, se intenta leer otro dato del archivo del sensor, y se repite el ciclo. Si el
request fue inválido, se utiliza la lectura anterior del sensor.

*/
#define _POSIX_C_SOURCE 200112L
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include <arpa/inet.h>

#define TEMPERATURA_LARGO_MAXIMO 100
#define MENSAJE_LARGO_MAXIMO 1000
#define PAQUETE_LARGO_MAXIMO 1024 // 1 k
/*
uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);
*/

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
    if (caracteres == NULL) {
        return false;
    }
    linea->caracteres[0] = '\0';
    return true; 
}
/*
Pre: recibe una cadena de caracteres terminada en \0.
Post: agrega tantos caracteres de la cadena hasta encontrar un \n
Devuelve la cantidad de caracteres que sobraron despues del \n.
Devuelve -1 en caso de no poder agregar ningun caracter, por falta
de memoria.
*/
int linea_agregar_caracteres(char *cadena) {
    const size_t factorRedimensionar = 2;
    size_t largoCadena = strlen(cadena);
    char *direccionSalto = strstr(cadena, "\n");
    size_t largoOcupar;
    size_t posicionSalto;
    size_t cantidadSobrantes;
    if (direccionSalto != NULL){
        size_t i;
        for (i = 0; &cadena[i]!=direccionSalto;++i) {}
        posicionSalto = i;
        largoOcupar = linea->largo + i;
        cantidadSobrantes = largoCadena - i; 
    } else {
        largoOcupar = linea->largo + largoCadena;
        cantidadSobrantes = 0;
    }
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
    size_t posicionHastaCopiar = largoCadena - cantidadSobrantes;
    size_t posicionInicial = linea->largo;
    size_t i;
    for (i = 0; i < posicionHastaCopiar; ++i){
        linea->caracteres[posicionInicial + i] = cadena[i];
    }
    linea->largo = posicionInicial + i; 
    return cantidadSobrantes;
    
}

/*
Pre: recibe una linea ya creada e inicializada.
Post: destruye la linea.
*/
void linea_destruir(linea_t linea) {
    free(linea->caracteres);
    return;
}

/*
Pre: Recibe un socket ya conectado.
Post: Recibe y procesa una peticion del cliente. 
Devuelve la primer linea de la peticion http (char *);
o NULL si hubo algun error con el socket.
Queda a responsabilidad del usuario liberar la memoria
reservada para la linea, por medio de la funcion free.
*/
char *servidor_procesar_peticion() {
    size_t factorRedimensionar = 2;
    int estado = 0;
    bool hayErrorDeSocket = false;
    bool estaSocketRemotoCerrado = false;
    int bytesRecibidos = 0;

    char parteDePeticion[PAQUETE_LARGO_MAXIMO];
    size_t largoMaximo = sizeof(parteDePeticion);
    size_t largoLinea = largoMaximo;
    char *primerLinea = malloc(sizeof(char)*largoLinea);
    if (primerLinea == NULL) {
        return NULL;
    }
    // Busco primerLinea
    size_t numeroLinea = 0;
    size_t posicionPrimerLinea = 0;
    while (numeroLinea <= 0 && hayErrorDeSocket == false && estaSocketRemotoCerrado == false) {
        estado = recv(skt, &parteDePeticion, largoMaximo - 1, MSG_NOSIGNAL);

        if (estado < 0) {
            printf("Error: %s\n", strerror(errno));
            hayErrorDeSocket = true;
        }
        else if (estado == 0) {
            //Suponemos que termino de recibir el mensaje 
            // y que no se cerró por otra razon
            estaSocketRemotoCerrado = true;
        }
        else {
            bytesRecibidos = estado; 
            parteDePeticion[bytesRecibidos] = 0;
            char *direccionSalto = strstr(parteDePeticion, "\n");
            if (posicionSalto == NULL) {
                size_t nuevoLargo = largoLinea * factorRedimensionar;
                size_t memoriaReservar;
                memoriaReservar = sizeof(char) * nuevoLargo;
                char *nuevaLinea = realloc(primerLinea, memoriaReservar);
                if (nuevaLinea == NULL) {
                    free(primerLinea);
                    return NULL;
                }
                primerLinea = nuevaLinea;
                largoLinea = nuevoLargo;
                for (int j = 0; j < bytesRecibidos; ++j){
                    primerLinea[posicionPrimerLinea + j] = parteDePeticion[j];
                }
                posicionPrimerLinea += bytesRecibidos; 
                bytesRecibidos = 0; 
            } else  {
                size_t i;
                for (i = 0; &parteDePeticion[i] != direccionSalto; ++i) {}
                size_t posicionSalto = i;
                largoFinal = posicionPrimerLinea + posicionSalto + 1;
                char *lineaFinal = realloc();
                //USAR TAD LINEA CREADO MAS ARRIBA
            }


        }
    }
    return hayErrorDeSocket;
}

/*
Pre: Recibe dos cadenas de caracteres terminadas en \0: 
un texto (char *) y una palabra (char *) que este en el 
primero.
Post: Devuelve un arreglo dinamico a dos cadenas de 
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
Pre: recibe dos cadenas de caracteres terminadas en \0 (char *).
Post: devuelve un nueva cadena, que es la concatenacion de las dos 
cadenas recibidas, anteponiendo la primera cadena antes de la 
segunda. La nueva cadena termina en \0. Si ocurrio algun problema 
devuelve NULL.
Queda a responsabilidad del usuario liberar la memoria reservada,
por medio de la funcion free(). 
*/
char *concatenar(char *primerCadena, char *segundaCadena) {
    size_t largoPrimera = strlen(primerCadena);
    size_t largoSegunda = strlen(segundaCadena);
    size_t memoriaReservar = largoPrimera + largoSegunda + 1; // + \0
    char *cadenaConcatenada = malloc(memoriaReservar);
    if (cadenaConcatenada == NULL) {
        return NULL;
    }
    snprintf(cadenaConcatenada, largoPrimera, "%s", primerCadena);
    size_t i;
    for (i = largoPrimera; segundaCadena[i-largoPrimera] != '\0'; ++i) {
        cadenaConcatenada[i] = segundaCadena[i-largoPrimera];
    }
    cadenaConcatenada[i] = '\0';
    return cadenaConcatenada;
}

/*
Pre: Recibe una ruta (const char *) a un archivo binario, 
el cual tiene numeros representados por 16 bits.
Post: Devuelve un puntero a un arreglo de short int, 
terminado en NULL, cuyos elementos, son cada uno de los 
numeros del archivo, representados en el endianess de la
maquina host; o NULL si ocurrio algun problema durante la 
carga.
Queda a responsabilidad del usuario liberar la memoria 
reservada para este arreglo por medio de la funcion
free(), primero de cada shor int del arreglo, y luego 
al puntero al arreglo.
*/
short int **cargar_binario(const char *rutaBinario){
    size_t factorRedimensionar = 2;
    FILE *archivoBinario; 
    if ((archivoBinario = fopen(rutaBinario, "rb")) == NULL) {
        fprintf(stderr, "Archivo binario no encontrado.\n");
        return NULL;
    }
    size_t largoArreglo = 100;
    size_t memoriaReservar = sizeof(short int *) * largoArreglo;
    short int **punteroArreglo = (short int **)malloc(memoriaReservar);
    if (punteroArreglo == NULL) {
        fclose(archivoBinario);
        return NULL;
    }

    size_t i = 0; 
    punteroArreglo[i] = (short int *)malloc(sizeof(short int));
    if (punteroArreglo[i] == NULL){
        free(punteroArreglo);
        fclose(archivoBinario);
        return NULL;
    }
    while (fread(punteroArreglo[i], sizeof(short int),1,archivoBinario)) { //(void *)
        *punteroArreglo[i] = ntohs(*punteroArreglo[i]);
        ++i;
        if (i >= largoArreglo) {
            size_t nuevoLargo;
            nuevoLargo = largoArreglo * sizeof(short int) * factorRedimensionar ;
            short int **nuevoPunteroArreglo = realloc(punteroArreglo, nuevoLargo);
            if (nuevoPunteroArreglo == NULL){
                for (int j = 0; j<i; ++j){
                    free(punteroArreglo[i]);
                }
                free(punteroArreglo);
                fclose(archivoBinario);
                return NULL;
            }
            punteroArreglo = nuevoPunteroArreglo;
        }
        punteroArreglo[i] = (short int *)malloc(sizeof(short int));
        if (punteroArreglo[i] == NULL){
            for (int j = 0; j<i; ++j){
                free(punteroArreglo[i]);
            }
            free(punteroArreglo);
            fclose(archivoBinario);
            return NULL;
        }
    }
    free(punteroArreglo[i]);
    punteroArreglo[i] = NULL;
    fclose(archivoBinario);
    size_t largoFinal = (i + 1) * sizeof(short int *);
    short int **nuevoPunteroArreglo = realloc(punteroArreglo, largoFinal);
    if (nuevoPunteroArreglo == NULL){
        for (int j = 0; j<i; ++j){
            free(punteroArreglo[i]);
        }
        free(punteroArreglo);
        return NULL;
    }
    punteroArreglo = nuevoPunteroArreglo;
    return punteroArreglo;
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
typedef struct recursoVisitado {
    char *recursoVisitado;
    size_t vecesVisitado;
} recursoVisitado_t;
*/
/*
Pre: recibe una cadena de caracteres con la peticion a responder 
(char *) terminada en '\0'.
Post: devuelve una cadena de caractares guardados en memoria 
reservada, con la respuesta a la peticion recibida; o NULL si 
hubo algun error.
*/
char *responder_peticion(char *peticion, char *templateRespuesta, bool *seUsoTemplate) {
    char *status = "HTTP/1.1 %s %s\n\n\0";
    size_t i;
    for (i = 0; peticion[i] != '\n'; ++i) {}
    char *primerLinea = malloc(sizeof(char)*(i+1));
    if (primerLinea == NULL) {
        return NULL;
    }
    strncpy(primerLinea, peticion, i); // Y si mejor uso snprintf ?
    primerLinea[i] = '\0';
    char **argumentosMetodo = split(primerLinea, ' ');
    free(primerLinea);
    if (argumentosMetodo == NULL) {
        return NULL;
    }
    char *metodo = argumentosMetodo[0];
    char *recurso = argumentosMetodo[1];
    for (i=0; argumentosMetodo[i]!=NULL; ++i) {}
    int cantidadArgumentos = i;
    bool cantArgumentosCorrecta = (cantidadArgumentos == 3);
    bool metodoCorrecto = (strcmp(metodo, "GET")==0);
    char cabecera[30];
    size_t largoCabecera = sizeof(cabecera);
    if (!(cantArgumentosCorrecta && metodoCorrecto)) {
        snprintf(cabecera, largoCabecera, status, "400", "Bad request");
        *seUsoTemplate = false;
    } else if (strcmp(recurso, "/sensor")!=0) {
        snprintf(cabecera, largoCabecera, status, "404", "Not found");
        *seUsoTemplate = false;
    } else {
        snprintf(cabecera, largoCabecera, status, "200", "OK");
        *seUsoTemplate = true;
    }
    free_strv(argumentosMetodo);
    largoCabecera = strlen(cabecera);
    size_t largoTemplate = strlen(templateRespuesta);
    size_t memoriaReservar;
    if (*seUsoTemplate == true) {
        memoriaReservar = sizeof(char)*(largoTemplate + largoCabecera + 3); // + \n + \n + \0
    } else {
        memoriaReservar = sizeof(char)*(largoCabecera + 1); // + \0
    }
    char *respuesta = (char *)malloc(memoriaReservar);
    if (respuesta == NULL) {
        return NULL;
    }
    respuesta[0] = '\0';
    strcat(respuesta, cabecera);
    if (*seUsoTemplate == true){
        strcat(respuesta, templateRespuesta);
        strcat(respuesta, "\n\n");
    }
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
    bool esLaAceptacionDelSocketValido = true;
    
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
    short int numeroLeido; 
    int cantidadLeidos;
    cantidadLeidos = fread(&numeroLeido, sizeof(short int),1,archivoBinario);
        
    while (cantidadLeidos > 0 && seguirEjecutando ) { 
        sktActivo = accept(sktPasivo, NULL, NULL);   // aceptamos un cliente
        if (sktActivo == -1) {
            printf("Error: %s\n", strerror(errno));
            seguirEjecutando = false;
            esLaAceptacionDelSocketValido = false;
        } else {
            char peticion[MENSAJE_LARGO_MAXIMO];
            size_t largoMaximo = sizeof(peticion);
            
            recibir_mensaje(sktActivo, peticion, largoMaximo-1); 
            
            
            printf("%s", peticion);


            //-----------------------------------------------------------------------
            double temperaturaSensada = (numeroLeido - 2000)/100;
            char temperatura[TEMPERATURA_LARGO_MAXIMO];
            size_t largoBuffer = sizeof(temperatura);
            snprintf(temperatura, largoBuffer, "%.2f", temperaturaSensada);
            size_t largoPrimera = strlen(partesTemplate[0]);
            size_t largoSegunda = strlen(partesTemplate[1]);
            size_t memoriaReservar; 
            memoriaReservar = largoPrimera + largoSegunda + largoBuffer + 1; // + \0
            char *templateRespuesta = malloc(memoriaReservar);
            if (templateRespuesta == NULL) {
                shutdown(sktActivo, SHUT_RDWR);
                close(sktActivo);
                break;
            }
            templateRespuesta[0] = '\0';
            strcat(templateRespuesta, partesTemplate[0]);
            strcat(templateRespuesta, temperatura);
            strcat(templateRespuesta, partesTemplate[1]);
            bool seUsoTemplate;
            char *respuesta = responder_peticion(peticion, templateRespuesta, &seUsoTemplate); 
            free(templateRespuesta);
            if (respuesta == NULL){
                shutdown(sktActivo, SHUT_RDWR);
                close(sktActivo);
                break;
            }
            size_t largoRespuesta = strlen(respuesta);
            //---------------------------------------------------------------------------------
            enviar_mensaje(sktActivo, respuesta, largoRespuesta);

            shutdown(sktActivo, SHUT_RDWR);
            close(sktActivo);
            free(respuesta);
            if (seUsoTemplate == true){
                cantidadLeidos = fread(&numeroLeido, sizeof(short int),1,archivoBinario);
            }
    	}
    }
    fclose(archivoBinario);
    free_arreglo_punteros((void **)partesTemplate);
    shutdown(sktPasivo, SHUT_RDWR);
    close(sktPasivo);

    if (esLaAceptacionDelSocketValido) {
       return 1;
    } else { 
        return 0;
    }
}


