#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "novedad.h"
#include "lectura.h"
#include "titulo.h"
#include "indice.h"
#define FORMATO_RECHAZO "  RECHAZADA  op=%c id=%-6d motivo: %s\n"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    recorre secuencialmente novedades_catalogo.dat y aplica cada
                operacion sobre el indice y titulos.dat: 'A' agrega si el ID
                no esta en el indice (validando como en la carga), 'B' pone
                estado 'B' en el archivo y elimina del indice, 'M' reemplaza
                titulo y genero y suma el stock (rechaza si quedaria
                negativo). Al final muestra el reporte de procesadas y
                rechazadas.
Parametros:     indice: indice de titulos.
                path_novedades: ruta de novedades_catalogo.dat.
                path_titulos: ruta de titulos.dat.
                fecha_proceso: fecha de proceso (para el contexto de
                validacion).
Retorno:        OK si se pudieron abrir los archivos, ERROR en caso contrario.
Observaciones:  Como el indice solo tiene activos, un 'A' sobre un ID dado de
                baja se agrega como registro nuevo al final del archivo.
******************************************************************************/
int novedades_procesar_ALU(t_indice *indice, const char *path_novedades,
                           const char *path_titulos, const t_fecha *fecha_proceso)
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */
    FILE *arch_novedades = NULL;
    FILE *arch_titulos = NULL;
    t_novedad novedad;
    t_titulo titulo_actual;
    t_titulo titulo_nuevo;
    t_contexto contexto;
    t_reg_indice buscado;
    t_reg_indice nuevo_reg_indice;
    char tipo_error[TAM_TIPO_ERROR];
    size_t bytes_leidos;
    long posicion_fin;
    long long stock_resultante;
    unsigned procesadas = 0;
    unsigned rechazadas = 0;
    unsigned nro_reg;
    int posicion_indice;
    int resultado = OK;

    if (indice == NULL || path_novedades == NULL ||
            path_titulos == NULL || fecha_proceso == NULL ||
            (indice->cantidad_elementos_actual > 0 &&
             indice->vindice == NULL))
        return ERROR;

    arch_novedades = fopen(path_novedades, "rb");

    if (arch_novedades == NULL)
    {
        printf("No se encuentra %s.\n", path_novedades);
        return ERROR;
    }

    arch_titulos = fopen(path_titulos, "r+b");

    if (arch_titulos == NULL)
    {
        printf("No se pudo abrir %s.\n", path_titulos);
        fclose(arch_novedades);
        return ERROR;
    }

    contexto.indice = indice;
    contexto.arch_bin = arch_titulos;
    contexto.fecha_proceso = fecha_proceso;
    contexto.id_modificando = 0;

    printf("Procesando %s...\n\n", path_novedades);

    while (!feof(arch_novedades))
    {
        bytes_leidos = fread(&novedad, 1, sizeof(novedad),
                             arch_novedades);

        /*
         * Si no se leyo ningun byte, se distingue el fin normal
         * del archivo de un error de lectura.
         */
        if (bytes_leidos == 0)
        {
            if (ferror(arch_novedades))
                resultado = ERROR;

            break;
        }

        /*
         * Un registro debe leerse completo. Si quedan bytes
         * insuficientes, el archivo esta incompleto.
         */
        if (bytes_leidos != sizeof(novedad))
        {
            printf("El archivo de novedades termina con un registro incompleto.\n");
            resultado = ERROR;
            break;
        }

        tipo_error[0] = '\0';
        buscado.nro_reg = 0;
        buscado.id = (long)novedad.titulo.id;

        switch (novedad.codigo_operacion)
        {
        case OP_ALTA:
            if (indice_buscar_ALU(indice, &buscado, sizeof(buscado),
                                  compara_reg_indice_ALU) != NO_EXISTE)
            {
                printf(FORMATO_RECHAZO, novedad.codigo_operacion,
                       novedad.titulo.id, "el ID ya existe");
                rechazadas++;
                break;
            }

            titulo_nuevo = novedad.titulo;
            titulo_nuevo.estado = 'A';
            contexto.id_modificando = 0;

            if (valida_titulo_ALU(&titulo_nuevo, &contexto,
                                  tipo_error) != OK)
            {
                printf(FORMATO_RECHAZO, novedad.codigo_operacion,
                       novedad.titulo.id, tipo_error);
                rechazadas++;
                break;
            }

            if (fseek(arch_titulos, 0L, SEEK_END) != 0)
            {
                resultado = ERROR;
                break;
            }

            posicion_fin = ftell(arch_titulos);

            if (posicion_fin < 0 ||
                    posicion_fin % (long)sizeof(t_titulo) != 0 ||
                    (unsigned long)(posicion_fin /
                                    (long)sizeof(t_titulo)) > UINT_MAX)
            {
                printf("El archivo de titulos tiene un tamano invalido.\n");
                resultado = ERROR;
                break;
            }

            nro_reg = (unsigned)(posicion_fin /
                                 (long)sizeof(t_titulo));

            nuevo_reg_indice.nro_reg = nro_reg;
            nuevo_reg_indice.id = (long)titulo_nuevo.id;

            if (indice_insertar_ALU(indice, &nuevo_reg_indice,
                                    sizeof(nuevo_reg_indice),
                                    compara_reg_indice_ALU) != OK)
            {
                printf("No se pudo insertar el titulo ID %d en el indice.\n",
                       titulo_nuevo.id);
                resultado = ERROR;
                break;
            }

            if (titulo_escribir(arch_titulos, nro_reg,
                                &titulo_nuevo) != OK ||
                    fflush(arch_titulos) != 0)
            {
                indice_eliminar_ALU(indice, &nuevo_reg_indice,
                                    sizeof(nuevo_reg_indice),
                                    compara_reg_indice_ALU);

                printf("Error al escribir el alta del titulo ID %d.\n",
                       titulo_nuevo.id);
                resultado = ERROR;
                break;
            }

            printf("  ALTA       id=%-6d %s\n", titulo_nuevo.id,
                   titulo_nuevo.titulo);
            procesadas++;
            break;

        case OP_BAJA:
            posicion_indice = indice_buscar_ALU(
                                  indice, &buscado, sizeof(buscado),
                                  compara_reg_indice_ALU);

            if (posicion_indice == NO_EXISTE)
            {
                printf(FORMATO_RECHAZO, novedad.codigo_operacion,
                       novedad.titulo.id,
                       "el ID no existe o ya esta dado de baja");
                rechazadas++;
                break;
            }

            if (titulo_leer(arch_titulos, buscado.nro_reg,
                            &titulo_actual) != OK)
            {
                printf("Error al leer el titulo ID %d.\n",
                       novedad.titulo.id);
                resultado = ERROR;
                break;
            }

            titulo_nuevo = titulo_actual;
            titulo_nuevo.estado = 'B';

            if (titulo_escribir(arch_titulos, buscado.nro_reg,
                                &titulo_nuevo) != OK ||
                    fflush(arch_titulos) != 0)
            {
                printf("Error al dar de baja el titulo ID %d.\n",
                       novedad.titulo.id);
                resultado = ERROR;
                break;
            }

            if (indice_eliminar_ALU(indice, &buscado,
                                    sizeof(buscado),
                                    compara_reg_indice_ALU) != OK)
            {
                /* Intentar restaurar el titulo como activo. */
                titulo_escribir(arch_titulos, buscado.nro_reg,
                                &titulo_actual);
                fflush(arch_titulos);

                printf("No se pudo eliminar el titulo ID %d del indice.\n",
                       novedad.titulo.id);
                resultado = ERROR;
                break;
            }

            printf("  BAJA       id=%-6d %s\n", titulo_actual.id,
                   titulo_actual.titulo);
            procesadas++;
            break;

        case OP_MODIFICACION:
            posicion_indice = indice_buscar_ALU(
                                  indice, &buscado, sizeof(buscado),
                                  compara_reg_indice_ALU);

            if (posicion_indice == NO_EXISTE)
            {
                printf(FORMATO_RECHAZO, novedad.codigo_operacion,
                       novedad.titulo.id,
                       "el ID no existe o esta dado de baja");
                rechazadas++;
                break;
            }

            if (titulo_leer(arch_titulos, buscado.nro_reg,
                            &titulo_actual) != OK)
            {
                printf("Error al leer el titulo ID %d.\n",
                       novedad.titulo.id);
                resultado = ERROR;
                break;
            }

            /* 1. Titulo y genero: se normalizan y validan como en la carga. */
            titulo_nuevo = novedad.titulo;
            titulo_nuevo.id = titulo_actual.id;
            titulo_nuevo.stock = titulo_actual.stock;
            titulo_nuevo.estado = titulo_actual.estado;

            contexto.id_modificando = (long)titulo_actual.id;

            if (valida_titulo_ALU(&titulo_nuevo, &contexto,
                                  tipo_error) != OK)
            {
                printf(FORMATO_RECHAZO, novedad.codigo_operacion,
                       novedad.titulo.id, tipo_error);
                rechazadas++;
                break;
            }

            /* 2. El stock se suma y no puede quedar por debajo de 0. */
            stock_resultante = (long long)titulo_actual.stock +
                               (long long)novedad.titulo.stock;

            if (stock_resultante < 0)
            {
                printf(FORMATO_RECHAZO, novedad.codigo_operacion,
                       novedad.titulo.id, "STOCK (quedaria negativo)");

                rechazadas++;
                break;
            }

            if (stock_resultante > INT_MAX)
            {
                printf(FORMATO_RECHAZO, novedad.codigo_operacion,
                       novedad.titulo.id, "STOCK (supera el maximo)");

                rechazadas++;
                break;
            }
            titulo_nuevo.stock = (int)stock_resultante;

            if (titulo_escribir(arch_titulos, buscado.nro_reg,
                                &titulo_nuevo) != OK ||
                    fflush(arch_titulos) != 0)
            {
                printf("Error al modificar el titulo ID %d.\n",
                       novedad.titulo.id);
                resultado = ERROR;
                break;
            }

            printf("  MODIFICADA id=%-6d %s (stock %d)\n", titulo_nuevo.id,
                   titulo_nuevo.titulo, titulo_nuevo.stock);
            procesadas++;
            break;

        default:
            printf(FORMATO_RECHAZO, novedad.codigo_operacion,
                   novedad.titulo.id, "codigo de operacion invalido");
            rechazadas++;
            break;
        }

        if (resultado != OK)
            break;
    }

    if (fclose(arch_novedades) != 0)
        resultado = ERROR;

    if (fclose(arch_titulos) != 0)
        resultado = ERROR;

    printf("\nNovedades procesadas: %u   Rechazadas: %u\n",
           procesadas, rechazadas);

    return resultado;

    //return novedades_procesar(indice, path_novedades, path_titulos, fecha_proceso);
}
