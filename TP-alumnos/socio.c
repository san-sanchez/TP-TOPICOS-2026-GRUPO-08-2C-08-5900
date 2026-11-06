#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "socio.h"
#include "cadena.h"
#include "util.h"
#include "auditoria.h"
#include "config.h"
#include "menu.h"



static void mostrar_socio_ALU(const t_socio *s)
{
    char fn[TAM_FECHA_TXT], fa[TAM_FECHA_TXT], fc[TAM_FECHA_TXT];

    if (s == NULL)
        return;

    sprintf(fn, "%02d/%02d/%04d", s->fecha_nacimiento.dia,
            s->fecha_nacimiento.mes, s->fecha_nacimiento.anio);
    sprintf(fa, "%02d/%02d/%04d", s->fecha_afiliacion.dia,
            s->fecha_afiliacion.mes, s->fecha_afiliacion.anio);
    sprintf(fc, "%02d/%02d/%04d", s->fecha_ultima_cuota.dia,
            s->fecha_ultima_cuota.mes, s->fecha_ultima_cuota.anio);

    printf("DNI: %ld | CUIL: %s | Apellidos y nombres: %s\n",
           s->dni, s->cuil, s->apellidos_nombres);
    printf("Nacimiento: %s | Sexo: %c | Afiliacion: %s\n", fn, s->sexo, fa);
    printf("Categoria: %s | Ultima cuota: %s | Plan: %s\n",
           s->categoria, fc, s->plan);
    printf("Email tutor: %s | Estado: %c\n",
           s->email_tutor, s->estado);
}

/* Busca el email en los socios activos. dni_excluir permite conservar el
   email propio durante una modificaci�n. */
static int email_existe_ALU(FILE *arch, const t_indice *indice,
                            const char *email, long dni_excluir)
{
    const t_reg_indice *v;
    unsigned i;
    t_socio s;

    if (arch == NULL || indice == NULL || email == NULL || email[0] == '\0')
        return 0;

    v = (const t_reg_indice *)indice->vindice;
    for (i = 0; i < indice->cantidad_elementos_actual; i++)
    {
        if (v[i].id == dni_excluir)
            continue;
        if (!socio_leer_ALU(arch, v[i].nro_reg, &s))
            return 1; /* Ante un error de lectura, rechazar por seguridad. */
        if (s.email_tutor[0] != '\0' && strcmp(s.email_tutor, email) == 0)
            return 1;
    }
    return 0;
}

static int registrar_error_socio_ALU(long dni, const char *dni_original,
                                     const char *tipo_error,
                                     const t_fecha *fecha_proceso)
{
    char texto_dni[32];

    if (tipo_error == NULL || fecha_proceso == NULL)
        return ERROR;

    /* El DNI original se conserva unicamente si falla el propio DNI. */
    if (strcmp(tipo_error, "DNI") == 0 && dni_original != NULL)
    {
        return auditoria_registrar_ALU(ARCH_ERRORES,
                                       dni_original,
                                       tipo_error,
                                       fecha_proceso);
    }

    snprintf(texto_dni, sizeof(texto_dni), "%ld", dni);

    return auditoria_registrar_ALU(ARCH_ERRORES,
                                   texto_dni,
                                   tipo_error,
                                   fecha_proceso);
}

