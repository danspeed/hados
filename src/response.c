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

void hados_response_init(struct hados_response *response) {
	response->status = 0;
	response->message = NULL;
}

void hados_response_free(struct hados_response *response) {
	if (response->message != NULL ) {
		free(response->message);
		response->message = NULL;
	}
}

int hados_response_set_status(struct hados_response *response, int status,
		const char* message) {
	response->status = status;
	if (response->message != NULL ) {
		free(response->message);
		response->message = NULL;
	}
	response->message = message == NULL ? NULL : strdup(message);
	return status;
}

int hados_response_set_errno(struct hados_response *response) {
	char errormsg[1024];
	strerror_r(errno, errormsg, sizeof(errormsg) - 1);
	int status = hados_response_set_status(response, HADOS_INTERNAL_ERROR,
			errormsg);
	return status;
}

int hados_response_set_success(struct hados_response *response) {
	return hados_response_set_status(response, HADOS_SUCCESS, "OK");
}

void hados_response_write(struct hados_response *response,
		struct hados_context *context, struct hados_request *request) {
	if (response->status == HADOS_BINARY_RESULT)
		return;
	hados_context_printf(context, "Content-type: application/json\r\n\r\n");
	hados_context_printf(context, "{\n\"version:\": 0.1");
	if (request->command != NULL )
		hados_context_printf(context, ",\n\"command\": \"%s\"",
				request->command);
	hados_context_printf(context, ",\n\"param_count\": %d", request->count);
	if (context->bytes_received != 0)
		hados_context_printf(context, ",\n\"bytes_received\": %d",
				context->bytes_received);
	if (context->data_dir != NULL )
		hados_context_printf(context, ",\n\"data_dir\": \"%s\"",
				context->data_dir);
	hados_context_printf(context, ",\n\"status\": %d", response->status);
	if (response->message != NULL )
		hados_context_printf(context, ",\n\"message\": \"%s\"",
				response->message);

//Compute elapsed time
	struct timeval endTime;
	gettimeofday(&endTime, NULL );
	long elapsedTime = (endTime.tv_sec - request->requestTime.tv_sec) * 1000.0; // sec to ms
	elapsedTime += (endTime.tv_usec - request->requestTime.tv_usec) / 1000.0; // us to ms
	hados_context_printf(context, ",\n\"time_ms\": %ld", elapsedTime);
	hados_context_printf(context, "\n}\n");
}
