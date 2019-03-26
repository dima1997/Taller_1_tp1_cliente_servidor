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


Formato de la respuesta HTTP
El formato de la respuesta (​ response ​ ) posee dos partes separadas por una línea en blanco. La
primer parte, la cabecera, está compuesta por la línea de estado (​ status ​ ), la sección de
cabecera, similar a la de un ​ request , ​ y el cuerpo del mensaje.La línea de estado tiene el siguiente formato:
El protocolo utilizado será "HTTP/1.1"
Si el petitorio fue válido (ver detalle más adelante en la sección del servidor), el status es "200"
y la descripción "OK".
Si el petitorio no tiene un formato válido o es una acción no válida, el status es "400" y la
descripción es "Bad request".
Si el petitorio es válido, pero el recurso buscado no es válido, el status es "404" y la descripción
es "Not found".


// Sensor
Los datos del sensor de temperatura son simulados por un archivo binario. Este archivo está
compuesto por números de 16 bits en formato ​ big-endian ​ . Los mismos se interpretan de la
siguiente forma: Temperatura = (datos - 2000) / 100.
Ejemplo:
Dado el archivo cuyo contenido es: ​ 10 00 03 E8​ , (4 bytes),
extraigo 2 números de 16 bits cada uno: ​ 0x1000​ (4096) y ​ 0x03E8​ (1000),
que corresponden a las temperaturas "20.96" y "-10.00" respectivamente.
Nótese que el archivo tiene 4 bytes: la notación ​ 10 00 03 E8​ es solo para
representar el contenido del mismo (binario) en números hexadecimal.


// template
El archivo ​ template ​ es un archivo de texto con la plantilla de respuesta del servidor. El mismo
posee código HTML, y en alguna parte del archivo el texto ​ {{datos}}​ . Este texto será
suplantado por la temperatura leída por el sensor.



// Servidor
El servidor se ejecuta con la siguiente línea de comandos:
./server ​ <port> <sensor-filename> <template-filename>
Donde ​ <port>​ es el puerto de escucha del servidor, ​ <sensor-filename>​ es el nombre del
archivo del sensor, y ​ <template-filename>​ es el template HTML para responder los petitorios
HTTP exitosos.


Si el servidor tiene un número de parámetros incorrecto, se imprime por salida de error
estándar lo siguiente:
Uso:
./server <puerto> <input> [<template>]


El servidor no recibe información de la entrada estándar. Luego de finalizar la lectura de datos
del sensor, escribe por salida estándar un resumen de los navegadores que visitaron al
dispositivo.
El informe del servidor posee el siguiente formato:
# Estadisticas de visitantes
* <nombre-agente-1>: <cantidad-visitas>
* <nombre-agente-2>: <cantidad-visitas>
* <nombre-agente-n>: <cantidad-visitas>
Los agentes se numeran según el ​ orden de aparición​ .


Tanto el cliente como el servidor deben retornar ​ 0 ​ si todo salió correctamente o ​ 1 ​ en caso
contrario.
*/
#define _POSIX_C_SOURCE 200809L 
//#define _POSIX_C_SOURCE 200112L
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include <arpa/inet.h>

