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
	context->data_dir = NULL;
	context->node = NULL;
	context->nodes = NULL;
	context->nodeArray = NULL;
	context->nodesNumber = 0;
	context->currentFilePath = 0;
}

/**
 * Load the global context of the server
 */
void hados_context_load(struct hados_context *context) {
	// Retrieve the data directory if not already set
	if (context->data_dir == NULL )
		context->data_dir = strdup(getenv("HADOS_DATADIR"));

	// Retrieve the my public URL if not already set
	if (context->node == NULL )
		context->node = strdup(getenv("HADOS_NODE"));

	// Retrieve list of other nodes if not already set
	if (context->nodes == NULL ) {
		context->nodes = strdup(getenv("HADOS_NODES"));
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

void hados_context_free(struct hados_context *context) {
	if (context->nodes != NULL ) {
		free(context->nodes);
		context->nodes = NULL;
	}
	if (context->nodeArray != NULL ) {
		free(context->nodeArray);
		context->nodeArray = NULL;
	}
	if (context->currentFilePath != NULL ) {
		free(context->currentFilePath);
		context->currentFilePath = NULL;
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

void hados_context_set_file_path(struct hados_context *context,
		const char* path) {
	if (context->currentFilePath != NULL )
		free(context->currentFilePath);
	context->currentFilePath = malloc(
			(strlen(path) + strlen(context->data_dir) + 2) * sizeof(char));
	context->paramPath = (char*) path;
	hados_concat_path(context->data_dir, path, context->currentFilePath);
}
