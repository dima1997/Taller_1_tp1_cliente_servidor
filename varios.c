/*
    size_t largoDato = strlen(dato);

    FILE *archivoTemplate; 
    if ((archivoTemplate = fopen(ruta_template, "rt")) == NULL) {
        fprintf(stderr, "Archivo template no encontrado.\n");
        return 1;
    }
    char* linea = NULL; 
    size_t capacidadLinea = 0; 
    ssize_t cantidadCaracteres;

    largoVector = 200;
    	
    vector_t *templateVector = vector_crear(largoVector);
    	
    if (templateVector == NULL){
   		free_strv(lineaSplit);
   		free(linea);
       	fclose(archivoTemplate);
       	printf("Error al cargar template\n");
       	return 1;
   	}
    	
   	size_t posicionVector = 0;
    size_t factorRedimensionar = 2;

    while((cantidadCaracteres = getline(&linea,&capacidadLinea,archivo)) > 0){
        char** lineaSplit = split(linea, "{{datos}}");
        if (lineaSplit == NULL){
        	free(linea);
        	fclose(archivoTemplate);
        	printf("Error al cargar template\n");
        	return NULL;
        }

        if (posicionVector + cantidadCaracteres + largoDato >= largoVector) {
            size_t nuevoLargoVector = (posicionVector + cantidadCaracteres + largoDato) * factorRedimensionar;
            bool seRedimensiono = vector_redimensionar(templateVector, nuevoLargoVector);
            if (!seRedimensiono){
                vector_destruir(templateVector);
                free_strv(lineaSplit);
                free(linea);
                fclose(archivoTemplate);
                return NULL;
            }
            largoVector = nuevoLargoVector;
        }

   		for (int j = 0; lineaSplit[0][j] != "\0"; ++j){
    		vector_guardar(templateVector, posicionVector, &lineaSplit[0][j]);
    		++posicionVector;
    	}
    	
    	for (int i = 1; lineaSplit[i] != NULL, ++i){
    	
    		for (int j = 0; datos[j] != "\0", ++j){
    			vector_guardar(templateVector, posicionVector, &datos[j]);
    			++posicionVector;
    		}
    	
    		for (int j = 0; lineaSplit[i][j] != "\0"; ++j){
    			vector_guardar(templateVector, posicionVector, &lineaSplit[i][j]);
    			++posicionVector;
    		}
    	}
        free_strv(lineaSplit);
    }
    free(linea);
    fclose(archivoTemplate)
    size_t largoFinalVector = posicionVector;
    bool seRedimensiono = vector_redimensionar(templateVector, nuevoLargoVector);
    if (!seRedimensiono){
        vector_destruir(templateVector);   
        return NULL;
    }
    return templateVector;
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
/*
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
*/

/*
Pre: recibe la ruta de un archivo binario compuesto por numeros de 16 bits en
formato big-endian.
Post: imprime los cada numero del archivo intepretado en forma decimal.
*/
/*
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
*/
/*
Pre: recibe un archivo (FILE*)ya abierto de donde leer la peticion.
Post: devuelve un puntero a un vector dinamico de chars (vector_t*) 
almacenado de memoria dinamica, que puede ser liberada por medio de
vector_destruir(<vector>).
El vector tiene un tamanio igual a la cantidad de caracteres que 
almacena (ni mas, ni menos).
Si hubo algun problema al cargar la peticion, devuelve NULL.
*/
/*vector_t *cargar_peticion(FILE* archivoPeticion){
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
*/
/*
typedef struct vector {
    size_t tam;
    char* datos;
} vector_t;
*/
/*
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
 * Destruye el vector.
 * Pre: recibe un vector (puntero de tipo vector_t) ya creado. 
 * Post: se eliminaron todos los elementos del vector.
 *
void vector_destruir(vector_t* vector) {
    free(vector->datos);
    free(vector);
}

// esta_entre()
 * Pre: recibe tres enteros (int).
 * Post: Devuelve true si el primero de esos enteros esta en el rango que 
 * forman los otros dos, sin contar al ultimo.
 *
bool esta_entre(size_t numero, size_t inicio, size_t final) {
    if ((numero < inicio) || (numero >= final)) {
        return false;
    }
    return true;
}

// vector_obtener()
 * Almacena en valor el dato guardado en la posición pos del vector.
 * Pre: recibe un vector (puntero de tipo vector_t) ya creado, pos (numero de 
 * tipo size_t) correspondiente a una posicion en el rango del vector, y valor 
 * (puntero de tipo char).
 * Post: se almacenó en valor el dato en la posición pos. Devuelve false si la
 * posición es inválida (fuera del rango del vector, que va de 0 a tamaño-1)
 *
bool vector_obtener(vector_t* vector, size_t pos, char* valor) {
    if (!(esta_entre(pos, 0, (vector_obtener_tamanio(vector)) ) )) {
        return false;
    }
    *valor = *(vector->datos + pos);
    return true;
}

// vector_guardar()
 * Almacena el valor en la posición pos.
 * Pre: recibe un vector (puntero de tipo vector_t) ya creado, una posicion pos 
 * (numero de tipo size_t) en el rango del vector, y un valor (char).
 * Post: se almacenó el valor en la posición pos. Devuelve false si la posición
 * es inválida (fuera del rango del vector, que va de 0 a tamaño-1) y true si
 * se guardó el valor con éxito.
 *
bool vector_guardar(vector_t* vector, size_t pos, char valor) {
    if (!(esta_entre(pos, 0, (vector_obtener_tamanio(vector)) ) )) {
        return false;
    }
    vector->datos[pos] = valor;
    return true;
}

// vector_obtener_tamanio
 * Devuelve el tamaño del vector.
 * Pre: recibe un vector (puntero de tipo vector_t) ya creado.
 *
size_t vector_obtener_tamanio(vector_t* vector) {
    return vector->tam;
}

//vector_imprimir
 * Pre: Recibe un puntero a vector dinamico de chars (vector_t).
 * Post: imprime por salida estandard (stdout) el contenido del
 * vector.
 *
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
*/
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
/*
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
*/
/*
Pre: recibe dos cadenas de caracteres terminadas en \0 (char *).
Post: devuelve un nueva cadena, que es la concatenacion de las dos 
cadenas recibidas, anteponiendo la primera cadena antes de la 
segunda. La nueva cadena termina en \0. Si ocurrio algun problema 
devuelve NULL.
Queda a responsabilidad del usuario liberar la memoria reservada,
por medio de la funcion free(). 
*/
/*
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

*/

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
/*
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
*/