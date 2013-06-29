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

#include "hados.h"

int main(void) {

	// The generic context of the running node
	struct hados_context context;

	// Parameters array pass to commands
	struct hados_request request;

	struct hados_response response;

	hados_context_init(&context);

	while (FCGI_Accept() >= 0) {

		//Main loop
		hados_context_load(&context);
		hados_request_init(&request);
		hados_request_load(&request, getenv("QUERY_STRING"));
		hados_response_init(&response);

		hados_command_dispatch(&context, &request, &response);

		hados_response_write(&response, &context, &request);
		hados_response_free(&response);
		hados_request_free(&request);
	}

	hados_context_free(&context);
	return 0;
}
