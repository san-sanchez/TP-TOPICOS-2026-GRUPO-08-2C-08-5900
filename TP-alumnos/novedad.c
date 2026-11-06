
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "novedad.h"
#include "lectura.h"
#include "titulo.h"
#include "indice.h"

#define FORMATO_RECHAZO "  RECHAZADA  op=%c id=%-6d motivo: %s\n"

/******************************************************************************
Descripcion: traduce el tipo de validacion a un motivo mas descriptivo.
******************************************************************************/
const char *novedad_motivo_validacion_ALU(const char *tipo_error)
{
    if (tipo_error == NULL)
        return "validacion de titulo fallida";

    if (strcmp(tipo_error, "ID") == 0)
        return "ID debe ser mayor que cero o no debe existir";

    if (strcmp(tipo_error, "TITULO") == 0)
        return "titulo vacio, demasiado largo o sin terminador";

    if (strcmp(tipo_error, "GENERO") == 0)
        return "genero no permitido";

    if (strcmp(tipo_error, "STOCK") == 0)
        return "stock no puede ser negativo";

    return "datos del titulo invalidos";
}


/******************************************************************************
Descripcion: procesa secuencialmente las novedades del catalogo.

Alta:
    Si el ID no existe, valida el titulo y lo agrega al archivo
    y al indice.

Baja:
    Si el ID existe, cambia su estado a 'B' y lo elimina del indice.

Modificacion:
    Reemplaza titulo y genero y suma el stock recibido al actual.

Retorno:
    OK si el procesamiento finaliza sin errores de archivo.
    ERROR ante errores de lectura, escritura o acceso.
******************************************************************************/
int novedades_procesar_ALU(t_indice *indice,
                           const char *path_novedades,
                           const char *path_titulos,
                           const t_fecha *fecha_proceso)
{
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
    unsigned long cantidad_registros;

    long long stock_resultante;

    unsigned procesadas = 0;
    unsigned rechazadas = 0;
    unsigned nro_reg;

    int posicion_indice;
    int resultado = OK;

    if (indice == NULL ||
            path_novedades == NULL ||
            path_titulos == NULL ||
            fecha_proceso == NULL ||
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

    /*
     * Se procesa un registro completo por iteracion.
     * fread devuelve la cantidad de bytes efectivamente leidos.
     */
    while ((bytes_leidos = fread(&novedad,
                                 1,
                                 sizeof(novedad),
                                 arch_novedades)) == sizeof(novedad))
    {
        tipo_error[0] = '\0';

        buscado.nro_reg = 0;
        buscado.id = (long)novedad.titulo.id;

        switch (novedad.codigo_operacion)
        {
        case OP_ALTA:

            posicion_indice = indice_buscar_ALU(
                                  indice,
                                  &buscado,
                                  sizeof(buscado),
                                  compara_reg_indice_ALU);

            if (posicion_indice != NO_EXISTE)
            {
                printf(FORMATO_RECHAZO,
                       novedad.codigo_operacion,
                       novedad.titulo.id,
                       "el ID ya existe");

                rechazadas++;
                break;
            }

            titulo_nuevo = novedad.titulo;
            titulo_nuevo.estado = 'A';

            contexto.id_modificando = 0;

            if (valida_titulo_ALU(&titulo_nuevo,
                                  &contexto,
                                  tipo_error) != OK)
            {
                printf(FORMATO_RECHAZO,
                       novedad.codigo_operacion,
                       novedad.titulo.id,
                       novedad_motivo_validacion_ALU(tipo_error));

                rechazadas++;
                break;
            }

            /* Buscar el final del archivo para agregar el registro. */
            if (fseek(arch_titulos, 0L, SEEK_END) != 0)
            {
                printf("Error al posicionarse al final de titulos.dat.\n");
                resultado = ERROR;
                break;
            }

            posicion_fin = ftell(arch_titulos);

            if (posicion_fin < 0 ||
                    posicion_fin % (long)sizeof(t_titulo) != 0)
            {
                printf("El archivo de titulos tiene un tamano invalido.\n");
                resultado = ERROR;
                break;
            }

            cantidad_registros =
                (unsigned long)(posicion_fin / (long)sizeof(t_titulo));

            if (cantidad_registros > UINT_MAX)
            {
                printf("El archivo de titulos supera la cantidad de registros permitida.\n");
                resultado = ERROR;
                break;
            }

            nro_reg = (unsigned)cantidad_registros;

            nuevo_reg_indice.nro_reg = nro_reg;
            nuevo_reg_indice.id = (long)titulo_nuevo.id;

            /*
             * Primero se escribe el registro en el archivo.
             * Despues se incorpora al indice.
             */
            if (titulo_escribir(arch_titulos,
                                nro_reg,
                                &titulo_nuevo) != OK ||
                    fflush(arch_titulos) != 0)
            {
                printf("Error al escribir el alta del titulo ID %d.\n",
                       titulo_nuevo.id);

                resultado = ERROR;
                break;
            }

            if (indice_insertar_ALU(indice,
                                    &nuevo_reg_indice,
                                    sizeof(nuevo_reg_indice),
                                    compara_reg_indice_ALU) != OK)
            {
                /*
                 * Si no pudo insertarse en el indice,
                 * se deja el registro como baja logica.
                 */
                titulo_nuevo.estado = 'B';

                if (titulo_escribir(arch_titulos,
                                    nro_reg,
                                    &titulo_nuevo) != OK ||
                        fflush(arch_titulos) != 0)
                {
                    printf("No se pudo marcar como inactivo el titulo sin indice.\n");
                }

                printf("No se pudo insertar el titulo ID %d en el indice.\n",
                       titulo_nuevo.id);

                resultado = ERROR;
                break;
            }

            printf("  ALTA       id=%-6d %s\n",
                   titulo_nuevo.id,
                   titulo_nuevo.titulo);

            procesadas++;
            break;


        case OP_BAJA:

            posicion_indice = indice_buscar_ALU(
                                  indice,
                                  &buscado,
                                  sizeof(buscado),
                                  compara_reg_indice_ALU);

            if (posicion_indice == NO_EXISTE)
            {
                printf(FORMATO_RECHAZO,
                       novedad.codigo_operacion,
                       novedad.titulo.id,
                       "el ID no existe o ya esta dado de baja");

                rechazadas++;
                break;
            }

            if (titulo_leer(arch_titulos,
                            buscado.nro_reg,
                            &titulo_actual) != OK)
            {
                printf("Error al leer el titulo ID %d.\n",
                       novedad.titulo.id);

                resultado = ERROR;
                break;
            }

            titulo_nuevo = titulo_actual;
            titulo_nuevo.estado = 'B';

            if (titulo_escribir(arch_titulos,
                                buscado.nro_reg,
                                &titulo_nuevo) != OK ||
                    fflush(arch_titulos) != 0)
            {
                printf("Error al dar de baja el titulo ID %d.\n",
                       novedad.titulo.id);

                resultado = ERROR;
                break;
            }

            if (indice_eliminar_ALU(indice,
                                    &buscado,
                                    sizeof(buscado),
                                    compara_reg_indice_ALU) != OK)
            {
                /* Rollback: restaurar el titulo como activo. */
                if (titulo_escribir(arch_titulos,
                                    buscado.nro_reg,
                                    &titulo_actual) != OK ||
                        fflush(arch_titulos) != 0)
                {
                    printf("No se pudo restaurar el titulo ID %d.\n",
                           titulo_actual.id);
                }

                printf("No se pudo eliminar el titulo ID %d del indice.\n",
                       novedad.titulo.id);

                resultado = ERROR;
                break;
            }

            printf("  BAJA       id=%-6d %s\n",
                   titulo_actual.id,
                   titulo_actual.titulo);

            procesadas++;
            break;


        case OP_MODIFICACION:

            posicion_indice = indice_buscar_ALU(
                                  indice,
                                  &buscado,
                                  sizeof(buscado),
                                  compara_reg_indice_ALU);

            if (posicion_indice == NO_EXISTE)
            {
                printf(FORMATO_RECHAZO,
                       novedad.codigo_operacion,
                       novedad.titulo.id,
                       "el ID no existe o esta dado de baja");

                rechazadas++;
                break;
            }

            if (titulo_leer(arch_titulos,
                            buscado.nro_reg,
                            &titulo_actual) != OK)
            {
                printf("Error al leer el titulo ID %d.\n",
                       novedad.titulo.id);

                resultado = ERROR;
                break;
            }

            /*
             * Se reemplazan titulo y genero.
             * El ID y el estado se conservan.
             * El stock se conserva temporalmente para validar
             * titulo y genero sin rechazar incrementos negativos.
             */
            titulo_nuevo = novedad.titulo;

            titulo_nuevo.id = titulo_actual.id;
            titulo_nuevo.stock = titulo_actual.stock;
            titulo_nuevo.estado = titulo_actual.estado;

            contexto.id_modificando = (long)titulo_actual.id;

            if (valida_titulo_ALU(&titulo_nuevo,
                                  &contexto,
                                  tipo_error) != OK)
            {
                printf(FORMATO_RECHAZO,
                       novedad.codigo_operacion,
                       novedad.titulo.id,
                       novedad_motivo_validacion_ALU(tipo_error));

                rechazadas++;
                break;
            }

            /*
             * El stock de la novedad representa un incremento.
             * Puede ser negativo, pero el resultado no.
             */
            stock_resultante =
                (long long)titulo_actual.stock +
                (long long)novedad.titulo.stock;

            if (stock_resultante < 0)
            {
                printf(FORMATO_RECHAZO,
                       novedad.codigo_operacion,
                       novedad.titulo.id,
                       "stock resultante menor que cero");

                rechazadas++;
                break;
            }

            if (stock_resultante > INT_MAX)
            {
                printf(FORMATO_RECHAZO,
                       novedad.codigo_operacion,
                       novedad.titulo.id,
                       "stock resultante supera el maximo de int");

                rechazadas++;
                break;
            }

            titulo_nuevo.stock = (int)stock_resultante;

            if (titulo_escribir(arch_titulos,
                                buscado.nro_reg,
                                &titulo_nuevo) != OK ||
                    fflush(arch_titulos) != 0)
            {
                /* Rollback ante un error de escritura. */
                if (titulo_escribir(arch_titulos,
                                    buscado.nro_reg,
                                    &titulo_actual) != OK ||
                        fflush(arch_titulos) != 0)
                {
                    printf("No se pudo restaurar el titulo ID %d.\n",
                           titulo_actual.id);
                }

                printf("Error al modificar el titulo ID %d.\n",
                       novedad.titulo.id);

                resultado = ERROR;
                break;
            }

            printf("  MODIFICADA id=%-6d %s (stock %d)\n",
                   titulo_nuevo.id,
                   titulo_nuevo.titulo,
                   titulo_nuevo.stock);

            procesadas++;
            break;


        default:

            printf(FORMATO_RECHAZO,
                   novedad.codigo_operacion,
                   novedad.titulo.id,
                   "codigo de operacion invalido");

            rechazadas++;
            break;
        }

        if (resultado != OK)
            break;
    }

    /*
     * Se comprueba el estado de lectura antes de cerrar el archivo.
     */
    if (ferror(arch_novedades))
    {
        printf("Error de lectura del archivo de novedades.\n");
        resultado = ERROR;
    }
    else if (bytes_leidos != sizeof(novedad) && bytes_leidos != 0)
    {
        printf("El archivo de novedades termina con un registro incompleto.\n");
        resultado = ERROR;
    }

    if (fclose(arch_novedades) != 0)
        resultado = ERROR;

    if (fclose(arch_titulos) != 0)
        resultado = ERROR;

    printf("\nNovedades procesadas: %u   Rechazadas: %u\n",
           procesadas,
           rechazadas);

    return resultado;
}
