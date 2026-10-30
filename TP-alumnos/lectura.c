#include <string.h>
#include <limits.h>
#include "lectura.h"
#include "util.h"
#include "cadena.h"
#include "socio.h"
#include "titulo.h"
#include "config.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    funcion generica de lectura de archivos de texto. Por cada
                linea no vacia: parsear -> validar -> escribir al final del
                archivo binario e insertar en el indice. Ante el primer error
                (de formato o de validacion) se rechaza la linea invocando
                rechazar y se continua con la siguiente.
Parametros:     path_txt: ruta del archivo de texto (CSV).
                arch_bin: archivo binario destino abierto en "w+b" o "r+b".
                indice: indice de la entidad (se usa para validar duplicados y
                se va completando).
                reg: buffer de un registro de la entidad.
                tamanyo: tamanyo en bytes del registro.
                fecha_proceso: fecha de proceso.
                parsear: convierte una linea en registro.
                validar: valida el registro y deja el tipo de error.
                rechazar: accion ante un rechazo (auditar, ignorar, etc.).
                obtener_clave: devuelve la clave del registro para el indice.
                aceptados, rechazados: salida; contadores de registros.
Retorno:        OK si se pudo abrir el archivo de texto, ERROR en caso
                contrario.
Observaciones:  Se reutiliza para Socios.csv y Titulos.csv; lo especifico de
                cada entidad lo resuelven los punteros a funcion.
******************************************************************************/
int leer_archivo_texto_ALU(const char *path_txt, FILE *arch_bin, t_indice *indice,
                           void *reg, size_t tamanyo, const t_fecha *fecha_proceso,
                           int (*parsear)(const char *, void *),
                           int (*validar)(void *, const t_contexto *, char *),
                           void (*rechazar)(const void *, const char *, const char *, const t_fecha *),
                           long (*obtener_clave)(const void *),
                           unsigned *aceptados, unsigned *rechazados)
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */
    FILE *arch_txt;
    char linea[TAM_LINEA];
    char tipo_error[TAM_TIPO_ERROR];
    t_contexto contexto;
    t_reg_indice reg_indice;
    size_t longitud;
    long posicion;
    long tamanyo_long;
    unsigned nro_reg;
    int resultado = OK;
    int caracter;
    int linea_larga;

    if (aceptados != NULL)
        *aceptados = 0;

    if (rechazados != NULL)
        *rechazados = 0;

    if (path_txt == NULL || arch_bin == NULL || indice == NULL ||
            reg == NULL || tamanyo == 0 || fecha_proceso == NULL ||
            parsear == NULL || validar == NULL || obtener_clave == NULL ||
            aceptados == NULL || rechazados == NULL ||
            tamanyo > (size_t)LONG_MAX)
        return ERROR;

    tamanyo_long = (long)tamanyo;

    arch_txt = fopen(path_txt, "r");

    if (arch_txt == NULL)
    {
        printf("No se pudo abrir el archivo %s.\n", path_txt);
        return ERROR;
    }

    contexto.indice = indice;
    contexto.arch_bin = arch_bin;
    contexto.fecha_proceso = fecha_proceso;
    contexto.id_modificando = 0;

    while (fgets(linea, sizeof(linea), arch_txt) != NULL)
    {
        linea_larga = 0;
        longitud = strlen(linea);

        /*
         * Si se lleno el buffer sin encontrar el salto de linea,
         * se descarta el resto de esa linea para no procesarla
         * como si fueran registros diferentes.
         */
        if (longitud == sizeof(linea) - 1 &&
                linea[longitud - 1] != '\n')
        {
            linea_larga = 1;

            while ((caracter = fgetc(arch_txt)) != '\n' &&
                    caracter != EOF)
            {
                /* Descartar el resto de la linea extensa. */
            }
        }

        /* Quitar los finales de linea CR/LF. */
        longitud = strlen(linea);

        while (longitud > 0 &&
                (linea[longitud - 1] == '\n' ||
                 linea[longitud - 1] == '\r'))
        {
            linea[--longitud] = '\0';
        }

        /* Las lineas vacias o con solo espacios se ignoran. */
        if (!linea_larga && cadena_vacia(linea))
            continue;

        memset(reg, 0, tamanyo);
        tipo_error[0] = '\0';

        if (linea_larga)
        {
            strcpy(tipo_error, "FORMATO");

            if (rechazar != NULL)
                rechazar(reg, linea, tipo_error, fecha_proceso);

            (*rechazados)++;
            continue;
        }

        /*
         * 1. Parsear: transformar la linea en un registro.
         */
        if (parsear(linea, reg) != OK)
        {
            strcpy(tipo_error, "FORMATO");

            if (rechazar != NULL)
                rechazar(reg, linea, tipo_error, fecha_proceso);

            (*rechazados)++;
            continue;
        }

        /*
         * 2. Validar: la funcion especifica de socios o titulos
         *    determina el primer campo invalido.
         */
        if (validar(reg, &contexto, tipo_error) != OK)
        {
            if (tipo_error[0] == '\0')
                strcpy(tipo_error, "VALIDACION");

            if (rechazar != NULL)
                rechazar(reg, linea, tipo_error, fecha_proceso);

            (*rechazados)++;
            continue;
        }

        /*
         * 3. Obtener la posicion donde se agregara el registro.
         *    Se busca el final porque la validacion puede haber
         *    movido el cursor del archivo al consultar registros.
         */
        if (fseek(arch_bin, 0L, SEEK_END) != 0)
        {
            resultado = ERROR;
            break;
        }

        posicion = ftell(arch_bin);

        if (posicion < 0 || posicion % tamanyo_long != 0 ||
                (unsigned long)(posicion / tamanyo_long) > UINT_MAX)
        {
            resultado = ERROR;
            break;
        }

        nro_reg = (unsigned)(posicion / tamanyo_long);

        reg_indice.nro_reg = nro_reg;
        reg_indice.id = obtener_clave(reg);

        /*
         * 4. Insertar en el indice manteniendo el orden por clave.
         */
        if (indice_insertar_ALU(indice, &reg_indice,
                                sizeof(reg_indice),
                                compara_reg_indice_ALU) != OK)
        {
            resultado = ERROR;
            break;
        }

        /*
         * 5. Escribir el registro al final del binario.
         */
        if (fseek(arch_bin, 0L, SEEK_END) != 0 ||
                fwrite(reg, tamanyo, 1, arch_bin) != 1 ||
                fflush(arch_bin) != 0)
        {
            /* Deshacer la insercion si no se pudo escribir. */
            indice_eliminar_ALU(indice, &reg_indice,
                                sizeof(reg_indice),
                                compara_reg_indice_ALU);

            resultado = ERROR;
            break;
        }

        (*aceptados)++;
    }

    if (ferror(arch_txt))
        resultado = ERROR;

    if (fclose(arch_txt) != 0)
        resultado = ERROR;

    if (resultado != OK)
        printf("Error procesando el archivo %s.\n", path_txt);

    return resultado;



    //return leer_archivo_texto(path_txt, arch_bin, indice, reg, tamanyo, fecha_proceso, parsear, validar, rechazar, obtener_clave, aceptados, rechazados);
}

