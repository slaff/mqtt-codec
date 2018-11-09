#ifndef MQTT_PARSER_H
#define MQTT_PARSER_H

#include <stdint.h>
#include <stddef.h>

#include "errors.h"
#include "message.h"

#if !(defined(MQTT_ENABLE_CLIENT) && defined(MQTT_ENABLE_SERVER))
#define MQTT_ENABLE_CLIENT 1
#define MQTT_ENABLE_SERVER 0
#endif

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
  MQTT_PARSER_STATE_VARIABLE_HEADER,
#ifdef MQTT_ENABLE_SERVER
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
  MQTT_PARSER_STATE_CONNACK,
  MQTT_PARSER_STATE_SUBACK,
  MQTT_PARSER_STATE_SUBACK_QOS,
  MQTT_PARSER_STATE_UNSUBACK,
  MQTT_PARSER_STATE_PINGRESP,
#endif /* MQTT_ENABLE_CLIENT */
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
	int(*on_connected)      (void*, mqtt_message_t* );
	int(*on_message_begin)	(void*, mqtt_message_t* );

	int(*on_data_begin)     (void*, mqtt_message_t* );
    int(*on_data_payload)   (void*, mqtt_message_t*, const char*, size_t);
    int(*on_data_end)       (void*, mqtt_message_t* );

    int(*on_message_end)	(void*, mqtt_message_t*);

} mqtt_parser_callbacks_t;

typedef struct mqtt_parser_s {
  const mqtt_parser_callbacks_t* callbacks;

  mqtt_parser_state_t state;
  mqtt_error_t error;

  size_t needs;          /* bytes needed to read the full content of a package */
  size_t nread;          /* bytes read */

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