static int leer_socio_manual_ALU(t_socio *s, int modificacion,
                                 char *dni_original, size_t tam_dni)
{
    char entrada[TAM_LINEA];
    t_fecha fecha;

    if (s == NULL)
        return ERROR;

    printf("DNI%s: ", modificacion ? " (no modificable)" : "");

    if (!modificacion)
    {
        leer_linea(entrada, sizeof(entrada));

        if (dni_original != NULL && tam_dni > 0)
            snprintf(dni_original, tam_dni, "%s", entrada);

        if (es_numero(entrada) && entrada[0] != '-')
            s->dni = strtol(entrada, NULL, 10);
        else
            s->dni = 0;
    }
    else
    {
        printf("%ld\n", s->dni);
    }

    /* CUIL */
    printf("CUIL%s: ", modificacion ? " (Enter conserva)" : "");
    leer_linea(entrada, sizeof(entrada));

    if (!modificacion || entrada[0] != '\0')
        copiar_o_invalidar(s->cuil, entrada, sizeof(s->cuil));

    /* Apellidos y nombres */
    printf("Apellidos y nombres%s: ",
           modificacion ? " (Enter conserva)" : "");
    leer_linea(entrada, sizeof(entrada));

    if (!modificacion || entrada[0] != '\0')
    {
        if (!normalizar_apyn_ALU(entrada,
                                 s->apellidos_nombres,
                                 sizeof(s->apellidos_nombres)))
        {
            s->apellidos_nombres[0] = '\0';
        }
    }

    /* Fecha de nacimiento */
    printf("Fecha de nacimiento dd/mm/aaaa%s: ",
           modificacion ? " (Enter conserva)" : "");
    leer_linea(entrada, sizeof(entrada));

    if (!modificacion || entrada[0] != '\0')
    {
        if (texto_a_fecha(entrada, &fecha) == OK)
            s->fecha_nacimiento = fecha;
        else
            memset(&s->fecha_nacimiento, 0,
                   sizeof(s->fecha_nacimiento));
    }

    /* Sexo */
    printf("Sexo H/M%s: ", modificacion ? " (Enter conserva)" : "");
    leer_linea(entrada, sizeof(entrada));

    if (!modificacion || entrada[0] != '\0')
    {
        s->sexo = (entrada[0] != '\0' && entrada[1] == '\0') ?
                  entrada[0] : '?';
    }

    /* Fecha de afiliacion */
    printf("Fecha de afiliacion dd/mm/aaaa%s: ",
           modificacion ? " (Enter conserva)" : "");
    leer_linea(entrada, sizeof(entrada));

    if (!modificacion || entrada[0] != '\0')
    {
        if (texto_a_fecha(entrada, &fecha) == OK)
            s->fecha_afiliacion = fecha;
        else
            memset(&s->fecha_afiliacion, 0,
                   sizeof(s->fecha_afiliacion));
    }

    /* Categoria */
    printf("Categoria MENOR/ADULTO%s: ",
           modificacion ? " (Enter conserva)" : "");
    leer_linea(entrada, sizeof(entrada));

    if (!modificacion || entrada[0] != '\0')
        copiar_o_invalidar(s->categoria,
                           entrada,
                           sizeof(s->categoria));

    /* Fecha de ultima cuota */
    printf("Fecha ultima cuota dd/mm/aaaa%s: ",
           modificacion ? " (Enter conserva)" : "");
    leer_linea(entrada, sizeof(entrada));

    if (!modificacion || entrada[0] != '\0')
    {
        if (texto_a_fecha(entrada, &fecha) == OK)
            s->fecha_ultima_cuota = fecha;
        else
            memset(&s->fecha_ultima_cuota, 0,
                   sizeof(s->fecha_ultima_cuota));
    }

    /* Plan */
    printf("Plan BASIC/PREMIUM/VIP/FAMILY%s: ",
           modificacion ? " (Enter conserva)" : "");
    leer_linea(entrada, sizeof(entrada));

    if (!modificacion || entrada[0] != '\0')
        copiar_o_invalidar(s->plan,
                           entrada,
                           sizeof(s->plan));

    /* Email tutor */
    printf("Email tutor%s: ",
           modificacion ? " (Enter conserva, '-' borra)" : "");
    leer_linea(entrada, sizeof(entrada));

    if (modificacion && strcmp(entrada, "-") == 0)
    {
        s->email_tutor[0] = '\0';
    }
    else if (!modificacion || entrada[0] != '\0')
    {
        copiar_o_invalidar(s->email_tutor,
                           entrada,
                           sizeof(s->email_tutor));
    }

    return OK;
}

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    convierte una linea CSV de 10 campos en t_socio. Los campos
                con formato invalido quedan con valores que haran fallar la
                validacion correspondiente (dni = 0, fecha 0/0/0, sexo '?',
                etc.). El estado se fija en 'A'.