/******************************************************************************
Descripcion:    compara dos t_reg_indice por su clave id.
Parametros:     a, b: punteros a t_reg_indice.
Retorno:        Negativo si a < b, 0 si son iguales, positivo si a > b.
Observaciones:  Se usa como cmp en todas las operaciones del TDA indice.
******************************************************************************/
int compara_reg_indice_ALU(const void *a, const void *b)
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */
    const t_reg_indice *reg_a = (const t_reg_indice *)a;
    const t_reg_indice *reg_b = (const t_reg_indice *)b;

    if (reg_a == NULL && reg_b == NULL)
        return 0;

    if (reg_a == NULL)
        return -1;

    if (reg_b == NULL)
        return 1;

    if (reg_a->id < reg_b->id)
        return -1;

    if (reg_a->id > reg_b->id)
        return 1;

    return 0;

    //return compara_reg_indice(a, b);
}

/******************************************************************************
Descripcion:    construye los indices de socios y de titulos a partir de los
                archivos binarios existentes (solo registros activos).
Parametros:     ind_socios: indice de socios.
                ind_titulos: indice de titulos.
Retorno:        1 si ambos binarios existen y se cargaron, 0 en caso
                contrario.
Observaciones:  -
******************************************************************************/
int cargar_indices_ALU(t_indice *ind_socios, t_indice *ind_titulos)
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */
    if (ind_socios == NULL || ind_titulos == NULL)
        return ERROR;

    indice_vaciar_ALU(ind_socios);
    indice_vaciar_ALU(ind_titulos);

    /*
     * Se necesitan ambos binarios para considerar que hay datos
     * cargados. archivo_existe no posee una version _ALU.
     */
    if (!archivo_existe(ARCH_SOCIOS_BIN) ||
            !archivo_existe(ARCH_TITULOS_BIN))
    {
        return ERROR;
    }

    /*
     * Cargar solo los socios activos.
     */
    if (indice_cargar_ALU(ARCH_SOCIOS_BIN,
                          ind_socios,
                          sizeof(t_socio),
                          clave_socio,
                          activo_socio,
                          compara_reg_indice_ALU) != OK)
    {
        goto error;
    }

    /*
     * Cargar solo los titulos activos.
     */
    if (indice_cargar_ALU(ARCH_TITULOS_BIN,
                          ind_titulos,
                          sizeof(t_titulo),
                          clave_titulo,
                          activo_titulo,
                          compara_reg_indice_ALU) != OK)
    {
        goto error;
    }

    return OK;

