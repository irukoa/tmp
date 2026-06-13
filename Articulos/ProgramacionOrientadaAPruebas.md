---
layout: default
title: Programación orientada a pruebas
---

# Programación orientada a pruebas

La programación orientada a pruebas consiste en diseñar el software de forma simultánea a una infraestructura que permita verificar su comportamiento. Esta infraestructura se materializa en forma de pruebas: ejecuciones controladas del código destinadas a comprobar la adecuada conservación de varias propiedades del software. Las ventajas de adoptar esta sistemática son numerosas. Obliga a definir interfaces cuidadas, proporciona un mecanismo para detectar regresiones de manera temprana y dota al proyecto de una referencia ejecutable acerca del comportamiento esperado del software. A menudo, esta colección de pruebas constituye además una valiosa fuente de documentación para nuevos desarrolladores, al mostrar ejemplos concretos de utilización de los distintos componentes del sistema.

Las pruebas pueden clasificarse de múltiples maneras. La literatura especializada distingue entre pruebas unitarias, de integración o funcionales; entre pruebas de caja blanca y caja negra; entre pruebas aisladas o dependientes del entorno de ejecución. Muchos de estos criterios son ortogonales entre sí y responden a necesidades distintas. Por este motivo, el propósito de este artículo no consiste en profundizar en una taxonomía concreta de pruebas, sino en retroceder un paso en el nivel de abstracción y reflexionar sobre la propia infraestructura que las hace posibles.

