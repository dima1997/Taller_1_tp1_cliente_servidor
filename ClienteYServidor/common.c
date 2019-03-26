#include "common.h"
#define ESPACIOS_EXTRAS 1

// Crea un vector de tamaño tam
// Post: vector es una vector vacío de tamaño tam
vector_t* vector_crear(size_t tam) {
    vector_t* vector = malloc(sizeof(vector_t));

    if (vector == NULL) {
        return NULL;
    }
    vector->datos = malloc(tam * sizeof(char));

    if (tam > 0 && vector->datos == NULL) {
        free(vector);
        return NULL;
    }
    vector->tam = tam;
    return vector;
}

// Cambia el tamaño del vector
// Pre: el vector fue creado
// Post: el vector cambió de tamaño a nuevo_tam y devuelve true
// o el vector queda intacto y devuelve false si no se pudo cambiar el tamaño
// a nuevo_tam
bool vector_redimensionar(vector_t* vector, size_t tam_nuevo) {
    char* datos_nuevo = realloc(vector->datos, tam_nuevo * sizeof(char));

    // Cuando tam_nuevo es 0, es correcto si se devuelve NULL.
    // En toda otra situación significa que falló el realloc.
    if (tam_nuevo > 0 && datos_nuevo == NULL) {
        return false;
    }

    vector->datos = datos_nuevo;
    vector->tam = tam_nuevo;
    return true;
}


// Funciones del alumno.

// vector_destruir()
/* Destruye el vector.
 * Pre: recibe un vector (puntero de tipo vector_t) ya creado. 
 * Post: se eliminaron todos los elementos del vector.
 */
void vector_destruir(vector_t* vector) {
    free(vector->datos);
    free(vector);
}

// esta_entre()
/* Pre: recibe tres enteros (int).
 * Post: Devuelve true si el primero de esos enteros esta en el rango que 
 * forman los otros dos, sin contar al ultimo.
 */
bool esta_entre(size_t numero, size_t inicio, size_t final) {
    if ((numero < inicio) || (numero >= final)) {
        return false;
    }
    return true;
}

// vector_obtener()
/* Almacena en valor el dato guardado en la posición pos del vector.
 * Pre: recibe un vector (puntero de tipo vector_t) ya creado, pos (numero de 
 * tipo size_t) correspondiente a una posicion en el rango del vector, y valor 
 * (puntero de tipo char).
 * Post: se almacenó en valor el dato en la posición pos. Devuelve false si la
 * posición es inválida (fuera del rango del vector, que va de 0 a tamaño-1)
 */
bool vector_obtener(vector_t* vector, size_t pos, char* valor) {
    if (!(esta_entre(pos, 0, (vector_obtener_tamanio(vector)) ) )) {
        return false;
    }
    *valor = *(vector->datos + pos);
    return true;
}

// vector_guardar()
/* Almacena el valor en la posición pos.
 * Pre: recibe un vector (puntero de tipo vector_t) ya creado, una posicion pos 
 * (numero de tipo size_t) en el rango del vector, y un valor (char).
 * Post: se almacenó el valor en la posición pos. Devuelve false si la posición
 * es inválida (fuera del rango del vector, que va de 0 a tamaño-1) y true si
 * se guardó el valor con éxito.
 */
bool vector_guardar(vector_t* vector, size_t pos, char valor) {
    if (!(esta_entre(pos, 0, (vector_obtener_tamanio(vector)) ) )) {
        return false;
    }
    vector->datos[pos] = valor;
    return true;
}

// vector_obtener_tamanio
/* Devuelve el tamaño del vector.
 * Pre: recibe un vector (puntero de tipo vector_t) ya creado.
 */
size_t vector_obtener_tamanio(vector_t* vector) {
    return vector->tam;
}

//vector_imprimir
/* Pre: Recibe un puntero a vector dinamico de chars (vector_t).
 * Post: imprime por salida estandard (stdout) el contenido del
 * vector.
 */
void vector_imprimir(vector_t *vector){
    size_t largoVector = vector_obtener_tamanio(vector);
    size_t i = 0;
    char caracter;
    while (i < largoVector) {
        vector_obtener(vector, i, &caracter);
        fprintf(stdout,"%c", caracter);
        ++i;
    }
}

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