Parametros:     linea: linea CSV.
                reg: puntero a t_socio destino.
Retorno:        OK si la cantidad de campos es correcta, ERROR en caso
                contrario.
Observaciones:  Se pasa por puntero como parsear a leer_archivo_texto.
******************************************************************************/
int texto_a_socio_ALU(const char *linea, void *reg)
{
    t_socio *s = (t_socio *)reg;
    char copia[TAM_LINEA];
    char *campos[CAMPOS_SOCIO + 1];

    if (linea == NULL || reg == NULL ||
            strlen(linea) >= sizeof(copia))
        return ERROR;

    strcpy(copia, linea);
    quitar_salto(copia);

    if (separar_campos_ALU(copia, campos,
                           CAMPOS_SOCIO + 1) != CAMPOS_SOCIO)
        return ERROR;

    memset(s, 0, sizeof(*s));

    /* DNI */
    if (es_numero(campos[0]) && campos[0][0] != '-')
    {
        char *fin;
        long dni = strtol(campos[0], &fin, 10);

        if (*fin == '\0')
            s->dni = dni;
    }

    /* CUIL */
    copiar_o_invalidar(s->cuil,
                       campos[1],
                       sizeof(s->cuil));

    /* Apellidos y nombres: normalizar antes de copiar al campo. */
    if (!normalizar_apyn_ALU(campos[2],
                             s->apellidos_nombres,
                             sizeof(s->apellidos_nombres)))
        s->apellidos_nombres[0] = '\0';

    /* Fecha de nacimiento */
    if (texto_a_fecha(campos[3],
                      &s->fecha_nacimiento) != OK)
        memset(&s->fecha_nacimiento, 0,
               sizeof(s->fecha_nacimiento));

    /* Sexo */
    s->sexo = (strlen(campos[4]) == 1) ?
              campos[4][0] : '?';

    /* Fecha de afiliación */
    if (texto_a_fecha(campos[5],
                      &s->fecha_afiliacion) != OK)
        memset(&s->fecha_afiliacion, 0,
               sizeof(s->fecha_afiliacion));

    /* Categoría: no truncar. */
    copiar_o_invalidar(s->categoria,
                       campos[6],
                       sizeof(s->categoria));

    /* Fecha de última cuota */
    if (texto_a_fecha(campos[7],
                      &s->fecha_ultima_cuota) != OK)
        memset(&s->fecha_ultima_cuota, 0,
               sizeof(s->fecha_ultima_cuota));

    /* Plan: no truncar. */
    copiar_o_invalidar(s->plan,
                       campos[8],
                       sizeof(s->plan));

    /* Email */
    copiar_o_invalidar(s->email_tutor,
                       campos[9],
                       sizeof(s->email_tutor));

    s->estado = 'A';

    return OK;
}

