/**
 * @file lab01_main.c
 * @brief Aplicación principal para NuttX (ESP32) del Laboratorio 01.
 *
 * Este programa implementa un cliente/servidor TCP/UDP que corre sobre NuttX.
 * Permite realizar operaciones matemáticas simples enviadas desde un cliente
 * o procesarlas si actúa como servidor.
 *
 * @author AndoniXXR
 * @date 2025
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <errno.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/** @brief Tamaño del buffer para envío y recepción de mensajes. */
#define BUFFER_SIZE 1024

/****************************************************************************
 * Private Types
 ****************************************************************************/

/**
 * @struct args_s
 * @brief Estructura para almacenar los argumentos de línea de comandos en NuttX.
 */
struct args_s
{
  char *protocol;  /**< Protocolo a utilizar: "TCP" o "UDP". */
  char *server_ip; /**< Dirección IP del servidor al que conectarse (modo cliente). */
  int port;        /**< Puerto de conexión o escucha. */
  char *mode;      /**< Modo de operación: "client" o "server". */
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/**
 * @brief Obtiene la fecha y hora actual formateada.
 *
 * @param buffer Buffer donde se escribirá la cadena de tiempo.
 * @param size Tamaño del buffer.
 */
static void get_timestamp(char *buffer, size_t size)
{
  time_t now;
  struct tm *tm_info;

  time(&now);
  tm_info = localtime(&now);

  strftime(buffer, size, "%Y/%m/%d %H:%M:%S", tm_info);
}

/**
 * @brief Imprime un mensaje de log con marca de tiempo en la consola (NSH).
 *
 * @param direction Dirección del mensaje (">" enviado, "<" recibido).
 * @param host Host remoto o local.
 * @param socket_type Tipo de socket ("client" o "server").
 * @param protocol Protocolo usado ("TCP" o "UDP").
 * @param description Contenido del mensaje o descripción del evento.
 */
static void log_msg(const char *direction, const char *host, const char *socket_type,
                    const char *protocol, const char *description)
{
  char timestamp[32];
  get_timestamp(timestamp, sizeof(timestamp));
  printf("%s %s %s [%s] %s: %s\n", direction, host, socket_type, timestamp, protocol, description);
}

/**
 * @brief Parsea los argumentos de la línea de comandos de NuttX.
 *
 * @param argc Número de argumentos.
 * @param argv Array de argumentos.
 * @param args Puntero a la estructura donde se guardarán los resultados.
 * @return 0 si tiene éxito, -1 si faltan argumentos obligatorios.
 */
static int parse_args(int argc, char *argv[], struct args_s *args)
{
  int i;

  /* Initialize defaults */
  args->protocol = "TCP";
  args->server_ip = "127.0.0.1";
  args->port = 3001;
  args->mode = NULL;

  /* First argument is the mode (client/server) because we are running as 'lab01 client ...' */
  if (argc < 2)
    {
      return -1;
    }

  args->mode = argv[1];

  for (i = 2; i < argc; i++)
    {
      if (strcmp(argv[i], "protocol") == 0 && i + 1 < argc)
        {
          args->protocol = argv[++i];
        }
      else if (strcmp(argv[i], "server") == 0 && i + 1 < argc)
        {
          args->server_ip = argv[++i];
        }
      else if (strcmp(argv[i], "port") == 0 && i + 1 < argc)
        {
          args->port = atoi(argv[++i]);
        }
    }
  return 0;
}

/**
 * @brief Realiza una operación matemática básica.
 *
 * @param input Cadena de entrada (ej. "5+3").
 * @param output Buffer donde se escribirá el resultado.
 */
static void calculate(const char *input, char *output)
{
  int a, b;
  char op;
  int res = 0;

  if (sscanf(input, "%d%c%d", &a, &op, &b) != 3)
    {
      sprintf(output, "Error: Invalid format");
      return;
    }

  switch (op)
    {
    case '+': res = a + b; break;
    case '-': res = a - b; break;
    case '*': res = a * b; break;
    case '/':
      if (b == 0)
        {
          sprintf(output, "Error: Div by zero");
          return;
        }
      res = a / b;
      break;
    case '%':
      if (b == 0)
        {
          sprintf(output, "Error: Div by zero");
          return;
        }
      res = a % b;
      break;
    default:
      sprintf(output, "Error: Unknown op");
      return;
    }
  sprintf(output, "%d", res);
}

/**
 * @brief Ejecuta la lógica del cliente en NuttX.
 *
 * @param args Argumentos de configuración.
 * @return 0 en éxito, 1 en error.
 */
static int run_client(struct args_s *args)
{
  int sock;
  struct sockaddr_in server_addr;
  char *buffer;
  int is_tcp = (strcasecmp(args->protocol, "TCP") == 0);

  buffer = (char *)malloc(BUFFER_SIZE);
  if (buffer == NULL)
    {
      perror("malloc");
      return 1;
    }

  if (is_tcp)
    {
      sock = socket(AF_INET, SOCK_STREAM, 0);
    }
  else
    {
      sock = socket(AF_INET, SOCK_DGRAM, 0);
    }

  if (sock < 0)
    {
      perror("socket");
      free(buffer);
      return 1;
    }

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(args->port);
  server_addr.sin_addr.s_addr = inet_addr(args->server_ip);

  if (is_tcp)
    {
      if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
          perror("connect");
          close(sock);
          free(buffer);
          return 1;
        }
    }

  printf("Connected to %s:%d via %s\n", args->server_ip, args->port, args->protocol);

