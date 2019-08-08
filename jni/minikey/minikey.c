/*
 * Copyright (C) 2019 Orange
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <time.h>
#include <libevdev.h>

#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <linux/uinput.h>
#include <pthread.h>

#define VERSION 1
#define DEFAULT_SOCKET_NAME "minikey"
#define NB_EVT_INPUTS 3

static void usage(const char* pname)
{
  fprintf(stderr,
    "Usage: %s [-h] [-w key] [-r input] [-i] [-f file]\n"
    "  -w <key>  : Write keys.\n"
    "  -r <input>: Read keys from a given input (eg. '/dev/input/event0' or 'all' for all inputs at a time).\n"
    "  -i        : Uses STDIN to send key values.\n"
    "  -f <file> : Runs a file with a list of commands, doesn't start socket.\n"
    "  -h        : Show help.\n",
    pname
  );
}

/*-------------------------------------------------*/

void emit(int fd, int type, int code, int val) {
   struct input_event ie;

   ie.type = type;
   ie.code = code;
   ie.value = val;
   ie.time.tv_sec = 0;
   ie.time.tv_usec = 0;

   write(fd, &ie, sizeof(ie));
}

void KaiOS_write(char *input_event, int key) {
  int fd, uifd;

  fd = open(input_event, O_RDWR|O_NONBLOCK);
  if (fd < 0) {
    fprintf(stderr, "Failed to open FD (%s)\n", strerror(-fd));
    goto out;
  }

  struct uinput_setup usetup;
  ioctl(fd, UI_SET_EVBIT, EV_KEY);
  ioctl(fd, UI_SET_KEYBIT, KEY_SPACE);

  memset(&usetup, 0, sizeof(usetup));
  usetup.id.bustype = BUS_USB;
  usetup.id.vendor = 0x1234; /* sample vendor */
  usetup.id.product = 0x5678; /* sample product */
  strcpy(usetup.name, "Example device");

  ioctl(fd, UI_DEV_SETUP, &usetup);
  ioctl(fd, UI_DEV_CREATE);

  emit(fd, EV_KEY, key, 1);
  emit(fd, EV_SYN, SYN_REPORT, 0);
  emit(fd, EV_KEY, key, 0);
  emit(fd, EV_SYN, SYN_REPORT, 0);

  out:
   ioctl(fd, UI_DEV_DESTROY);
   close(fd);
}

static void parse_keys(char *keys)
{
  int i;
  for(i=0; i < strlen(keys); i++) {
    char key = keys[i];
    switch(key)
    {
      case '0': // NUM Keys
        KaiOS_write("/dev/input/event0", KEY_0);
        break;
      case '1':
        KaiOS_write("/dev/input/event0", KEY_1);
        break;
      case '2':
        KaiOS_write("/dev/input/event0", KEY_2);
        break;
      case '3':
        KaiOS_write("/dev/input/event0", KEY_3);
        break;
      case '4':
        KaiOS_write("/dev/input/event0", KEY_4);
        break;
      case '5':
        KaiOS_write("/dev/input/event0", KEY_5);
        break;
      case '6':
        KaiOS_write("/dev/input/event0", KEY_6);
        break;
      case '7':
        KaiOS_write("/dev/input/event0", KEY_7);
        break;
      case '8':
        KaiOS_write("/dev/input/event0", KEY_8);
        break;
      case '9':
        KaiOS_write("/dev/input/event0", KEY_9);
        break;
      case 's':
        KaiOS_write("/dev/input/event0", KEY_NUMERIC_STAR);
        break;
      case 'h':
        KaiOS_write("/dev/input/event0", KEY_NUMERIC_POUND);
        break;

      case 'p': // BUTTON Power
        KaiOS_write("/dev/input/event3", KEY_POWER);
        break;
      
      case 'd': // PAD Down
        KaiOS_write("/dev/input/event3", KEY_DOWN);
        break;
      case 'l': // PAD Left
        KaiOS_write("/dev/input/event0", KEY_LEFT);
        break;
      case 'r': // PAD Right
        KaiOS_write("/dev/input/event0", KEY_RIGHT);
        break;
      case 'u': // PAD Up
        KaiOS_write("/dev/input/event4", KEY_UP);
        break;
      case 'o': // PAD Center
        KaiOS_write("/dev/input/event0", KEY_OK);
        break;

      case 'm': // BUTTON Menu (right)
        KaiOS_write("/dev/input/event0", KEY_BACK);
        break;
      case 'n': // BUTTON Notif (left)
        KaiOS_write("/dev/input/event0", KEY_MENU);
        break;
      case 'b': // BUTTON Back
        KaiOS_write("/dev/input/event4", KEY_POWER);
        break;
      case 'c': // BUTTON Call
        KaiOS_write("/dev/input/event0", KEY_SEND);
        break;

      default:
        fprintf(stderr, "Key '%c' not found!\n", key);
    }
    sleep(1);
  }
}

