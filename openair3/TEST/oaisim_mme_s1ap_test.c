/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "s1ap_common.h"
#include "s1ap_eNB_decoder.h"
#include "s1ap_mme_decoder.h"
#include "s1ap_eNB_encoder.h"

#define MAX_BUF_LENGTH (1024)

typedef struct {
  char    *procedure_name;
  uint8_t  buffer[MAX_BUF_LENGTH];
  uint32_t buf_len;
} s1ap_test_t;

s1ap_test_t s1ap_test[] = {
  //     {
  //         .procedure_name = "Downlink NAS transport",
  //         .buffer = {
  //             0x00, 0x0B, 0x40, 0x21, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
  //             0x05, 0xC0, 0x01, 0x10, 0xCE, 0xCC, 0x00, 0x08, 0x00, 0x03,
  //             0x40, 0x01, 0xB3, 0x00, 0x1A, 0x00, 0x0A, 0x09, 0x27, 0xAB,
  //             0x1F, 0x7C, 0xEC, 0x01, 0x02, 0x01, 0xD9
  //         },
  //         .buf_len = 37,
  //     },
  {
    .procedure_name = "Uplink NAS transport",
    .buffer = {
      0x00, 0x0D, 0x40, 0x41, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
      0x05, 0xC0, 0x01, 0x10, 0xCE, 0xCC, 0x00, 0x08, 0x00, 0x03,
      0x40, 0x01, 0xB3, 0x00, 0x1A, 0x00, 0x14, 0x13, 0x27, 0xD3,
      0x77, 0xED, 0x4C, 0x01, 0x02, 0x01, 0xDA, 0x28, 0x08, 0x03,
      0x69, 0x6D, 0x73, 0x03, 0x70, 0x66, 0x74, 0x00, 0x64, 0x40,
      0x08, 0x00, 0x02, 0xF8, 0x29, 0x00, 0x00, 0x20, 0x40, 0x00,
      0x43, 0x40, 0x06, 0x00, 0x02, 0xF8, 0x29, 0x00, 0x04,
    },
    .buf_len = 69,
  },
  {
    .procedure_name = "UE capability info indication",
    .buffer = {
      0x00, 0x16, 0x40, 0x37, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
      0x05, 0xC0, 0x01, 0x10, 0xCE, 0xCC, 0x00, 0x08, 0x00, 0x03,
      0x40, 0x01, 0xB3, 0x00, 0x4A, 0x40, 0x20, 0x1F, 0x00, 0xE8,
      0x01, 0x01, 0xA8, 0x13, 0x80, 0x00, 0x20, 0x83, 0x13, 0x05,
      0x0B, 0x8B, 0xFC, 0x2E, 0x2F, 0xF0, 0xB8, 0xBF, 0xAF, 0x87,
      0xFE, 0x40, 0x44, 0x04, 0x07, 0x0C, 0xA7, 0x4A, 0x80,
    },
    .buf_len = 59,
  },
  {
    .procedure_name = "Initial Context Setup Request",
    .buffer = {
      0x00, 0x09, 0x00, 0x80, 0xD4, 0x00, 0x00, 0x06, 0x00, 0x00,
      0x00, 0x05, 0xC0, 0x01, 0x10, 0xCE, 0xCC, 0x00, 0x08, 0x00,
      0x03, 0x40, 0x01, 0xB3, 0x00, 0x42, 0x00, 0x0A, 0x18, 0x08,
      0xF0, 0xD1, 0x80, 0x60, 0x02, 0xFA, 0xF0, 0x80, 0x00, 0x18,
      0x00, 0x80, 0x81, 0x00, 0x00, 0x34, 0x00, 0x7C, 0x45, 0x00,
      0x09, 0x3D, 0x0F, 0x80, 0x0A, 0x05, 0x00, 0x02, 0x03, 0x78,
      0x48, 0x86, 0x6D, 0x27, 0xC7, 0x97, 0x8E, 0xA1, 0x02, 0x07,
      0x42, 0x01, 0x49, 0x06, 0x00, 0x02, 0xF8, 0x29, 0x00, 0x04,
      0x00, 0x48, 0x52, 0x01, 0xC1, 0x01, 0x09, 0x1B, 0x03, 0x69,
      0x6D, 0x73, 0x03, 0x70, 0x66, 0x74, 0x06, 0x6D, 0x6E, 0x63,
      0x30, 0x39, 0x32, 0x06, 0x6D, 0x63, 0x63, 0x32, 0x30, 0x38,
      0x04, 0x67, 0x70, 0x72, 0x73, 0x05, 0x01, 0x0A, 0x80, 0x00,
      0x24, 0x5D, 0x01, 0x00, 0x30, 0x10, 0x23, 0x93, 0x1F, 0x93,
      0x96, 0xFE, 0xFE, 0x74, 0x4B, 0xFF, 0xFF, 0x00, 0xC5, 0x00,
      0x6C, 0x00, 0x32, 0x0B, 0x84, 0x34, 0x01, 0x08, 0x5E, 0x04,
      0xFE, 0xFE, 0xC5, 0x6C, 0x50, 0x0B, 0xF6, 0x02, 0xF8, 0x29,
      0x80, 0x00, 0x01, 0xF0, 0x00, 0x70, 0x8A, 0x53, 0x12, 0x64,
      0x01, 0x01, 0x00, 0x6B, 0x00, 0x05, 0x18, 0x00, 0x0C, 0x00,
      0x00, 0x00, 0x49, 0x00, 0x20, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    },
    .buf_len = 217,
  },
  {
    .procedure_name = "Initial context setup response",
    .buffer = {
      0x20, 0x09, 0x00, 0x26, 0x00, 0x00, 0x03, 0x00, 0x00, 0x40,
      0x05, 0xC0, 0x01, 0x10, 0xCE, 0xCC, 0x00, 0x08, 0x40, 0x03,
      0x40, 0x01, 0xB3, 0x00, 0x33, 0x40, 0x0F, 0x00, 0x00, 0x32,
      0x40, 0x0A, 0x0A, 0x1F, 0x0A, 0x05, 0x02, 0x05, 0x00, 0x0F,
      0x7A, 0x03,
    },
    .buf_len = 42,
  }
};

