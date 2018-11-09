#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "errors.h"
#include "message.h"

#include "parser.h"

#define READ_STRING(into) { \
    if ((len - parser->nread) < 2) { \
      return MQTT_PARSER_RC_INCOMPLETE; \
    } \
    \
    size_t str_length = data[parser->nread] * 256 + data[parser->nread + 1]; \
    \
    if ((len - parser->nread - 2) < str_length) { \
      return MQTT_PARSER_RC_INCOMPLETE; \
    } \
    \
    if (parser->buffer_pending == 0) { \
      parser->buffer_length = str_length; \
      \
      return MQTT_PARSER_RC_WANT_MEMORY; \
    } \
    \
    parser->buffer_pending = 0; \
    \
    if (parser->buffer != NULL) { \
      memcpy(parser->buffer, data + parser->nread + 2, ((str_length) < (parser->buffer_length) ? (str_length) : (parser->buffer_length))); \
      \
      into.length = ((str_length) < (parser->buffer_length) ? (str_length) : (parser->buffer_length)); \
      into.data = parser->buffer; \
      \
      parser->buffer = NULL; \
      parser->buffer_length = 0; \
    } \
    \
    parser->nread += 2 + str_length; \
}

void mqtt_parser_init(mqtt_parser_t* parser, mqtt_parser_callbacks_t* callbacks) {
  parser->state = MQTT_PARSER_STATE_INITIAL;
  parser->nread = 0;
  parser->needs = 0;
  parser->buffer_pending = 0;
  parser->buffer = NULL;
  parser->buffer_length = 0;
  parser->callbacks = callbacks;
}

void mqtt_parser_buffer(mqtt_parser_t* parser, uint8_t* buffer, size_t buffer_length) {
  parser->buffer_pending = 1;
  parser->buffer = buffer;
  parser->buffer_length = buffer_length;
}

