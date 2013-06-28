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

int main(void) {

	// The generic context of the running node
	struct hados_context context;

	// Parameters array pass to commands
	struct hados_parameters parameters;

	// The query string
	char *query_string;

	// The command requested
	char *command;

	// The final status/error of the commmand
	int status;

	// Variables to compute elapsed time
	struct timeval t1, t2;
	long elapsedTime;

	hados_context_init(&context);
	hados_parameters_init(&parameters);

	while (FCGI_Accept() >= 0) {

		//Main loop
		hados_context_load(&context);

		gettimeofday(&t1, NULL );

		status = 0;
		command = NULL;
		query_string = getenv("QUERY_STRING");
		if (query_string != NULL && strlen(query_string) > 0) {
			hados_parameters_load(&parameters, query_string);
			command = hados_parameters_getvalue(&parameters, "cmd");
		}

		if (command != NULL ) {

			if (strcmp(command, "put") == 0) {
				status = hados_put(&context, &parameters);
			} else if (strcmp(command, "get") == 0) {
				status = hados_get(&context, &parameters);
			} else if (strcmp(command, "delete") == 0) {
				status = hados_delete(&context, &parameters);
			} else if (strcmp(command, "exists") == 0) {
				status = hados_exists(&context, &parameters);
			} else {
				status = HADOS_UNKNOWN_COMMAND;
			}
		}

		if (status != HADOS_BINARY_RESULT) {
			printf("Content-type: application/json\r\n\r\n");
			printf("{\n\"version:\": 0.1\n");
			if (command != NULL )
				printf(",\n\"command\": \"%s\"", command);
			printf(",\n\"param_count\": %d", parameters.count);
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

	return 0;
}
