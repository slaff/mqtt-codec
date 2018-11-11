#include <string.h>

#include "message.h"

#include "serialiser.h"

#define WRITE_STRING(name) { \
  buffer[offset++] = name.length / 0xff; \
  buffer[offset++] = name.length & 0xff; \
  memcpy(&(buffer[offset]), name.data, name.length); \
  offset += name.length; \
}

#define WRITE_ID(id) { \
	size_t sequence = id; \
    if(!sequence) { \
    	sequence = serialiser->sequence++; \
    } \
    buffer[offset++] = sequence >> 8; \
    buffer[offset++] = sequence & 0xFF; \
}

void mqtt_serialiser_init(mqtt_serialiser_t* serialiser) {
  memset(serialiser, 0, sizeof (mqtt_serialiser_t));
}

size_t mqtt_serialiser_size(mqtt_serialiser_t* serialiser, mqtt_message_t* message) {
  size_t len = 0;

  switch(message->common.type) {

#ifdef MQTT_ENABLE_CLIENT
      // Client to server messages
	  case MQTT_TYPE_CONNECT: {
		len += 8;

		len += message->connect.protocol_name.length;
		len += message->connect.client_id.length;

		if (message->connect.flags.username_follows) {
		  len += 2;
		  len += message->connect.username.length;
		}

		if (message->connect.flags.password_follows) {
		  len += 2;
		  len += message->connect.password.length;
		}

		if (message->connect.flags.will) {
		  len += 4;
		  len += message->connect.will_topic.length;
		  len += message->connect.will_message.length;
		}

		break;
	  }

	  case MQTT_TYPE_SUBSCRIBE: {
		mqtt_topicpair_t* cur = message->subscribe.topics;
		while(cur) {
			len += cur->name.length + 3;
			cur = cur->next;
		}

		break;
	  }

	  case MQTT_TYPE_UNSUBSCRIBE: {
	    len+=2; // message id

	    mqtt_topic_t* cur = message->unsubscribe.topics;
	    while(cur) {
	    	len += cur->name.length + 2;
	    	cur = cur->next;
	    }

	    break;
	  }
#endif /* MQTT_ENABLE_CLIENT */

#ifdef MQTT_ENABLE_SERVER
	  // Server to client messages
	  case MQTT_TYPE_CONNACK: {
		  len += 2;
		  break;
	  }

	  case MQTT_TYPE_SUBACK: {
		  len += 2; // message id
		  mqtt_topicpair_t* cur = message->suback.topics;
		  while(cur) {
			len += 1;
			cur = cur->next;
		  }

		  break;
	  }

	  case MQTT_TYPE_UNSUBACK: {
		  len += 2; // message id

		  break;
	  }

#endif /* MQTT_ENABLE_CLIENT */

	  case MQTT_TYPE_PUBLISH: {
	    len += message->publish.topic_name.length + 2;

	  	// The Packet Identifier field is only present in PUBLISH Packets where the QoS level is 1 or 2.
	  	if(message->common.qos) {
	  		len += 2;
	  	}

	  	len += message->publish.content.length;

	  	break;

	  }

	  case MQTT_TYPE_PUBACK:
      case MQTT_TYPE_PUBREC:
      case MQTT_TYPE_PUBREL:
      case MQTT_TYPE_PUBCOMP: {
		  len += 2; // message id

		  break;
	  }

#if MQTT_ENABLE_CLIENT
	case MQTT_TYPE_PINGREQ:
	case MQTT_TYPE_DISCONNECT:
		break;
#endif /* MQTT_ENABLE_CLIENT */
#if MQTT_ENABLE_SERVER
	case MQTT_TYPE_PINGRESP:
		break;
#endif /* MQTT_ENABLE_SERVER */


      default: {
    	  return 0; // 0 is returned when the package is not valid
      }
  }


  message->common.length = len;
  len += 1; // initial byte with flags and type

  // the bytes needed for the common length
  if (len <= 127) {
    len += 1;
  } else if (len <= 16383) {
    len += 2;
  } else if (len <= 2097151) {
    len += 3;
  } else if (len <= 268435455) {
    len += 4;
  }

  return len;
}