mqtt_parser_rc_t mqtt_parser_execute(mqtt_parser_t* parser, mqtt_message_t* message, uint8_t* data, size_t len) {
  do {
    switch (parser->state) {
      case MQTT_PARSER_STATE_INITIAL: {
        if ((len - parser->nread) < 1) {
          return MQTT_PARSER_RC_INCOMPLETE;
        }

        int rc = parser->callbacks->on_message_begin(parser->data, message);
		if(rc) {
			return rc;
		}

        message->common.retain = (data[parser->nread + 0] >> 0) & 0x01;
        message->common.qos    = (data[parser->nread + 0] >> 1) & 0x03;
        message->common.dup    = (data[parser->nread + 0] >> 3) & 0x01;
        message->common.type   = (data[parser->nread + 0] >> 4) & 0x0f;

        parser->nread += 1;

        parser->state = MQTT_PARSER_STATE_REMAINING_LENGTH;

        break;
      }

      case MQTT_PARSER_STATE_REMAINING_LENGTH: {
        size_t digit_bytes = 0,
            multiplier = 1,
            remaining_length = 0;

        do {
          digit_bytes += 1;

          if ((len - parser->nread) < digit_bytes) {
            return MQTT_PARSER_RC_INCOMPLETE;
          }

          remaining_length += (data[parser->nread + digit_bytes - 1] & 0x7f) * multiplier;
          multiplier *= 128;
        } while (data[parser->nread + digit_bytes - 1] >= 0x80 && digit_bytes < 4);

        if (data[parser->nread + digit_bytes - 1] >= 0x80) {
          parser->error = MQTT_ERROR_PARSER_INVALID_REMAINING_LENGTH;
          return MQTT_PARSER_RC_ERROR;
        }

        message->common.length = remaining_length;

        parser->nread += digit_bytes;

#define CASE_TYPE(A) case MQTT_TYPE_##A: { \
		parser->state = MQTT_PARSER_STATE_##A; \
        break; \
}
        switch (message->common.type) {
#if MQTT_ENABLE_SERVER
        CASE_TYPE(CONNECT)
		CASE_TYPE(SUBSCRIBE)
		CASE_TYPE(UNSUBSCRIBE)
#endif /* MQTT_ENABLE_SERVER */

#if MQTT_ENABLE_CLIENT
		CASE_TYPE(CONNACK)
		CASE_TYPE(SUBACK)
		CASE_TYPE(UNSUBACK)
#endif /* MQTT_ENABLE_CLIENT */

		CASE_TYPE(PUBLISH)
		CASE_TYPE(PUBACK)
		CASE_TYPE(PUBREC)
		CASE_TYPE(PUBREL)
		CASE_TYPE(PUBCOMP)

// Below are all the message types that must not have variable header and payload
#if MQTT_ENABLE_SERVER
		case MQTT_TYPE_PINGREQ:
		case MQTT_TYPE_DISCONNECT:
#endif /* MQTT_ENABLE_SERVER */
#if MQTT_ENABLE_CLIENT
		case MQTT_TYPE_PINGRESP:
#endif /* MQTT_ENABLE_CLIENT */

		default: {
			if(!message->common.length) {
				goto DONE;
			}

            parser->error = MQTT_ERROR_PARSER_INVALID_MESSAGE_ID;
            return MQTT_PARSER_RC_ERROR;
          }
        } /* message->common.type */

        break;
      }

      case MQTT_PARSER_STATE_VARIABLE_HEADER: {
#if MQTT_ENABLE_SERVER
        if (message->common.type == MQTT_TYPE_CONNECT) {
          parser->state = MQTT_PARSER_STATE_CONNECT_PROTOCOL_NAME;
        }
#endif

        break;
      }

#if MQTT_ENABLE_SERVER
      case MQTT_PARSER_STATE_CONNECT: {
        parser->state = MQTT_PARSER_STATE_CONNECT_PROTOCOL_NAME;

        break;
      }

      case MQTT_PARSER_STATE_CONNECT_PROTOCOL_NAME: {
        READ_STRING(message->connect.protocol_name)

        parser->state = MQTT_PARSER_STATE_CONNECT_PROTOCOL_VERSION;

        break;
      }

      case MQTT_PARSER_STATE_CONNECT_PROTOCOL_VERSION: {
        if ((len - parser->nread) < 1) {
          return MQTT_PARSER_RC_INCOMPLETE;
        }

        message->connect.protocol_version = data[parser->nread];

        parser->nread += 1;

        parser->state = MQTT_PARSER_STATE_CONNECT_FLAGS;

        break;
      }

      case MQTT_PARSER_STATE_CONNECT_FLAGS: {
        if ((len - parser->nread) < 1) {
          return MQTT_PARSER_RC_INCOMPLETE;
        }

        message->connect.flags.username_follows = (data[parser->nread] >> 7) & 0x01;
        message->connect.flags.password_follows = (data[parser->nread] >> 6) & 0x01;
        message->connect.flags.will_retain      = (data[parser->nread] >> 5) & 0x01;
        message->connect.flags.will_qos         = (data[parser->nread] >> 4) & 0x02;
        message->connect.flags.will             = (data[parser->nread] >> 2) & 0x01;
        message->connect.flags.clean_session    = (data[parser->nread] >> 1) & 0x01;

        parser->nread += 1;

        parser->state = MQTT_PARSER_STATE_CONNECT_KEEP_ALIVE;

        break;
      }

      case MQTT_PARSER_STATE_CONNECT_KEEP_ALIVE: {
        if ((len - parser->nread) < 2) {
          return MQTT_PARSER_RC_INCOMPLETE;
        }

        message->connect.keep_alive = (data[parser->nread] << 8) + data[parser->nread + 1];

        parser->nread += 2;

        parser->state = MQTT_PARSER_STATE_CONNECT_CLIENT_IDENTIFIER;

        break;
      }

      case MQTT_PARSER_STATE_CONNECT_CLIENT_IDENTIFIER: {
        READ_STRING(message->connect.client_id)

        parser->state = MQTT_PARSER_STATE_CONNECT_WILL_TOPIC;

        break;
      }

      case MQTT_PARSER_STATE_CONNECT_WILL_TOPIC: {
        if (message->connect.flags.will) {
          READ_STRING(message->connect.will_topic)
        }

        parser->state = MQTT_PARSER_STATE_CONNECT_WILL_MESSAGE;

        break;
      }

      case MQTT_PARSER_STATE_CONNECT_WILL_MESSAGE: {
        if (message->connect.flags.will) {
          READ_STRING(message->connect.will_message);
        }

        parser->state = MQTT_PARSER_STATE_CONNECT_USERNAME;

        break;
      }

      case MQTT_PARSER_STATE_CONNECT_USERNAME: {
        if (message->connect.flags.username_follows) {
          READ_STRING(message->connect.username);
        }

        parser->state = MQTT_PARSER_STATE_CONNECT_PASSWORD;

        break;
      }

      case MQTT_PARSER_STATE_CONNECT_PASSWORD: {
        if (message->connect.flags.password_follows) {
          READ_STRING(message->connect.password);
        }

        goto DONE;
      }

      case MQTT_PARSER_STATE_SUBSCRIBE: {
    	  if ((len - parser->nread) < 2) {
			return MQTT_PARSER_RC_INCOMPLETE;
		  }

		  message->subscribe.message_id = (data[parser->nread] << 8) + data[parser->nread + 1];
		  parser->nread += 2;

		  // Use the subscribe.length
		  parser->needs = message->common.length - 2;

		  parser->state = MQTT_PARSER_STATE_SUBSCRIBE_TOPICS;
		  break;
      }

      case MQTT_PARSER_STATE_SUBSCRIBE_TOPICS: {
    	  mqtt_topicpair_t* next = NULL;
    	  if(message->subscribe.topics) {
    		  next = message->subscribe.topics;
    	  }

    	  message->subscribe.topics = (mqtt_topicpair_t*)malloc(sizeof(mqtt_topicpair_t));
		  memset(message->subscribe.topics, 0, sizeof(mqtt_topicpair_t));
		  message->subscribe.topics->next = next;

		  parser->state = MQTT_PARSER_STATE_SUBSCRIBE_TOPIC_NAME;
		  break;
      }

      case MQTT_PARSER_STATE_SUBSCRIBE_TOPIC_NAME: {
    	  READ_STRING(message->subscribe.topics->name);

    	  parser->needs -= message->subscribe.topics->name.length + 2;
    	  parser->state = MQTT_PARSER_STATE_SUBSCRIBE_TOPIC_QOS;
    	  break;
      }

      case MQTT_PARSER_STATE_SUBSCRIBE_TOPIC_QOS: {
    	  if ((len - parser->nread) < 1) {
			return MQTT_PARSER_RC_INCOMPLETE;
		  }

    	  message->subscribe.topics->qos = data[parser->nread];
    	  parser->nread += 1;
    	  parser->needs -= 1;

    	  if((!parser->needs) ) {
    		  goto DONE;
    	  }

    	  parser->state = MQTT_PARSER_STATE_SUBSCRIBE_TOPICS;
    	  break;
      }

      case MQTT_PARSER_STATE_UNSUBSCRIBE: {
    	  if ((len - parser->nread) < 2) {
			return MQTT_PARSER_RC_INCOMPLETE;
		  }

		  message->unsubscribe.message_id = (data[parser->nread] << 8) + data[parser->nread + 1];
		  parser->nread += 2;
		  parser->needs = message->common.length - 2;

		  parser->state = MQTT_PARSER_STATE_UNSUBSCRIBE_TOPICS;
		  break;
      }

      case MQTT_PARSER_STATE_UNSUBSCRIBE_TOPICS: {
		  mqtt_topic_t* next = NULL;
		  if(message->unsubscribe.topics) {
			  next = message->unsubscribe.topics;
		  }

		  message->unsubscribe.topics = (mqtt_topic_t*)malloc(sizeof(mqtt_topic_t));
		  memset(message->unsubscribe.topics, 0, sizeof(mqtt_topic_t));
		  message->unsubscribe.topics->next = next;

		  parser->state = MQTT_PARSER_STATE_UNSUBSCRIBE_TOPIC_NAME;
		  break;
		}

	  case MQTT_PARSER_STATE_UNSUBSCRIBE_TOPIC_NAME: {
		  READ_STRING(message->unsubscribe.topics->name);

		  parser->needs -= message->unsubscribe.topics->name.length + 2;

		  if(!parser->needs) {
			  goto DONE;
		  }

		  parser->state = MQTT_PARSER_STATE_UNSUBSCRIBE_TOPICS;

		  break;
	  }


#endif /* MQTT_ENABLE_SERVER */

#if MQTT_ENABLE_CLIENT
      case MQTT_PARSER_STATE_CONNACK: {
        if ((len - parser->nread) < 2) {
          return MQTT_PARSER_RC_INCOMPLETE;
        }

        message->connack._unused     = data[parser->nread];
        message->connack.return_code = data[parser->nread + 1];
        parser->nread += 2;

        goto DONE;
      }

      case MQTT_PARSER_STATE_SUBACK: {
    	  if ((len - parser->nread) < 2) {
			return MQTT_PARSER_RC_INCOMPLETE;
		  }

    	  message->suback.message_id = (data[parser->nread] << 8) + data[parser->nread + 1];
    	  parser->nread += 2;
    	  parser->state = MQTT_PARSER_STATE_SUBACK_QOS;
    	  parser->needs = message->common.length - 2;

    	  break;

      case MQTT_PARSER_STATE_SUBACK_QOS:
    	  if ((len - parser->nread) < 1) {
    		  return MQTT_PARSER_RC_INCOMPLETE;
    	  }

    	  mqtt_topicpair_t* next = NULL;
		  if(message->suback.topics) {
			  next = message->suback.topics;
		  }

		  message->suback.topics = (mqtt_topicpair_t*)malloc(sizeof(mqtt_topicpair_t));
		  memset(message->suback.topics, 0, sizeof(mqtt_topicpair_t));
		  message->suback.topics->next = next;

    	  message->suback.topics->qos = data[parser->nread];
		  parser->nread += 1;
		  parser->needs -= 1;

    	  if(!parser->needs) {
    		  goto DONE;
    	  }

    	  parser->state = MQTT_PARSER_STATE_SUBACK_QOS;
    	  break;
      }

      case MQTT_PARSER_STATE_UNSUBACK: {
		  if ((len - parser->nread) < 2) {
			return MQTT_PARSER_RC_INCOMPLETE;
		  }

		  message->unsuback.message_id = (data[parser->nread] << 8) + data[parser->nread + 1];
		  parser->nread += 2;

		  goto DONE;
	  }

#endif /* MQTT_ENABLE_CLIENT */

      case MQTT_PARSER_STATE_PUBLISH: {
        parser->state = MQTT_PARSER_STATE_PUBLISH_TOPIC_NAME;
        parser->needs = message->common.length;
        break;
      }

      case MQTT_PARSER_STATE_PUBLISH_TOPIC_NAME: {
    	READ_STRING(message->publish.topic_name)
    	parser->needs -= message->publish.topic_name.length + 2;

		// The Packet Identifier field is only present in PUBLISH Packets where the QoS level is 1 or 2.
		if(message->common.qos) {
			parser->state = MQTT_PARSER_STATE_PUBLISH_MESSAGE_ID;
		}
		else {
			parser->state = MQTT_PARSER_STATE_PUBLISH_PAYLOAD;
		}

        break;
      }

      case MQTT_PARSER_STATE_PUBLISH_MESSAGE_ID: {
        if ((len - parser->nread) < 2) {
          return MQTT_PARSER_RC_INCOMPLETE;
        }

        message->publish.message_id = (data[parser->nread] << 8) + data[parser->nread + 1];
        parser->nread += 2;
        parser->needs -= 2;
        parser->state = MQTT_PARSER_STATE_PUBLISH_PAYLOAD;

        break;
      }

      case MQTT_PARSER_STATE_PUBLISH_PAYLOAD: {
    	  message->publish.content.length = parser->needs;

    	  int rc = parser->callbacks->on_data_begin(parser->data, message);
		  if(rc) {
			return rc;
		  }

    	  parser->state = MQTT_PARSER_STATE_PUBLISH_PAYLOAD_READ;

    	  break;
      }

      case MQTT_PARSER_STATE_PUBLISH_PAYLOAD_READ: {
    	  if ((len - parser->nread) < 1) {
			return MQTT_PARSER_RC_INCOMPLETE;
		  }

    	  int available = len - parser->nread;
    	  size_t consume = available < parser->needs ? available: parser->needs;

    	  int rc = parser->callbacks->on_data_payload(parser->data, message, (const char*)(data + parser->nread), consume);
		  parser->nread += consume;
		  parser->needs -= consume;
    	  if(rc) {
			return rc;
		  }

		  if(!parser->needs) {
			  rc = parser->callbacks->on_data_end(parser->data, message);
			  if(rc) {
				return rc;
			  }

			  goto DONE;
		  }

		  break;
      }

      case MQTT_PARSER_STATE_PUBACK: {
        if ((len - parser->nread) < 2) {
          return MQTT_PARSER_RC_INCOMPLETE;
        }

        message->puback.message_id = (data[parser->nread] << 8) + data[parser->nread + 1];

        parser->nread += 2;

        goto DONE;
      }

      case MQTT_PARSER_STATE_PUBREC: {
        if ((len - parser->nread) < 2) {
          return MQTT_PARSER_RC_INCOMPLETE;
        }

        message->pubrec.message_id = (data[parser->nread] << 8) + data[parser->nread + 1];

        parser->nread += 2;

        goto DONE;
      }

      case MQTT_PARSER_STATE_PUBREL: {
        if ((len - parser->nread) < 2) {
          return MQTT_PARSER_RC_INCOMPLETE;
        }

        message->pubrel.message_id = (data[parser->nread] << 8) + data[parser->nread + 1];

        parser->nread += 2;

        goto DONE;
      }

      case MQTT_PARSER_STATE_PUBCOMP: {
        if ((len - parser->nread) < 2) {
          return MQTT_PARSER_RC_INCOMPLETE;
        }

        message->pubcomp.message_id = (data[parser->nread] << 8) + data[parser->nread + 1];

        parser->nread += 2;

        goto DONE;
      }

DONE: {
    	int rc = parser->callbacks->on_message_end(parser->data, message);
		if(rc) {
			return rc;
		}

		parser->state = MQTT_PARSER_STATE_INITIAL;
		parser->needs = 0;

		// TODO: release the memory for the message

    	break;
	  }

      default: {
        parser->error = MQTT_ERROR_PARSER_INVALID_STATE;

        return MQTT_PARSER_RC_ERROR;
      }
    }
  } while (1);
}
