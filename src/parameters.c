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
 Name        : parameters.c
 Author      : Emmanuel Keller
 Version     :
 Copyright   : Jaeksoft / Emmanuel Keller
 ============================================================================
 */

#include "fcgi_stdio.h"
#include <stdlib.h>
#include <string.h>
#include "hados.h"

void hados_parameters_init(struct hados_parameters *parameters) {
	parameters->count = 0;
	parameters->key = NULL;
	parameters->value = NULL;
	parameters->keyvalue = NULL;
}

void hados_parameters_free(struct hados_parameters *parameters) {
	if (parameters->key != NULL ) {
		free(parameters->key);
		parameters->key = NULL;
	}
	if (parameters->value != NULL ) {
		free(parameters->value);
		parameters->value = NULL;
	}
	if (parameters->keyvalue != NULL ) {
		free(parameters->keyvalue);
		parameters->keyvalue = NULL;
	}
	if (parameters->queryString != NULL ) {
		free(parameters->queryString);
		parameters->queryString = NULL;
	}
	parameters->count = 0;
}

/**
 * Build the parameters structure using the query string
 */
void hados_parameters_load(struct hados_parameters *parameters,
		const char* queryString) {
	hados_parameters_free(parameters);

	// Count the number of parameters
	char *qs = strdup(queryString);
	char *token = strtok(qs, "&");
	while (token != NULL ) {
		parameters->count++;
		token = strtok(NULL, "&");
	}
	free(qs);
	if (parameters->count == 0)
		return;
	parameters->keyvalue = (char **) malloc(parameters->count * sizeof(char *));
	parameters->key = (char **) malloc(parameters->count * sizeof(char *));
	parameters->value = (char **) malloc(parameters->count * sizeof(char *));

	// Populate the keyvalue array
	parameters->queryString = strdup(queryString);
	token = strtok(qs, "&");
	int pos = 0;
	while (token != NULL ) {
		parameters->keyvalue[pos++] = token;
		token = strtok(NULL, "&");
	}

	// Populate the key/value array
	int i = 0;
	for (i = 0; i < parameters->count; i++) {
		parameters->key[i] = strtok(parameters->keyvalue[i], "=");
		parameters->value[i] = strtok(NULL, "=");
	}
}

/**
 * Retrieve the value of the parameter with the passed key
 */
char* hados_parameters_getvalue(struct hados_parameters *parameters,
		const char* key) {
	int i;
	for (i = 0; i < parameters->count; i++)
		if (parameters->key[i] != NULL )
			if (strcmp(key, parameters->key[i]) == 0)
				return parameters->value[i];
	return NULL ;
}