mqtt_serialiser_rc_t mqtt_serialiser_write(mqtt_serialiser_t* serialiser, mqtt_message_t* message, uint8_t* buffer, size_t len) {
  unsigned int offset = 0;

  buffer[offset++] = message->common.retain + (message->common.qos << 1) + (message->common.dup << 3) + (message->common.type << 4);

  uint32_t remaining_length = message->common.length;
  do {
    buffer[offset++] = remaining_length & 0x7f;
    remaining_length >>= 7;
  } while (remaining_length > 0);

  switch (message->common.type) {

#ifdef MQTT_ENABLE_CLIENT
  	// Client to server messages
    case MQTT_TYPE_CONNECT: {
      WRITE_STRING(message->connect.protocol_name);

      buffer[offset++] = message->connect.protocol_version;

      buffer[offset++] =
        (message->connect.flags.username_follows << 7) +
        (message->connect.flags.password_follows << 6) +
        (message->connect.flags.will_retain      << 5) +
        (message->connect.flags.will_qos         << 3) +
        (message->connect.flags.will             << 2) +
        (message->connect.flags.clean_session    << 1);

      buffer[offset++] = message->connect.keep_alive >> 8;
      buffer[offset++] = message->connect.keep_alive & 0xff;

      WRITE_STRING(message->connect.client_id);

      if (message->connect.flags.will) {
        WRITE_STRING(message->connect.will_topic);
        WRITE_STRING(message->connect.will_message);
      }

      if (message->connect.flags.username_follows) {
        WRITE_STRING(message->connect.username);
      }

      if (message->connect.flags.password_follows) {
        WRITE_STRING(message->connect.password);
      }

      break;
    }

    case MQTT_TYPE_SUBSCRIBE: {
    	mqtt_topicpair_t* cur = message->subscribe.topics;
		while(cur) {
			WRITE_STRING(cur->name);
			buffer[offset++] = cur->qos;
			cur = cur->next;
		}

    	break;
    }

    case MQTT_TYPE_UNSUBSCRIBE: {
    	WRITE_ID(message->unsubscribe.message_id);

    	mqtt_topic_t* cur = message->unsubscribe.topics;
    	while(cur) {
    		WRITE_STRING(cur->name);
    		cur = cur->next;
    	}

    	break;
    }
#endif /* MQTT_ENABLE_CLIENT */

#ifdef MQTT_ENABLE_SERVER
    // Server to client messages
    case MQTT_TYPE_CONNACK: {
      buffer[offset++] = message->connack._unused;
      buffer[offset++] = message->connack.return_code;

      break;
    }

    case MQTT_TYPE_SUBACK: {
    	buffer[offset++] = message->suback.message_id >> 8;
    	buffer[offset++] = message->suback.message_id & 0xFF;

    	mqtt_topicpair_t* cur = message->suback.topics;
    	while(cur) {
    		buffer[offset++] = cur->qos;
    		cur = cur->next;
    	}

    	break;
    }

    case MQTT_TYPE_UNSUBACK: {
    	buffer[offset++] = message->unsuback.message_id >> 8;
    	buffer[offset++] = message->unsuback.message_id & 0xFF;

		break;
	}
#endif /* MQTT_ENABLE_SERVER */

// Bi-directional messages
    case MQTT_TYPE_PUBLISH: {
    	if(message->publish.qos) {
    		WRITE_ID(message->publish.message_id)
    	}

    	WRITE_STRING(message->publish.topic_name);

    	if(message->publish.content.data) {
    		memcpy(&buffer, message->publish.content.data, message->publish.content.length);
    		buffer += message->publish.content.length;
    	}

    	break;
    }

    case MQTT_TYPE_PUBACK: {
    	buffer[offset++] = message->puback.message_id >> 8;
    	buffer[offset++] = message->puback.message_id & 0xFF;

		break;
	}

    case MQTT_TYPE_PUBREC: {
    	buffer[offset++] = message->pubrec.message_id >> 8;
    	buffer[offset++] = message->pubrec.message_id & 0xFF;

    	break;
    }

    case MQTT_TYPE_PUBREL: {
    	buffer[offset++] = message->pubrel.message_id >> 8;
    	buffer[offset++] = message->pubrel.message_id & 0xFF;

    	break;
    }

    case MQTT_TYPE_PUBCOMP: {
    	buffer[offset++] = message->pubcomp.message_id >> 8;
		buffer[offset++] = message->pubcomp.message_id & 0xFF;

    	break;
    }

// Below are all the message types that must not have variable header and payload
#if MQTT_ENABLE_CLIENT
	case MQTT_TYPE_PINGREQ:
	case MQTT_TYPE_DISCONNECT:
#endif /* MQTT_ENABLE_CLIENT */
#if MQTT_ENABLE_SERVER
	case MQTT_TYPE_PINGRESP:
#endif /* MQTT_ENABLE_SERVER */

    default: {
      if(!message->common.length) {
    	  break;
      }

      serialiser->error = MQTT_ERROR_SERIALISER_INVALID_MESSAGE_ID;

      return MQTT_SERIALISER_RC_ERROR;
    }
  }

  return MQTT_SERIALISER_RC_SUCCESS;
}
