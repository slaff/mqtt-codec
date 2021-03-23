/* * Copyright (c)
 * 		2018 - present: Slavey Karadzhov <slav@attachix.com>
 * 		2014: Deoxxa Development (https://github.com/deoxxa/mqtt-protocol-c/)
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *  * Neither the name of Slavey Karadzhov and Deoxxa Development nor the names
 * of its contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MQTT_ERRORS_H
#define MQTT_ERRORS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum mqtt_error_e {
  MQTT_ERROR_NONE = 0,
  MQTT_ERROR_PARSER_INVALID_STATE,
  MQTT_ERROR_PARSER_INVALID_REMAINING_LENGTH,
  MQTT_ERROR_PARSER_INVALID_MESSAGE_ID,
  MQTT_ERROR_PARSER_CALLBACK_FAILED,
  MQTT_ERROR_SERIALISER_INVALID_MESSAGE_ID,

} mqtt_error_t;

typedef enum mqtt_connect_error_e {
  MQTT_CONNECT_ERROR_NONE = 0,
  MQTT_CONNECT_ERROR_INVALID_VERSION,
  MQTT_CONNECT_ERROR_CLIENT_ID,
  MQTT_CONNECT_ERROR_UNAVAILABLE,
  MQTT_CONNECT_ERROR_BAD_USERNAME_OR_PASSWORD,
  MQTT_CONNECT_ERROR_NOT_AUTHORIZED
} mqtt_connect_error_t;

const char* mqtt_error_string(mqtt_error_t error);
const char* mqtt_connect_error_string(mqtt_connect_error_t error);

#ifdef __cplusplus
}
#endif

#endif
