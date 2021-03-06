/*
 HADOS
 High Availability Distributed Search Engine
 Copyright (C) 2013 Jaeksoft / Emmanuel Keller

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 ============================================================================
 Name        : response.c
 Author      : Emmanuel Keller
 Version     :
 Copyright   : Jaeksoft / Emmanuel Keller
 ============================================================================
 */

#include "hados.h"

const char *HADOS_VERSION = "0.2";

/**
 * Initiate the content of the response structure
 */
void hados_response_init(struct hados_response *response,
		struct hados_context *context) {
	response->status = 0;
	response->http_status = 200;
	response->moreheader = NULL;
	response->message = NULL;
	response->morejson = NULL;
	response->context = context;
}

/**
 * Free the memory used by the response structure
 */
void hados_response_free(struct hados_response *response) {
	if (response->message != NULL ) {
		free(response->message);
		response->message = NULL;
	}
	if (response->moreheader != NULL ) {
		free(response->moreheader);
		response->moreheader = NULL;
	}
	if (response->morejson != NULL ) {
		free(response->morejson);
		response->morejson = NULL;
	}
}

/**
 * The the status and the message with the passed parameters
 */
int hados_response_set_status(struct hados_response *response, int status,
		const char* message) {
	response->status = status;
	switch (response->status) {
	case HADOS_SUCCESS:
		response->http_status = 200;
		break;
	case HADOS_OBJECT_NOT_FOUND:
		response->http_status = 404;
		break;
	case HADOS_REDIRECT:
		response->http_status = 302;
		break;
	default:
		response->http_status = 500;
		break;
	}
	if (response->message != NULL ) {
		free(response->message);
		response->message = NULL;
	}
	response->message = message == NULL ? NULL : strdup(message);
	return status;
}

/**
 * Add JSON formatted string to the current JSON response
 */
void hados_response_more_json(struct hados_response *response, const char* json) {
	response->morejson = hados_utils_strcat(response->morejson, json);
}

/**
 * Add HEADER to the current HTTP response
 */
void hados_response_more_header(struct hados_response *response,
		const char* header, const char* value) {
	const char *format = "%s: %s\r\n";
	char *headerstring = malloc(
			(strlen(header) + strlen(value) + strlen(format) + 1)
					* sizeof(char));
	sprintf(headerstring, format, header, value);
	response->moreheader = hados_utils_strcat(response->moreheader,
			headerstring);
	free(headerstring);
}

/**
 * Fill the status with errno value, and the returned message with the standard error text
 */
int hados_response_set_errno(struct hados_response *response) {
	char errormsg[1024];
	strerror_r(errno, errormsg, sizeof(errormsg) - 1);
	int status = hados_response_set_status(response, HADOS_INTERNAL_ERROR,
			errormsg);
	return status;
}

/**
 * Set the status to HADOS_SUCCESS
 */
int hados_response_set_success(struct hados_response *response) {
	return hados_response_set_status(response, HADOS_SUCCESS, "OK");
}

/**
 * Write the response to the standard output using JSON format.
 * If the status is HADOS_BINARY_RESULT, nothing is send
 */
void hados_response_write(struct hados_response *response) {
	if (response->status == HADOS_BINARY_RESULT)
		return;

	// First we send the HTTP header
	if (response->http_status != 200)
		hados_context_printf(response->context, "Status: %d\r\n",
				response->http_status);
	if (response->http_status == 302)
		hados_context_printf(response->context, "Location: %s\r\n",
				response->message);
	hados_context_printf(response->context,
			"Content-type: application/json\r\n");
	if (response->moreheader != NULL )
		hados_context_printf(response->context, "%s", response->moreheader);
	hados_context_printf(response->context, "%s: %d\r\n\r\n",
			HADOS_HEADER_STATUS, response->status);

	// Then we send the JSON content
	hados_context_printf(response->context, "{\n\"version\": %s",
			HADOS_VERSION);
	if (response->context->request.command != NULL )
		hados_context_printf(response->context, ",\n\"command\": \"%s\"",
				response->context->request.command);
	if (response->context->bytes_received != 0)
		hados_context_printf(response->context, ",\n\"bytes_received\": %d",
				response->context->bytes_received);
	hados_context_printf(response->context, ",\n\"status\": %d",
			response->status);
	if (response->message != NULL )
		hados_context_printf(response->context, ",\n\"message\": \"%s\"",
				response->message);

	//Compute elapsed time
	struct timeval endTime;
	gettimeofday(&endTime, NULL );
	long elapsedTime = (endTime.tv_sec
			- response->context->request.requestTime.tv_sec) * 1000.0; // sec to ms
	elapsedTime += (endTime.tv_usec
			- response->context->request.requestTime.tv_usec) / 1000.0; // us to ms
	hados_context_printf(response->context, ",\n\"time_ms\": %ld", elapsedTime);

	// Print any additional JSON information
	if (response->morejson != NULL )
		hados_context_printf(response->context, "%s", response->morejson);

	hados_context_printf(response->context, "%s", "\n}\n");
}
