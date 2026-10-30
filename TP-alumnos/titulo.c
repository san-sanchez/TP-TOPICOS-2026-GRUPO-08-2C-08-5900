#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "titulo.h"
#include "cadena.h"
#include "util.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    convierte una linea CSV de 4 campos en t_titulo. Los campos
                numericos invalidos quedan en valores que fallan la validacion
                (id = 0, stock = -1). El estado se fija en 'A'.
Parametros:     linea: linea CSV.
                reg: puntero a t_titulo destino.
Retorno:        OK si la cantidad de campos es correcta, ERROR en caso
                contrario.
Observaciones:  Se pasa por puntero como parsear a leer_archivo_texto.
******************************************************************************/
int texto_a_titulo_ALU(const char *linea, void *reg)
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */
    t_titulo *titulo = (t_titulo *)reg;

    char copia[TAM_LINEA];
    char *campos[CAMPOS_TITULO];

    char *fin;
    long valor;
    size_t longitud;

    char titulo_normalizado[TAM_LINEA];

    if (linea == NULL || titulo == NULL ||
            strlen(linea) >= sizeof(copia))
        return ERROR;

    strcpy(copia, linea);
    quitar_salto(copia);

    if (separar_campos_ALU(copia, campos, CAMPOS_TITULO)
            != CAMPOS_TITULO)
        return ERROR;

    memset(titulo, 0, sizeof(*titulo));

    /* ID */
    if (campos[0][0] != '\0' && es_numero(campos[0]))
    {
        errno = 0;
        valor = strtol(campos[0], &fin, 10);

        if (errno != ERANGE &&
                fin != campos[0] &&
                *fin == '\0' &&
                valor >= INT_MIN &&
                valor <= INT_MAX)
        {
            titulo->id = (int)valor;
        }
        else
        {
            titulo->id = 0;
        }
    }
    else
    {
        titulo->id = 0;
    }

    /* Titulo: primero normalizar, luego controlar el tamaño. */
    copiar_cadena(titulo_normalizado,
                  campos[1],
                  sizeof(titulo_normalizado));

    arreglar_titulo(titulo_normalizado);

    longitud = strlen(titulo_normalizado);

    if (longitud < sizeof(titulo->titulo))
        memcpy(titulo->titulo, titulo_normalizado, longitud + 1);
    else
        titulo->titulo[0] = '\0';

    /* Genero */
    longitud = strlen(campos[2]);

    if (longitud < sizeof(titulo->genero))
        memcpy(titulo->genero, campos[2], longitud + 1);
    else
        titulo->genero[0] = '\0';

    /* Stock */
    if (campos[3][0] != '\0' && es_numero(campos[3]))
    {
        errno = 0;
        valor = strtol(campos[3], &fin, 10);

        if (errno != ERANGE &&
                fin != campos[3] &&
                *fin == '\0' &&
                valor >= INT_MIN &&
                valor <= INT_MAX)
        {
            titulo->stock = (int)valor;
        }
        else
        {
            titulo->stock = -1;
        }
    }
    else
    {
        titulo->stock = -1;
    }

    titulo->estado = 'A';

    return OK;
    //return texto_a_titulo(linea, reg);
}

