# Proyecto NuttX ESP32 - Laboratorio 01

Este repositorio contiene el port del "Laboratorio 01" para el sistema operativo en tiempo real NuttX ejecutándose en un microcontrolador ESP32.

## Descripción

El objetivo principal de este proyecto es establecer comunicación de red (TCP/UDP) entre un ESP32 corriendo NuttX y una PC. El proyecto incluye tanto el código fuente del sistema operativo NuttX y sus aplicaciones, como el código del lado del PC para realizar las pruebas de comunicación.

## Estructura del Repositorio

*   **`nuttx/`**: Código fuente del kernel de NuttX y el sistema operativo base.
*   **`apps/`**: Aplicaciones de usuario, ejemplos y comandos del sistema (Nsh, etc.).
*   **`lab01_pc.c`**: Código fuente de la aplicación para PC (Linux) que actúa como contraparte (Cliente/Servidor) para probar la comunicación con el ESP32.

## Características

*   **Soporte ESP32**: Configurado para ejecutarse en el SoC ESP32.
*   **Networking**: Pila de red habilitada para comunicación TCP/IP.
*   **Modo Cliente/Servidor**: Capacidad para actuar como cliente o servidor TCP/UDP.
*   **Comando EXIT**: Implementación de un mecanismo de cierre limpio de conexiones mediante el comando "EXIT".

## Instrucciones de Uso

### Requisitos Previos

*   Toolchain de Espressif para ESP32.
*   Herramientas de construcción de NuttX (`kconfig-frontends`, etc.).
*   `esptool` para flashear el firmware.

### Compilación y Flasheo

1.  Configurar el entorno de NuttX.
2.  Compilar el proyecto:
    ```bash
    cd nuttx
    make
    ```
3.  Flashear el binario en el ESP32:
    ```bash
    esptool.py -p /dev/ttyUSB0 -b 460800 write_flash 0x1000 nuttx.bin
    ```

### Ejecución de la Contraparte en PC

Para compilar y ejecutar el programa de prueba en el PC:

```bash
gcc lab01_pc.c -o lab01_pc
./lab01_pc <modo> <protocolo> <puerto> [ip]
```
*   **Modo**: `client` o `server`.
*   **Protocolo**: `tcp` o `udp`.

## Comandos Disponibles en NuttX (NSH)

Una vez que el sistema arranca y accedes a la consola NSH (NuttShell) a través del puerto serie, puedes utilizar los siguientes comandos:

### Comandos Generales del Sistema
*   `help`: Muestra la lista completa de comandos disponibles y aplicaciones registradas.
*   `ls`: Lista archivos y directorios.
*   `ps`: Muestra los procesos en ejecución y su uso de memoria.
*   `free`: Muestra el estado de la memoria (total, usada, libre).
*   `reboot`: Reinicia el sistema.

### Comandos de Red
*   `ifconfig`: Muestra o configura las interfaces de red (dirección IP, máscara, gateway).
*   `ping <ip>`: Verifica la conectividad con otro host en la red.
*   `renew <interfaz>`: Renueva la dirección IP si se usa DHCP.

### Comando de la Aplicación (Laboratorio 01)
*   `lab01 <client|server> <tcp|udp> <puerto> [ip_servidor]`
    *   **Descripción**: Inicia la aplicación principal del laboratorio en modo cliente o servidor.
    *   **Ejemplos**:
        *   Iniciar servidor TCP en puerto 3001:
            `nsh> lab01 server tcp 3001`
        *   Iniciar cliente TCP conectando a 192.168.1.50:
            `nsh> lab01 client tcp 3001 192.168.1.50`

## Notas Importantes

*   Este repositorio ha sido optimizado para excluir archivos de historial de git excesivamente grandes, asegurando una descarga ligera y funcional.
*   Se han excluido archivos de documentación local temporal para mantener el repositorio limpio.
