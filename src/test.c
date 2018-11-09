#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "message.h"
#include "parser.h"
#include "serialiser.h"
#include "errors.h"

int on_message_begin(void* user_data,mqtt_message_t* message)
{
	printf("Starting....");
	return 0;
}

int on_data_begin(void* user_data,mqtt_message_t* message)
{
	return 0;
}

int on_data_payload(void* user_data,mqtt_message_t* message, const char* data, size_t length)
{
	char x[length+1];
	memcpy(x, data, length);
	x[length] = 0;
	printf("Got Data: %s\n", x);

	return 0;
}

int on_data_end(void* user_data,mqtt_message_t* message)
{
	return 0;
}

int on_message_end(void* user_data,mqtt_message_t* message)
{
	mqtt_message_dump(message);
	return 0;
}


int main(int argc, char** argv) {
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
	0x00, 0x06,
	  0x4d, 0x51, 0x49, 0x73, 0x64, 0x70,
	// protocol version
	0x03,
	// flags
	0xf6,
	// keep-alive
	0x00, 0x1e,
	// client id
	0x00, 0x05,
	  0x68, 0x65, 0x6c, 0x6c, 0x6f,
	// will topic
	0x00, 0x06,
	  0x73, 0x68, 0x6f, 0x75, 0x74, 0x73,
	// will message
	0x00, 0x16,
	  0x59, 0x4f, 0x20, 0x57, 0x48, 0x41, 0x54, 0x27, 0x53, 0x20, 0x55, 0x50, 0x20, 0x4d, 0x59, 0x20, 0x48, 0x4f, 0x4d, 0x49, 0x45, 0x53,
	// username
	0x00, 0x07,
	  0x74, 0x68, 0x65, 0x75, 0x73, 0x65, 0x72,
	// password (md5)
	0x00, 0x20,
	  0x62, 0x61, 0x33, 0x63, 0x38, 0x33, 0x33, 0x34, 0x38, 0x62, 0x64, 0x64, 0x66, 0x37, 0x62, 0x33, 0x36, 0x38, 0x62, 0x34, 0x37, 0x38, 0x61, 0x63, 0x30, 0x36, 0x64, 0x33, 0x33, 0x34, 0x30, 0x65,

  /* Connect ACK */
  0x20, 0x02, 0x00, 0x00,

  /* Subscribe */
  0x82, 0x1e, 0x00, 0x01,

  0x00, 0x0b,
  0x53, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x54, 0x6f, 0x70, 0x69, 0x63,
  0x00,

  0x00, 0x0b,
  0x53, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x54, 0x6f, 0x70, 0x69, 0x63,
  0x01,

  /* Suback */
  0x90, 0x04, 0x00, 0x01, 0x00, 0x01,

  /* Publish */
  0x31, 0x30, 0x00, 0x0b, 0x53, 0x61, 0x6d, 0x70,
  0x6c, 0x65, 0x54, 0x6f, 0x70, 0x69, 0x63, 0x48,
  0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x66, 0x72, 0x6f,
  0x6d, 0x20, 0x74, 0x68, 0x65, 0x20, 0x50, 0x61,
  0x68, 0x6f, 0x20, 0x62, 0x6c, 0x6f, 0x63, 0x6b,
  0x69, 0x6e, 0x67, 0x20, 0x63, 0x6c, 0x69, 0x65,
  0x6e, 0x74,

  /* Ping Request */
  0xc0, 0x00,
  /* Ping Response */
  0xd0, 0x00,
  /* Publish */
  0x30, 0x17, 0x00, 0x0b, 0x53, 0x61, 0x6d, 0x70,
  0x6c, 0x65, 0x54, 0x6f, 0x70, 0x69, 0x63, 0x48,
  0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x4d, 0x51, 0x54,
  0x54,

  /* Disconnect */

  };

  mqtt_parser_callbacks_t callbacks;
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.on_message_begin = on_message_begin;
  callbacks.on_data_begin = on_data_begin;
  callbacks.on_data_payload = on_data_payload;
  callbacks.on_data_end = on_data_end;
  callbacks.on_message_end = on_message_end;

  mqtt_parser_init(&parser, &callbacks);
  mqtt_serialiser_init(&serialiser);
  mqtt_message_init(&message);

  int rc = 0, loops = 0;

  size_t data_length = sizeof data;
  printf("parser running (%zu bytes)\n", data_length);

  size_t chunk_size = (rand() % data_length - 1) + 1;
  size_t total_chunks = (data_length / chunk_size) + 1;

  printf("Data Length: %zd, Total data chunks: %zd\n", data_length, total_chunks);

  size_t total = 0;
  size_t offset = 0;
  for(unsigned i=0; i < total_chunks; i++ ) {
	  uint8_t* chunk = data + i*chunk_size;

	  if(i == total_chunks -1) {
		  chunk_size = data_length % chunk_size;
	  }

	  printf("===> Iteration: %d, Offset: %zd, Chunk size: %zd", i, i*chunk_size, chunk_size);
	  total += chunk_size;

	  // CODE TO PARSE A DATA CHUNK INTO MQTT MESSAGE
	  parser.nread = 0;

	  // main loop
	  do {
	      printf("  loop %d\n", ++loops);
	      printf("    state: %d\n", parser.state);
	      printf("    offset: %zu\n", parser.nread);

	      rc = mqtt_parser_execute(&parser, &message, chunk, chunk_size);
	      printf("    rc: %d\n", rc);

	      if (rc == MQTT_PARSER_RC_WANT_MEMORY) {
	        printf("    bytes requested: %zu\n", parser.buffer_length);
	        mqtt_parser_buffer(&parser, malloc(parser.buffer_length), parser.buffer_length);
	      }
	  } while (rc == MQTT_PARSER_RC_CONTINUE || rc == MQTT_PARSER_RC_WANT_MEMORY);

	  // END CODE TO PARSE A PACKAGE

	  offset+= chunk_size;
  }

  if(total != data_length) {
	  printf("Error: Unprocessed bytes: %zd", data_length - total);
  }

  printf("\n");
  printf("parser info\n");
  printf("  state: %d\n", parser.state);
  if (rc == MQTT_PARSER_RC_ERROR) {
    printf("  error: %s\n", mqtt_error_string(parser.error));
  }
  printf("  nread: %zd\n", parser.nread);
  printf("  loops: %d\n", loops);
  printf("\n");

  return 0;
}
