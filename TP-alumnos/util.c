#include "util.h"


/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    opcion Salir: copia socios.dat y titulos.dat con la fecha de
                proceso en el nombre, vuelca la matriz de consumo a disco y
                libera la memoria de ambos indices.
Parametros:     ind_socios: indice de socios.
                ind_titulos: indice de titulos.
                matriz: matriz de consumo.
                fecha_proceso: fecha de proceso.
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void salir_y_persistir_ALU(t_indice *ind_socios, t_indice *ind_titulos,
                              t_matriz_consumo matriz_consumo, const t_fecha *fecha_proceso)
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */
     char fecha_compacta[9] = "";

    char path_socios[
        sizeof(PREFIJO_SOCIOS_BKP) + sizeof(fecha_compacta) + 4
    ];

    char path_titulos[
        sizeof(PREFIJO_TITULOS_BKP) + sizeof(fecha_compacta) + 4
    ];

    char path_matriz[
        sizeof(PREFIJO_MATRIZ) + sizeof(fecha_compacta) + 4
    ];

    int resultado;

    if (fecha_proceso != NULL)
    {
        fecha_a_texto_compacto(fecha_proceso, fecha_compacta);

        snprintf(path_socios, sizeof(path_socios),
                 "%s%s.dat",
                 PREFIJO_SOCIOS_BKP,
                 fecha_compacta);

        snprintf(path_titulos, sizeof(path_titulos),
                 "%s%s.dat",
                 PREFIJO_TITULOS_BKP,
                 fecha_compacta);

        snprintf(path_matriz, sizeof(path_matriz),
                 "%s%s.dat",
                 PREFIJO_MATRIZ,
                 fecha_compacta);

        resultado = archivo_copiar(ARCH_SOCIOS_BIN, path_socios);

        printf("%s -> %s%s\n",
               ARCH_SOCIOS_BIN,
               path_socios,
               resultado == OK ? "" : " (no se pudo copiar)");

        resultado = archivo_copiar(ARCH_TITULOS_BIN, path_titulos);

        printf("%s -> %s%s\n",
               ARCH_TITULOS_BIN,
               path_titulos,
               resultado == OK ? "" : " (no se pudo copiar)");

        resultado = matriz_consumo_persistir(matriz_consumo, path_matriz);

        printf("matriz de consumo -> %s%s\n",
               path_matriz,
               resultado == OK ? "" : " (no se pudo grabar)");
    }

    if (ind_socios != NULL)
        indice_destruir_ALU(ind_socios);

    if (ind_titulos != NULL)
        indice_destruir_ALU(ind_titulos);

    printf("Memoria liberada. Hasta luego.\n");
    //salir_y_persistir(ind_socios, ind_titulos, matriz_consumo, fecha_proceso);
}
