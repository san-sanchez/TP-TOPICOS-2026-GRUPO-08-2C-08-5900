#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "validaciones.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    calcula el CUIL con el algoritmo modulo 11 del Anexo A.1:
                prefijo 20 (hombre) o 27 (mujer), DNI a 8 digitos y digito
                verificador. Si el resto es 1 el prefijo pasa a 23 y Z es 9
                (hombre) o 4 (mujer).
Parametros:     dni: numero de documento.
                sexo: 'H' o 'M'.
                dest: buffer de al menos TAM_CUIL bytes donde se deja "XY-
                NNNNNNNN-Z".
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void calcular_cuil_ALU(long dni, char sexo, char *dest)
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */
    //calcular_cuil(dni, sexo, dest);
    /* pesos del modulo 11 para los 10 digitos "XYNNNNNNNN" */
    int prefijo;
    int pesos[] = {5, 4, 3, 2, 7, 6, 5, 4, 3, 2};
    int digitos[10];
    int suma = 0;
    int resto;
    int verificador;
    int i;
    char dni_str[9];

    if (dest == NULL)
        return;

    dest[0] = '\0';

    if (dni < 0 || dni > 99999999)
        return;

    if (sexo != 'H' && sexo != 'M')
        return;

    prefijo = (sexo == 'H') ? PREFIJO_HOMBRE : PREFIJO_MUJER;

    sprintf(dni_str, "%08ld", dni);

    digitos[0] = prefijo / 10;
    digitos[1] = prefijo % 10;

    for (i = 0; i < 8; i++)
        digitos[i + 2] = dni_str[i] - '0';

    for (i = 0; i < 10; i++)
        suma += digitos[i] * pesos[i];

    resto = suma % 11;

    if (resto == 0)
        verificador = 0;
    else if (resto == 1)
    {
        prefijo = PREFIJO_RESTO1;
        verificador = (sexo == 'H') ? 9 : 4;
    }
    else
        verificador = 11 - resto;

    sprintf(dest, "%02d-%08ld-%d", prefijo, dni, verificador);
}

/******************************************************************************
Descripcion:    valida un email segun el Anexo A.3: hasta 30 caracteres sin
                espacios, una unica '@', parte local de letras/digitos/puntos
                que no empieza ni termina con punto, y dominio de
                letras/digitos/guiones/puntos con al menos un punto, que no
                empieza ni termina con punto o guion y cuyo ultimo segmento
                tiene al menos 2 letras.
Parametros:     email: cadena a validar.
Retorno:        1 si es valido, 0 en caso contrario.
Observaciones:  No verifica unicidad; eso lo hace socio_email_existe.
******************************************************************************/
int email_valido_ALU(const char *email)

/* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
   implementacion. En la entrega no puede quedar ninguna llamada a una
   funcion de la biblioteca que tenga version _ALU. */
//return email_valido(email);
{
    const char *arroba;
    const char *punto;
    const char *p;
    const char *fin;
    size_t longitud;

    if (email == NULL || *email == '\0')
        return 0;

    longitud = strlen(email);

    if (longitud > EMAIL_MAX_LEN)
        return 0;

    arroba = strchr(email, '@');

    if (arroba == NULL || arroba == email)
        return 0;

    if (strchr(arroba + 1, '@') != NULL)
        return 0;

    fin = email + longitud;

    for (p = email; p < fin; p++)
    {
        if (*p == ' ')
            return 0;
    }

    for (p = email; p < arroba; p++)
    {
        if (!ES_LETRA(*p) && !ES_DIGITO(*p) && *p != '.')
            return 0;
    }

    if (email[0] == '.' || arroba[-1] == '.')
        return 0;

    punto = strrchr(arroba + 1, '.');

    if (punto == NULL)
        return 0;

    if (punto == arroba + 1 || punto == fin - 1)
        return 0;

    if (arroba[1] == '.' || arroba[1] == '-')
        return 0;

    for (p = arroba + 1; p < fin; p++)
    {
        if (!ES_LETRA(*p) && !ES_DIGITO(*p) &&
                *p != '-' && *p != '.')
            return 0;
    }

    if (fin[-1] == '.' || fin[-1] == '-')
        return 0;

    p = punto + 1;

    if (fin - p < 2)
        return 0;

    for (; p < fin; p++)
    {
        if (!ES_LETRA(*p))
            return 0;
    }

    return 1;
}
