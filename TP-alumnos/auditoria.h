#ifndef AUDITORIA_H_INCLUDED
#define AUDITORIA_H_INCLUDED

#include "fecha.h"
#include "lectura.h"
#include "cadena.h"
#include "util.h"
#include "indice.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int auditoria_registrar(const char *path, const char *dni_texto,
                        const char *tipo_error, const t_fecha *fecha_proceso);
int auditoria_mostrar(const char *path);

typedef struct
{
    char *dni;
    char tipo[TAM_TIPO_ERROR];
    unsigned long cantidad;
} t_incidencia_ALU;

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

int auditoria_registrar_ALU(const char *path, const char *dni_texto,
                        const char *tipo_error, const t_fecha *fecha_proceso);
int auditoria_mostrar_ALU(const char *path);

#endif /* AUDITORIA_H_INCLUDED */
