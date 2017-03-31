/*
 * Copyright (C) 2017 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>

#include <libskylark.h>

#define VERBOSE

static const char *named_source = NULL;
static bool verbose_logging = false;

//
// Upload Daemon - connects to Skylark and sends SBP messages.
//

static void usage(char *command) {
  printf("Usage: %s\n", command);
  puts("\nPipe - Pipe to read data from");
  puts("\t-s, --sub <FIFO name>");
  puts("\t-v --verbose");
  puts("\t\tWrite status to stderr");
}

static int parse_options(int argc, char *argv[]) {
  const struct option long_opts[] = {
    {"sub", required_argument, 0, 's'},
    {"verbose", no_argument, 0, 'v'},
    {0, 0, 0, 0}};
  int c;
  int opt_index;
  while ((c = getopt_long(argc, argv, "s:v", long_opts, &opt_index)) != -1) {
    switch (c) {
    case 's': {
      named_source = optarg;
    }
      break;
    case 'v': {
      verbose_logging = true;
    }
      break;
    default: {
      printf("invalid option\n");
      return -1;
    }
      break;
    }
  }
  if (named_source == NULL) {
    printf("Must specify the name of a pipe to read from.\n");
    return -1;
  }
  return 0;
}

int main(int argc, char *argv[])
{
  log_error("starting upload daemon\n");
  if (parse_options(argc, argv) != 0) {
    usage(argv[0]);
    exit(1);
  }
  int fd;
  if ((fd = open(named_source, O_RDONLY)) < 0) {
    printf("Error opening %s.\n", named_source);
    return -1;
  }
  client_config_t config;
  (void)init_config(&config);
  config.fd = fd;
  config.enabled = 1;
  log_client_config(&config);
  (void)setup_globals();
  (void)upload_process(&config, &upload_callback);
  log_debug("stopping upload daemon\n");
  teardown_globals();
  close(fd);
  exit(EXIT_FAILURE);
}