/******************************************************************************
Descripcion:    aplica las validaciones del punto 6.1 en orden (DNI, Sexo,
                CUIL, Apellidos y Nombres, Fecha de nacimiento, Fecha de
                afiliacion, Categoria, Fecha de ultima cuota, Plan, Email
                tutor). Normaliza apellidos y nombres. Se detiene en el primer
                error.
Parametros:     reg: puntero a t_socio a validar (se modifica al normalizar).
                ctx: contexto con indice, archivo binario, fecha de proceso e
                id_modificando.
                tipo_error: salida; nombre de la validacion que fallo.
Retorno:        OK si el socio es valido, ERROR en caso contrario.
Observaciones:  Si ctx->id_modificando == dni no se considera duplicado y se
                excluye ese DNI al verificar unicidad del email. Cumple con el
                puntero validar de leer_archivo_texto.
******************************************************************************/
int valida_socio_ALU(void *reg, const t_contexto *ctx, char *tipo_error)
{
    t_socio *s = (t_socio *)reg;
    t_reg_indice buscado;
    char cuil_calculado[TAM_CUIL];
    int anios;
    int i;
    int plan_valido = 0;

    if (tipo_error == NULL)
        return ERROR;

    tipo_error[0] = '\0';

    if (s == NULL || ctx == NULL ||
            ctx->fecha_proceso == NULL ||
            ctx->indice == NULL)
        return ERROR;

    /* 1. DNI */
    if (s->dni <= DNI_MIN || s->dni >= DNI_MAX)
    {
        strcpy(tipo_error, "DNI");
        return ERROR;
    }

    buscado.id = s->dni;
    buscado.nro_reg = 0;

    i = indice_buscar_ALU(ctx->indice,
                          &buscado,
                          sizeof(buscado),
                          compara_reg_indice_ALU);

    if (i != NO_EXISTE && ctx->id_modificando != s->dni)
    {
        strcpy(tipo_error, "DNI");
        return ERROR;
    }

    /* La consigna exige validar sexo antes del CUIL. */
    if (s->sexo != 'H' && s->sexo != 'M')
    {
        strcpy(tipo_error, "SEXO");
        return ERROR;
    }

    /* 2. CUIL */
    if (memchr(s->cuil, '\0', sizeof(s->cuil)) == NULL)
    {
        strcpy(tipo_error, "CUIL");
        return ERROR;
    }

    calcular_cuil_ALU(s->dni, s->sexo, cuil_calculado);

    if (strcmp(s->cuil, cuil_calculado) != 0)
    {
        strcpy(tipo_error, "CUIL");
        return ERROR;
    }

    /* 3. Apellidos y nombres */
    if (memchr(s->apellidos_nombres, '\0',
               sizeof(s->apellidos_nombres)) == NULL ||
            !normalizar_apyn_ALU(s->apellidos_nombres,
                                 s->apellidos_nombres,
                                 sizeof(s->apellidos_nombres)))
    {
        strcpy(tipo_error, "APELLIDOS_NOMBRES");
        return ERROR;
    }

    /* 4. Fecha de nacimiento */
    if (!fecha_correcta(&s->fecha_nacimiento) ||
            compara_fechas(&s->fecha_nacimiento,
                           ctx->fecha_proceso) >= 0 ||
            edad(&s->fecha_nacimiento,
                 ctx->fecha_proceso) < EDAD_MINIMA)
    {
        strcpy(tipo_error, "FECHA_NACIMIENTO");
        return ERROR;
    }

    /* 5. Fecha de afiliación */
    if (!fecha_correcta(&s->fecha_afiliacion) ||
            compara_fechas(&s->fecha_afiliacion,
                           &s->fecha_nacimiento) <= 0 ||
            compara_fechas(&s->fecha_afiliacion,
                           ctx->fecha_proceso) > 0)
    {
        strcpy(tipo_error, "FECHA_AFILIACION");
        return ERROR;
    }

    /* 6. Categoría */
    if (memchr(s->categoria, '\0',
               sizeof(s->categoria)) == NULL)
    {
        strcpy(tipo_error, "CATEGORIA");
        return ERROR;
    }

    anios = edad(&s->fecha_nacimiento, ctx->fecha_proceso);

    if ((anios < EDAD_ADULTO &&
            strcmp(s->categoria, CAT_MENOR) != 0) ||
            (anios >= EDAD_ADULTO &&
             strcmp(s->categoria, CAT_ADULTO) != 0))
    {
        strcpy(tipo_error, "CATEGORIA");
        return ERROR;
    }

    /* 7. Fecha de última cuota */
    if (!fecha_correcta(&s->fecha_ultima_cuota) ||
            compara_fechas(&s->fecha_ultima_cuota,
                           &s->fecha_afiliacion) < 0 ||
            compara_fechas(&s->fecha_ultima_cuota,
                           ctx->fecha_proceso) > 0)
    {
        strcpy(tipo_error, "FECHA_ULTIMA_CUOTA");
        return ERROR;
    }

    /* 8. Plan: usar la tabla común. */
    if (memchr(s->plan, '\0', sizeof(s->plan)) == NULL)
    {
        strcpy(tipo_error, "PLAN");
        return ERROR;
    }

    for (i = 0; i < CANT_PLANES; i++)
    {
        if (PLANES[i] != NULL &&
                strcmp(s->plan, PLANES[i]) == 0)
        {
            plan_valido = 1;
            break;
        }
    }

    if (!plan_valido)
    {
        strcpy(tipo_error, "PLAN");
        return ERROR;
    }

    /* 9. Email del tutor */
    if (memchr(s->email_tutor, '\0',
               sizeof(s->email_tutor)) == NULL)
    {
        strcpy(tipo_error, "EMAIL_TUTOR");
        return ERROR;
    }

    if (strcmp(s->categoria, CAT_MENOR) == 0)
    {
        if (!email_valido_ALU(s->email_tutor) ||
                ctx->arch_bin == NULL ||
                email_existe_ALU(
                    ctx->arch_bin,
                    ctx->indice,
                    s->email_tutor,
                    ctx->id_modificando == s->dni ?
                    s->dni : 0))
        {
            strcpy(tipo_error, "EMAIL_TUTOR");
            return ERROR;
        }
    }
    else if (!cadena_vacia(s->email_tutor))
    {
        strcpy(tipo_error, "EMAIL_TUTOR");
        return ERROR;
    }
    else
        s->email_tutor[0] = '\0';

    return OK;
}

