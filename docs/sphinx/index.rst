Documentación del Proyecto NuttX ESP32 Lab01
============================================

.. toctree::
   :maxdepth: 2
   :caption: Contenidos:

Introducción
------------
Esta es la documentación técnica generada automáticamente para el proyecto de portabilidad de Laboratorio 01 a NuttX en ESP32.
El proyecto consiste en una aplicación cliente/servidor TCP/UDP que se ejecuta en el microcontrolador ESP32 y se comunica con una contraparte en PC.

Descripción del Proyecto
------------------------
El objetivo de este laboratorio es demostrar la capacidad del sistema operativo NuttX ejecutándose en un SoC ESP32 para manejar comunicaciones de red estándar (TCP/IP).

Se ha implementado una aplicación llamada ``lab01`` que puede funcionar en dos modos:
*   **Modo Servidor**: Escucha peticiones de cálculo matemático simple (suma, resta, multiplicación, división, módulo) y devuelve el resultado.
*   **Modo Cliente**: Permite al usuario ingresar operaciones matemáticas y enviarlas a un servidor remoto.

Características Implementadas
-----------------------------
1.  **Soporte Dual TCP/UDP**: La aplicación puede configurarse en tiempo de ejecución para usar cualquiera de los dos protocolos de transporte.
2.  **Calculadora Remota**: El protocolo de aplicación es texto plano simple. El cliente envía una operación (ej. ``5+3``) y el servidor responde con el resultado (ej. ``8``).
3.  **Comando EXIT**: Se implementó un mecanismo de cierre limpio. Si el cliente envía la cadena ``EXIT``, el servidor cierra la conexión actual (en TCP) o termina su ejecución (si así se desea), y el cliente termina su proceso.
4.  **Logging**: Ambas partes (Cliente y Servidor) registran en consola los mensajes enviados y recibidos con marcas de tiempo.

Guía de Uso
-----------

Comandos en ESP32 (NuttShell)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Una vez que el sistema NuttX ha arrancado y tienes acceso a la consola NSH a través del puerto serie:

1.  **Verificar Red**: Asegúrate de tener IP.
    
    .. code-block:: bash

       nsh> ifconfig

2.  **Ejecutar como Servidor**:
    
    .. code-block:: bash

       # Iniciar servidor TCP en puerto 3001
       nsh> lab01 server tcp 3001

3.  **Ejecutar como Cliente**:
    
    .. code-block:: bash

       # Conectar a un servidor en PC (ej. 192.168.1.50)
       nsh> lab01 client tcp 3001 192.168.1.50

Comandos en PC (Linux)
~~~~~~~~~~~~~~~~~~~~~~
Primero compila la aplicación de contraparte:

.. code-block:: bash

   gcc lab01_pc.c -o lab01_pc

1.  **Ejecutar como Servidor** (para probar el Cliente ESP32):

    .. code-block:: bash

       ./lab01_pc server tcp 3001

2.  **Ejecutar como Cliente** (para probar el Servidor ESP32):

    .. code-block:: bash

       ./lab01_pc client tcp 3001 <IP_DEL_ESP32>

Detalles de Implementación
==========================

Estructura de Argumentos
------------------------
Tanto la aplicación de NuttX como la de PC utilizan una estructura común para manejar los argumentos de línea de comandos:

.. code-block:: c

   struct args_s
   {
     char *protocol;  /* "TCP" o "UDP" */
     char *server_ip; /* Dirección IP del servidor */
     int port;        /* Puerto de escucha o conexión */
     char *mode;      /* "client" o "server" */
   };

Lógica del Servidor
-------------------
El servidor crea un socket (STREAM para TCP, DGRAM para UDP), lo vincula al puerto especificado y entra en un bucle infinito.
*   En **TCP**, acepta una conexión, procesa mensajes en un bucle interno hasta recibir ``EXIT`` o error, y luego cierra el socket cliente para volver a aceptar otro.
*   En **UDP**, recibe paquetes individuales y responde a la dirección de origen.

Lógica del Cliente
------------------
El cliente lee entradas del usuario desde ``stdin``.
*   Envía la cadena ingresada al servidor.
*   Espera la respuesta (bloqueante).
*   Imprime la respuesta.
*   Si el usuario escribe ``EXIT``, envía el comando para notificar al servidor y termina la ejecución.

Documentación de la API (Código Fuente)
=======================================

Aplicación PC (lab01_pc.c)
--------------------------
Este archivo contiene la lógica para el cliente/servidor que se ejecuta en el host (Linux).

.. doxygenfile:: lab01_pc.c
   :project: NuttX ESP32 Lab01

Aplicación ESP32 (lab01_main.c)
-------------------------------
Este archivo contiene la lógica principal de la aplicación NuttX.
Se registra como una aplicación del sistema en NuttX y se invoca desde NSH.

.. doxygenfile:: lab01_main.c
   :project: NuttX ESP32 Lab01
