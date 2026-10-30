#include "cadena.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    normaliza un campo Apellidos y Nombres segun el Anexo A.2 (con
                regla de coma).
Parametros:     s: cadena a normalizar (se modifica in place).
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void arreglar_apyn_ALU(char *s)
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */
    //arreglar_apyn(s);
    char  temp[TAM_CADENA_AUX] = {0};  /* cadena temporal */
    char *p_ori = s;                   /* puntero a la cadena origen */
    char *p_dst = temp;                /* puntero a la cadena destino */
    int   hay_coma;                    /* flag: la entrada ya trae la coma */
    int   coma_puesta = 0;             /* flag: ya se escribio la ", " */
    int   nueva_palabra = 1;           /* flag: el proximo caracter inicia palabra */

    if(s == NULL)
        return;

    /* si la entrada trae coma se respeta su lugar (apellidos compuestos);
       si no la trae, se agrega despues de la primera palabra */
    hay_coma = (strchr(s, ',') != NULL);

    /* doble corte: fin de la cadena origen o temporal llena */
    while(*p_ori != '\0' && (p_dst - temp) < TAM_CADENA_AUX - 2)
    {
        if(*p_ori == ',')
        {
            /* solo la primera coma y si no estamos al principio de todo */
            if(!coma_puesta && p_dst > temp)
            {
                if(*(p_dst - 1) == ' ')     /* saca el espacio antes de la coma */
                    p_dst--;
                *p_dst++ = ',';
                *p_dst++ = ' ';
                coma_puesta = 1;
                nueva_palabra = 1;
            }
        }
        else if(isspace((unsigned char)*p_ori))
        {
            /* no al principio, no repetir espacios y no dejar espacio
               despues de un apostrofo */
            if(p_dst > temp && *(p_dst - 1) != ' ' && *(p_dst - 1) != '\'')
            {
                if(!hay_coma && !coma_puesta)
                {
                    /* termino la primera palabra y no hay coma: se agrega */
                    *p_dst++ = ',';
                    *p_dst++ = ' ';
                    coma_puesta = 1;
                }
                else
                    *p_dst++ = ' ';     /* un unico espacio entre palabras */

                nueva_palabra = 1;
            }
        }
        else
        {
            /* letra (u otro caracter): mayuscula si inicia palabra */
            if(nueva_palabra)
            {
                *p_dst++ = toupper((unsigned char)*p_ori);
                nueva_palabra = 0;
            }
            else
                *p_dst++ = tolower((unsigned char)*p_ori);

            /* despues de un apostrofo arranca palabra: D'Angelo, O'Brien */
            if(*p_ori == '\'')
                nueva_palabra = 1;
        }
        p_ori++;
    }

    /* si termino con un espacio o una coma suelta, se retrocede */
    if(p_dst > temp && *(p_dst - 1) == ' ')
        p_dst--;
    if(p_dst > temp && *(p_dst - 1) == ',')
        p_dst--;
    *p_dst = '\0';

    /* copia la cadena ya formateada sobre la original */
    p_ori = s;
    p_dst = temp;
    while(*p_dst != '\0')
        *p_ori++ = *p_dst++;
    *p_ori = '\0';
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