/******************************************************************************
Descripcion:    aplica las validaciones del punto 6.2 en orden: ID > 0 y no
                existente en el indice, titulo normalizado no vacio, genero
                dentro de GENEROS[] y stock >= 0. Se detiene en el primer
                error.
Parametros:     reg: puntero a t_titulo (se modifica al normalizar el titulo).
                ctx: contexto con indice e id_modificando.
                tipo_error: salida; nombre de la validacion que fallo.
Retorno:        OK si el titulo es valido, ERROR en caso contrario.
Observaciones:  Se pasa por puntero como validar a leer_archivo_texto. Se reutiliza en las altas del
                lote de novedades.
******************************************************************************/
int valida_titulo_ALU(void *reg, const t_contexto *ctx, char *tipo_error)
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */
    t_titulo *titulo = (t_titulo *)reg;
    t_reg_indice buscado;
    int posicion;
    int i;
    int genero_valido = 0;

    if (tipo_error != NULL)
        tipo_error[0] = '\0';

    if (titulo == NULL || ctx == NULL || ctx->indice == NULL)
    {
        if (tipo_error != NULL)
            strcpy(tipo_error, "VALIDACION");
        return ERROR;
    }

    /* 1. ID mayor que cero y no existente, salvo la propia modificacion. */
    if (titulo->id <= 0)
    {
        if (tipo_error != NULL)
            strcpy(tipo_error, "ID");
        return ERROR;
    }

    buscado.nro_reg = 0;
    buscado.id = (long)titulo->id;
    posicion = indice_buscar_ALU(ctx->indice, &buscado, sizeof(buscado),
                                 compara_reg_indice_ALU);

    if (posicion != NO_EXISTE && ctx->id_modificando != (long)titulo->id)
    {
        if (tipo_error != NULL)
            strcpy(tipo_error, "ID");
        return ERROR;
    }

    /* 2. La cadena debe estar terminada y normalizarse sin exceder el campo. */
    if (memchr(titulo->titulo, '\0', sizeof(titulo->titulo)) == NULL)
    {
        if (tipo_error != NULL)
            strcpy(tipo_error, "TITULO");
        return ERROR;
    }

    arreglar_titulo(titulo->titulo);

    if (titulo->titulo[0] == '\0' ||
            strlen(titulo->titulo) >= sizeof(titulo->titulo))
    {
        if (tipo_error != NULL)
            strcpy(tipo_error, "TITULO");
        return ERROR;
    }

    /* 3. El genero debe ser exactamente uno de los provistos por la catedra. */
    if (memchr(titulo->genero, '\0', sizeof(titulo->genero)) == NULL)
    {
        if (tipo_error != NULL)
            strcpy(tipo_error, "GENERO");
        return ERROR;
    }

    for (i = 0; i < CANT_GENEROS; i++)
    {
        if (GENEROS[i] != NULL && strcmp(titulo->genero, GENEROS[i]) == 0)
        {
            genero_valido = 1;
            break;
        }
    }

    if (!genero_valido)
    {
        if (tipo_error != NULL)
            strcpy(tipo_error, "GENERO");
        return ERROR;
    }

    /* 4. El stock no puede ser negativo. */
    if (titulo->stock < 0)
    {
        if (tipo_error != NULL)
            strcpy(tipo_error, "STOCK");
        return ERROR;
    }

    return OK;

    //return valida_titulo(reg, ctx, tipo_error);
}

/******************************************************************************
Descripcion:    recorre el indice (ordenado por ID y solo con activos) y lista
                los titulos leyendo cada registro del archivo.
Parametros:     indice: indice de titulos.
                path: ruta de titulos.dat.
Retorno:        OK si se pudo abrir el archivo, ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int titulos_listar_ALU(const t_indice *indice, const char *path)
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */

    FILE *arch;
    const t_reg_indice *reg_indice;
    t_titulo titulo;
    unsigned i;
    int resultado = OK;

    if (indice == NULL || path == NULL ||
            (indice->cantidad_elementos_actual > 0 && indice->vindice == NULL))
        return ERROR;

    arch = fopen(path, "rb");
    if (arch == NULL)
    {
        printf("No hay datos de titulos cargados.\n");
        return ERROR;
    }

    reg_indice = (const t_reg_indice *)indice->vindice;
    titulos_encabezado();

    for (i = 0; i < indice->cantidad_elementos_actual; i++)
    {
        if (titulo_leer(arch, reg_indice[i].nro_reg, &titulo) != OK)
        {
            resultado = ERROR;
            break;
        }

        /* El indice debe contener activos; se verifica tambien el registro. */
        if (titulo.estado == 'A')
            titulo_mostrar(&titulo);
    }

    if (fclose(arch) != 0)
        resultado = ERROR;

    if (resultado != OK)
        printf("Error al leer el archivo de titulos.\n");
    else
        printf("\nTotal de titulos activos: %u\n",
               indice->cantidad_elementos_actual);

    return resultado;
    //return titulos_listar(indice, path);
}
