#set page(
  paper: "a4",
  numbering: "1"
)

#show title: set align(center)
#show title: set block(below: 1.2em)

#title[
    Sistema de Control de Cámara de Termovacío
]

#grid(
    columns: (1fr, 1fr, 1fr, 1fr),
    align(center)[
        Franco Berni \
        #link("mailto:fberni@fi.uba.ar") \
        110007
    ],
    align(center)[
        Lautaro García V. \
        #link("mailto:lagarciav@fi.uba.ar") \
        110028
    ],
    align(center)[
        Juan Ignacio Giachetti \
        #link("mailto:jgiachetti@fi.uba.ar") \
        110877
    ],
    align(center)[
        Manuel Hirsch \
        #link("mailto:mhirsch@fi.uba.ar") \
        110221
    ],
)

#set heading(numbering: "1.")

#set par(justify: true)
#set text(size: 11pt)

= Descripción del proyecto

== Descripción general

El proyecto a desarrollar consiste en la realización del producto mı́nimo viable de un sistema de control para una cámara de termovacı́o. El sistema permitirá monitorear y controlar automáticamente la temperatura y la presión dentro de una cámara. El sistema será controlado a través de una interfaz de usuario que permita su configuración y operación de forma intuitiva.

== Componentes del sistema

El sistema estará basado en un microcontrolador STM32, e incluirá un display LCD para la visualización de los parámetros y menús interactivos. La interfaz de usuario se controlará por medio de botones y switches. Se incluirá además un buzzer para la señalización de alarmas.

El sistema contará con sensores de presión y temperatura, los cuales serán simulados en el MVP a través de potenciómetros que generen señales analógicas similares a las proporcionadas por los sensores reales. Estas serán medidas por el ADC del microcontrolador. Los actuadores del sistema serán un calentador y un sistema de refrigeración para el control de temperatura, y una bomba de vacı́o para el control de la presión. Los mismos serán simulados por LEDs indicadores que se encenderán cuando deba encenderse el correspondiente actuador.

Se utilizará memoria no volátil (EEPROM u otra) para guardar la configuración de forma persistente. Se utilizará algún protocolo de comunicación (como I2C) para su lectura y escritura.

== Modos de operación

El sistema embebido tendrá dos modos de operación principales. En el modo normal, se realizará el monitoreo continuo de temperatura y presión, con una visualización en tiempo real en el LCD. Se ejecutará el control automático de los actuadores de temperatura y presión, a través de un controlador con histéresis.

El modo de setup permitirá que el usuario configure los setpoints de temperatura y presión deseados, así como los límites de la alarma. Las configuraciones establecidas serán guardadas en memoria no volátil para persistir entre reinicios.

== Funcionalidades principales

El sistema implementará el control de temperatura mediante la lectura continua del sensor simulado y controlará el encendido y apagado del calentador o el sistema de refrigeración, contemplando la histéresis. Similarmente, el control de presión monitoreará continuamente la presión en la cámara y activará la bomba de vacío según el setpoint. En caso de que los parámetros superen los lı́mites establecidos en la configuración, se activarán las alarmas correspondientes.

La interfaz de usuario proporcionará información en tiempo real a través del display LCD y permitirá navegar por menús mediante botones, con un guardado de la configuración de los parámetros en memoria externa no volátil. El sistema de alarmas será acústico.

// vim: ts=4 sts=4 sw=4 et lbr
