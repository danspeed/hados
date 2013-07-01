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
	hados_object_init(&context->object);
	context->data_dir = NULL;
	context->file_dir = NULL;
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

int hados_context_error_printf(struct hados_context *context,
		const char *format, ...) {
	va_list argList;
	va_start(argList, format);
	int result = FCGX_VFPrintF(context->fcgxRequest.err, format, argList);
	va_end(argList);
	return result;
}
/**
 * Load the global context of the server
 */
void hados_context_load(struct hados_context *context) {
	context->bytes_received = 0;

	struct stat st;

	// Retrieve the data directory if not already set
	if (context->data_dir == NULL ) {
		context->data_dir = hados_context_get_env_dup(context, "HADOS_DATADIR");
		int err = stat(context->data_dir, &st);
		if (err == 0) {
			if (!S_ISDIR(st.st_mode)) {
				hados_context_error_printf(context,
						"HADOS_DATADIR is not a directory: %s",
						context->data_dir);
				err = -1;
			}
		} else {
			if (errno == ENOENT)
				hados_context_error_printf(context,
						"HADOS_DATADIR does not exists: %s", context->data_dir);
		}
		//Check and or create the file directory
		if (err == 0) {
			if (context->file_dir == NULL ) {
				context->file_dir = malloc(
						(strlen(context->data_dir) + 7) * sizeof(char));
				hados_utils_concat_path(context->data_dir, "/files",
						context->file_dir);
			}
			err = stat(context->file_dir, &st);
			if (err == 0) {
				if (!S_ISDIR(st.st_mode)) {
					hados_context_error_printf(context,
							"File path is not a directory: %s",
							context->file_dir);
					err = -1;
				}
			} else {
				if (errno == ENOENT) {
					err = mkdir(context->file_dir, S_IRWXU);
				} else
					hados_context_error_printf(context,
							"Issue with file directory %s", context->file_dir);
			}
			if (err == -1) {
				free(context->file_dir);
				context->file_dir = NULL;
			}
		}
	}

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
	return hados_object_load(&context->object, context, request, response);
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
	if (context->file_dir != NULL ) {
		free(context->file_dir);
		context->file_dir = NULL;
	}
	if (context->node != NULL ) {
		free(context->node);
		context->node = NULL;
	}
}
