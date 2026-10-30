#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "alquiler.h"
#include "cadena.h"
#include "util.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    imprime por pantalla la matriz de consumo con totales por fila
                (plan) y por columna (genero).
Parametros:     m: matriz de consumo.
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void tablero_mostrar_ALU(t_matriz_consumo m)
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */
    long long total_general = 0;
    long long totales_genero[CANT_GENEROS] = {0};
    int i, j;

    printf("\n%-10s", "PLAN");

    for (j = 0; j < CANT_GENEROS; j++)
        printf(" %16s", GENEROS[j]);

    printf(" %8s\n", "TOTAL");

    for (i = 0; i < CANT_PLANES; i++)
    {
        long long total_plan = 0;

        printf("%-10s", PLANES[i]);

        for (j = 0; j < CANT_GENEROS; j++)
        {
            printf(" %16d", m[i][j]);

            total_plan += m[i][j];
            totales_genero[j] += m[i][j];
        }

        printf(" %8lld\n", total_plan);
        total_general += total_plan;
    }

    printf("%-10s", "TOTAL");

    for (j = 0; j < CANT_GENEROS; j++)
        printf(" %16lld", totales_genero[j]);

    printf(" %8lld\n", total_general);
    //tablero_mostrar(m);
}

/******************************************************************************
Descripcion:    ejecuta el flujo de alquiler de un titulo: 1) habilitacion del
                socio (debe estar en el indice), 2) inventario (el titulo debe
                estar en el indice y con stock > 0), 3) descuenta una unidad
                de stock en el archivo, 4) incrementa la celda [plan][genero]
                de la matriz, 5) emite el comprobante.
Parametros:     ind_socios: indice de socios.
                ind_titulos: indice de titulos.
                path_socios: ruta de socios.dat.
                path_titulos: ruta de titulos.dat.
                matriz: matriz de consumo a alimentar.
                fecha_proceso: fecha de proceso para el comprobante.
Retorno:        OK si el alquiler se concreto, ERROR si alguna etapa fallo.
Observaciones:  Si una etapa falla se informa el motivo y no se ejecutan las
                siguientes. No se exige cuota al dia.
******************************************************************************/
int alquilar_ALU(t_indice *ind_socios, t_indice *ind_titulos, const char *path_socios,
                 const char *path_titulos, t_matriz_consumo matriz_consumo, const t_fecha *fecha_proceso)
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */
    char entrada[TAM_LINEA];
    char *fin;
    long dni;
    long id;
    t_reg_indice reg_indice;
    t_socio socio;
    t_titulo titulo;
    FILE *arch_socios = NULL;
    FILE *arch_titulos = NULL;
    int plan, genero;
    int posicion;

    if (ind_socios == NULL || ind_titulos == NULL || path_socios == NULL ||
            path_titulos == NULL || fecha_proceso == NULL)
        return ERROR;

    printf("DNI del socio: ");
    leer_linea(entrada, sizeof(entrada));
    if (entrada[0] == '\0' || !es_numero(entrada) || entrada[0] == '-')
    {
        printf("Alquiler rechazado: DNI invalido.\n");
        return ERROR;
    }

    errno = 0;
    dni = strtol(entrada, &fin, 10);
    if (errno == ERANGE || fin == entrada || *fin != '\0' || dni <= 0)
    {
        printf("Alquiler rechazado: DNI invalido.\n");
        return ERROR;
    }

    reg_indice.nro_reg = 0;
    reg_indice.id = dni;
    posicion = indice_buscar_ALU(ind_socios, &reg_indice,
                                 sizeof(reg_indice), compara_reg_indice_ALU);
    if (posicion == NO_EXISTE)
    {
        printf("Alquiler rechazado: el socio no existe o esta dado de baja.\n");
        return ERROR;
    }

    arch_socios = fopen(path_socios, "rb");
    if (arch_socios == NULL ||
            socio_leer_ALU(arch_socios, reg_indice.nro_reg, &socio) != OK)
    {
        if (arch_socios != NULL)
            fclose(arch_socios);
        printf("Alquiler rechazado: no se pudo leer el socio.\n");
        return ERROR;
    }
    if (fclose(arch_socios) != 0)
    {
        printf("Alquiler rechazado: no se pudo leer el socio.\n");
        return ERROR;
    }
    if (socio.estado != 'A')
    {
        printf("Alquiler rechazado: el socio no existe o esta dado de baja.\n");
        return ERROR;
    }

    printf("ID de la pelicula: ");
    leer_linea(entrada, sizeof(entrada));
    if (entrada[0] == '\0' || !es_numero(entrada) || entrada[0] == '-')
    {
        printf("Alquiler rechazado: ID invalido.\n");
        return ERROR;
    }

    errno = 0;
    id = strtol(entrada, &fin, 10);
    if (errno == ERANGE || fin == entrada || *fin != '\0' ||
            id <= 0 || id > INT_MAX)
    {
        printf("Alquiler rechazado: ID invalido.\n");
        return ERROR;
    }

    reg_indice.nro_reg = 0;
    reg_indice.id = id;
    posicion = indice_buscar_ALU(ind_titulos, &reg_indice,
                                 sizeof(reg_indice), compara_reg_indice_ALU);
    if (posicion == NO_EXISTE)
    {
        printf("Alquiler rechazado: el titulo no existe o esta dado de baja.\n");
        return ERROR;
    }

    arch_titulos = fopen(path_titulos, "r+b");
    if (arch_titulos == NULL ||
            titulo_leer(arch_titulos, reg_indice.nro_reg, &titulo) != OK)
    {
        if (arch_titulos != NULL)
            fclose(arch_titulos);
        printf("Alquiler rechazado: no se pudo leer el titulo.\n");
        return ERROR;
    }

    if (titulo.estado != 'A')
    {
        fclose(arch_titulos);
        printf("Alquiler rechazado: el titulo no existe o esta dado de baja.\n");
        return ERROR;
    }
    if (titulo.stock <= 0)
    {
        printf("Alquiler rechazado: sin stock de \"%s\".\n", titulo.titulo);
        fclose(arch_titulos);
        return ERROR;
    }

    plan = plan_indice(socio.plan);
    genero = genero_indice(titulo.genero);
    if (plan < 0 || plan >= CANT_PLANES || genero < 0 ||
            genero >= CANT_GENEROS)
    {
        printf("Alquiler rechazado: datos de plan o genero invalidos.\n");
        fclose(arch_titulos);
        return ERROR;
    }
    if (matriz_consumo[plan][genero] == INT_MAX)
    {
        printf("Alquiler rechazado: el contador de consumo alcanzo su limite.\n");
        fclose(arch_titulos);
        return ERROR;
    }

    titulo.stock--;
    if (titulo_escribir(arch_titulos, reg_indice.nro_reg, &titulo) != OK)
    {
        fclose(arch_titulos);
        printf("Alquiler rechazado: no se pudo actualizar el stock.\n");
        return ERROR;
    }
    if (fclose(arch_titulos) != 0)
    {
        printf("Alquiler rechazado: no se pudo cerrar el archivo de titulos.\n");
        return ERROR;
    }

    matriz_consumo[plan][genero]++;

    printf("\n========== COMPROBANTE DE ALQUILER ==========\n");
    printf(" Fecha    : %02d/%02d/%04d\n", fecha_proceso->dia,
           fecha_proceso->mes, fecha_proceso->anio);
    printf(" Socio    : %ld - %s (plan %s)\n", socio.dni,
           socio.apellidos_nombres, socio.plan);
    printf(" Titulo   : %d - %s (%s)\n", titulo.id, titulo.titulo,
           titulo.genero);
    printf(" Stock restante: %d\n", titulo.stock);
    printf("=============================================\n");

    return OK;
    //return alquilar(ind_socios, ind_titulos, path_socios, path_titulos, matriz_consumo, fecha_proceso);
}
