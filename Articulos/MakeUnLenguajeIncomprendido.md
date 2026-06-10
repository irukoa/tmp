---
layout: default
title: Make, un lenguaje incomprendido
---

# Make, un lenguaje incomprendido

[Make](https://www.gnu.org/software/make/) es un lenguaje para describir relaciones de dependencia y las acciones necesarias para satisfacerlas. Aunque suele asociarse con la compilación de programas, su ámbito de aplicación es considerablemente más amplio. Durante décadas ha formado parte de la infraestructura habitual de los sistemas tipo Unix y continúa presente, por defecto, en una gran cantidad de entornos de desarrollo.

Sin embargo, Make goza de una reputación desigual. Para algunos desarrolladores constituye una herramienta sobria, fiable y extraordinariamente flexible. Para otros, representa una reliquia obsoleta: un lenguaje envejecido cuyos archivos de configuración, los denominados *Makefiles*, resultan difíciles de comprender y costosos de mantener.

Estas críticas no son enteramente infundadas. No es difícil encontrar proyectos cuyos *Makefiles* han crecido sin disciplina alguna hasta convertirse en un entramado opaco y frágil. Tampoco puede ignorarse que el desarrollo de software contemporáneo plantea exigencias que Make no siempre satisface con comodidad, especialmente en contextos marcadamente multiplataforma o cuando resulta necesario integrar un número elevado de herramientas heterogéneas.

En respuesta a estas dificultades han surgido, a lo largo del tiempo, diversos sistemas de construcción, tales como CMake, Meson o Bazel. Estas herramientas introducen distintos niveles de abstracción sobre el proceso de compilación con el propósito de simplificar tareas frecuentes, mejorar la portabilidad o proporcionar mecanismos adicionales de organización.

Nada de esto convierte a Make en una herramienta obsoleta.

---

Con cierta frecuencia, la discusión en torno a Make adopta la forma de una falsa dicotomía: o bien se considera un vestigio histórico que conviene evitar siempre que sea posible, o bien se le atribuyen virtudes casi universales. Ninguna de estas posiciones resulta particularmente útil. Como ocurre con tantas otras herramientas de programación, la cuestión relevante no consiste en determinar si Make es superior o inferior a sus alternativas, sino en comprender cuáles son sus capacidades, cuáles son sus limitaciones y en qué circunstancias constituye una elección razonable.

Este artículo articula una idea modesta: Make es un sistema perfectamente válido para una amplia variedad de proyectos, siempre que se emplee con disciplina y se comprendan adecuadamente los principios sobre los que se sustenta. De hecho, el conocimiento de dichos principios proporciona competencias que trascienden al propio Make. La comprensión explícita de las dependencias entre componentes, la reconstrucción incremental de artefactos, la composición de herramientas sencillas o la automatización reproducible de tareas son conceptos que conservan su relevancia independientemente del sistema de construcción finalmente adoptado.

Por otra parte, muchas de las críticas dirigidas hacia Make se refieren, en realidad, a determinados estilos de escritura de *Makefiles* más que al lenguaje en sí mismo. Un *Makefile* desorganizado puede resultar tan dañino como cualquier otro artefacto software mal diseñado. Del mismo modo, un uso disciplinado de Make puede dar lugar a sistemas de construcción sorprendentemente robustos, expresivos y duraderos.

No pretendemos argumentar que Make sea la solución adecuada para cualquier problema ni que los sistemas alternativos carezcan de justificación técnica. Existen escenarios en los que herramientas de mayor nivel ofrecen ventajas evidentes. Tampoco defenderemos que todo proyecto deba renunciar a ellas en favor de un retorno a planteamientos más tradicionales.

El propósito de este artículo es otro. Trataremos de exponer una interpretación de Make que permita comprender qué problemas resuelve realmente, cuáles son los mecanismos que pone a disposición del programador y por qué continúa ocupando un lugar destacado dentro de la infraestructura del software contemporáneo. A partir de esa exposición, corresponderá al lector juzgar si Make se adecua a sus necesidades o si, por el contrario, su proceso de construcción requiere una capa adicional de abstracción.

Las afirmaciones anteriores resultarían poco convincentes si permaneciesen en el terreno de la abstracción. Por este motivo, el resto del artículo se articula en torno a un ejemplo concreto: el diseño progresivo de un sistema de construcción basado en Make. Nuestro objetivo no es presentar una receta universal, sino ilustrar una serie de principios que, en nuestra opinión, permiten emplear Make de forma sostenible.

## Diseño de un *Makefile* modular

El sistema de construcción que diseñamos en esta sección tiene unos requisitos sencillos. Debe brindarle a Make todos los artefactos que construir, y debe expresar las interdependencias entre artefactos correctamente. El objetivo de este ejemplo no es presentar una receta universal ni una única forma correcta de emplear Make. Pretendemos, más modestamente, ilustrar una serie de principios de diseño que, a nuestro juicio, permiten construir sistemas de compilación sostenibles y relativamente sencillos de mantener.

Comenzamos por diseñar una estructura de directorios. En general, y con cierta independencia del lenguaje empleado en el proyecto, almacenaremos el código fuente en un directorio `src/`. Esta carpeta podrá contener subdirectorios, tales como `src/ModuloA/`, etc. Las carpetas contendrán archivos de código fuente que compilaremos.

Make dispone de la directiva [`include`](https://www.gnu.org/software/make/manual/html_node/Include.html). Al leer esta directiva, Make incluirá los archivos especificados, de forma análoga a `#include` en el caso del preprocesador de C. Make dispone de un mecanismo para clasificar la ausencia de un archivo incluido: al determinar que el archivo no existe, Make procesara el *Makefile* completo en busca de una regla para crear ese archivo. En caso de encontrar una regla, Make la ejecutará, y volverá a procesar el *Makefile*, esta vez incluyendo el archivo. En caso de no encontrar una regla, reportará un error. Este hecho será de importancia capital a la hora de determinar de forma programática las dependencias de los archivos de código fuente. Por el momento, la directiva `include` nos permitirá extender Make a subdirectorios.

Personalmente, me gusta crear un archivo de registro de módulos que luego incluyo en el *Makefile* raíz:

```make
# RegistroDeModulos.mkr
include src/ModuloA/Reg.mk
# [...]
```

A su vez, los archivos `src/**/Reg.mk` disponen las reglas de compilación e interdependencias de los artefactos del módulo particular.

Supondremos ahora, por mera simplicidad, que el proyecto compila código fuente de C y crea un binario ejecutable a partir de `main.c`:

```bash
$ tree
.
├── main.c
├── Makefile
├── RegistroDeModulos.mkr
└── src
    ├── ModuloA
    │   ├── ModuloA1.c
    │   ├── ModuloA2.c
    │   ├── [...]
    │   ├── ModuloA.h
    │   └── Reg.mk
    └── [...]
```

Centralizaremos la generación de archivos objeto, así como la de ejecutables en un único directorio, respectivamente. El siguiente *Makefile* no constituye todavía una solución completa. Su propósito es ilustrar cómo distribuiremos las responsabilidades dentro del sistema de construcción. En particular, observaremos que:
- la configuración global del compilador permanece centralizada.
- La determinación de artefactos se delega a los módulos.
- La composición del sistema se realiza mediante inclusiones sucesivas.

```make
# Cabecera
MKDIR := mkdir -p
CP    := cp
MV    := mv
RM    := rm -rf

CC     := gcc
INC    := -I./
DEFS   :=
CFLAGS := -g -Wall -O0 --coverage

LDFLAGS :=
LDLIBS  :=

# Entorno
OBJ := obj
BIN := bin
EXE := $(BIN)/Ejecutable.x

# Reglas
.PHONY: main clean
.DEFAULT_GOAL := main
ifeq ($(MAKECMDGOALS), )
override MAKECMDGOALS := main
endif

# Grafo de dependencias (delegado)
SRCS :=
OBJS :=
include RegistroDeModulos.mkr

# Reglas para main.c
main: $(EXE)
# [...] Regla para main.o
$(EXE): $(OBJ)/main.o $(OBJS) | $(BIN)
    $(CC) $(CFLAGS) $(INC) $(DEFS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

$(OBJ) $(BIN):
    $(MKDIR) $@

clean::
    $(RM) $(OBJ) $(BIN)
```

Pese a la incompletitud de este *Makefile*, ya hemos conseguido algo valioso: separar las decisiones globales de las locales. Las opciones del compilador, los directorios de trabajo y los objetivos principales permanecen centralizados, mientras que cada módulo describe únicamente los artefactos que aporta al proyecto. Este modelo híbrido resulta fácilmente extensible y permite incorporar nuevos componentes sin alterar significativamente la estructura general del sistema de construcción. Más importante aún, obliga a hacer explícita la organización lógica del software.

## Extractores de dependencias

Aun queda por determinar cual es la estructura de los archivos `Reg.mk`. En esencia, nos enfrentamos a dos problemas distintos. Por un lado, debemos especificar cómo se construye cada artefacto. Por otro, debemos expresar correctamente las relaciones de dependencia entre ellos. Conviene tratar ambas cuestiones de forma separada. La opción mas sencilla es la de listar, artesanalmente, todos los artefactos y sus dependencias:

```make
# src/ModuloA/Reg.mk
DIR  := src/ModuloA
SRCS += $(wildcard $(DIR)/*.c)
OBJS += $(patsubst $(DIR)/%.c,$(OBJ)/%.o,$(wildcard $(DIR)/*.c))

$(OBJ)/ModuloA1.o: $(DIR)/ModuloA1.c $(DIR)/ModuloA.h # [...]
$(OBJ)/ModuloA2.o: $(DIR)/ModuloA2.c $(DIR)/ModuloA.h # [...]
# [...]

$(OBJ)/%.o: | $(OBJ)
    $(CC) $(CFLAGS) $(INC) $(DEFS) -c $< -o $@
# Nota: $< traza adecuadamente el primer requisito pese a especificarse en otra regla.

DIR  :=
```

Este método, pese a ser el mas sencillo, también es el mas laborioso. El problema no es dar las recetas para crear los artefactos, si no tener que mantener el registro de las interdependencias para cada uno de los artefactos. En esta sección propondremos un método ligeramente mas sofisticado, el cual se basa en extractores de dependencias. Gracias a este método, solventaremos esta clase de problemas.

Un extractor de dependencias genera reglas tales como

```make
artefacto1: dependencia1A dependencia1B ...
```

nótese que en ningún caso determinan las recetas para compilar artefactos, solo sus dependencias. Determinar qué depende de qué y determinar cómo se construye cada artefacto son problemas conceptualmente distintos. Los extractores de dependencias permiten que el sistema de construcción trate ambas cuestiones de manera independiente. Esta separación simplifica el mantenimiento del *Makefile*.

Previamente a ejecutar una receta para compilar un artefacto, Make combina todas sus dependencias. Esa es la gran utilidad de los extractores de dependencias: nos permiten separar, por un lado, la gestión de dependencias y por el otro, la generación de artefactos. Por lo tanto, podemos generalizar el *Makefile* que diseñamos para integrar la gestión de dependencias. El patrón de uso es sencillo: diseñaremos una herramienta, `tools/GetCDeps.sh`, que tome como entrada un archivo fuente y devuelva un archivo de texto en lenguaje Make listando sus dependencias. Después, incluiremos todos los archivos en el *Makefile*, de forma que Make disponga de toda la información necesaria para la creación de artefactos. Modificamos el *Makefile*:

```make
# Cabecera
MKDIR := mkdir -p
CP    := cp
MV    := mv
RM    := rm -rf

# Gestión de dependencias
DEPSCRIPT := ./tools/GetCDeps.sh

CC     := gcc
INC    := -I./
DEFS   :=
CFLAGS := -g -Wall -O0 --coverage

LDFLAGS :=
LDLIBS  :=

# Entorno
OBJ := obj
BIN := bin
EXE := $(BIN)/Ejecutable.x

# Reglas
.PHONY: main clean
.DEFAULT_GOAL := main
ifeq ($(MAKECMDGOALS), )
override MAKECMDGOALS := main
endif

# Grafo de dependencias (delegado)
SRCS :=
OBJS :=
include RegistroDeModulos.mkr
# Inclusión de dependencias
DEPS := $(patsubst $(OBJ)/%.o,$(OBJ)/%.d,$(OBJS))
ifneq ($(filter-out clean, $(MAKECMDGOALS)), )
include $(DEPS)
endif

# Reglas para main.c
main: $(EXE)
$(OBJ)/main.o: main.c | $(OBJ)
    $(CC) $(CFLAGS) $(INC) $(DEFS) -c $< -o $@
$(OBJ)/main.d: main.c | $(OBJ)
    $(DEPSCRIPT) $< $(OBJ) > $@ || $(RM) $@
ifneq ($(filter-out clean, $(MAKECMDGOALS)), )
include $(OBJ)/main.d
endif
$(EXE): $(OBJ)/main.o $(OBJS) | $(BIN)
    $(CC) $(CFLAGS) $(INC) $(DEFS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

$(OBJ) $(BIN):
    $(MKDIR) $@

clean::
    $(RM) $(OBJ) $(BIN)
```

Y los archivos `Reg.mk`:

```make
# src/ModuloA/Reg.mk
DIR  := src/ModuloA
SRCS += $(wildcard $(DIR)/*.c)
OBJS += $(patsubst $(DIR)/%.c,$(OBJ)/%.o,$(wildcard $(DIR)/*.c))

$(OBJ)/%.o: | $(OBJ)
    $(CC) $(CFLAGS) $(INC) $(DEFS) -c $< -o $@
# Nota: $< traza adecuadamente el primer requisito pese a especificarse en otra regla. En este caso el archivo de dependencias.

$(OBJ)/%.d: $(DIR)/%.c | $(OBJ)
    $(DEPSCRIPT) $< $(OBJ) > $@ || $(RM) $@

DIR  :=
```

Hemos delegado la determinación de dependencias en una herramienta especializada, `tools/GetCDeps.sh`, manteniendo bajo nuestro control las recetas encargadas de generar artefactos. Esta estrategia permite aprovechar el conocimiento específico del lenguaje empleado sin renunciar a una infraestructura de construcción uniforme. A continuación, examinaremos cómo materializar este enfoque en dos contextos muy distintos: el lenguaje C y el lenguaje Fortran.

Este enfoque no constituye una renuncia a Make, sino una consecuencia natural de su filosofía. Make proporciona un mecanismo general para expresar dependencias y ejecutar acciones; corresponde al desarrollador decidir cómo obtener la información necesaria para alimentar dicho mecanismo.

### El caso de C

POR REDACTAR

### El caso de Fortran

El caso de Fortran resulta especialmente interesante porque pone de manifiesto una idea importante: la complejidad del sistema de construcción no siempre es consecuencia de las limitaciones de Make. Con frecuencia, refleja la complejidad inherente al problema que intentamos resolver.

POR REDACTAR

## Integración de artefactos de terceros

Los proyectos reales rara vez se limitan al código desarrollado dentro del propio repositorio. Bibliotecas externas, generadores de código o herramientas auxiliares forman parte habitual del proceso de construcción. Conviene, por tanto, disponer de mecanismos que permitan integrar estos artefactos sin comprometer la claridad del sistema global.

POR REDACTAR

## Perfiles

POR REDACTAR

### Depuración y pruebas

POR REDACTAR

### Producción y distribución

POR REDACTAR

## Conclusión

Make obliga al programador a hacer explícitas ciertas relaciones que otras herramientas tienden a ocultar. Esta explicitud tiene un coste: exige disciplina y comprensión del proceso de construcción. Sin embargo, también proporciona una ventaja significativa. El sistema de compilación deja de ser una caja negra y pasa a convertirse en una característica más del software, susceptible de ser razonada, revisada y mantenida con el mismo cuidado que el resto del código.

La cuestión no consiste en determinar si Make es superior o inferior a otros sistemas de construcción. La cuestión consiste en comprender qué problemas resuelve adecuadamente y qué enseñanzas proporciona incluso cuando finalmente optamos por herramientas de mayor nivel.
