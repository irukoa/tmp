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
    $(DEPSCRIPT) $< $(OBJ) $(DEFS) > $@ || $(RM) $@
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
    $(DEPSCRIPT) $< $(OBJ) $(DEFS) > $@ || $(RM) $@

DIR  :=
```

Hemos delegado la determinación de dependencias en una herramienta especializada, `tools/GetCDeps.sh`, manteniendo bajo nuestro control las recetas encargadas de generar artefactos. Esta estrategia permite aprovechar el conocimiento específico del lenguaje empleado sin renunciar a una infraestructura de construcción uniforme. A continuación, examinaremos cómo materializar este enfoque en dos contextos muy distintos: el lenguaje C y el lenguaje Fortran.

Este enfoque no constituye una renuncia a Make, sino una consecuencia natural de su filosofía. Make proporciona un mecanismo general para expresar dependencias y ejecutar acciones; corresponde al desarrollador decidir cómo obtener la información necesaria para alimentar dicho mecanismo.

### El caso de C

Históricamente, el lenguaje C y Make contribuyeron conjuntamente al desarrollo de una infraestructura común para la programación de sistemas. No resulta sorprendente, por tanto, que muchos compiladores de C incorporen mecanismos para extraer automáticamente las dependencias entre archivos fuente. Nos centraremos en el compilador `gcc`. Este dispone una serie de [opciones](https://gcc.gnu.org/onlinedocs/gcc/Preprocessor-Options.html#index-M) para controlar la extracción de dependencias.

En C, las dependencias provienen de directivas `#include`. `gcc` es capaz de trazar directivas de forma recursiva, tanto para archivos incluidos desde directorios del sistema (`#include <>`) como de directorios locales (`#include ""`).

Anteriormente, propusimos la forma de emplear la utilidad de dependencias `tools/GetCDeps.sh`. La utilidad toma dos argumentos, el archivo que procesar y una ruta de directorio. Los demás argumentos controlan la compilación condicional. Una implementación mínima de esta utilidad podría ser la siguiente:

```bash
#!/bin/bash
File="$1"
Dir="$2"
shift 2
gcc -MM -MP -I. $@ $File | # Obtiene dependencias.
sed "1s|^|$Dir/|" | # Añade una ruta directorio al inicio.
sed '1s|^\([^:]*\)\.o:|\1.d \1.o:|' # Añade como objetivo el propio archivo de dependencias.
```

La última línea merece cierta atención, pues añade el propio archivo de dependencias entre los artefactos generados. Gracias a ello, cualquier modificación en la estructura de inclusiones provoca automáticamente la regeneración de la información de dependencias correspondiente. De otro modo, el sistema podría conservar información obsoleta y dejar de reflejar fielmente el estado del proyecto.

