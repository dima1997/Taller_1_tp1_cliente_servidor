#include "common.h"
#define ESPACIOS_EXTRAS 1
/*
Pre: Recibe un socket ya conectado: skt (int), y un puntero al 
mensaje que a enviar (char *).
Post: Devuelve true si logro enviar toda la peticion, false en caso 
contrario, dado un error en el socket o si el socket remoto fue cerrado.
*/
bool enviar_mensaje(int skt, char *mensaje, size_t largoMensaje) {
    int estado = 0;
    bool hayErrorDeSocket = false;
    bool estaSocketRemotoCerrado = false;
    int bytesEnviados = 0;
    
    while (bytesEnviados < largoMensaje && hayErrorDeSocket == false && estaSocketRemotoCerrado == false) {
        estado = send(skt, &mensaje[bytesEnviados], largoMensaje - bytesEnviados, MSG_NOSIGNAL);
        if (estado < 0) { 
            printf("Error: %s\n", strerror(errno));
            hayErrorDeSocket = true;
        }
        else if (estado == 0) { 
            estaSocketRemotoCerrado = true;
        }
        else {
            bytesEnviados += estado;
        }
    }
    
    return (estaSocketRemotoCerrado || hayErrorDeSocket);
}

/*
Pre: recibe un socket ya conectado: skt (int), y
recibe un puntero al buffer donde guardar el mensaje
recibido.
Post: Devuelve true si logro recibir todo el mensaje,
false en caso contrario, dado algun error de socket.
*/
bool recibir_mensaje(int skt, char *mensaje, size_t largoMaximoMensaje) {
    int estado = 0;
    bool hayErrorDeSocket = false;
    bool estaSocketRemotoCerrado = false;
    int bytesRecibidos = 0;

    while (hayErrorDeSocket == false && estaSocketRemotoCerrado == false) {
        estado = recv(skt, &mensaje[bytesRecibidos], largoMaximoMensaje - bytesRecibidos - 1, MSG_NOSIGNAL);

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
            mensaje[bytesRecibidos] = 0;
            //printf("%s", mensaje);
            bytesRecibidos = 0; 
        }
    }
    return hayErrorDeSocket;
}

/* Pre: recibe una arreglo dinamico de punteros a char, la capacidad del
 * arreglo (cantidad de elementos que puede guardar), la posicion
 * del arreglo donde se desea guardar en forma de cadena un arreglo de char estatico, 
 * y la cantidad de elementos de este ultimo arreglo.
 * Post: transcribe y guarda el la cadena (de formato de arreglo) a una cadena de 
 * memoria dinamica, y la guarda en el arreglo de memoria dinamica recibido, si es 
 * posible. 
 * Devuelve true si pudo completar la operacion, false en caso contrario.
 */
bool split_agregar_str_al_arr(char** arr,size_t cap_arr, size_t pos_arr, char buffer[], size_t len_buff){
    if (pos_arr >= cap_arr) return false; 

    buffer[len_buff] = '\0'; 

    char* str_dinamico = malloc(sizeof(char)*(len_buff+1));
    if (!str_dinamico) return false;

    strcpy(str_dinamico, buffer); 

    arr[pos_arr] = str_dinamico;
    return true;
}
/*
 * Devuelve en un arreglo dinámico terminado en NULL con todos los subsegmentos
 * de ‘str’ separados por el carácter ‘sep’. Tanto el arreglo devuelto como las
 * cadenas que contiene son allocadas dinámicamente.
 *
 * Quien llama a la función toma responsabilidad de la memoria dinámica del
 * arreglo devuelto. La función devuelve NULL si falló alguna llamada a
 * malloc(), o si ‘sep’ es '\0'.
 */

char** split(const char* str, char sep){

    size_t i_str = 0;
    size_t i_arr_str = 0;
    size_t i_str_aux = 0;
    
    char str_aux[1000];
    bool ok = true;
    
    size_t cant_palabras = 1 + ESPACIOS_EXTRAS;
    //
    while (str[i_str] != '\0' ) {
        if (str[i_str] == sep) cant_palabras += 1;
        i_str++;
    }
    char** arr_str = malloc(sizeof(char*)*cant_palabras); 
    if (!arr_str) return NULL;

    i_str = 0;
    //|| str[i_str] != '\n'
    while (str[i_str] != '\0' ) {
        str_aux[i_str_aux] = str[i_str];
        if (str[i_str] == sep) {
            str_aux[i_str_aux] = '\0';
            ok = split_agregar_str_al_arr(arr_str,cant_palabras, i_arr_str, str_aux, i_str_aux);
            if (!ok) {
                free(arr_str);
                return NULL;
            }
            i_arr_str ++;
            i_str ++;
            i_str_aux = 0;
            continue;
        }
        i_str++;
        i_str_aux++;
    }
    str_aux[i_str_aux] = '\0';
    ok = split_agregar_str_al_arr(arr_str, cant_palabras, i_arr_str, str_aux, i_str_aux);
    if (!ok) {
        free(arr_str);
        return NULL;
    }
    i_arr_str++;
    arr_str[i_arr_str] = NULL;

    return arr_str;
}


/*
 * Libera un arreglo dinámico de cadenas, y todas las cadenas que contiene.
 */
void free_strv(char* strv[]){
    size_t i = 0;
    while (strv[i] != NULL) {
        free(strv[i]);
        i++;
    }
    free(strv);
}

/*
Pre: recibe un archivo de texto ya abierto.
Post: Devuelve una cadena de caracteres (terminada en \0) que 
contiene todos los carateres del archivo desde el inicio hasta
una linea vacia o fin de archivo; o NULL si hubo algun error 
durante la carga.
Queda a respondabilidad del usuario liberar la memoria reservada 
para la cadena, por medio de free
*/
char *cargar_archivo(FILE *archivo){
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
                //Para cuando archivoPeticion es stdin
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