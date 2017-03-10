/*
 * Copyright (C) 2016 Swift Navigation Inc.
 * Contact: Jacob McNamee <jacob@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "framer_rtcm3.h"

#include <string.h>
#include <stdio.h>

#define RTCM3_PREAMBLE 0xD3
#define RTCM3_HEADER_LENGTH 3
#define RTCM3_FOOTER_LENGTH 3

void framer_rtcm3_init(void *framer_rtcm3_state)
{
  framer_rtcm3_state_t *s = (framer_rtcm3_state_t *)framer_rtcm3_state;

  s->buffer_length = 0;
  s->refill_count = 0;
  s->remove_count = 0;
}

uint32_t framer_rtcm3_process(void *framer_rtcm3_state,
                              const uint8_t *data, uint32_t data_length,
                              const uint8_t **frame, uint32_t *frame_length)
{
  framer_rtcm3_state_t *s = (framer_rtcm3_state_t *)framer_rtcm3_state;

  uint32_t data_offset = 0;
  while (1) {

    /* Remove from front of the buffer if requested */
    if (s->remove_count != 0) {

      /* Search for the first valid preamble after the removed bytes */
      uint32_t offset = s->remove_count;
      while (offset < s->buffer_length) {
        if (s->buffer[offset] == RTCM3_PREAMBLE) {
          break;
        }
        offset++;
      }

      /* Shift the data down to the start of the buffer */
      if (offset < s->buffer_length) {
        memmove(&s->buffer[0], &s->buffer[offset], s->buffer_length - offset);
        s->buffer_length -= offset;
      } else {
        s->buffer_length = 0;
      }

      /* Note: remove count should never be greater than the buffer length */
      s->remove_count = 0;
    }

    /* Refill the buffer if requested */
    if (s->refill_count != 0) {

      uint32_t count = s->refill_count;
      uint32_t available = data_length - data_offset;
      if (count > available) {
        count = available;
      }

      memcpy(&s->buffer[s->buffer_length], &data[data_offset], count);
      s->buffer_length += count;
      data_offset += count;
      s->refill_count -= count;

      /* Return if there is still not enough data available */
      if (s->refill_count != 0) {
        *frame = NULL;
        *frame_length = 0;
        return data_offset;
      }
    }

    /* Wait for header */
    if (s->buffer_length < RTCM3_HEADER_LENGTH) {
      s->refill_count = RTCM3_HEADER_LENGTH - s->buffer_length;
      continue;
    }

    /* Verify preamble */
    if (s->buffer[0] != RTCM3_PREAMBLE) {
      s->remove_count = 1;
      continue;
    }

    /* Get message length */
    uint32_t message_length = ((s->buffer[1] & 0x3) << 8) | s->buffer[2];
    uint32_t total_length = RTCM3_HEADER_LENGTH + message_length +
                            RTCM3_FOOTER_LENGTH;

    /* Wait for full frame */
    if (s->buffer_length < total_length) {
      s->refill_count = total_length - s->buffer_length;
      continue;
    }

    /* Verify CRC */
    if (0) {
      s->remove_count = 1;
      continue;
    }

    /* Decoded frame */
    s->remove_count = total_length;
    *frame = s->buffer;
    *frame_length = total_length;
    return data_offset;
  }
}
