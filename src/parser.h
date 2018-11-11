#ifndef MQTT_PARSER_H
#define MQTT_PARSER_H

#include <stdint.h>
#include <stddef.h>

#include "errors.h"
#include "message.h"

typedef enum mqtt_parser_rc_e {
  MQTT_PARSER_RC_INCOMPLETE = 0,
  MQTT_PARSER_RC_ERROR,
  MQTT_PARSER_RC_CONTINUE,
  MQTT_PARSER_RC_DONE,
  MQTT_PARSER_RC_WANT_MEMORY,
} mqtt_parser_rc_t;

typedef enum mqtt_parser_state_e {
  MQTT_PARSER_STATE_INITIAL,
  MQTT_PARSER_STATE_REMAINING_LENGTH,
  MQTT_PARSER_STATE_REMAINING_LENGTH_DONE,
  MQTT_PARSER_STATE_VARIABLE_HEADER,
#ifdef MQTT_ENABLE_SERVER
  // Client to server
  MQTT_PARSER_STATE_CONNECT,
  MQTT_PARSER_STATE_CONNECT_PROTOCOL_NAME,
  MQTT_PARSER_STATE_CONNECT_PROTOCOL_VERSION,
  MQTT_PARSER_STATE_CONNECT_FLAGS,
  MQTT_PARSER_STATE_CONNECT_KEEP_ALIVE,
  MQTT_PARSER_STATE_CONNECT_CLIENT_IDENTIFIER,
  MQTT_PARSER_STATE_CONNECT_WILL_TOPIC,
  MQTT_PARSER_STATE_CONNECT_WILL_MESSAGE,
  MQTT_PARSER_STATE_CONNECT_USERNAME,
  MQTT_PARSER_STATE_CONNECT_PASSWORD,
  MQTT_PARSER_STATE_SUBSCRIBE,
  MQTT_PARSER_STATE_SUBSCRIBE_TOPICS,
  MQTT_PARSER_STATE_SUBSCRIBE_TOPIC_NAME,
  MQTT_PARSER_STATE_SUBSCRIBE_TOPIC_QOS,
  MQTT_PARSER_STATE_UNSUBSCRIBE,
  MQTT_PARSER_STATE_UNSUBSCRIBE_TOPICS,
  MQTT_PARSER_STATE_UNSUBSCRIBE_TOPIC_NAME,
  MQTT_PARSER_STATE_PINGREQ,
  MQTT_PARSER_STATE_DISCONNECT,
#endif /* MQTT_ENABLE_SERVER */
#ifdef MQTT_ENABLE_CLIENT
  // Server to client
  MQTT_PARSER_STATE_CONNACK,
  MQTT_PARSER_STATE_SUBACK,
  MQTT_PARSER_STATE_SUBACK_QOS,
  MQTT_PARSER_STATE_UNSUBACK,
  MQTT_PARSER_STATE_PINGRESP,
#endif /* MQTT_ENABLE_CLIENT */

  // Bi-directional
  MQTT_PARSER_STATE_PUBLISH,
  MQTT_PARSER_STATE_PUBLISH_TOPIC_NAME,
  MQTT_PARSER_STATE_PUBLISH_MESSAGE_ID,
  MQTT_PARSER_STATE_PUBLISH_PAYLOAD,
  MQTT_PARSER_STATE_PUBLISH_PAYLOAD_READ,
  MQTT_PARSER_STATE_PUBACK,
  MQTT_PARSER_STATE_PUBREC,
  MQTT_PARSER_STATE_PUBREL,
  MQTT_PARSER_STATE_PUBCOMP,

  MQTT_PARSER_STATE_DONE,
} mqtt_parser_state_t;

typedef struct {
	/* In the message the correct type and expected length will be set. */
	int(*on_message_begin)	(void*, mqtt_message_t* );

	/* If the message is PUBLISH the following data methods will be called */
	int(*on_data_begin)     (void*, mqtt_message_t* );
    int(*on_data_payload)   (void*, mqtt_message_t*, const char*, size_t);
    int(*on_data_end)       (void*, mqtt_message_t* );

    /* The complete message will be delivered. If PUBLISH the payload will not be set by default. */
    int(*on_message_end)	(void*, mqtt_message_t*);

} mqtt_parser_callbacks_t;

#define MQTT_PARSER_STATE_READ_STRING  (1 << 7)

typedef struct mqtt_parser_s {
  const mqtt_parser_callbacks_t* callbacks;

  mqtt_parser_state_t state;
  mqtt_error_t error;

  size_t needs;          /* bytes needed to read the full content of a package */
  size_t string_length;
  size_t nread;          /* bytes read */
  uint8_t stored[2];     /* stored characters */
  uint8_t flags;         /*
  	  	  	  	  	  	   Order: First bit is the on the right, Last bit is on the left.
   	   	   	   	   	   	   7 - set if a string is started to be read
   	   	   	   	   	   	   6-2 - unused
   	   	   	   	   	   	   1-0 - the last two bits are used to specify the number of stored bytes
   	   	   	   	   	   	   */

  char buffer_pending;
  uint8_t* buffer;
  size_t buffer_length;

  /** PUBLIC **/
  void *data; /* A pointer to get hook to the "connection" or "socket" object */

} mqtt_parser_t;

void mqtt_parser_init(mqtt_parser_t* parser, mqtt_parser_callbacks_t* callbacks);
void mqtt_parser_buffer(mqtt_parser_t* parser, uint8_t* buffer, size_t buffer_length);
mqtt_parser_rc_t mqtt_parser_execute(mqtt_parser_t* parser, mqtt_message_t* message, uint8_t* data, size_t len);

#endif
