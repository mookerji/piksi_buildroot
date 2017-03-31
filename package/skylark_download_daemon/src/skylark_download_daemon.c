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

static const char *named_sink = NULL;
static bool verbose_logging = false;

//
// Download Daemon - connects to Skylark and receives SBP messages.
//

static void usage(char *command) {
  printf("Usage: %s\n", command);
  puts("\nPipe - Pipe to write data to");
  puts("\t-p, --pub <FIFO name>");
  puts("\t-v --verbose");
  puts("\t\tWrite status to stderr");
}

/* * Illegal or missing hexadecimal sequence in chunked-encoding */
/* 56 Error */
/* stopping download daemon */
/* starting download daemon */

// * transfer closed with outstanding read data remaining
// 18 Error

static int parse_options(int argc, char *argv[]) {
  const struct option long_opts[] = {
      {"pub", required_argument, 0, 'p'},
      {"verbose", no_argument, 0, 'v'},
      {0, 0, 0, 0}};
  int c;
  int opt_index;
  while ((c = getopt_long(argc, argv, "p:v", long_opts, &opt_index)) != -1) {
    switch (c) {
      case 'p': {
        named_sink = optarg;
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
  if (named_sink == NULL) {
    printf("Must specify the name of a pipe to write to.\n");
    return -1;
  }
  return 0;
}

int main(int argc, char *argv[])
{
  log_error("starting download daemon\n");
  if (parse_options(argc, argv) != 0) {
    usage(argv[0]);
    exit(1);
  }
  int fd;
  if ((fd = open(named_sink, O_WRONLY)) < 0) {
    printf("Error opening %s.\n", named_sink);
    return -1;
  }
  client_config_t config;
  (void)init_config(&config);
  config.fd = fd;
  config.enabled = 1;
  log_client_config(&config);
  (void)setup_globals();
  (void)download_process(&config, &download_callback);
  log_error("stopping download daemon\n");
  close(fd);
  teardown_globals();
  exit(EXIT_FAILURE);
}