/******************************************************************************
Descripcion:    lee el registro nro_reg del archivo binario de socios.
Parametros:     arch: archivo abierto en modo binario.
                nro_reg: numero de registro.
                s: socio destino.
Retorno:        OK si se pudo leer, ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int socio_leer_ALU(FILE *arch, unsigned nro_reg, t_socio *s)
{
    if(arch == NULL || s == NULL || fseek(arch, (long)nro_reg * (long)sizeof(*s), SEEK_SET) != 0)
        return ERROR;
    return fread(s, sizeof(*s), 1, arch) == 1 ? OK : ERROR;
}

/******************************************************************************
Descripcion:    escribe el socio en la posicion nro_reg del archivo binario y
                hace flush.
Parametros:     arch: archivo abierto en modo de escritura binaria.
                nro_reg: numero de registro.
                s: socio a grabar.
Retorno:        OK si se pudo grabar, ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int socio_escribir_ALU(FILE *arch, unsigned nro_reg, const t_socio *s)
{
    if(arch == NULL || s == NULL || fseek(arch, (long)nro_reg * (long)sizeof(*s), SEEK_SET) != 0)
        return ERROR;
    if(fwrite(s, sizeof(*s), 1, arch) != 1)
        return ERROR;
    return fflush(arch) == 0 ? OK : ERROR;
}

/******************************************************************************
Descripcion:    aplica la regla del Anexo A.4: el socio esta al dia si la
                ultima cuota es del mes de la fecha de proceso, o del mes
                anterior cuando el dia de la fecha de proceso es <= 10.
Parametros:     ultima_cuota: fecha de ultima cuota paga.
                fecha_proceso: fecha de proceso.
Retorno:        1 si esta al dia, 0 en caso contrario.
Observaciones:  -
******************************************************************************/
int cuota_al_dia_ALU(const t_fecha *ultima_cuota, const t_fecha *fecha_proceso)
{
    int diferencia_meses;

    if(ultima_cuota == NULL || fecha_proceso == NULL ||
            !fecha_correcta(ultima_cuota) || !fecha_correcta(fecha_proceso) ||
            compara_fechas(ultima_cuota, fecha_proceso) > 0)
        return 0;

    diferencia_meses = (fecha_proceso->anio - ultima_cuota->anio) * 12 +
                       fecha_proceso->mes - ultima_cuota->mes;

    if(diferencia_meses == 0)
        return 1;
    if(diferencia_meses == 1 && fecha_proceso->dia <= DIA_LIMITE_CUOTA)
        return 1;
    return 0;
}

