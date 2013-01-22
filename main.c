#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <memory.h>

#include <task.h>

typedef struct HttpCon HttpCon;

struct HttpCon {
  int fd; // client fd
  int rport;
  char rhost[16];
};

enum
{
  STACK = 32768,
};

int port = 8080;

void httptask(void *);

void
taskmain(int argc, char **argv)
{
  int cfd, fd;
  int rport;
  char remote[16];

  if((fd = netannounce(TCP, 0, port)) < 0) {
    fprintf(stderr, "can't announce on tcp!*!%d: %s\n", port, strerror(errno));
    taskexitall(1);
  }

  fdnoblock(fd);
  while((cfd = netaccept(fd, remote, &rport)) >= 0) {
    fprintf(stderr, "connection from tcp!%s!%d\n", remote, rport);

    HttpCon *c = malloc(sizeof(HttpCon));
    if(!c) {
      fprintf(stderr, "HttpCon malloc failed\n");
      continue;
    }

    c->fd = cfd;
    c->rport = rport;
    memcpy(c->rhost, remote, 16);

    taskcreate(httptask, (void *) c, STACK);
  }

}

const char reply_header[] =
  "HTTP/1.0 200 OK\r\n" /* response code + string */
  "Content-Type: %s\r\n"
  "Connection: close\r\n"
  "Content-Length: %lu\r\n\r\n"; /* payload length */

const char static_content[] =
  "<doctype !html>\n"
  "<html>\n"
  "<body>\n"
  "<h1>Hello, World!</h1>\n"
  "</body>\n"
  "</html>\n";


void
httptask(void *c)
{
  HttpCon *con;
  int  fd, nbytes;

  con = (HttpCon *) c;
  fd = con->fd;

  char buf[8 * 1024] = {0};

  fdnoblock(fd);

  if((nbytes = fdread(fd, buf, 8*1024)) <= 0) {
    if(nbytes == 0) {
      fprintf(stderr, "tcp!%s!%d: short read\n", con->rhost, con->rport);
    } else {
      fprintf(stderr, "fdread: %s\n", strerror(errno));
    }
  } else {
    // request ok
    char *header, *content;
    asprintf(&header, reply_header, "text/html", sizeof(static_content));

    fdwrite(fd, header, strlen(header));
    fdwrite(fd, static_content, sizeof(static_content));

    shutdown(fd, 2);
    close(fd);
  }

  free(con);
  taskexit(0);
}

