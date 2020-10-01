/* * Copyright (c)
 * 		2018 - present: Slavey Karadzhov <slav@attachix.com>
 * 		2014: Deoxxa Development (https://github.com/deoxxa/mqtt-protocol-c/)
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *  * Neither the name of Slavey Karadzhov and Deoxxa Development nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "message.h"
#include "parser.h"
#include "serialiser.h"
#include "errors.h"

int on_message_begin(void* user_data, mqtt_message_t* message) {
  printf("Starting....");
  return 0;
}

int on_data_begin(void* user_data, mqtt_message_t* message) {
  return 0;
}

int on_data_payload(void* user_data, mqtt_message_t* message, const char* data, size_t length) {
  char x[length + 1];
  memcpy(x, data, length);
  x[length] = 0;
  printf("Got Data: %s (%lu)\n", x, length);

  return 0;
}

int on_data_end(void* user_data, mqtt_message_t* message) {
  return 0;
}

int on_message_end(void* user_data, mqtt_message_t* message) {
#ifdef DEBUG
  mqtt_message_dump(message);
#else
  printf("Recompile with `make DEBUG=1` to see message details");
#endif
  return 0;
}

void test_parser() {
  mqtt_parser_t parser;
  mqtt_serialiser_t serialiser;
  mqtt_message_t message;

  uint8_t data[] = {

      /* Connect Packet */
      // type 1
      0x10,
      // length 94
      0x5e,
      // protocol name
      0x00, 0x06, 0x4d, 0x51, 0x49, 0x73, 0x64, 0x70,
      // protocol version
      0x03,
      // flags
      0xf6,
      // keep-alive
      0x00, 0x1e,
      // client id
      0x00, 0x05, 0x68, 0x65, 0x6c, 0x6c, 0x6f,
      // will topic
      0x00, 0x06, 0x73, 0x68, 0x6f, 0x75, 0x74, 0x73,
      // will message
      0x00, 0x16, 0x59, 0x4f, 0x20, 0x57, 0x48, 0x41, 0x54, 0x27, 0x53, 0x20, 0x55, 0x50, 0x20, 0x4d, 0x59, 0x20, 0x48,
      0x4f, 0x4d, 0x49, 0x45, 0x53,
      // username
      0x00, 0x07, 0x74, 0x68, 0x65, 0x75, 0x73, 0x65, 0x72,
      // password (md5)
      0x00, 0x20, 0x62, 0x61, 0x33, 0x63, 0x38, 0x33, 0x33, 0x34, 0x38, 0x62, 0x64, 0x64, 0x66, 0x37, 0x62, 0x33, 0x36,
      0x38, 0x62, 0x34, 0x37, 0x38, 0x61, 0x63, 0x30, 0x36, 0x64, 0x33, 0x33, 0x34, 0x30, 0x65,

      /* Connect ACK */
      0x20, 0x02, 0x00, 0x00,

      /* Subscribe */
      0x82, 0x1e, 0x00, 0x01,

      0x00, 0x0b, 0x53, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x54, 0x6f, 0x70, 0x69, 0x63, 0x00,

      0x00, 0x0b, 0x53, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x54, 0x6f, 0x70, 0x69, 0x63, 0x01,

      /* Suback */
      0x90, 0x04, 0x00, 0x01, 0x00, 0x01,

      /* Publish */
      0x31, 0x30, 0x00, 0x0b, 0x53, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x54, 0x6f, 0x70, 0x69, 0x63, 0x48, 0x65, 0x6c, 0x6c,
      0x6f, 0x20, 0x66, 0x72, 0x6f, 0x6d, 0x20, 0x74, 0x68, 0x65, 0x20, 0x50, 0x61, 0x68, 0x6f, 0x20, 0x62, 0x6c, 0x6f,
      0x63, 0x6b, 0x69, 0x6e, 0x67, 0x20, 0x63, 0x6c, 0x69, 0x65, 0x6e, 0x74,

      /* Ping Request */
      0xc0, 0x00,
      /* Ping Response */
      0xd0, 0x00,

      /* Publish */
      0x30, 0x17, 0x00, 0x0b, 0x53, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x54, 0x6f, 0x70, 0x69, 0x63, 0x48, 0x65, 0x6c, 0x6c,
      0x6f, 0x20, 0x4d, 0x51, 0x54, 0x54,

      /* Publish without payload */
      0x30, 0x0d, 0x00, 0x0b, 'S', 'a', 'm', 'p', 'l', 'e', 'T', 'o', 'p', 'i', 'c',

      /* Disconnect */
      0xe0, 0x00};

  mqtt_parser_callbacks_t callbacks;
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.on_message_begin = on_message_begin;
  callbacks.on_data_begin    = on_data_begin;
  callbacks.on_data_payload  = on_data_payload;
  callbacks.on_data_end      = on_data_end;
  callbacks.on_message_end   = on_message_end;

  mqtt_parser_init(&parser, &callbacks);
  mqtt_serialiser_init(&serialiser);
  mqtt_message_init(&message);

  int rc = 0, loops = 0;

  size_t data_length = sizeof data;
  printf("parser running (%zu bytes)\n", data_length);

  //  size_t chunk_size = (rand() % data_length - 1) + 1;
  size_t chunk_size   = 3;
  size_t remainder    = data_length % chunk_size;
  size_t total_chunks = (data_length / chunk_size) + (remainder ? 1 : 0);

  printf("Data Length: %zd, Total data chunks: %zd\n", data_length, total_chunks);

  size_t total = 0;
  for(unsigned i = 0; i < total_chunks; i++) {
    uint8_t* chunk = data + i * chunk_size;

    if((i == total_chunks - 1) && remainder) {
      chunk_size = remainder;
    }

    printf("===> Iteration: %d, Offset: %zd, Chunk size: %zd <===\n", i, i * chunk_size, chunk_size);
    total += chunk_size;

    rc = mqtt_parser_execute(&parser, &message, chunk, chunk_size);
    loops++;
  }

  if(total != data_length) {
    printf("Error: Unprocessed bytes: %zd", data_length - total);
  }

  printf("\n");
  printf("parser info\n");
  printf("  state: %d\n", parser.state);
  if(rc == MQTT_PARSER_RC_ERROR) {
    printf("  error: %s\n", mqtt_error_string(parser.error));
  }
  printf("  nread: %zd\n", parser.nread);
  printf("  loops: %d\n", loops);
  printf("\n");
}

