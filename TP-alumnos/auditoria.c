#include "auditoria.h"


/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    agrega al final del archivo de auditoria una linea con el
                formato DNI;TIPO_VALIDACION;FECHA_PROCESO.
Parametros:     path: ruta del archivo de texto de errores.
                dni_texto: DNI tal como vino en la entrada.
                tipo_error: nombre de la validacion que fallo.
                fecha_proceso: fecha de proceso.
Retorno:        OK si se pudo grabar, ERROR en caso contrario.
Observaciones:  El archivo se abre en modo append; si no existe se crea.
******************************************************************************/
int auditoria_registrar_ALU(const char *path, const char *dni_texto,
                            const char *tipo_error, const t_fecha *fecha_proceso)
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */
    FILE *arch;
    int escrito;
    int cerrado;

    if (path == NULL || dni_texto == NULL || tipo_error == NULL ||
            fecha_proceso == NULL || tipo_error[0] == '\0')
        return ERROR;

    arch = fopen(path, "a");
    if (arch == NULL)
        return ERROR;

    escrito = fprintf(arch, "%s;%s;%02d/%02d/%04d\n", dni_texto,
                      tipo_error, fecha_proceso->dia, fecha_proceso->mes,
                      fecha_proceso->anio) >= 0;
    if (fflush(arch) != 0)
        escrito = 0;
    cerrado = fclose(arch) == 0;

    return escrito && cerrado ? OK : ERROR;

    //return auditoria_registrar(path, dni_texto, tipo_error, fecha_proceso);
}

/******************************************************************************
Descripcion:    lee el archivo de auditoria y muestra, por cada par (DNI, tipo
                de validacion), la cantidad de incidencias registradas y el
                total general.
Parametros:     path: ruta del archivo de texto de errores.
Retorno:        OK si se pudo procesar (aunque el archivo no exista), ERROR si
                fallo la memoria.
Observaciones:  Acumula en un vector dinamico que crece con realloc.
******************************************************************************/
int auditoria_mostrar_ALU(const char *path)
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */
    FILE *arch;
    char linea[2 * TAM_LINEA];

    t_incidencia_ALU *incidencias = NULL;

    size_t cantidad = 0;
    size_t capacidad = 0;

    unsigned long total = 0;
    int hubo_error = 0;

    if (path == NULL)
        return ERROR;

    arch = fopen(path, "r");

    if (arch == NULL)
    {
        printf("No hay errores registrados (no existe %s).\n", path);
        return OK;
    }

    while (fgets(linea, sizeof(linea), arch) != NULL)
    {
        char *campos[4];
        char *salto;

        size_t i;
        size_t longitud_dni;

        int encontrada = 0;

        t_incidencia_ALU *nuevo;

        salto = strpbrk(linea, "\r\n");

        if (salto != NULL)
            *salto = '\0';
        else if (!feof(arch))
        {
            int c;

            while ((c = fgetc(arch)) != '\n' && c != EOF)
                ;

            continue;
        }

        if (linea[0] == '\0' ||
                separar_campos_ALU(linea, campos, 4) != 3)
            continue;

        if (campos[0][0] == '\0' ||
                campos[1][0] == '\0' ||
                campos[2][0] == '\0' ||
                strlen(campos[1]) >= TAM_TIPO_ERROR)
            continue;

        /* Buscar si ya existe el par DNI + tipo de validacion. */
        for (i = 0; i < cantidad; i++)
        {
            if (strcmp(incidencias[i].dni, campos[0]) == 0 &&
                    strcmp(incidencias[i].tipo, campos[1]) == 0)
            {
                incidencias[i].cantidad++;
                encontrada = 1;
                break;
            }
        }

        if (encontrada)
        {
            total++;
            continue;
        }

        /* Agrandar el vector si se completo su capacidad. */
        if (cantidad == capacidad)
        {
            size_t nueva_capacidad =
                capacidad == 0 ? 8 : capacidad * 2;

            if (nueva_capacidad < capacidad ||
                    nueva_capacidad >
                    ((size_t)-1) / sizeof(*incidencias))
            {
                hubo_error = 1;
                break;
            }

            nuevo = realloc(incidencias,
                            nueva_capacidad * sizeof(*incidencias));

            if (nuevo == NULL)
            {
                hubo_error = 1;
                break;
            }

            incidencias = nuevo;
            capacidad = nueva_capacidad;
        }

        longitud_dni = strlen(campos[0]);

        incidencias[cantidad].dni =
            malloc(longitud_dni + 1);

        if (incidencias[cantidad].dni == NULL)
        {
            hubo_error = 1;
            break;
        }

        memcpy(incidencias[cantidad].dni,
               campos[0],
               longitud_dni + 1);

        strcpy(incidencias[cantidad].tipo, campos[1]);

        incidencias[cantidad].cantidad = 1;

        cantidad++;
        total++;
    }

    if (ferror(arch))
        hubo_error = 1;

    if (fclose(arch) != 0)
        hubo_error = 1;

    if (hubo_error)
    {
        size_t i;

        for (i = 0; i < cantidad; i++)
            free(incidencias[i].dni);

        free(incidencias);

        printf("Error al procesar el archivo de auditoria.\n");

        return ERROR;
    }

    /* Formato de salida de la implementacion original. */
    printf("\n%-15s %-22s %s\n",
           "DNI",
           "TIPO DE VALIDACION",
           "CANTIDAD");

    printf("%-15s %-22s %s\n",
           "--------",
           "----------------------",
           "---------------");

    {
        size_t i;

        for (i = 0; i < cantidad; i++)
        {
            printf("%-15s %-22s %8lu\n",
                   incidencias[i].dni,
                   incidencias[i].tipo,
                   incidencias[i].cantidad);
        }
    }

    printf("\nTotal de incidencias: %lu\n", total);

    {
        size_t i;

        for (i = 0; i < cantidad; i++)
            free(incidencias[i].dni);

        free(incidencias);
    }

    return OK;

    //return auditoria_mostrar(path);
}