error:
    indice_vaciar_ALU(ind_socios);
    indice_vaciar_ALU(ind_titulos);

    return ERROR;

    //return cargar_indices(ind_socios, ind_titulos);
}

/******************************************************************************
Descripcion:    opcion 1 del menu: lee Socios.csv y Titulos.csv con la funcion
                generica de lectura, genera socios.dat y titulos.dat
                (sobrescribiendolos) y reconstruye los indices desde los
                binarios generados.
Parametros:     ind_socios: indice de socios.
                ind_titulos: indice de titulos.
                fecha_proceso: fecha de proceso.
Retorno:        1 si los indices quedaron cargados, 0 en caso contrario.
Observaciones:  Los rechazos de socios se auditan en errores_socios.txt; los
                de titulos solo se cuentan.
******************************************************************************/
int generar_binarios_ALU(t_indice *ind_socios, t_indice *ind_titulos, const t_fecha *fecha_proceso)
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */
    FILE *prueba;
    FILE *arch_socios = NULL;
    FILE *arch_titulos = NULL;

    t_socio socio;
    t_titulo titulo;

    unsigned socios_aceptados = 0;
    unsigned socios_rechazados = 0;
    unsigned titulos_aceptados = 0;
    unsigned titulos_rechazados = 0;

    int resultado = OK;

    if (ind_socios == NULL || ind_titulos == NULL ||
            fecha_proceso == NULL)
    {
        return ERROR;
    }

    /*
     * Verificar que ambos CSV puedan abrirse antes de truncar
     * los binarios existentes.
     */
    prueba = fopen(ARCH_SOCIOS_CSV, "r");

    if (prueba == NULL)
    {
        printf("No se pudo abrir %s.\n", ARCH_SOCIOS_CSV);
        return ERROR;
    }

    fclose(prueba);

    prueba = fopen(ARCH_TITULOS_CSV, "r");

    if (prueba == NULL)
    {
        printf("No se pudo abrir %s.\n", ARCH_TITULOS_CSV);
        return ERROR;
    }

    fclose(prueba);

    /*
     * Crear los binarios desde cero.
     */
    arch_socios = fopen(ARCH_SOCIOS_BIN, "w+b");

    if (arch_socios == NULL)
    {
        printf("No se pudo crear %s.\n", ARCH_SOCIOS_BIN);
        return ERROR;
    }

    arch_titulos = fopen(ARCH_TITULOS_BIN, "w+b");

    if (arch_titulos == NULL)
    {
        printf("No se pudo crear %s.\n", ARCH_TITULOS_BIN);
        fclose(arch_socios);
        return ERROR;
    }

    /*
     * La validacion durante la ingesta debe trabajar con indices
     * vacios que se van completando a medida que se aceptan filas.
     */
    indice_vaciar_ALU(ind_socios);
    indice_vaciar_ALU(ind_titulos);

    /*
     * Procesar Socios.csv.
     */
    resultado = leer_archivo_texto_ALU(
                    ARCH_SOCIOS_CSV,
                    arch_socios,
                    ind_socios,
                    &socio,
                    sizeof(socio),
                    fecha_proceso,
                    texto_a_socio_ALU,
                    valida_socio_ALU,
                    rechaza_socio,
                    clave_socio,
                    &socios_aceptados,
                    &socios_rechazados
                );

    /*
     * Procesar Titulos.csv solo si la lectura de socios termino
     * sin errores operativos.
     */
    if (resultado == OK)
    {
        resultado = leer_archivo_texto_ALU(
                        ARCH_TITULOS_CSV,
                        arch_titulos,
                        ind_titulos,
                        &titulo,
                        sizeof(titulo),
                        fecha_proceso,
                        texto_a_titulo_ALU,
                        valida_titulo_ALU,
                        rechaza_titulo,
                        clave_titulo,
                        &titulos_aceptados,
                        &titulos_rechazados
                    );
    }

    if (fclose(arch_socios) != 0)
        resultado = ERROR;

    if (fclose(arch_titulos) != 0)
        resultado = ERROR;

    if (resultado != OK)
    {
        indice_vaciar_ALU(ind_socios);
        indice_vaciar_ALU(ind_titulos);

        printf("La generacion de los archivos binarios fallo.\n");
        return ERROR;
    }

    printf("Socios:  %u aceptados, %u rechazados (ver %s)\n",
           socios_aceptados, socios_rechazados, ARCH_ERRORES);
    printf("Titulos: %u aceptados, %u rechazados\n",
           titulos_aceptados, titulos_rechazados);

    /*
     reconstruir los indices a partir de los
     * binarios una vez terminada la generacion.
     */
    indice_vaciar_ALU(ind_socios);
    indice_vaciar_ALU(ind_titulos);

    if (cargar_indices_ALU(ind_socios, ind_titulos) != OK)
    {
        printf("No se pudieron reconstruir los indices.\n");
        return ERROR;
    }

    return OK;

    //return generar_binarios(ind_socios, ind_titulos, fecha_proceso);
}