static void test_serialiser_proccess_message(mqtt_serialiser_t* serialiser, mqtt_message_t* message) {
  size_t packet_length    = mqtt_serialiser_size(serialiser, message);
  uint8_t packet[packet_length];
  mqtt_serialiser_write(serialiser, message, packet, packet_length);

  printf("packet length: %zu\n", packet_length);
  printf("packet data:   ");
  for(int j = 0; j < packet_length; j++) {
    printf("%02x ", packet[j]);
  }
  printf("\n");

  mqtt_message_clear(message, 0);
}


void test_serialiser() {
  mqtt_serialiser_t serialiser;
  mqtt_message_t message;

  const char topic[] = "a/b/c";
  const char topic_120[] = "12345678901234567890"
                           "12345678901234567890"
                           "12345678901234567890"
                           "12345678901234567890"
                           "12345678901234567890"
                           "12345678901234567890";

  // PUBLISH message
  memset(&message, 0, sizeof(message));
  message.common.type = MQTT_TYPE_PUBLISH;
  message.common.qos  = MQTT_QOS_AT_LEAST_ONCE;

  message.publish.topic_name.length = strlen(topic);
  message.publish.topic_name.data   = (uint8_t*)MQTT_MALLOC(strlen(topic));
  memcpy(message.publish.topic_name.data, topic, strlen(topic));
  message.publish.content.length = 3; // NO actual data is assigned just yet.
  message.publish.content.data   = (uint8_t*)MQTT_MALLOC(3);
  memcpy(message.publish.content.data, "txt", 3);

  test_serialiser_proccess_message(&serialiser, &message);
  
  
  // PUBLISH message - size of Remaining Length field = 1 byte
  // topic_len + topic + PacketID + payload = 2 + 120 + 2 + 3 = 127 = remainig length 
  // ==> packet length = 129
  memset(&message, 0, sizeof(message));
  message.common.type = MQTT_TYPE_PUBLISH;
  message.common.qos  = MQTT_QOS_AT_LEAST_ONCE;

  message.publish.topic_name.length = strlen(topic_120);
  message.publish.topic_name.data   = (uint8_t*)MQTT_MALLOC(strlen(topic_120));
  memcpy(message.publish.topic_name.data, topic_120, strlen(topic_120));
  message.publish.content.length = 3; // NO actual data is assigned just yet.
  message.publish.content.data   = (uint8_t*)MQTT_MALLOC(3);
  memcpy(message.publish.content.data, "txt", 3);

  test_serialiser_proccess_message(&serialiser, &message);


  // PUBLISH message - size of Remaining Length field = 2 bytes 
  // topic_len + topic + PacketID + payload = 2 + 120 + 2 + 4 = 128 = remainig length 
  // ==> packet length = 131
  memset(&message, 0, sizeof(message));
  message.common.type = MQTT_TYPE_PUBLISH;
  message.common.qos  = MQTT_QOS_AT_LEAST_ONCE;

  message.publish.topic_name.length = strlen(topic_120);
  message.publish.topic_name.data   = (uint8_t*)MQTT_MALLOC(strlen(topic_120));
  memcpy(message.publish.topic_name.data, topic_120, strlen(topic_120));
  message.publish.content.length = 4; // NO actual data is assigned just yet.
  message.publish.content.data   = (uint8_t*)MQTT_MALLOC(4);
  memcpy(message.publish.content.data, "txtx", 4);

  test_serialiser_proccess_message(&serialiser, &message);


  // SUBSCRIBE message
  memset(&message, 0, sizeof(message));
  message.common.type = MQTT_TYPE_SUBSCRIBE;

  mqtt_topicpair_t* topics = (mqtt_topicpair_t*)MQTT_MALLOC(sizeof(mqtt_topicpair_t));
  topics->name.length      = strlen(topic);
  topics->name.data        = (uint8_t*)MQTT_MALLOC(topics->name.length);
  topics->next             = NULL;
  memcpy(topics->name.data, topic, topics->name.length);
  message.subscribe.topics = topics;
  test_serialiser_proccess_message(&serialiser, &message);

  
  // PINGRESP
  memset(&message, 0, sizeof(message));
  message.common.type = MQTT_TYPE_PINGRESP;
  test_serialiser_proccess_message(&serialiser, &message);

}

int main(int argc, char** argv) {
  test_parser();
  test_serialiser();
  return 0;
}