static void io_handler(FILE* input, FILE* output)
{
  setvbuf(input, NULL, _IOLBF, 1024);
  setvbuf(output, NULL, _IOLBF, 1024);

  // Tell version
  fprintf(output, "v %d\n", VERSION);

  // Tell pid
  fprintf(output, "$ %d\n", getpid());

  char read_buffer[80];

  while (fgets(read_buffer, sizeof(read_buffer), input) != NULL)
  {
    read_buffer[strcspn(read_buffer, "\r\n")] = 0;
    parse_keys(read_buffer);
  }
}

static int start_server(char* sockname)
{
  int fd = socket(AF_UNIX, SOCK_STREAM, 0);

  if (fd < 0)
  {
    perror("creating socket");
    return fd;
  }

  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(&addr.sun_path[1], sockname, strlen(sockname));

  if (bind(fd, (struct sockaddr*) &addr,
    sizeof(sa_family_t) + strlen(sockname) + 1) < 0)
  {
    perror("binding socket");
    close(fd);
    return -1;
  }

  listen(fd, 1);

  return fd;
}

/*-------------------------------------------------*/

static void build_key(int key, char *input)
{
  switch(key)
  {
    case KEY_0: // NUM Keys
      printf("0");
      break;
    case KEY_1:
      printf("1");
      break;
    case KEY_2:
      printf("2");
      break;
    case KEY_3:
      printf("3");
      break;
    case KEY_4:
      printf("4");
      break;
    case KEY_5:
      printf("5");
      break;
    case KEY_6:
      printf("6");
      break;
    case KEY_7:
      printf("7");
      break;
    case KEY_8:
      printf("8");
      break;
    case KEY_9:
      printf("9");
      break;
    case KEY_NUMERIC_STAR:
      printf("s");
      break;
    case KEY_NUMERIC_POUND:
      printf("h");
      break;

    case KEY_POWER:
      if (strcmp(input, "/dev/input/event3") == 0) {
        // BUTTON Power
        printf("p");
      } else
      if (strcmp(input, "/dev/input/event4") == 0) {
        // BUTTON Back
        printf("b");
      }
      break;
    
    case KEY_DOWN: // PAD Down
      printf("d");
      break;
    case KEY_LEFT: // PAD Left
      printf("l");
      break;
    case KEY_RIGHT: // PAD Right
      printf("r");
      break;
    case KEY_UP: // PAD Up
      printf("u");
      break;
    case KEY_OK: // PAD Center
      printf("o");
      break;

    case KEY_BACK: // BUTTON Menu (right)
      printf("m");
      break;
    case KEY_MENU: // BUTTON Notif (left)
      printf("n");
      break;
    case KEY_SEND: // BUTTON Call
      printf("c");
      break;

    default:
      fprintf(stderr, "Key '%d' not found for %s!\n", key, input);
  }
  fflush(stdout);
}

void KaiOS_read(char *input_event) {
  struct libevdev *dev = NULL;
  int fd;
  int rc = 1;

  fd = open(input_event, O_RDONLY|O_NONBLOCK);

  rc = libevdev_new_from_fd(fd, &dev);
  if (rc < 0) {
    fprintf(stderr, "Failed to init libevdev (%s)\n", strerror(-rc));
    goto out;
  }

  do {
    struct input_event ev;
    rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL|LIBEVDEV_READ_FLAG_BLOCKING, &ev);
    if (rc == LIBEVDEV_READ_STATUS_SYNC) {
      while (rc == LIBEVDEV_READ_STATUS_SYNC) {
        rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_SYNC, &ev);
      }
    } else if (rc == LIBEVDEV_READ_STATUS_SUCCESS) {
      if (ev.type == EV_KEY && ev.value == 0) {
        build_key(ev.code, input_event);
      }
    }
  } while (rc == LIBEVDEV_READ_STATUS_SYNC || rc == LIBEVDEV_READ_STATUS_SUCCESS || rc == -EAGAIN);

  if (rc != LIBEVDEV_READ_STATUS_SUCCESS && rc != -EAGAIN)
    fprintf(stderr, "Failed to handle events: %s\n", strerror(-rc));

  rc = 0;
