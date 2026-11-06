#ifndef CADENA_H_INCLUDED
#define CADENA_H_INCLUDED

#define SEPARADOR_CSV ';'
#include <string.h>
#include <ctype.h>
#include <stddef.h>

void arreglar_apyn(char *);      /* normaliza "Apellidos, Nombres" */
void arreglar_titulo(char *);    /* normaliza titulos (sin coma) */
void quitar_salto(char *);       /* elimina '\n' y '\r' finales */
int  separar_campos(char *linea, char *campos[], int max);
int  es_numero(const char *);    /* solo digitos (opcionalmente '-') */
int  cadena_vacia(const char *);
void copiar_cadena(char *dest, const char *orig, size_t tam);

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

#define TAM_CADENA_AUX 512
#define TAM_APYN_NORMALIZADO 61

int normalizar_segmento_ALU(const char *origen, size_t longitud,
                            char *destino, size_t tam_destino);

int normalizar_apyn_ALU(const char *origen, char *destino,
                        size_t tam_destino);

void arreglar_apyn_ALU(char *s);
void arreglar_apyn_ALU(char *s);
int separar_campos_ALU(char *linea, char *campos[], int max);

#endif /* CADENA_H_INCLUDED */
