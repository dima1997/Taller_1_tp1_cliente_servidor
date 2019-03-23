// Cliente

/*
El cliente debe conectarse a un ​ host ​ y un ​ puerto ​ , utilizando el protocolo ​ TCP . ​ Si recibió el ​ path
de un archivo de entrada enviará su contenido al servidor y al finalizar cerrará el canal de escritura del socket. 
Si no recibe el ​ path ​ se utiliza la entrada estándar como fuente de datos.
Luego de enviar el petitorio, el cliente recibe la respuesta del servidor y la escribe en pantalla.
Una vez que termina de recibir datos, se cierra el socket, se liberan todos los recursos y se
termina la aplicación con 0 como código de salida.


Formato de un pedido HTTP
Un pedido HTTP del cliente (​ request ) ​ está formada por lineas de texto separadas por un
caracter de retorno y otro de salto de línea (​ CRLF ​ ). Estas líneas forman 3 bloques
●
Método del request y sus parámetros:
Se envía la acción deseada (GET, PUT, POST, etc), y sus parámetros, separados por
espacios. Sus parámetros son: el recurso al que se quiere acceder, y la versión del protocolo
HTTP. Para este proyecto se utilizará el protocolo HTTP/1.1
Ejemplo:
GET /template.html HTTP/1.1
●
Cabecera (header):
Le provee información de la conexión al servidor. Está compuesto por lineas con el formato
clave: valor​ . El único parámetro obligatorio para el protocolo HTTP/1.1 es el de Host, debido
a que se utiliza para discernir hosts virtuales.
En este proyecto se utilizará el parámetro User-Agent para identificar la aplicación del cliente.
Ejemplo:
Host: localhost:8080
User-Agent: netcat
●
Cuerpo (body):
Este bloque se utiliza para mandar datos en los pedidos principalmente de tipo ​ POST . ​ La
longitud de este bloque se especifica en el header, con el parámetro ​ Content-Length ​ .
No​ se utilizará en este proyecto.
El cuerpo y cabecera finalizan con una línea en blanco. Una vez que el cliente envía el petitorio,
cierra el canal de escritura y espera la respuesta del servidor.


El cliente se ejecuta con la siguiente línea de comandos:
./client ​ <host> <port> ​ [ ​ <filename>​ ]
Donde ​ <host>​ y ​ <port>​ son la dirección IPv4 o ​ hostname ​ y el puerto o servicio donde el
servidor estará escuchando la conexión TCP.
<filename>​ es un argumento opcional que indica el ​ archivo de texto ​ con el requ


Si el cliente tiene un número de parámetros incorrecto, se imprime por salida de error estándar
lo siguiente:
Uso:
./client <direccion> <puerto> [<input>]


El cliente, en caso de no recibir por parámetro el nombre del archivo de entrada, lee el
contenido del request mediante la entrada estándar.
Luego de enviar el ​ request HTTP ​ , espera la respuesta del servidor y la imprime por salida
estándar


Tanto el cliente como el servidor deben retornar ​ 0 ​ si todo salió correctamente o ​ 1 ​ en caso
contrario.
*/


/*
El cliente se ejecuta con la siguiente línea de comandos:
./client ​ <host> <port> ​ [ ​ <filename>​ ]
Donde ​ <host>​ y ​ <port>​ son la dirección IPv4 o ​ hostname ​ y el puerto o servicio donde el
servidor estará escuchando la conexión TCP.
<filename>​ es un argumento opcional que indica el ​ archivo de texto ​ con el requ
*/

//1ero: Abrir archivo e imprimirlo en pantalla
#define _POSIX_C_SOURCE 200809L 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vector_dinamico_char.h"

/*
Pre: recibe un archivo (FILE*)ya abierto de donde leer la peticion.
Post: devuelve un puntero a un vector dinamico de chars (vector_t*) 
almacenado de memoria dinamica, que puede ser liberada por medio de
vector_destruir(<vector>).
El vector tiene un tamanio igual a la cantidad de caracteres que 
almacena (ni mas, ni menos).
Si hubo algun problema al cargar la peticion, devuelve NULL.
*/
vector_t *cargar_peticion(FILE* archivoPeticion){
    size_t largoPeticion = 200;
    vector_t *peticion = vector_crear(largoPeticion);

    char caracter;
    char caracterAnterior = '\0';
    size_t i = 0; 
    while ((caracter = getc(archivoPeticion)) != -1){ //eof
        if (caracter == '\n') {
            if (caracterAnterior == '\n' || caracterAnterior == '\0'){
                //Una linea vacia marca el final de la peticion
                //Para cuando archivoPeticion es stdin
                break;
            }
        }

        if (i >= largoPeticion){
            size_t nuevoLargo = largoPeticion*2;
            bool seRedimensiono = vector_redimensionar(peticion, nuevoLargo);
            if (!seRedimensiono) {
                vector_destruir(peticion);
                return NULL;
            }
            largoPeticion = nuevoLargo;
        }
        vector_guardar(peticion, i, caracter);
        ++i;
        caracterAnterior = caracter;
    }
    size_t largoFinal = i;
    bool seRedimensiono = vector_redimensionar(peticion, largoFinal);
    if (!seRedimensiono) {
        vector_destruir(peticion);
        return NULL;
        }
    return peticion;
    }
/*
Pre: Recibe un puntero a vector dinamico de chars (vector_t).
Post: imprime por salida estandard (stdout) el contenido del 
vector.
*/
void imprimir_vector(vector_t *vector){
    size_t largoVector = vector_obtener_tamanio(vector);
    size_t i = 0;
    char caracter;
    while (i < largoVector) {
        vector_obtener(vector, i, &caracter);
        fprintf(stdout,"%c", caracter);
        ++i;
    }
}

int main(int argc, const char* argv[]) {
    if (argc < 3 || argc > 4){
        /*
        Si el cliente tiene un número de parámetros incorrecto, se imprime por salida de error estándar
        lo siguiente:
        Uso:
        ./client <direccion> <puerto> [<input>]
        */
        fprintf(stderr,"Uso:\n./client <direccion> <puerto> [<input>]\n");
        return 1;
    }
    if (argc == 3) {
        /*
        Si no recibe el ​ path ​ se utiliza la entrada estándar como fuente de datos.
        */
        vector_t *peticionVector;
        peticionVector = cargar_peticion(stdin);
            if (peticionVector == NULL){
            return 1;
        }
        imprimir_vector(peticionVector);
        vector_destruir(peticionVector);
        return 0;
    }

    // Por descarte argc es igual a 4, entonces se recibio un path al archivo 
    // con la peticion
    FILE *archivoPeticion;
    const char *nombreArchivoPeticion = argv[3]; 
    if ((archivoPeticion = fopen(nombreArchivoPeticion, "rt")) == NULL) {
        fprintf(stderr, "Archivo no encontrado.\n");
        return 1;
    }
    vector_t *peticionVector;
    peticionVector = cargar_peticion(archivoPeticion);
    fclose(archivoPeticion);
    if (peticionVector == NULL){
        return 1;
    }
    imprimir_vector(peticionVector);
    vector_destruir(peticionVector);
    return 0;
}
