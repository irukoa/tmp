---
layout: default
title: El enlazador y la interfaz binaria
---

# El enlazador y la interfaz binaria

La construcción de un programa suele dividirse en varias etapas. Habitualmente distinguimos entre el preprocesamiento, la compilación propiamente dicha y el enlace. Esta última fase suele poder deshabilitarse mediante una opción del compilador (típicamente `-c`), obteniendo como resultado un archivo objeto en lugar de un ejecutable o una biblioteca.

Durante la fase de enlace, el compilador invoca al enlazador, normalmente [`ld`](https://linux.die.net/man/1/ld), con el objetivo de resolver las referencias existentes entre los distintos archivos objeto que forman el programa. Aunque rara vez es necesario emplear el enlazador de manera directa, comprender su función permite introducir uno de los conceptos fundamentales de esta serie de artículos: la interfaz binaria.

Comencemos con un ejemplo sencillo en lenguaje C. Consideremos un módulo hipotético `MDL`, compuesto por los archivos `MDL.h` (interfaz) y `MDL.c` (implementación). Supongamos además un archivo `Usuario.c` que emplea parte de la funcionalidad definida por dicho módulo.

Tras compilar ambos archivos podemos inspeccionar sus símbolos mediante la utilidad [`nm`](https://linux.die.net/man/1/nm):
```bash
$ nm MDL.o
0000000000000000 T MDL_Func1
0000000000000290 T MDL_Func2
00000000000003c0 T MDL_Func3

$ nm Usuario.o
                 U MDL_Func1
                 U MDL_Func3
0000000000000000 T Usuario_Func1
```
La letra `T` indica que el símbolo está definido en la sección de código del archivo objeto. Por el contrario, `U` indica que el símbolo es desconocido: el archivo objeto contiene referencias a dicho símbolo, pero no proporciona una definición.

En el ejemplo anterior, `Usuario.o` emplea las funciones `MDL_Func1` y `MDL_Func3`, aunque no las implementa. Corresponde al enlazador localizar una definición adecuada de esos símbolos y asociarla con las referencias presentes en el archivo objeto. Una vez resueltas todas las referencias, el enlazador puede generar el binario final.

A primera vista podría parecer que el proceso consiste únicamente en hacer coincidir nombres de símbolos. Sin embargo, la situación es algo más sutil. El enlazador trabaja principalmente con nombres, pero para que el programa resultante funcione correctamente es necesario que exista un acuerdo acerca de cómo se representan los datos y cómo se comunican entre sí las distintas unidades compiladas.

Ese acuerdo recibe el nombre de interfaz binaria.

## Interfaz binaria

La interfaz de aplicaciones (API, por sus siglas en inglés) describe cómo debe utilizarse una biblioteca desde el punto de vista del programador. En el ejemplo anterior, el archivo `MDL.h` constituye parte de esa interfaz.

La interfaz binaria (ABI, por sus siglas en inglés), por el contrario, describe cómo se materializa esa interfaz en el código objeto generado por el compilador. Comprende aspectos tales como la representación de los tipos de datos, la alineación de estructuras, la convención de llamada de funciones, la gestión de la pila o el mecanismo empleado para devolver resultados.

Estas cuestiones suelen permanecer ocultas mientras todo el programa se compile con la misma familia de compiladores. Sin embargo, adquieren una importancia decisiva cuando intervienen bibliotecas externas, compiladores diferentes o incluso lenguajes distintos.

Resulta importante observar que el enlazador carece de conocimiento semántico sobre el programa. Puede resolver correctamente una referencia a un símbolo y, aun así, producir un ejecutable defectuoso si las unidades compiladas discrepan acerca de la representación de los datos o del protocolo de llamada empleado. La compatibilidad binaria es, por tanto, una condición necesaria para que el resultado del enlace sea correcto. Desde la perspectiva del enlazador, un programa no es una colección de archivos fuente, sino una colección de unidades compiladas que intercambian símbolos mediante una interfaz binaria común.

Los compiladores de C suelen compartir una misma interfaz binaria dentro de una plataforma determinada. Gracias a ello, resulta habitual combinar bibliotecas y archivos objeto generados por herramientas diferentes sin mayores dificultades. Aunque no constituye una garantía absoluta, esta compatibilidad es una de las razones que explican el papel central que ocupa C en la programación de sistemas.

La situación es distinta en otros lenguajes. Los compiladores disponen de mayor libertad para organizar la representación interna de los datos y la generación del código objeto. Un ejemplo clásico es Fortran. En términos generales, los compiladores de Fortran no producen archivos objeto mutuamente compatibles. Con frecuencia, ni siquiera los nombres de los símbolos pueden darse por sentados.

La interfaz binaria constituye uno de los pilares de la interoperabilidad entre bibliotecas, compiladores y lenguajes. A lo largo de esta serie de artículos volveremos sobre ella en numerosas ocasiones, especialmente en el contexto de Fortran y de la interoperabilidad entre Fortran y C.