  /* Disable buffering for stdout to ensure prompts appear immediately */
  setvbuf(stdout, NULL, _IONBF, 0);

  while (1)
    {
      printf("Ingrese la operacion a realizar: ");
      if (fgets(buffer, BUFFER_SIZE, stdin) == NULL)
        {
          break;
        }

      /* Remove newline */
      buffer[strcspn(buffer, "\n")] = 0;

      if (strlen(buffer) == 0) continue;

      /* Log Request */
      log_msg("<", "client", "client", args->protocol, buffer);

      if (is_tcp)
        {
          send(sock, buffer, strlen(buffer), 0);
        }
      else
        {
          sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        }

      if (strcasecmp(buffer, "EXIT") == 0)
        {
          break;
        }

      memset(buffer, 0, BUFFER_SIZE);
      if (is_tcp)
        {
          int len = recv(sock, buffer, BUFFER_SIZE - 1, 0);
          if (len <= 0) break;
          buffer[len] = 0;
        }
      else
        {
          socklen_t addr_len = sizeof(server_addr);
          int len = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *)&server_addr, &addr_len);
          if (len <= 0) break;
          buffer[len] = 0;
        }

      log_msg(">", args->server_ip, "server", args->protocol, buffer);
    }

  close(sock);
  free(buffer);
  return 0;
}

/**
 * @brief Ejecuta la lógica del servidor en NuttX.
 *
 * @param args Argumentos de configuración.
 * @return 0 en éxito, 1 en error.
 */
static int run_server(struct args_s *args)
{
  int server_sock, client_sock;
  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_len = sizeof(client_addr);
  char *buffer;
  char *response;
  int is_tcp = (strcasecmp(args->protocol, "TCP") == 0);
  int should_exit = 0;

  buffer = (char *)malloc(BUFFER_SIZE);
  response = (char *)malloc(BUFFER_SIZE);

  if (buffer == NULL || response == NULL)
    {
      perror("malloc");
      if (buffer) free(buffer);
      if (response) free(response);
      return 1;
    }

  if (is_tcp)
    {
      server_sock = socket(AF_INET, SOCK_STREAM, 0);
    }
  else
    {
      server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    }

  if (server_sock < 0)
    {
      perror("socket");
      free(buffer);
      free(response);
      return 1;
    }

  int opt = 1;
  setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(args->port);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
      perror("bind");
      close(server_sock);
      free(buffer);
      free(response);
      return 1;
    }

  if (is_tcp)
    {
      if (listen(server_sock, 5) < 0)
        {
          perror("listen");
          close(server_sock);
          free(buffer);
          free(response);
          return 1;
        }
    }

  printf("Server listening on port %d (%s)\n", args->port, args->protocol);

  while (1)
    {
      if (is_tcp)
        {
          client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);
          if (client_sock < 0)
            {
              perror("accept");
              continue;
            }
          
          char *client_ip = inet_ntoa(client_addr.sin_addr);
          
          while (1)
            {
              memset(buffer, 0, BUFFER_SIZE);
              int len = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
              if (len <= 0) break;
              buffer[len] = 0;

              log_msg(">", client_ip, "client", args->protocol, buffer);

              if (strcasecmp(buffer, "EXIT") == 0)
                {
                  /* Echo EXIT and close */
                  log_msg("<", "server", "server", args->protocol, "EXIT");
                  send(client_sock, "EXIT", 4, 0);
                  should_exit = 1;
                  break;
                }

              calculate(buffer, response);
              log_msg("<", "server", "server", args->protocol, response);
              send(client_sock, response, strlen(response), 0);
            }
          close(client_sock);
          if (should_exit) break;
        }
      else /* UDP */
        {
          memset(buffer, 0, BUFFER_SIZE);
          int len = recvfrom(server_sock, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *)&client_addr, &addr_len);
          if (len > 0)
            {
              buffer[len] = 0;
              char *client_ip = inet_ntoa(client_addr.sin_addr);
              
              log_msg(">", client_ip, "client", args->protocol, buffer);

              if (strcasecmp(buffer, "EXIT") == 0)
                {
                   /* UDP doesn't maintain connection, but we can reply EXIT */
                   log_msg("<", "server", "server", args->protocol, "EXIT");
                   sendto(server_sock, "EXIT", 4, 0, (struct sockaddr *)&client_addr, addr_len);
                   break;
                }
              else
                {
                  calculate(buffer, response);
                  log_msg("<", "server", "server", args->protocol, response);
                  sendto(server_sock, response, strlen(response), 0, (struct sockaddr *)&client_addr, addr_len);
                }
            }
        }
    }

  close(server_sock);
  free(buffer);
  free(response);
  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/**
 * @brief Punto de entrada principal de la aplicación NuttX.
 *
 * @param argc Número de argumentos.
 * @param argv Array de argumentos.
 * @return 0 en éxito, 1 en error.
 */
int main(int argc, FAR char *argv[])
{
  struct args_s args;

  if (parse_args(argc, argv, &args) < 0)
    {
      printf("Usage: lab01 <client|server> [protocol TCP|UDP] [server IP] [port N]\n");
      return 1;
    }

  if (strcasecmp(args.mode, "client") == 0)
    {
      return run_client(&args);
    }
  else if (strcasecmp(args.mode, "server") == 0)
    {
      return run_server(&args);
    }
  else
    {
      printf("Invalid mode: %s\n", args.mode);
      return 1;
    }
}
