#include "fecha.h"
#include "util.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    solicita la fecha de proceso por teclado hasta obtener una
                valida. Si el usuario solo presiona Enter toma la fecha actual
                del sistema.
Parametros:     fecha_proceso: fecha destino.
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void pedir_fecha_proceso_ALU(t_fecha *fecha_proceso)
{
    /* TODO: reemplazar esta llamada a la funcion de la catedra por su propia
       implementacion. En la entrega no puede quedar ninguna llamada a una
       funcion de la biblioteca que tenga version _ALU. */
    char entrada[TAM_LINEA];

    if (fecha_proceso == NULL)
        return;

    for (;;)
    {
        printf("Ingrese fecha de proceso (dd/mm/aaaa) [Enter = fecha actual]: ");
        leer_linea(entrada, sizeof(entrada));

        if (entrada[0] == '\0')
        {
            fecha_actual(fecha_proceso);
            return;
        }

        if (texto_a_fecha(entrada, fecha_proceso) == OK &&
                fecha_correcta(fecha_proceso))
            return;

        printf("FECHA FORMALMENTE INCORRECTA\n");
    }


    //pedir_fecha_proceso(fecha_proceso);
}
