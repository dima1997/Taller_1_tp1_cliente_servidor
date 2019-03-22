#include "vector_dinamico_char.h"

// Funciones implementadas por la catedra.

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


