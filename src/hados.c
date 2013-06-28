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
 Name        : hados.c
 Author      : Emmanuel Keller
 Version     :
 Copyright   : Jaeksoft / Emmanuel Keller
 ============================================================================
 */

#include "fcgi_stdio.h"
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "hados.h"

#define HADOS_QS_LEN 65536
#define HADOS_MAX_PARAMETERS 64

int main(void) {

	// The generic context of the running node
	struct hados_context context;

	// The query string
	char *query_string;

	// Internal buffer copying the query string
	char *data;

	// Internal buffer used to parse the query string parameters
	char *token;

	// Internal buffer used to store the key and the value of the parameters found in the query string
	char *key, *value;

	// Count the number of parameters in the query string
	int paramCount;

	// The command requested
	char *command;

	// The final status/error of the commmand
	int status;

	// Parameters array pass to commands
	struct hados_parameter parameters[HADOS_MAX_PARAMETERS];

	// Variables to compute elapsed time
	struct timeval t1, t2;
	long elapsedTime;

	data = malloc(HADOS_QS_LEN);
	token = malloc(HADOS_QS_LEN);
	key = malloc(HADOS_QS_LEN);
	value = malloc(HADOS_QS_LEN);

	context.data_dir = NULL;
	context.node = NULL;
	context.nodes = NULL;

	while (FCGI_Accept() >= 0) {

		//Main loop

		gettimeofday(&t1, NULL );

		// Retrieve the data directory if not already set
		if (context.data_dir == NULL )
			context.data_dir = getenv("HADOS_DATADIR");

		// Retrieve the my public URL if not already set
		if (context.node == NULL )
			context.node = getenv("HADOS_NODE");

		// Retrieve list of other nodes if not already set
		if (context.nodes == NULL )
			context.nodes = getenv("HADOS_NODES");

		status = 0;
		paramCount = 0;
		command = NULL;
		parameters[0].key = NULL;
		query_string = getenv("QUERY_STRING");
		if (query_string != NULL && strlen(query_string) > 0) {

			// Extracting parameters (only first 64K characters)
			strncpy(data, query_string, 65536);

			// Iterate over the characters
			char *pdata = data;
			paramCount = 0;
			char c;
			char *keypos = NULL;
			char *valuepos = NULL;
			struct hados_parameter *currentParam = NULL;
			for (;;) {
				c = *pdata;
				if (c == 0) {
					break;
				}
				// Set the key
				if (keypos == NULL ) {
					currentParam = &parameters[paramCount];
					currentParam->key = pdata;
					currentParam->value = NULL;
					paramCount++;
					parameters[paramCount].key = NULL;
					keypos = pdata;
					valuepos = pdata;
				}
				// Set the value
				if (valuepos == NULL ) {
					currentParam->value = pdata;
					valuepos = pdata;
				}
				if (c == '&') {
					*pdata = 0;
					keypos = NULL;
				} else if (c == '=') {
					*pdata = 0;
					valuepos = NULL;
				}
				pdata++;
			}

			command = hados_getParameter(parameters, "cmd");
		}

		if (command != NULL ) {

			if (strcmp(command, "put") == 0) {
				status = hados_put(&context, parameters);
			} else if (strcmp(command, "get") == 0) {
				status = hados_get(&context, parameters);
			} else if (strcmp(command, "delete") == 0) {
				status = hados_delete(&context, parameters);
			} else if (strcmp(command, "exists") == 0) {
				status = hados_exists(&context, parameters);
			} else {
				status = HADOS_UNKNOWN_COMMAND;
			}
		}

		if (status != HADOS_BINARY_RESULT) {
			printf("Content-type: application/json\r\n\r\n");
			printf("{\n\"version:\": 0.1\n");
			if (command != NULL )
				printf(",\n\"command\": \"%s\"", command);
			printf(",\n\"param_count\": %d", paramCount);
			if (context.data_dir != NULL )
				printf(",\n\"data_dir\": \"%s\"", context.data_dir);
			printf(",\n\"status\": %d", status);

			//Compute elapsed time
			gettimeofday(&t2, NULL );
			elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
			elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
			printf(",\n\"time_ms\": %ld", elapsedTime);

			printf("}\n");
		}
	}

	free(data);
	free(token);
	free(key);
	free(value);
	return 0;
}

char* hados_getParameter(struct hados_parameter *parameters, const char* key) {
	struct hados_parameter *parameter;
	int i;
	for (i = 0; i < HADOS_MAX_PARAMETERS; i++) {
		parameter = &parameters[i];
		if (parameter->key == NULL ) {
			return NULL ;
		}
		if (strcmp(key, parameter->key) == 0) {
			return parameter->value;
		}
	}
	return NULL ;
}