/******************************************************************************
Descripcion:    opcion Alta: carga un socio por teclado, lo valida con
                valida_socio, lo agrega al final de socios.dat con estado 'A'
                y lo inserta en el indice. Si es rechazado lo audita.
Parametros:     indice: indice de socios.
                path: ruta de socios.dat.
                fecha_proceso: fecha de proceso.
Retorno:        OK si el socio fue agregado, ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int socio_alta_manual_ALU(t_indice *indice, const char *path, const t_fecha *fecha_proceso)
{
    t_socio s;
    t_contexto ctx;
    char tipo_error[TAM_TIPO_ERROR] = "";
    char dni_original[TAM_LINEA] = "";
    FILE *arch;
    long bytes;
    unsigned nro_reg;
    t_reg_indice nuevo;

    if (indice == NULL || path == NULL || fecha_proceso == NULL)
        return ERROR;

    memset(&s, 0, sizeof(s));
    s.estado = 'A';
    leer_socio_manual_ALU(&s, 0, dni_original, sizeof(dni_original));

    arch = fopen(path, "r+b");
    if (arch == NULL)
        arch = fopen(path, "w+b");
    if (arch == NULL)
    {
        printf("No se pudo abrir el archivo de socios.\n");
        return ERROR;
    }

    ctx.indice = indice;
    ctx.arch_bin = arch;
    ctx.fecha_proceso = fecha_proceso;
    ctx.id_modificando = 0;

    if (!valida_socio_ALU(&s, &ctx, tipo_error))
    {
        registrar_error_socio_ALU(s.dni, dni_original, tipo_error, fecha_proceso);
        printf("Alta rechazada: %s.\n", tipo_error);
        fclose(arch);
        return ERROR;
    }

    if (fseek(arch, 0, SEEK_END) != 0 || (bytes = ftell(arch)) < 0 ||
            bytes % (long)sizeof(s) != 0)
    {
        fclose(arch);
        return ERROR;
    }

    nro_reg = (unsigned)(bytes / (long)sizeof(s));
    if (fwrite(&s, sizeof(s), 1, arch) != 1 || fflush(arch) != 0)
    {
        fclose(arch);
        return ERROR;
    }

    nuevo.nro_reg = nro_reg;
    nuevo.id = s.dni;
    if (!indice_insertar_ALU(indice, &nuevo, sizeof(nuevo), compara_reg_indice_ALU))
    {
        /* Si no se pudo indexar, deja el registro inactivo para que no entre
           al sistema al reconstruir los �ndices. */
        s.estado = 'B';
        socio_escribir_ALU(arch, nro_reg, &s);
        fclose(arch);
        printf("No se pudo insertar el socio en el indice.\n");
        return ERROR;
    }

    fclose(arch);
    printf("Socio dado de alta correctamente.\n");
    return OK;
}

