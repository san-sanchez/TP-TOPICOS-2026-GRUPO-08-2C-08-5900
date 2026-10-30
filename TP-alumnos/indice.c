#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "indice.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    toma memoria para nmemb elementos e inicializa la estructura
                vacia.
Parametros:     indice: TDA indice.
                nmemb: cantidad inicial de elementos (CANTIDAD_ELEMENTOS).
                tamanyo: espacio en bytes ocupado por cada elemento.
Retorno:        OK si la operacion fue exitosa y ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int indice_crear_ALU(t_indice *indice, size_t nmemb, size_t tamanyo)
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */
    if (indice == NULL || nmemb == 0 || tamanyo == 0)
        return ERROR;

    indice->vindice = malloc(nmemb * tamanyo);

    if (indice->vindice == NULL)
        return ERROR;

    indice->cantidad_elementos_actual = 0;
    indice->cantidad_elementos_maxima = nmemb;

    return OK;

    //return indice_crear(indice, nmemb, tamanyo);
}

/******************************************************************************
Descripcion:    redimensiona el arreglo interno del indice con realloc. La
                nueva capacidad es la actual multiplicada por INCREMENTO (30%
                mas); la calcula la propia funcion.
Parametros:     indice: TDA indice.
                tamanyo: espacio en bytes ocupado por cada elemento.
Retorno:        OK si la operacion fue exitosa y ERROR en caso contrario.
Observaciones:  Si el producto no aumenta la capacidad (arreglos muy chicos)
                se garantiza al menos un elemento mas.
******************************************************************************/
int indice_redimensionar_ALU(t_indice *indice, size_t tamanyo)
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */

    unsigned max;
    unsigned nueva;
    void *aux;

    if (indice == NULL || tamanyo == 0)
        return ERROR;

    max = indice->cantidad_elementos_maxima;
    nueva = (unsigned)(max * INCREMENTO);

    if (nueva <= max)
        nueva = max + 1;

    aux = realloc(indice->vindice, nueva * tamanyo);

    if (aux == NULL)
        return ERROR;

    indice->vindice = aux;
    indice->cantidad_elementos_maxima = nueva;

    return OK;
    //return indice_redimensionar(indice, tamanyo);
}

/******************************************************************************
Descripcion:    inserta un elemento manteniendo el orden segun la clave. Si el
                arreglo esta lleno invoca indice_redimensionar.
Parametros:     indice: TDA indice.
                registro: elemento a insertar.
                tamanyo: espacio en bytes ocupado por el elemento.
                cmp: funcion de comparacion provista.
Retorno:        OK si la operacion fue exitosa y ERROR en caso contrario
                (clave duplicada o falta de memoria).
Observaciones:  -
******************************************************************************/
int indice_insertar_ALU(t_indice *indice, const void *registro, size_t tamanyo,
                        int (*cmp)(const void *, const void *))
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */
    unsigned i = 0, j;
    unsigned ce;
    char *base;

    if (indice == NULL || registro == NULL ||
            cmp == NULL || tamanyo == 0)
        return ERROR;

    ce = indice->cantidad_elementos_actual;
    base = (char *)indice->vindice;

    while (i < ce &&
            cmp(base + i * tamanyo, registro) < 0)
        i++;

    if (i < ce &&
            cmp(base + i * tamanyo, registro) == 0)
        return ERROR;

    if (indice_lleno_ALU(indice) &&
            !indice_redimensionar_ALU(indice, tamanyo))
        return ERROR;

    base = (char *)indice->vindice;

    for (j = ce; j > i; j--)
        memcpy(base + j * tamanyo,
               base + (j - 1) * tamanyo,
               tamanyo);

    memcpy(base + i * tamanyo, registro, tamanyo);

    indice->cantidad_elementos_actual++;

    return OK;
    //return indice_insertar(indice, registro, tamanyo, cmp);
}

/******************************************************************************
Descripcion:    elimina el elemento cuya clave coincide con la de registro,
                desplazando los siguientes.
Parametros:     indice: TDA indice.
                registro: elemento a eliminar (solo importa su clave).
                tamanyo: espacio en bytes ocupado por el elemento.
                cmp: funcion de comparacion provista.
Retorno:        OK si la operacion fue exitosa y ERROR si la clave no existe.
Observaciones:  -
******************************************************************************/
int indice_eliminar_ALU(t_indice *indice, const void *registro, size_t tamanyo,
                        int (*cmp)(const void *, const void *))
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */
    unsigned i, j;
    char *base;

    if (indice == NULL || registro == NULL || cmp == NULL ||
            tamanyo == 0 || indice->vindice == NULL)
        return ERROR;

    base = (char *)indice->vindice;

    i = 0;

    while (i < indice->cantidad_elementos_actual &&
            cmp(registro, base + i * tamanyo) != 0)
        i++;

    if (i == indice->cantidad_elementos_actual)
        return ERROR;

    for (j = i; j < indice->cantidad_elementos_actual - 1; j++)
        memcpy(base + j * tamanyo,
               base + (j + 1) * tamanyo,
               tamanyo);

    indice->cantidad_elementos_actual--;

    return OK;


    //return indice_eliminar(indice, registro, tamanyo, cmp);
}