#define TEMPERATURA_LARGO_MAXIMO 100
#define MENSAJE_LARGO_MAXIMO 1000
/*
uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);
*/
/*
Pre: recibe la ruta a un archivo de texto tipo template, 
en la que en algun lugar del mismo se encuentra la cadena:
{{datos}}, y un dato numerico en cadena de caracteres (char*).
Post: Devuelve un vector dinamico (vector_t *) reemplazando 
la cadena {{datos}}, por el dato recibido por parametro. O
NULL si surgió algun problema durante la carga.
Restriccion: de haber mas de una {{datos}} por linea, 
reemplaza solo al primero de ellos.
Queda a responsabilidad de quien usa esta funcion, de liberar
el espacio reservado para el vector, por medio de la funcion
vector_destruir(vector_t* vector)
*/
vector_t *cargar_template(const char *ruta_template, char *dato){
    size_t largoDato = strlen(dato);

    FILE *archivoTemplate; 
    if ((archivoTemplate = fopen(ruta_template, "rt")) == NULL) {
        fprintf(stderr, "Archivo template no encontrado.\n");
        return NULL;
    }
    char* linea = NULL; 
    size_t capacidadLinea = 0; 
    ssize_t largoLinea;

    size_t largoVector = 200;
    	
    vector_t *templateVector = vector_crear(largoVector);
    	
    if (templateVector == NULL){
   		free(linea);
       	fclose(archivoTemplate);
       	printf("Error al cargar template\n");
       	return NULL;
   	}
    	
   	size_t posicionVector = 0;
    size_t factorRedimensionar = 2;
    const char *sustitutoDato = "{{datos}}";
    size_t largoSustitutoDato = strlen(sustitutoDato);

    while ((largoLinea = getline(&linea,&capacidadLinea,archivoTemplate)) > 0){ 
    // que hacemos si el getline falla ?
        char *datoPosicion = strstr(linea, sustitutoDato);
        size_t proximoUltimoIndice = posicionVector+largoLinea+largoDato;
        if (proximoUltimoIndice >= largoVector) {
            size_t nuevoLargo = proximoUltimoIndice * factorRedimensionar;
            bool seRedimensiono;
            seRedimensiono = vector_redimensionar(templateVector, nuevoLargo);
            if (!seRedimensiono){
                vector_destruir(templateVector);
                free(linea);
                fclose(archivoTemplate);
                return NULL;
            }
            largoVector = nuevoLargo;
        }
        if (datoPosicion == NULL){
            for (int i = 0; i < largoLinea; ++i){
                vector_guardar(templateVector, posicionVector, linea[i]);
                ++posicionVector;
            }
            continue;
        }
        size_t i;
        for (i = 0;  &linea[i] != datoPosicion; ++i){
            vector_guardar(templateVector, posicionVector, linea[i]);
            ++posicionVector;
        }
        size_t j;
        for (j = 0; j < largoDato; ++j){
            vector_guardar(templateVector, posicionVector, dato[j]);
            ++posicionVector;
        }
        for (i += largoSustitutoDato; i < largoLinea; ++i){
            vector_guardar(templateVector, posicionVector, linea[i]);
            ++posicionVector;
        }   		
    }
    free(linea);
    fclose(archivoTemplate);
    size_t largoFinalVector = posicionVector;
    bool seRedimensiono;
    seRedimensiono = vector_redimensionar(templateVector, largoFinalVector);
    if (!seRedimensiono){
        vector_destruir(templateVector);   
        return NULL;
    }
    return templateVector;
}

/*
// Sensor
Los datos del sensor de temperatura son simulados por un archivo binario. Este archivo está
compuesto por números de 16 bits en formato ​ big-endian ​ . Los mismos se interpretan de la
siguiente forma: Temperatura = (datos - 2000) / 100.
Ejemplo:
Dado el archivo cuyo contenido es: ​ 10 00 03 E8​ , (4 bytes),
extraigo 2 números de 16 bits cada uno: ​ 0x1000​ (4096) y ​ 0x03E8​ (1000),
que corresponden a las temperaturas "20.96" y "-10.00" respectivamente.
Nótese que el archivo tiene 4 bytes: la notación ​ 10 00 03 E8​ es solo para
representar el contenido del mismo (binario) en números hexadecimal.
*/

