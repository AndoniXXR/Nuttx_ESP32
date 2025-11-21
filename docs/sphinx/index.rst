Documentación del Proyecto NuttX ESP32 Lab01
============================================

.. toctree::
   :maxdepth: 2
   :caption: Contenidos:

Introducción
------------
Esta es la documentación técnica generada automáticamente para el proyecto de portabilidad de Laboratorio 01 a NuttX en ESP32.
El proyecto consiste en una aplicación cliente/servidor TCP/UDP que se ejecuta en el microcontrolador ESP32 y se comunica con una contraparte en PC.

Componentes del Sistema
-----------------------

El sistema se divide en dos componentes principales:

1.  **Aplicación ESP32 (NuttX)**: Código que corre en el microcontrolador.
2.  **Aplicación PC (Linux)**: Código de prueba que corre en el ordenador.

Documentación de la API
=======================

Aplicación PC (lab01_pc.c)
--------------------------
Este archivo contiene la lógica para el cliente/servidor que se ejecuta en el host (Linux).

.. doxygenfile:: lab01_pc.c
   :project: NuttX ESP32 Lab01

Aplicación ESP32 (lab01_main.c)
-------------------------------
Este archivo contiene la lógica principal de la aplicación NuttX.

.. doxygenfile:: lab01_main.c
   :project: NuttX ESP32 Lab01
