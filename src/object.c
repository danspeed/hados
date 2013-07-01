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
 Name        : object.c
 Author      : Emmanuel Keller
 Version     :
 Copyright   : Jaeksoft / Emmanuel Keller
 ============================================================================
 */

#include "hados.h"

void hados_object_init(struct hados_object *object) {
	object->filepath = NULL;
	object->filename = NULL;
}

int hados_object_load(struct hados_object *object,
		struct hados_context *context, struct hados_request *request,
		struct hados_response *response) {
	hados_object_free(object);
	if (request->paramPath == NULL )
		return hados_response_set_status(response, HADOS_PATH_IS_MISSING,
				"The path is missing");
	// We don't want changing directory
	if (strstr(request->paramPath, "../") != NULL )
		return hados_response_set_status(response,
				HADOS_WRONG_CHARACTER_IN_PATH,
				"Not allowed character in the path");
	if (strstr(request->paramPath, "./") != NULL )
		return hados_response_set_status(response,
				HADOS_WRONG_CHARACTER_IN_PATH,
				"Not allowed character in the path");
	if (strstr(request->paramPath, "//") != NULL )
		return hados_response_set_status(response,
				HADOS_WRONG_CHARACTER_IN_PATH,
				"Not allowed character in the path");
	// Find the name of the file
	object->filename = strrchr(request->paramPath, '/');
	if (object->filename == NULL )
		object->filename = request->paramPath;
	else
		object->filename++;

	char *pc = object->filename;
	char c;
	int pos = 0;
	while ((c = *pc) != 0) {
		pc++;
		pos++;
		if (c == 37 || c == 43 || c == 47 || c == 46)
			continue;
		if (c >= 48 && c <= 57)
			continue;
		if (c >= 65 && c <= 90)
			continue;
		if (c >= 97 && c <= 122)
			continue;
		if (pos > HADOS_MAX_PATH_LENGTH)
			return hados_response_set_status(response, HADOS_PATH_TOO_LONG,
					"The path is too long");
		return hados_response_set_status(response,
				HADOS_WRONG_CHARACTER_IN_PATH,
				"Not allowed character in the path");
	}

	object->filepath = malloc(
			(strlen(request->paramPath) + strlen(context->file_dir) + 2)
					* sizeof(char));
	hados_utils_concat_path(context->file_dir, request->paramPath,
			object->filepath);
	return HADOS_SUCCESS;
}

void hados_object_free(struct hados_object *object) {
	if (object->filepath != NULL ) {
		free(object->filepath);
		object->filepath = NULL;
	}
}
