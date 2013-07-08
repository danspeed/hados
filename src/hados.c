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

#define THREAD_COUNT 20

static void *hados_thread(void * parm) {

	int rc;

	// The generic context of the running node
	struct hados_context context;

	// Parameters array pass to commands
	struct hados_request request;

	struct hados_response response;

	hados_context_init(&context);

	for (;;) {

		static pthread_mutex_t accept_mutex = PTHREAD_MUTEX_INITIALIZER;

		pthread_mutex_lock(&accept_mutex);
		rc = FCGX_Accept_r(&context.fcgxRequest);
		pthread_mutex_unlock(&accept_mutex);

		if (rc < 0)
			break;

		//Main loop
		hados_context_load(&context);
		hados_request_init(&request);
		hados_request_load(&request, &context);

		hados_response_init(&response);

		hados_command_dispatch(&context, &request, &response);

		hados_response_write(&response, &context, &request);
		hados_response_free(&response);
		hados_request_free(&request);

		FCGX_Finish_r(&context.fcgxRequest);
	}

	hados_context_free(&context);
	return NULL ;
}

int main(void) {
	int i;
	pthread_t id[THREAD_COUNT];

	FCGX_Init();

	for (i = 1; i < THREAD_COUNT; i++)
		pthread_create(&id[i], NULL, hados_thread, NULL );

	hados_thread(NULL );
	return 0;
}