Con frecuencia, la discusión en torno a la programación orientada a pruebas se centra en qué herramientas emplear. Existen marcos ampliamente adoptados, como [GoogleTest](https://github.com/google/googletest), que proporcionan soluciones maduras a problemas recurrentes. Sin embargo, no siempre resulta evidente que la incorporación de una infraestructura externa constituya la opción más adecuada para un proyecto determinado. Del mismo modo, desarrollar una infraestructura propia tampoco es una decisión exenta de costes.

En este artículo examinaremos distintas estrategias para implementar una infraestructura de pruebas, prestando especial atención a los compromisos que cada una de ellas implica. Nuestro objetivo no es defender una solución universal, sino proporcionar al lector los elementos necesarios para evaluar qué enfoque se ajusta mejor a las características y necesidades de su propio software. Este artículo puede leerse como una descripcion de técnicas y usos en la implementación de esta metodología.

## Diseño de infraestructura

Una prueba consta de tres secuencias. Primero, diseñamos un escenario. Esto aporta contexto a la prueba. Seguidamente, la prueba ejecuta cierta funcionalidad del proyecto. Finalmente, usando aserciones, verificamos el comportamiento esperado y restauramos el entorno a su estado original. Desde esta perspectiva, los requisitos fundamentales de una infraestructura de pruebas son relativamente modestos.
- Permitir el diseño de escenarios.
- Habilitar un registro de pruebas, ya sea manual o automático.
- Proveer un ejecutor de pruebas.
- Diseñar un sistema de aserciones.
- Emitir informes: al menos, debe de identificar las pruebas fallidas.

## Infraestructura propia o externa

Un marco externo aporta documentación y convenciones consolidadas, además de externalizar el mantenimiento de la infraestructura. Por otro lado, introduce costes, tales como la integración en el proyecto y posibles disonancias con respecto al lenguaje de programación. En esencia, los marcos de pruebas consolidados resuelven problemas reales, pero también introducen dependencias, convenciones y costes de integración que no siempre resultan proporcionales a las necesidades del proyecto.

En consecuencia, la elección de una infraestructura de pruebas no debe responder únicamente a criterios de disponibilidad. Debe tener en cuenta las características del proyecto, los lenguajes implicados y el coste real de integración y mantenimiento.

En proyectos extensos escritos mayoritariamente en C++, marcos como GoogleTest ofrecen ventajas evidentes: proporcionan un lenguaje de aserciones expresivo, mecanismos avanzados de parametrización y una amplia colección de utilidades auxiliares. En estos contextos, los beneficios suelen compensar sobradamente la dependencia introducida. Sin embargo, esta situación no es universal. En proyectos escritos principalmente en C, la incorporación de una infraestructura fuertemente ligada a C++ puede introducir una cierta disonancia técnica y conceptual. Del mismo modo, en proyectos desarrollados en Fortran u otros lenguajes menos representados en esta clase de marcos, la integración puede exigir adaptaciones adicionales y aumentar la complejidad del mantenimiento.

La cuestión relevante no consiste en determinar qué infraestructura es objetivamente superior, sino en identificar cuál resulta proporcionada a las necesidades reales del proyecto. Debe emplearse la herramienta adecuada. Cuando ninguna de las alternativas disponibles se ajusta razonablemente a los requisitos del proyecto, el desarrollo de una infraestructura propia constituye también una opción legítima.

## Infraestructura mínima en C

En esta sección expondremos brevemente una pequeña implementación de marco de pruebas escrito en C. Elegimos este lenguaje porque las interfaces binarias generadas por compiladores de C constituyen un punto de interoperabilidad común: numerosos lenguajes proporcionan mecanismos para invocar funciones escritas en C o enlazar bibliotecas generadas por compiladores de C, lo que facilita reutilizar la infraestructura de pruebas desde distintos entornos. C dispone además de una facil integración con Make, el minimo común denominador entre los sistemas de construcción. Finalmente, C es un lenguaje orientado a la programación de sistemas, lo cual aporta muchas utilidades auxiliares en la implementación.

El marco de pruebas que expondremos está cimientado sobre [TSD](https://github.com/irukoa/ApuntesInfraestructuraSoftware/tree/main/TSD), un marco que yo desarrollé. Las abstracciones fundamentales son las siguientes. Representamos una prueba así:

```c
typedef void (*TSD_FncProto)(void *Data);

typedef struct _TSD_Context {
  const TSD_FncProto SetUp;
  const TSD_FncProto TearDown;
  const size_t ContextSize;
} TSD_Context;

typedef struct _TSD_Test {
  const char *const  Name;
  const TSD_FncProto Procedure;
  const TSD_Context *Ctx; // Opcional.
} TSD_Test;
```

En este contexto, una prueba no es más que una función acompañada de cierta información descriptiva y, opcionalmente, de un contexto de ejecución. Las pruebas pueden agruparse en un conjunto de pruebas:

```c
typedef struct _TSD_TestSuite {
  const char *const Name;
  const TSD_Test   *Tests; // Matriz de pruebas.
  const size_t      Count;
} TSD_TestSuite;
```

Finalmente, mencionamos las funciones ejecutoras de la implementación:

```c
void TSD_RunTest(const TSD_Test *TestInstance,
                 size_t         *NFailedTests);
void TSD_RunSuite(const TSD_TestSuite *Suite,
                  size_t              *NFailedTests);
void TSD_RunAll(const TSD_TestSuite *const Suites[],
                const size_t               SuitesCount,
                size_t                    *NFailedTests);
```

Desde el punto de vista del uso, conviene definir macros, que habilitan la creación automática de pruebas:

```c
#define TEST(NAME)                                                  \
  static void TEST_##NAME(void);                                    \
  static void TEST_##NAME##_Wrapper(void *ContextData) {            \
    (void)ContextData;                                              \
    TEST_##NAME();                                                  \
  }                                                                 \
  static const TSD_Test NAME = {.Name      = #NAME,                 \
                                .Procedure = TEST_##NAME##_Wrapper, \
                                .Ctx       = NULL};                 \
  static void           TEST_##NAME(void)
```

Algunas implementaciones automatizan el registro de pruebas mediante extensiones específicas del compilador. Sin embargo, el registro manual suele resultar suficiente y presenta una portabilidad mayor. TSD emplea una simple variable global

```c
static const TSD_TestSuite **Suites; // Matriz de punteros.
```

Finalmente, la integración con Make resulta natural. Basta con compilar la implementación del marco de pruebas junto con la implementación de las pruebas y los archivos objeto del proyecto. Así generamos un ejecutable específico. Desde el punto de vista del sistema de construcción, las pruebas constituyen simplemente otro artefacto del software.

El objetivo de esta implementación no consiste en sustituir infraestructuras más sofisticadas. Pretende únicamente mostrar que los mecanismos fundamentales sobre los que se construyen estos sistemas son relativamente sencillos. Comprenderlos facilita evaluar con criterio cuándo conviene adoptar una solución existente y cuándo una infraestructura mínima puede resultar suficiente.

## Aislamiento funcional

Una dificultad habitual al diseñar pruebas consiste en aislar la funcionalidad que deseamos validar del resto del sistema. En muchos casos, el comportamiento de una unidad de software depende de componentes externos: asignadores dinámicos de memoria, sistemas de archivos, conexiones de red o bibliotecas de terceros. Estas dependencias dificultan la reproducción controlada de determinadas situaciones, especialmente aquellas asociadas a vías de error poco frecuentes. Por ejemplo, validar la respuesta del software ante un fallo de la función [`malloc`](https://man7.org/linux/man-pages/man3/malloc.3.html) de C puede resultar extraordinariamente complicado si dependemos exclusivamente del comportamiento real del sistema operativo. En estos contextos, resulta útil disponer de mecanismos que permitan sustituir temporalmente ciertas implementaciones por versiones alternativas específicamente diseñadas para la prueba.

Esta familia de técnicas suele agruparse bajo el término inglés *mocking*. La idea fundamental consiste en interceptar determinadas funciones y reemplazar su comportamiento nominal por implementaciones alternativas implementadas a nuestro sabor. De esta manera, las pruebas pueden controlar explícitamente aspectos concretos del entorno de ejecución.

Hay [diversas estrategias](https://stackoverflow.com/q/65794793) para implementar este aislamiento funcional. Entre las más habituales encontramos:
- Enlace circunstancial: el principio de este método es el de enlazar las implementaciones alternativas ignorando las originales. Pese a ser conceptualmente simple, requiere diseñar los procesos de construcción prácticamente de forma artesanal. Además requiere aislar completamente las implementaciones originales en unidades de compilación.
- Envoltura de función durante el enlace: el [principio](https://drewdevault.com/blog/Using-Wl-wrap-for-mocking-in-C/) de este método es el de emplear la funciónalidad `-Wl,--wrap` de `ld`. De esta forma, redirigimos la llamada de ciertas funciones a implementaciones alternativas.
- Modificación de punteros a función: el principio de este método es el de emplear condicionalmente punteros a función en la implementación de software y alterar el puntero a lo largo de la prueba.

En lo que resta de esta sección nos centraremos en la modificación de punteros a función, ya que constituye una técnica sencilla, portable y fácilmente integrable en sistemas de construcción tradicionales.

### Modificación de punteros a función

El principio sobre el que se sustenta este método es simple. Durante la compilación del software de producción, las funciones se implementan de la forma habitual. Sin embargo, cuando construimos el perfil de pruebas, ciertas funciones dejan de exponerse como símbolos absolutos y pasan a representarse mediante punteros a función inicializados con su implementación nominal. Esta transformación introduce un punto controlado de indirección. Las pruebas pueden modificar temporalmente dichos punteros para sustituir el comportamiento original por implementaciones específicas para cada escenario. El resto del software continúa invocando la función mediante el mismo identificador, sin necesidad de conocer si se trata de una implementación fija o de una referencia modificable.

Por ejemplo, bajo el perfil de pruebas, definiríamos:

```c
#define APIDEF(RET_TYPE, FUNC_NAME, ...)               \
  RET_TYPE FUNC_NAME##_OG(__VA_ARGS__);                \
  RET_TYPE (*FUNC_NAME)(__VA_ARGS__) = FUNC_NAME##_OG; \
  RET_TYPE FUNC_NAME##_OG(__VA_ARGS__)
#define APIDEC(RET_TYPE, FUNC_NAME, ...)               \
  extern RET_TYPE (*FUNC_NAME)(__VA_ARGS__);           \
  RET_TYPE FUNC_NAME##_OG(__VA_ARGS__)
```

mientras que en el de producción:

```c
#define APIDEF(RET_TYPE, FUNC_NAME, ...) RET_TYPE FUNC_NAME(__VA_ARGS__)
#define APIDEC(RET_TYPE, FUNC_NAME, ...) RET_TYPE FUNC_NAME(__VA_ARGS__)
```

para implementar:

```c
// Cabecera.
APIDEC(void*, MallocInterceptable, size_t Sz);
// Implementación.
APIDEF(void*, MallocInterceptable, size_t Sz) {
  return malloc(Sz);
}
```

Bajo el perfil de producción, la API conserva exactamente la misma apariencia que cualquier otra función de C. Bajo el perfil de pruebas, dichas funciones pasan a representarse mediante punteros inicializados con sus implementaciones nominales.

Esto permite hacer lo siguiente en una prueba:

```c
// Implementaciones alternativas.
void* MallocQueFalla(size_t Sz) {
  (void)Sz;
  return NULL;
}
// Prueba.
MallocInterceptable = MallocQueFalla;
// [...] prueba donde malloc falla.
RESET_FN(MallocInterceptable);
```

La implementación completa que yo empleo generaliza esta idea mediante varias macros auxiliares destinadas a distinguir entre funciones públicas y privadas, así como a restaurar el comportamiento original tras cada prueba.

```c
#ifndef _FUNCTIONMACROS_H
#define _FUNCTIONMACROS_H

#ifdef TESTABLE
#define APIDEF(RET_TYPE, FUNC_NAME, ...)               \
  RET_TYPE FUNC_NAME##_OG(__VA_ARGS__);                \
  RET_TYPE (*FUNC_NAME)(__VA_ARGS__) = FUNC_NAME##_OG; \
  RET_TYPE FUNC_NAME##_OG(__VA_ARGS__)
#define APIDEC(RET_TYPE, FUNC_NAME, ...)               \
  extern RET_TYPE (*FUNC_NAME)(__VA_ARGS__);           \
  RET_TYPE FUNC_NAME##_OG(__VA_ARGS__)
#define INTDEF(RET_TYPE, FUNC_NAME, ...)               \
  RET_TYPE FUNC_NAME##_OG(__VA_ARGS__);                \
  RET_TYPE (*FUNC_NAME)(__VA_ARGS__) = FUNC_NAME##_OG; \
  RET_TYPE FUNC_NAME##_OG(__VA_ARGS__)
#define INTDEC(RET_TYPE, FUNC_NAME, ...)               \
  extern RET_TYPE (*FUNC_NAME)(__VA_ARGS__);           \
  RET_TYPE FUNC_NAME##_OG(__VA_ARGS__)
#define RESET_FN(FUNC_NAME) FUNC_NAME = FUNC_NAME##_OG
#else
#define APIDEF(RET_TYPE, FUNC_NAME, ...)               \
  RET_TYPE FUNC_NAME(__VA_ARGS__)
#define APIDEC(RET_TYPE, FUNC_NAME, ...)               \
  RET_TYPE FUNC_NAME(__VA_ARGS__)
#define INTDEF(RET_TYPE, FUNC_NAME, ...)               \
  static RET_TYPE FUNC_NAME(__VA_ARGS__)
#define INTDEC(RET_TYPE, FUNC_NAME, ...)
#define RESET_FN(FUNC_NAME)
#endif

#endif
```

Este enfoque exige cierta planificación previa. Las funciones cuyo comportamiento deseemos aislar deben definirse empleando los mecanismos de indirección correspondientes. Sin embargo, esta inversión inicial suele verse compensada por una considerable simplificación de la infraestructura de pruebas. La integración con Make resulta especialmente sencilla: basta con habilitar o deshabilitar la macro `TESTABLE` en función del perfil de construcción empleado.

Más allá de las pruebas, esta técnica pone de manifiesto una idea general: la capacidad de probar constituye también una propiedad del diseño del software. Pequeñas decisiones arquitectónicas adoptadas durante el desarrollo pueden mejorar significativamente la capacidad del sistema para ser inspeccionado, validado y mantenido a lo largo del tiempo.

## Conclusión

La infraestructura de pruebas constituye una decisión arquitectónica más del proyecto. Elegir un marco consolidado, desarrollar una solución mínima o diseñar mecanismos específicos de aislamiento funcional son decisiones que introducen compromisos distintos en términos de complejidad, mantenimiento e integración. Por este motivo, conviene adoptarlas de forma consciente, atendiendo a las necesidades reales del software, en lugar de asumirlas por simple inercia o por la popularidad de determinadas herramientas.

Por otra parte, una prueba no es únicamente un mecanismo destinado a detectar defectos. También representa una especificación ejecutable del comportamiento esperado del sistema. Describe cómo deben utilizarse sus componentes, qué propiedades se consideran relevantes y qué garantías pretende ofrecer el software. En consecuencia, la infraestructura de pruebas trasciende la mera validación: pasa a formar parte del conocimiento acumulado del proyecto.

Finalmente, la capacidad de probar un sistema rara vez aparece como una propiedad accidental. El software que no se diseña para ser observado, manipulado y aislado de manera controlada suele resultar también más difícil de comprender y mantener. La programación orientada a pruebas no consiste, por tanto, en la adopción de una herramienta concreta ni en la adhesión estricta a una metodología determinada. Consiste en incorporar la validación como una preocupación legítima durante el diseño del software. Las técnicas y herramientas pueden variar; la idea fundamental permanece: construir sistemas cuyo comportamiento pueda comprenderse, verificarse y preservarse a lo largo del tiempo.