Muchos de los detalles prácticos asociados a la generación automática de dependencias fueron discutidos con gran claridad por Peter Miller en su [conocido artículo](https://accu.org/journals/overload/14/71/miller_2004/) sobre este tema. No repetiremos aquí dicho análisis; baste señalar que los principios descritos anteriormente resultan suficientes para comprender el enfoque adoptado en este trabajo.

Conviene señalar que la elección de `gcc` como extractor de dependencias no es inocente. Además de proporcionar mecanismos suficientemente maduros para esta tarea, la familia de compiladores GCC constituye una de las herramientas más ampliamente disponibles en los sistemas informáticos actuales. En consecuencia, resulta razonable apoyarse en ella incluso cuando el proyecto emplea otros compiladores para generar los artefactos finales.

### El caso de Fortran

El caso de C resulta relativamente sencillo porque las dependencias derivan exclusivamente de las directivas `#include`. Fortran presenta escenarios considerablemente más complejos y constituye una prueba más exigente para cualquier sistema de construcción. Esto pone de manifiesto una idea importante: la complejidad del sistema de construcción no siempre es consecuencia de las limitaciones de Make. Con frecuencia, refleja la complejidad inherente del problema que intentamos resolver.

Fortran dispone de una semántica más rica que C. Un módulo permite encapsular definiciones y controlar su visibilidad. Además, el lenguaje permite separar interfaz e implementación mediante submódulos y varias unidades de programa pueden coexistir en un mismo archivo fuente. Esta riqueza semántica tiene consecuencias directas sobre el proceso de compilación.

La mayoría de compiladores de Fortran generan archivos auxiliares, habitualmente con extensión `.mod`, al compilar módulos. Estos archivos contienen información necesaria para compilar otras unidades que empleen sentencias `use`. De forma análoga, los submódulos suelen inducir la generación de archivos `.smod`. Observamos, por tanto, que las dependencias en Fortran no son únicamente textuales: forman parte del propio proceso de compilación. En consecuencia, el sistema de construcción debe decidir cómo tratar estos artefactos. En este apartado consideraremos un módulo de proyecto escrito en Fortran, `src/ModuloF`, y discutiremos el diseño de su correspondiente `Reg.mk`.

#### Serialización

El método más sencillo consiste en ignorar completamente los archivos `.mod` y describir manualmente el orden de compilación entre las distintas unidades del proyecto. Este enfoque no requiere extractores de dependencias ni la consideración explícita de los archivos `.mod` y `.smod` como objetivos del sistema de construcción. El compilador se encarga de generarlos y consumirlos conforme avanza la compilación.

```make
# src/ModuloF/Reg.mk
DIR  := src/ModuloF
SRCS += $(wildcard $(DIR)/*.F90)
OBJS += $(patsubst $(DIR)/%.F90,$(OBJ)/%.o,$(wildcard $(DIR)/*.F90))

ModuloF1.o: ModuloF1.F90 | $(OBJ)
    $(FC) $(FFLAGS) $(INC) $(DEFS) -c $< -o $@

ModuloF2.o: ModuloF2.F90 ModuloF1.o | $(OBJ)
    $(FC) $(FFLAGS) $(INC) $(DEFS) -c $< -o $@

# [...]

DIR  :=
```

Este método es perfectamente funcional y, en proyectos pequeños, puede resultar suficiente. Sin embargo, obliga al desarrollador a mantener manualmente las relaciones de dependencia entre módulos. Además, al imponer un orden de compilación explícito, dificulta la explotación del paralelismo disponible durante la construcción del proyecto.

#### Archivos `.mod` como artefactos de primer orden

La idea fundamental de este método, también propuesto en [otros artículos](https://aoterodelaroza.github.io/devnotes/modern-fortran-makefiles/), consiste en dejar de considerar los archivos `.mod` y `.smod` como un detalle interno del compilador y tratarlos, en cambio, como artefactos legítimos del sistema de construcción. Desde esta perspectiva, los archivos `.mod` dejan de ser un efecto secundario de la compilación y pasan a formar parte del grafo de dependencias del proyecto. Otros archivos fuente dependen de ellos y condicionan qué unidades pueden compilarse simultáneamente. Ignorar estos artefactos equivale, en cierta medida, a ocultar parte de la estructura real del software.

Este enfoque requiere un mayor esfuerzo inicial, pero permite expresar las dependencias de forma más fiel y explotar el paralelismo inherente del proyecto. Make puede entonces razonar sobre archivos `.mod`, `.smod` y archivos objeto de forma completamente natural. La principal dificultad radica en que los compiladores de Fortran ofrecen mecanismos heterogéneos para gestionar estos artefactos. Algunos proporcionan opciones específicas para generar únicamente archivos `.mod`, mientras que otros no disponen de una funcionalidad equivalente. Por ejemplo, `gfortran` dispone [`-fsyntax-only`](https://gcc.gnu.org/onlinedocs/gfortran/Error-and-Warning-Options.html#index-fsyntax-only). Una estrategia razonablemente portable consiste en compilar normalmente y descartar el archivo objeto generado cuando únicamente deseemos obtener los archivos `.mod`. De forma análoga, podemos generar archivos objeto y descartar posteriormente los correspondientes `.mod`. Este planteamiento exige conocer adecuadamente las opciones de gestión de directorios del compilador empleado. Por ejemplo, `gfortran` emplea `-I` para localizar archivos `.mod`, mientras que la opción [`-J`](https://gcc.gnu.org/onlinedocs/gfortran/Directory-Options.html#index-Jdir) permite tanto especificar el directorio donde generarlos como incorporarlo automáticamente a la ruta de búsqueda.

Los extractores de dependencias desempeñan aquí un papel fundamental. Su función consiste en identificar las relaciones entre archivos fuente y artefactos `.mod` y `.smod`, permitiendo que Make reconstruya el grafo de dependencias del proyecto. Una vez disponible esta información, el sistema de construcción puede determinar qué artefactos deben generarse y cuáles pueden compilarse en paralelo. Lamentablemente, no existe una herramienta dominante para la extracción de dependencias en Fortran. Existen [diversas alternativas](https://fortran-lang.discourse.group/t/automatic-make-recipes-generation/10570/5), algunas estrechamente ligadas a compiladores concretos y otras con un mantenimiento limitado. Ante esta situación, desarrollé [`FortranDep`](https://github.com/irukoa/FortranDep) con el propósito de disponer de un extractor sencillo y portable, inspirado en la filosofía de `gcc`: una herramienta pequeña, fácil de integrar y centrada exclusivamente en la generación de dependencias. Es la utilidad que empleo habitualmente en mis propios proyectos. Tiendo a descargarlo en la carpeta `tools/` del proyecto y olvidarme de que está ahí. Esto permite construir archivos `Reg.mk` considerablemente más sencillos de mantener:

```make
# src/ModuloF/Reg.mk
DIR  := src/ModuloF
SRCS += $(wildcard $(DIR)/*.F90)
OBJS += $(patsubst $(DIR)/%.F90,$(OBJ)/%.o,$(wildcard $(DIR)/*.F90))

# Nota: empleamos las opciones de directorios de gfortran.
$(OBJ)/%.o: | $(OBJ) $(DSC)
    $(FC) $(FFLAGS) -J$(DSC) -I$(OBJ) $(INC) $(DEFS) -c $< -o $@
    $(RM) $(DSC)/*

$(OBJ)/%.d: $(DIR)/%.F90 | $(OBJ)
    $(FDEPSCRIPT) $< $(OBJ) $(DEFS) > $@ || $(RM) $@

$(OBJ)/%.mod $(OBJ)/%.smod &: | $(OBJ) $(DSC)
    $(RM) $(OBJ)/$*.mod $(OBJ)/$*.smod
    $(FC) $(FFLAGS) -J$(OBJ) -I$(OBJ) $(INC) $(DEFS) -c $< -o $(DSC)/$@
    $(RM) $(DSC)/*

DIR  :=
```

La variable `$(FDEPSCRIPT)` desempeña un papel análogo a `$(DEPSCRIPT)` en el caso de C. Generamos los archivos objeto de la forma habitual y descartamos los archivos `.mod` producidos durante dicho proceso. Por otro lado, generamos los archivos `.mod` y `.smod`, descartando los correspondientes archivos objeto. Las dependencias obtenidas mediante el extractor se incorporan posteriormente al sistema de construcción mediante la directiva `include`, análogamente al caso de C. Además, la [regla múltiple](https://www.gnu.org/software/make/manual/html_node/Multiple-Targets.html) empleada para los archivos `.mod` y `.smod` informa explícitamente a Make de que ambos artefactos se generan simultáneamente. Este detalle evita reconstrucciones innecesarias y refleja con mayor fidelidad el comportamiento del compilador.

En última instancia, este enfoque permite trasladar al caso de Fortran la misma filosofía empleada anteriormente en C: separar la generación de artefactos de la determinación de dependencias. La diferencia es que, en Fortran, dichas dependencias forman parte de la propia semántica del lenguaje. El sistema de construcción no puede ignorarlas; debe representarlas adecuadamente.

Finalmente, recomiendo crear una utilidad análoga a la de C. Yo suelo hacer `FDEPSCRIPT := ./tools/GetFDeps.sh`, donde una implementación mínima es

```bash
#!/bin/bash
File="$1"
Dir="$2"
shift 2
./tools/FortranDep -d $File $@ | # Obtiene dependencias.
sed -E "s@(^|[[:space:]])([^[:space:]]+\.(o|d|mod|smod))@\1$Dir/\2@g" | # Añade una ruta directorio al inicio.
sed -E 's@^([^:]+)\.mod:@\1.mod \1.smod:@' # Añade una entrada `.smod`.
```

## Perfiles

Gran parte de los sistemas de software actuales distinguen habitualmente dos perfiles con los que trabajar, el de depuración y prueba, y el de producción y distribución. Aunque cada proyecto establece sus propios requisitos, resulta habitual distinguir entre un perfil orientado al desarrollo y otro orientado a la distribución del software. En esta sección trataremos sobre el papel que Make juega en cada uno de los perfiles, asi como sus características mas transversales.

### Depuración y pruebas

Este es un perfil adaptado al desarrollador. Se caracteriza por compilar con símbolos de depuración, asi como con métricas de cobertura. Esto habilita un control total sobre el código. Habitualmente, esta clase de perfiles incorporan una infraestructura de pruebas que permite validar el comportamiento del software y detectar regresiones durante el desarrollo.

En este contexto, Make actúa como orquestador del proceso de validación. Además de compilar el proyecto, puede encargarse de construir las herramientas auxiliares necesarias, ejecutar las pruebas y recopilar métricas asociadas al proceso de validación. En [otro artículo](https://irukoa.github.io/ApuntesInfraestructuraSoftware/Articulos/ProgramacionOrientadaAPruebas.html) tratamos mas a fondo este concepto.

### Producción y distribución

Este es un perfil adaptado al consumidor. Aquí tiende a haber demasiada variedad como para identificar un denominador comun mas allá del siguiente contrato: el perfil encapsula el proceso de compilación del software. En este sentido, diseñamos una capa de abstracción sobre el código fuente y tratamos de convertir la construcción en una interfaz estable. Idealmente, la construcción completa del software debería poder iniciarse mediante una interfaz estable y sencilla, delegando en Make la ejecución de los pasos necesarios.

En este contexto, Make actúa como la implementación del proceso de construcción, mientras que el perfil constituye la interfaz expuesta al consumidor del software.

---

Los perfiles ponen de manifiesto que el sistema de construcción no es un mera nota en el proyecto. Constituye una parte más de su diseño y debe responder a las necesidades de quienes interactúan con el software en distintas etapas de su ciclo de vida.

## Conclusión

Make obliga al programador a hacer explícitas ciertas relaciones que otras herramientas tienden a ocultar. Esta explicitud tiene un coste: exige disciplina y comprensión del proceso de construcción. Sin embargo, también proporciona una ventaja significativa. El sistema de compilación deja de ser una caja negra y pasa a convertirse en una característica más del software, susceptible de ser razonada, revisada y mantenida con el mismo cuidado que el resto del código.

Quizá la principal enseñanza que ofrece Make sea que la construcción del software forma parte del propio software. Las dependencias entre componentes, la generación incremental de artefactos o la organización de los distintos perfiles de trabajo no son preocupaciones accesorias: condicionan la forma en que desarrollamos, validamos y distribuimos nuestros programas. Comprender estos mecanismos conserva su valor incluso cuando decidimos delegarlos en herramientas más sofisticadas.

La cuestión no consiste en determinar si Make es superior o inferior a otros sistemas de construcción. La cuestión consiste en comprender qué problemas resuelve adecuadamente y qué enseñanzas proporciona incluso cuando finalmente optamos por herramientas de mayor nivel.