out:
  libevdev_free(dev);
}

void *read_thread(void *arg) {
  KaiOS_read(arg);
}

pthread_t createThread(char *input_event) {
  pthread_t thread;
  if (pthread_create(&thread, NULL, read_thread, input_event) != 0) {
    fprintf(stderr, "Thread creation error!\n");
    return 0;
  }
  return thread;
}

void KaiOS_readAllInputs() {
  // Note: event1, evnet2, event5 & event6 are not used
  pthread_t thread_clients[NB_EVT_INPUTS];
  thread_clients[0] = createThread("/dev/input/event0");
  thread_clients[1] = createThread("/dev/input/event3");
  thread_clients[2] = createThread("/dev/input/event4");

  int i;
  for(i = 0; i < NB_EVT_INPUTS; i++) {
    if (thread_clients[i] != 0) {
        pthread_join(thread_clients[i], NULL);
    }
  }
}

/*-------------------------------------------------*/

int main(int argc, char* argv[])
{
  const char* pname = argv[0];
  char* sockname = DEFAULT_SOCKET_NAME;
  char* stdin_file = NULL;
  int use_stdin = 0;

  int opt;
  while ((opt = getopt(argc, argv, "?::w:r:i::f:h::")) != -1) {
    switch (opt) {
      case 'w':
        parse_keys(optarg);
        return EXIT_SUCCESS;
      case 'r':
        if (strcmp(optarg, "all") == 0) {
          KaiOS_readAllInputs();
        } else {
          KaiOS_read(optarg);
        }
        return EXIT_SUCCESS;
      case 'i':
        use_stdin = 1;
        break;
      case 'f':
        stdin_file = optarg;
        break;
      case '?':
        usage(pname);
        return EXIT_FAILURE;
      case 'h':
        usage(pname);
        return EXIT_SUCCESS;
    }
  }

  FILE* input;
  FILE* output;

  if (use_stdin || stdin_file != NULL)
  {
    if (stdin_file != NULL)
    {
      // Reading from a file
      input = fopen(stdin_file, "r");
      if (NULL == input)
      {
        fprintf(stderr, "Unable to open '%s': %s\n",
                stdin_file, strerror(errno));
        exit(EXIT_FAILURE);
      }
      else
      {
        fprintf(stderr, "Reading keys from '%s'\n",
                stdin_file);
      }
    }
    else
    {
      // Reading from terminal
      input = stdin;
      fprintf(stderr, "Reading from STDIN\n");
    }

    output = stderr;
    io_handler(input, output);
    fclose(input);
    fclose(output);
    exit(EXIT_SUCCESS);
  }

  struct sockaddr_un client_addr;
  socklen_t client_addr_length = sizeof(client_addr);

  int server_fd = start_server(sockname);

  if (server_fd < 0)
  {
    fprintf(stderr, "Unable to start server on %s\n", sockname);
    return EXIT_FAILURE;
  }

  while (1)
  {
    int client_fd = accept(server_fd, (struct sockaddr *) &client_addr,
      &client_addr_length);

    if (client_fd < 0)
    {
      perror("accepting client");
      exit(1);
    }

    fprintf(stderr, "Connection established\n");

    input = fdopen(client_fd, "r");
    if (input == NULL)
    {
      fprintf(stderr, "%s: fdopen(client_fd,'r')\n", strerror(errno));
      exit(1);
    }

    output = fdopen(dup(client_fd), "w");
    if (output == NULL)
    {
      fprintf(stderr, "%s: fdopen(client_fd,'w')\n", strerror(errno));
      exit(1);
    }

    io_handler(input, output);

    fprintf(stderr, "Connection closed\n");
    fclose(input);
    fclose(output);
    close(client_fd);
  }

  close(server_fd);

  return EXIT_SUCCESS;
}