/*
Pre: recibe la ruta de un archivo binario compuesto por numeros de 16 bits en
formato big-endian.
Post: imprime los cada numero del archivo intepretado en forma decimal.
*/
void imprimir_sensor(const char *rutaSensor){
    FILE *archivoSensor; 
    if ((archivoSensor = fopen(rutaSensor, "rb")) == NULL) {
        fprintf(stderr, "Archivo binario no encontrado.\n");
        return;
    }
    short int *numeroLeido = malloc(sizeof(short int)); 
    while (fread(numeroLeido, sizeof(short int),1,archivoSensor)) {
        double temperaturaSensada = (ntohs(*numeroLeido)-2000)/100;
        printf("%.2f\n", temperaturaSensada);
    }
    
    fclose(archivoSensor);
    return;
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

int main(int argc, const char *argv[]){
    if (argc != 4) {
        fprintf(stderr, "Uso:\n./server <puerto> <input> [<template>]\n");
        return 1;
    }
    const char *sensorBinario = argv[2];
    const char *template = argv[3];
    short int **numerosLeidos = cargar_binario(sensorBinario);
    if (numerosLeidos == NULL) {
        fprintf(stderr, "Error al cargar el binario\n");
        return 1;
    }
    size_t i;
    for (i = 0; numerosLeidos[i] != NULL; ++i) {}
    size_t largoLeidos = i;
    for (i = 0; numerosLeidos[i] != NULL; ++i) {
        double temperaturaSensada = (*numerosLeidos[i] - 2000)/100;
        char temperaturaBuffer[TEMPERATURA_LARGO_MAXIMO];
        size_t largoBuffer = sizeof(temperaturaBuffer);
        snprintf(temperaturaBuffer, largoBuffer, "%.2f", temperaturaSensada);
        vector_t *templateVector;
        templateVector = cargar_template(template, temperaturaBuffer);
        if (templateVector == NULL){
            for (int j = 0; j<largoLeidos+1; ++j){
                free(numerosLeidos[i]);
            }
            free(numerosLeidos);
            fprintf(stderr, "Error al cargar el template.\n");
            return 1;
        }
        vector_imprimir(templateVector);
        vector_destruir(templateVector);
    }
    for (int j = 0; j<largoLeidos+1; ++j){
        free(numerosLeidos[j]);
    }
    free(numerosLeidos);

    //Sockets
    int estado = 0;
    unsigned short len = 0;
    bool seguirEjecutando = true;
    bool esLaAceptacionDelSocketValido = true;
    
    struct addrinfo hints;
    struct addrinfo *ptr;

    int sktPasivo, sktActivo = 0;
    int val;

    char small_buf[MAX_SMALL_BUF_LEN];
    char *tmp;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;       // IPv4 (or AF_INET6 for IPv6)
    hints.ai_socktype = SOCK_STREAM; // TCP  (or SOCK_DGRAM for UDP)
    hints.ai_flags = AI_PASSIVE; // AI_PASSIVE for server
    
    estado = getaddrinfo(NULL, argv[1], &hints, &ptr);

    if (estado != 0) { 
        printf("Error in getaddrinfo: %s\n", gai_strerror(estado));
        return 1;
    }

    sktPasivo = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

    if (sktPasivo == -1) {
        printf("Error: %s\n", strerror(errno));
        freeaddrinfo(ptr);
        return 1;
    }

    // Activamos la opcion de Reusar la Direccion en caso de que esta
    // no este disponible por un TIME_WAIT
    val = 1;
    estado = setsockopt(sktPasivo, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    if (estado == -1) {
        printf("Error: %s\n", strerror(errno));
        close(sktPasivo);
        freeaddrinfo(ptr);
        return 1;
    }

    // Decimos en que direccion local queremos escuchar, en especial el puerto
    // De otra manera el sistema operativo elegiria un puerto random
    // y el cliente no sabria como conectarse
    estado = bind(sktPasivo, ptr->ai_addr, ptr->ai_addrlen);
    if (estado == -1) {
        printf("Error: %s\n", strerror(errno));
        close(sktPasivo);
        freeaddrinfo(ptr);
        return 1;
    }

    freeaddrinfo(ptr);

    // Cuanto clientes podemos mantener en espera antes de poder acceptarlos?
    estado = listen(sktPasivo, 20);
    if (estado == -1) {
        printf("Error: %s\n", strerror(errno));
        close(sktPasivo);
        return 1;
    }

    while (seguirEjecutando) {
        sktActivo = accept(sktPasivo, NULL, NULL);   // aceptamos un cliente
        if (sktActivo == -1) {
            printf("Error: %s\n", strerror(errno));
            seguirEjecutando = false;
            esLaAceptacionDelSocketValido = false;
        } else {
            char buffer[MENSAJE_LARGO_MAXIMO];
            size_t largoMaximo = sizeof(buffer);
            int bytesRecibidos; 
            bytesRecibidos = recibir_mensaje(sktActivo, buffer, largoMaximo-1); 
         
            //len = atoi(small_buf);
            //printf("Echo %i bytes\n", len);
            buffer[bytesRecibidos] = "\0";
            printf("%s\n", buffer);

            templateVector = cargar_template(template, temperaturaBuffer);
            if (templateVector == NULL){
                shutdown(sktActivo, SHUT_RDWR);
                close(sktActivo);
                free(sktActivo);
                break;  
                }
            
            fprintf(stderr, "Error al cargar el template.\n");
            return 1;
            }
            size_t largoTemplate = vector_obtener_tamanio(templateVector);
            for (int i = 0; i < largoTemplate; ++i) {
                vector_obtener(templateVector, i, &buffer[i]);
            }
            vector_destruir(templateVector);

            int bytesEnviados;
            bytesEnviados = enviar_mensaje(sktActivo, buffer, largoTemplate); 

            shutdown(sktActivo, SHUT_RDWR);
            close(sktActivo);
    }
    
   
    shutdown(sktPasivo, SHUT_RDWR);
    close(sktPasivo);

    if (esLaAceptacionDelSocketValido) {
       return 1;
    } else { 
        return 0;
    }
}
