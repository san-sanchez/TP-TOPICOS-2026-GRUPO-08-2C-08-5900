#include "cadena.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

int normalizar_segmento_ALU(const char *origen, size_t longitud,
                            char *destino, size_t tam_destino)
{
    size_t i, j = 0;
    int nueva_palabra = 1;

    if (origen == NULL || destino == NULL || tam_destino == 0)
        return 0;

    destino[0] = '\0';

    for (i = 0; i < longitud; i++)
    {
        unsigned char c = (unsigned char)origen[i];

        if (isspace(c))
        {
            if (j > 0 && destino[j - 1] != ' ')
            {
                if (j + 1 >= tam_destino)
                {
                    destino[0] = '\0';
                    return 0;
                }

                destino[j++] = ' ';
            }

            nueva_palabra = 1;
        }
        else
        {
            if (j + 1 >= tam_destino)
            {
                destino[0] = '\0';
                return 0;
            }

            destino[j++] = nueva_palabra ?
                           (char)toupper(c) : (char)tolower(c);

            nueva_palabra = (c == '\'');
        }
    }

    if (j > 0 && destino[j - 1] == ' ')
        j--;

    destino[j] = '\0';

    return j > 0;
}


int normalizar_apyn_ALU(const char *origen, char *destino,
                        size_t tam_destino)
{
    char entrada[TAM_CADENA_AUX];
    char apellido[TAM_CADENA_AUX];
    char nombres[TAM_CADENA_AUX];
    char completo[TAM_CADENA_AUX];

    const char *coma;
    const char *segunda_coma;
    const char *separador;

    size_t longitud;
    size_t longitud_apellido;
    size_t longitud_nombres;

    if (destino == NULL || tam_destino == 0)
        return 0;

    if (origen == NULL)
    {
        destino[0] = '\0';
        return 0;
    }

    /* Permite que origen y destino sean el mismo arreglo. */
    longitud = strlen(origen);

    if (longitud >= sizeof(entrada))
    {
        destino[0] = '\0';
        return 0;
    }

    memcpy(entrada, origen, longitud + 1);

    origen = entrada;
    destino[0] = '\0';

    coma = strchr(origen, ',');

    if (coma != NULL)
    {
        segunda_coma = strchr(coma + 1, ',');

        /* Solo se admite una coma separadora. */
        if (segunda_coma != NULL)
            return 0;

        if (!normalizar_segmento_ALU(
                    origen,
                    (size_t)(coma - origen),
                    apellido,
                    sizeof(apellido)) ||

                !normalizar_segmento_ALU(
                    coma + 1,
                    longitud - (size_t)(coma + 1 - origen),
                    nombres,
                    sizeof(nombres)))
            return 0;
    }
    else
    {
        /* Si no hay coma, se agrega después de la primera palabra. */
        if (!normalizar_segmento_ALU(
                    origen,
                    longitud,
                    completo,
                    sizeof(completo)))
            return 0;

        separador = strchr(completo, ' ');

        if (separador == NULL)
            return 0;

        longitud_apellido = (size_t)(separador - completo);

        memcpy(apellido, completo, longitud_apellido);
        apellido[longitud_apellido] = '\0';

        while (*separador == ' ')
            separador++;

        if (*separador == '\0')
            return 0;

        if (strlen(separador) >= sizeof(nombres))
            return 0;

        strcpy(nombres, separador);
    }

    longitud_apellido = strlen(apellido);
    longitud_nombres = strlen(nombres);

    /*
     * apellido + coma + espacio + nombres + '\0'.
     * El campo permite como máximo 60 caracteres.
     */
    if (longitud_apellido + 2 + longitud_nombres >= tam_destino)
        return 0;

    memcpy(destino, apellido, longitud_apellido);

    destino[longitud_apellido] = ',';
    destino[longitud_apellido + 1] = ' ';

    memcpy(destino + longitud_apellido + 2,
           nombres,
           longitud_nombres + 1);

    return 1;
}


void arreglar_apyn_ALU(char *s)
{
    if (s == NULL)
        return;

    if (!normalizar_apyn_ALU(s, s, TAM_APYN_NORMALIZADO))
        s[0] = '\0';
}

/******************************************************************************
Descripcion:    separa una linea CSV in place por SEPARADOR_CSV, reemplazando
                cada separador por '\0' y guardando en campos[] el puntero al
                inicio de cada campo.
Parametros:     linea: linea a separar (se modifica).
                campos: vector donde se guardan los punteros a cada campo.
                max: cantidad maxima de campos a separar.
Retorno:        Cantidad de campos encontrados (los vacios cuentan). Nunca mas
                de max.
Observaciones:  Si la linea tiene mas separadores que max, el ultimo campo
                conserva el resto de la linea.
******************************************************************************/
int separar_campos_ALU(char *linea, char *campos[], int max)
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */
    //return separar_campos(linea, campos, max);
    int   cant = 0;
    char *sep;

    if(max <= 0)
        return 0;

    campos[cant++] = linea;     /* el primer campo arranca al inicio */

    while(cant < max && (sep = strchr(linea, SEPARADOR_CSV)) != NULL)
    {
        *sep = '\0';            /* corto el campo anterior */
        linea = sep + 1;
        campos[cant++] = linea; /* el siguiente arranca despues del ';' */
    }

    return cant;
}
