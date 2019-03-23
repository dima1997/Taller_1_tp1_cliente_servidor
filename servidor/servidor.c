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
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include <arpa/inet.h>
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

int main(int argc, const char *argv[]){
    if (argc != 4) {
        fprintf(stderr, "Uso:\n./server <puerto> <input> [<template>]\n");
        return 1;
    }
    const char *sensorBinario = argv[2];
    const char *template = argv[3];
    FILE *archivoSensor; 
    if ((archivoSensor = fopen(sensorBinario, "rb")) == NULL) {
        fprintf(stderr, "Archivo binario no encontrado.\n");
        return 1;
    }
    short int *numeroLeido = malloc(sizeof(short int)); 
    while (fread(numeroLeido, sizeof(short int),1,archivoSensor)) {
        double temperaturaSensada = (ntohs(*numeroLeido)-2000)/100;
        char temperaturaBuffer[100];
        size_t largoBuffer = sizeof(temperaturaBuffer);
        snprintf(temperaturaBuffer, largoBuffer, "%.2f", temperaturaSensada);
        vector_t *templateVector;
        templateVector = cargar_template(template, temperaturaBuffer);
        if (templateVector == NULL){
            free(numeroLeido);
            fclose(archivoSensor);
            fprintf(stderr, "Error al cargar el template.\n");
            return 1;
        }
        vector_imprimir(templateVector);
        vector_destruir(templateVector);
    }
    free(numeroLeido);
    fclose(archivoSensor);
    return 0;
}