/******************************************************************************
Descripcion: modifica un socio activo sin cambiar DNI ni estado.
******************************************************************************/
int socio_modificacion_manual_ALU(t_indice *indice, const char *path, const t_fecha *fecha_proceso)
{
    t_reg_indice buscado;
    t_socio original, modificado;
    t_contexto ctx;
    char entrada[64], tipo_error[TAM_TIPO_ERROR] = "";
    FILE *arch;
    int posicion;

    if (indice == NULL || path == NULL || fecha_proceso == NULL)
        return ERROR;

    printf("Ingrese DNI del socio a modificar: ");
    leer_linea(entrada, sizeof(entrada));
    if (!es_numero(entrada) || entrada[0] == '-')
    {
        printf("DNI invalido.\n");
        return ERROR;
    }

    buscado.id = strtol(entrada, NULL, 10);
    buscado.nro_reg = 0;
    posicion = indice_buscar_ALU(indice, &buscado, sizeof(buscado), compara_reg_indice_ALU);
    if (posicion == NO_EXISTE)
    {
        printf("El socio no existe o esta dado de baja.\n");
        return ERROR;
    }

    arch = fopen(path, "r+b");
    if (arch == NULL)
        return ERROR;

    if (!socio_leer_ALU(arch, buscado.nro_reg, &original))
    {
        fclose(arch);
        return ERROR;
    }

    mostrar_socio_ALU(&original);
    modificado = original;
    printf("Ingrese los nuevos valores; Enter conserva el valor actual.\n");
    leer_socio_manual_ALU(&modificado, 1, NULL, 0);
    modificado.dni = original.dni;
    modificado.estado = original.estado;

    ctx.indice = indice;
    ctx.arch_bin = arch;
    ctx.fecha_proceso = fecha_proceso;
    ctx.id_modificando = original.dni;

    if (!valida_socio_ALU(&modificado, &ctx, tipo_error))
    {
        registrar_error_socio_ALU(original.dni, entrada, tipo_error, fecha_proceso);
        printf("Modificacion rechazada: %s.\n", tipo_error);
        fclose(arch);
        return ERROR;
    }

    if (!socio_escribir_ALU(arch, buscado.nro_reg, &modificado))
    {
        fclose(arch);
        return ERROR;
    }

    fclose(arch);
    printf("Socio modificado correctamente.\n");
    return OK;
}

/******************************************************************************
Descripcion: baja logica del socio.
******************************************************************************/
int socio_baja_ALU(t_indice *indice, const char *path)
{
    char entrada[64];
    t_reg_indice buscado;
    t_socio s;
    FILE *arch;
    int posicion;

    if (indice == NULL || path == NULL)
        return ERROR;

    printf("DNI del socio a dar de baja: ");
    leer_linea(entrada, sizeof(entrada));
    if (!es_numero(entrada) || entrada[0] == '-')
    {
        printf("DNI invalido.\n");
        return ERROR;
    }

    buscado.id = strtol(entrada, NULL, 10);
    buscado.nro_reg = 0;
    posicion = indice_buscar_ALU(indice, &buscado, sizeof(buscado), compara_reg_indice_ALU);
    if (posicion == NO_EXISTE)
    {
        printf("El socio no existe o ya esta dado de baja.\n");
        return ERROR;
    }

    arch = fopen(path, "r+b");
    if (arch == NULL)
        return ERROR;

    if (!socio_leer_ALU(arch, buscado.nro_reg, &s))
    {
        fclose(arch);
        return ERROR;
    }

    s.estado = 'B';
    if (!socio_escribir_ALU(arch, buscado.nro_reg, &s))
    {
        fclose(arch);
        return ERROR;
    }
    fclose(arch);

    if (!indice_eliminar_ALU(indice, &buscado, sizeof(buscado), compara_reg_indice_ALU))
        return ERROR;

    printf("Socio %ld (%s) dado de baja.\n", s.dni, s.apellidos_nombres);
    return OK;
}

/******************************************************************************
Descripcion: lista socios activos en el orden del indice.
******************************************************************************/
int socios_listar_ALU(const t_indice *indice, const char *path)
{
    const t_reg_indice *v;
    FILE *arch;
    unsigned i;
    t_socio s;

    if (indice == NULL || path == NULL)
        return ERROR;
    arch = fopen(path, "rb");
    if (arch == NULL)
        return ERROR;

    printf("%-9s %-13s %-30s %-10s %s %-10s %-6s %-10s %s %-7s %s\n",
           "DNI", "CUIL", "APELLIDOS Y NOMBRES", "NACIMIENTO", "S",
           "AFILIACION", "CATEG.", "ULT.CUOTA", "E", "PLAN", "EMAIL TUTOR");

    v = (const t_reg_indice *)indice->vindice;
    for (i = 0; i < indice->cantidad_elementos_actual; i++)
    {
        if (!socio_leer_ALU(arch, v[i].nro_reg, &s))
        {
            fclose(arch);
            return ERROR;
        }
        if (s.estado == 'A')
        {
            socio_mostrar(&s);
        }
    }

    fclose(arch);
    printf("\nTotal de socios activos: %u\n", indice->cantidad_elementos_actual);
    return OK;
}