/******************************************************************************
Descripcion:    busca la clave de registro en el indice. Si existe copia el
                elemento completo (nro_reg incluido) en registro.
Parametros:     indice: TDA indice.
                registro: elemento a buscar (entrada/salida).
                tamanyo: espacio en bytes ocupado por el elemento.
                cmp: funcion de comparacion provista.
Retorno:        NO_EXISTE si la clave no existe; en caso contrario la posicion
                que ocupa dentro del arreglo.
Observaciones:  La comparacion entre claves se realiza exclusivamente mediante
                cmp.
******************************************************************************/
int indice_buscar_ALU(const t_indice *indice, void *registro, size_t tamanyo,
                      int (*cmp)(const void *, const void *))
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */

    int li, ls, medio;
    char *base;

    if (indice == NULL || registro == NULL || cmp == NULL ||
            tamanyo == 0 || indice->vindice == NULL)
        return NO_EXISTE;

    li = 0;
    ls = (int)indice->cantidad_elementos_actual - 1;
    base = (char *)indice->vindice;

    while (li <= ls)
    {
        medio = (li + ls) / 2;

        if (cmp(registro, base + medio * tamanyo) == 0)
        {
            memcpy(registro, base + medio * tamanyo, tamanyo);
            return medio;
        }

        if (cmp(registro, base + medio * tamanyo) < 0)
            ls = medio - 1;
        else
            li = medio + 1;
    }

    return NO_EXISTE;


    //return indice_buscar(indice, registro, tamanyo, cmp);
}

/******************************************************************************
Descripcion:    determina si el indice contiene 0 (cero) elementos.
Parametros:     indice: TDA indice.
Retorno:        OK si esta vacio, ERROR si no lo esta.
Observaciones:  -
******************************************************************************/
int indice_vacio_ALU(const t_indice *indice)
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */
    if (indice == NULL)
        return ERROR;

    return indice->cantidad_elementos_actual == 0 ? OK : ERROR;

    //return indice_vacio(indice);
}

/******************************************************************************
Descripcion:    determina si el indice alcanzo su capacidad maxima actual.
Parametros:     indice: TDA indice.
Retorno:        OK si esta lleno, ERROR si no lo esta.
Observaciones:  -
******************************************************************************/
int indice_lleno_ALU(const t_indice *indice)
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */
    if (indice == NULL)
        return ERROR;

    return indice->cantidad_elementos_actual ==
           indice->cantidad_elementos_maxima ? OK : ERROR;

    //return indice_lleno(indice);
}

/******************************************************************************
Descripcion:    deja el indice vacio sin liberar la memoria del arreglo.
Parametros:     indice: TDA indice.
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void indice_vaciar_ALU(t_indice *indice)
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */
    if (indice == NULL)
        return;

    indice->cantidad_elementos_actual = 0;

    //indice_vaciar(indice);
}

/******************************************************************************
Descripcion:    libera toda la memoria dinamica del indice.
Parametros:     indice: TDA indice.
Retorno:        No posee.
Observaciones:  Luego de invocarla el indice no puede usarse sin volver a
                llamar a indice_crear.
******************************************************************************/
void indice_destruir_ALU(t_indice *indice)
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */
    if (indice == NULL)
        return;

    free(indice->vindice);

    indice->vindice = NULL;
    indice->cantidad_elementos_actual = 0;
    indice->cantidad_elementos_maxima = 0;
    //indice_destruir(indice);
}

/******************************************************************************
Descripcion:    construye el indice a partir de un archivo binario de
                registros. Recorre el archivo secuencialmente y por cada
                registro activo inserta un t_reg_indice (nro_reg = posicion en
                el archivo, id = clave obtenida con obtener_clave).
Parametros:     path: ruta al archivo binario.
                indice: TDA indice (ya creado; se vacia antes de cargar).
                tamanyo_reg: tamanyo en bytes de cada registro del archivo.
                obtener_clave: funcion que devuelve la clave (long) de un
                registro.
                es_activo: funcion que devuelve distinto de 0 si el registro
                tiene estado 'A'.
                cmp: funcion de comparacion provista.
Retorno:        OK si la operacion fue exitosa y ERROR en caso contrario.
Observaciones:  Los registros con estado 'B' se omiten. El archivo no necesita
                estar ordenado: el orden lo garantiza indice_insertar.
******************************************************************************/
int indice_cargar_ALU(const char *path, t_indice *indice, size_t tamanyo_reg,
                      long (*obtener_clave)(const void *),
                      int (*es_activo)(const void *),
                      int (*cmp)(const void *, const void *))
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */

    FILE *arch;
    void *reg;
    size_t leidos;
    unsigned long long nro_reg = 0;
    t_reg_indice reg_indice;
    int resultado = OK;

    if (path == NULL || indice == NULL || tamanyo_reg == 0 ||
            obtener_clave == NULL || es_activo == NULL || cmp == NULL ||
            indice->vindice == NULL)
        return ERROR;

    arch = fopen(path, "rb");

    if (arch == NULL)
        return ERROR;

    reg = malloc(tamanyo_reg);

    if (reg == NULL)
    {
        fclose(arch);
        return ERROR;
    }

    indice_vaciar_ALU(indice);

    while ((leidos = fread(reg, 1, tamanyo_reg, arch)) > 0)
    {
        if (leidos != tamanyo_reg || nro_reg > UINT_MAX)
        {
            resultado = ERROR;
            break;
        }

        if (es_activo(reg))
        {
            reg_indice.nro_reg = (unsigned)nro_reg;
            reg_indice.id = obtener_clave(reg);

            if (indice_insertar_ALU(indice, &reg_indice,
                                    sizeof(reg_indice), cmp) != OK)
            {
                resultado = ERROR;
                break;
            }
        }

        nro_reg++;
    }

    if (ferror(arch))
        resultado = ERROR;

    free(reg);

    if (fclose(arch) != 0)
        resultado = ERROR;

    if (resultado != OK)
        indice_vaciar_ALU(indice);

    return resultado;
    //return indice_cargar(path, indice, tamanyo_reg, obtener_clave, es_activo, cmp);
}
