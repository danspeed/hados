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
 Name        : context.c
 Author      : Emmanuel Keller
 Version     :
 Copyright   : Jaeksoft / Emmanuel Keller
 ============================================================================
 */

#include "hados.h"

void hados_context_init(struct hados_context *context) {
	FCGX_InitRequest(&context->fcgxRequest, 0, 0);
	context->data_dir = NULL;
	context->node = NULL;
	context->nodes = NULL;
	context->nodeArray = NULL;
	context->nodesNumber = 0;
}

const char* hados_context_get_env(struct hados_context *context,
		const char* param) {
	return FCGX_GetParam(param, context->fcgxRequest.envp);
}

char* hados_context_get_env_dup(struct hados_context *context,
		const char* param) {
	const char* env = FCGX_GetParam(param, context->fcgxRequest.envp);
	if (env == NULL )
		return NULL ;
	return strdup(env);
}

int hados_context_printf(struct hados_context *context, const char *format, ...) {
	va_list argList;
	va_start(argList, format);
	int result = FCGX_VFPrintF(context->fcgxRequest.out, format, argList);
	va_end(argList);
	return result;
}
/**
 * Load the global context of the server
 */
void hados_context_load(struct hados_context *context) {
	context->bytes_received = 0;
	// Retrieve the data directory if not already set
	if (context->data_dir == NULL )
		context->data_dir = hados_context_get_env_dup(context, "HADOS_DATADIR");

	// Retrieve the my public URL if not already set
	if (context->node == NULL )
		context->node = hados_context_get_env_dup(context, "HADOS_NODE");

	// Retrieve list of other nodes if not already set
	if (context->nodes == NULL ) {
		context->nodes = hados_context_get_env_dup(context, "HADOS_NODES");
		if (context->nodes == NULL )
			return;

		char *nodes2 = strdup(context->nodes);
		char *node = strtok(nodes2, " ");
		context->nodesNumber = 0;
		while (node != NULL ) {
			context->nodesNumber++;
			node = strtok(NULL, " ");
		}
		free(nodes2);

		if (context->nodesNumber > 0) {
			context->nodeArray = (char **) malloc(
					context->nodesNumber * sizeof(char *));
			char *node = strtok(context->nodes, " ");
			int i = 0;
			while (node != NULL ) {
				context->nodeArray[i++] = node;
				node = strtok(NULL, " ");
			}
		}
	}
}

int hados_context_set_object(struct hados_context *context,
		struct hados_request *request, struct hados_response *response) {
	return hados_object_init(&context->object, context, request, response);
}

void hados_context_free(struct hados_context *context) {
	hados_object_free(&context->object);
	if (context->nodes != NULL ) {
		free(context->nodes);
		context->nodes = NULL;
	}
	if (context->nodeArray != NULL ) {
		free(context->nodeArray);
		context->nodeArray = NULL;
	}
	if (context->data_dir != NULL ) {
		free(context->data_dir);
		context->data_dir = NULL;
	}
	if (context->node != NULL ) {
		free(context->node);
		context->node = NULL;
	}
}