/******************************************************************************
Descripcion: exporta socios PREMIUM/VIP con cuota al dia.
******************************************************************************/
int socios_exportar_ALU(const t_indice *indice,
                        const char *path,
                        const char *path_export,
                        const t_fecha *fecha_proceso)
{
    const t_reg_indice *v;
    FILE *arch;
    FILE *salida;
    unsigned i;
    unsigned exportados = 0;
    t_socio s;
    int resultado = OK;

    if (indice == NULL || path == NULL ||
            path_export == NULL || fecha_proceso == NULL ||
            (indice->cantidad_elementos_actual > 0 &&
             indice->vindice == NULL))
        return ERROR;

    arch = fopen(path, "rb");

    if (arch == NULL)
    {
        printf("No hay datos cargados.\n");
        return ERROR;
    }

    salida = fopen(path_export, "wb");

    if (salida == NULL)
    {
        fclose(arch);
        return ERROR;
    }

    v = (const t_reg_indice *)indice->vindice;

    for (i = 0; i < indice->cantidad_elementos_actual; i++)
    {
        if (!socio_leer_ALU(arch, v[i].nro_reg, &s))
        {
            resultado = ERROR;
            break;
        }

        if ((strcmp(s.plan, PLANES[INDICE_PLAN_PREMIUM]) == 0 ||
             strcmp(s.plan, PLANES[INDICE_PLAN_VIP]) == 0) &&
                s.estado == 'A' &&
                cuota_al_dia_ALU(&s.fecha_ultima_cuota,
                                 fecha_proceso))
        {
            if (fwrite(&s, sizeof(s), 1, salida) != 1)
            {
                resultado = ERROR;
                break;
            }

            exportados++;
        }
    }

    if (fclose(salida) != 0)
        resultado = ERROR;

    if (fclose(arch) != 0)
        resultado = ERROR;

    if (resultado != OK)
        return ERROR;

    printf("Socios exportados a %s: %u\n",
           path_export, exportados);

    return OK;
}
/******************************************************************************
Descripcion:    opcion 2 del menu: submenu para elegir entre alta y
                modificacion manual de socio.
Parametros:     ind_socios: indice de socios.
                fecha_proceso: fecha de proceso.
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void abm_socio_ALU(t_indice *indice, const t_fecha *fecha_proceso)
{
    static const char opciones[][LENGTH_MENU] =
    {
        "AMV",
        "Alta de socio",
        "Modificacion de socio",
        "Volver"
    };
    char op;

    if (indice == NULL || fecha_proceso == NULL)
        return;

    do
    {
        op = opcion(opciones, "ABM de socios", "Seleccione una opcion:");
        switch (op)
        {
        case 'A':
            socio_alta_manual_ALU(indice, ARCH_SOCIOS_BIN, fecha_proceso);
            PAUSA();
            break;
        case 'M':
            socio_modificacion_manual_ALU(indice, ARCH_SOCIOS_BIN, fecha_proceso);
            PAUSA();
            break;
        case 'V':
            break;
        default:
            printf("Opcion invalida.\n");
            PAUSA();
        }
    }
    while (op != 'V');
}

void copiar_o_invalidar(char *dest, const char *orig, size_t tam)
{
    if (dest == NULL || orig == NULL || tam == 0)
        return;

    if (strlen(orig) >= tam)
        strcpy(dest, "?");
    else
        copiar_cadena(dest, orig, tam);
}