static int compare_buffer(uint8_t *buffer, uint32_t length_buffer,
                          uint8_t *pattern,
                          uint32_t length_pattern)
{
  int i;

  if (length_buffer != length_pattern) {
    printf("Length mismatch, expecting %d bytes, got %d bytes\n", length_pattern,
           length_buffer);
    return -1;
  }

  for (i = 0; i < length_buffer; i++) {
    if (pattern[i] != buffer[i]) {
      printf("Mismatch fount in bytes %d\nExpecting 0x%02x, got 0x%02x\n",
             i, pattern[i], buffer[i]);
    }
  }

  return 0;
}

int main(int argc, char *argv[])
{
  int i;
  asn1_xer_print = 2;

  if (argc > 1) {
    asn_debug = 1;
  }

  for (i = 0; i < sizeof(s1ap_test) / sizeof(s1ap_test_t); i++) {
    struct s1ap_message_s message;
    uint8_t *buffer;
    uint32_t length;
    memset(&message, 0, sizeof(struct s1ap_message_s));
    printf("Trying to decode %s procedure with asn1c decoder\n",
           s1ap_test[i].procedure_name);

    if (s1ap_mme_decode_pdu(&message, s1ap_test[i].buffer,
                            s1ap_test[i].buf_len) < 0) {
      if (s1ap_eNB_decode_pdu(&message, s1ap_test[i].buffer,
                              s1ap_test[i].buf_len) < 0) {
        printf("Failed to decode this message\n");
      } else {
        printf("Succesfully decoded %s with eNB decoder\n", s1ap_test[i].procedure_name);
      }
    } else {
      printf("Succesfully decoded %s with MME decoder\n", s1ap_test[i].procedure_name);
    }

    printf("Trying to encode %s procedure with asn1c encoder\n",
           s1ap_test[i].procedure_name);

    if (s1ap_eNB_encode_pdu(&message, &buffer, &length) < 0) {
      printf("Failed to encode this message on MME side, trying eNB side\n");
    } else {
      compare_buffer(buffer, length, s1ap_test[i].buffer, s1ap_test[i].buf_len);
      free(buffer);
    }
  }

  return 0;
}
