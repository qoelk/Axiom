#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

static pid_t server_pid, client_pid;

static void on_exit(int sig) {
  (void)sig;
  if (server_pid)
    kill(server_pid, SIGTERM);
  if (client_pid)
    kill(client_pid, SIGTERM);
  exit(0);
}
int main() {
  signal(SIGINT, on_exit);

  server_pid = fork();
  if (!server_pid)
    execl("./server", "./server", "./assets/maps/dev.json",
          "./assets/game.json", NULL);

  sleep(1);

  client_pid = fork();
  if (!client_pid)
    execl("./client", "./client", "http://localhost:8000", NULL);

  waitpid(client_pid, NULL, 0);
  kill(server_pid, SIGTERM);

  return 0;
}
