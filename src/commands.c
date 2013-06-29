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
 Name        : commands.c
 Author      : Emmanuel Keller
 Version     :
 Copyright   : Jaeksoft / Emmanuel Keller
 ============================================================================
 */

#include "hados.h"

static int checkDataDir(const char *data_dir, struct hados_response *response) {
	if (data_dir == NULL || strlen(data_dir) == 0)
		return hados_response_set_status(response, HADOS_DATADIR_NOT_SET,
				"The data directory has not been set");
	struct stat s;
	int err = stat(data_dir, &s);
	if (err == -1) {
		if (errno == ENOENT)
			return hados_response_set_status(response, HADOS_DATADIR_NOT_SET,
					"The data directory does not exits");
		return hados_response_set_errno(response);
	}
	if (!S_ISDIR(s.st_mode))
		return hados_response_set_status(response,
				HADOS_DATADIR_IS_NOT_A_DIRECTORY,
				"The data directory path is not a directory");
	return hados_response_set_success(response);
}

static char* checkPath(struct hados_request *request,
		struct hados_response *response) {
	char* path = hados_request_getvalue(request, "path");
	if (path == NULL ) {
		hados_response_set_status(response, HADOS_PATH_IS_MISSING,
				"The path is missing");
		return NULL ;
	}
	if (strstr(path, "../") != NULL ) {
		hados_response_set_status(response, HADOS_WRONG_CHARACTER_IN_PATH,
				"Not allowed character in the path");
		return NULL ;
	}
	if (strstr(path, "./") != NULL ) {
		hados_response_set_status(response, HADOS_WRONG_CHARACTER_IN_PATH,
				"Not allowed character in the path");
		return NULL ;
	}
	if (strstr(path, "//") != NULL ) {
		hados_response_set_status(response, HADOS_WRONG_CHARACTER_IN_PATH,
				"Not allowed character in the path");
		return NULL ;
	}
	char *pc = path;
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
		if (pos > HADOS_MAX_PATH_LENGTH) {
			hados_response_set_status(response, HADOS_PATH_TOO_LONG,
					"The path is too long");
			return NULL ;
		}
		hados_response_set_status(response, HADOS_WRONG_CHARACTER_IN_PATH,
				"Not allowed character in the path");
		return NULL ;
	}
	return path;
}

static int mkdirs(const char *file_path, struct hados_response *response) {
	struct stat st;
	char path[strlen(file_path) + 1];
	strcpy(path, file_path);
	char *p = path;
	int dirExists = 0;
	while (*p != 0) {
		if (*p == '/') {
			*p = 0;
			if (strlen(path) > 0) {
				dirExists = 0;
				if (stat(path, &st) == 0)
					if (S_ISDIR(st.st_mode))
						dirExists = 1;
				if (dirExists == 0)
					if (mkdir(path, S_IRWXU) == -1)
						return hados_response_set_errno(response);
			}
			*p = '/';
		}
		p++;
	}
	return hados_response_set_success(response);
}

static int hados_command_put(struct hados_context *context,
		struct hados_request *request, struct hados_response *response) {
	if (checkDataDir(context->data_dir, response) != HADOS_SUCCESS)
		return response->status;
	char *path = checkPath(request, response);
	if (response->status != HADOS_SUCCESS)
		return response->status;
	hados_context_set_file_path(context, path);
	if (mkdirs(context->currentFilePath, response) != HADOS_SUCCESS)
		return response->status;
	FILE *file = fopen(context->currentFilePath, "wb");
	if (file == NULL )
		return hados_response_set_errno(response);
	long bytes = 0;
	int c;
	while ((c = getchar()) != EOF) {
		bytes++;
		if (fputc(c, file) == EOF) {
			hados_response_set_errno(response);
			break;
		}
	}
	fclose(file);
	return response->status;
}

static int hados_command_get(struct hados_context *context,
		struct hados_request *request, struct hados_response *response) {
	if (checkDataDir(context->data_dir, response) != HADOS_SUCCESS)
		return response->status;
	char *path = checkPath(request, response);
	if (response->status != HADOS_SUCCESS)
		return response->status;
	hados_context_set_file_path(context, path);
	struct stat st;
	if (stat(context->currentFilePath, &st) != 0) {
		if (errno == ENOENT)
			return hados_response_set_status(response, HADOS_OBJECT_NOT_FOUND,
					"Object/file not found");
		return hados_response_set_errno(response);
	}
	FILE *file = fopen(context->currentFilePath, "rb");
	if (file == NULL )
		return hados_response_set_errno(response);
	response->status = HADOS_BINARY_RESULT;
	printf("Content-type: application/octet-stream\r\n");
	printf("Content-Transfer-Encoding: binary\r\n\r\n");
	int c;
	while ((c = fgetc(file)) != EOF)
		putchar(c);
	fclose(file);
	return response->status;
}

static int hados_command_delete(struct hados_context *context,
		struct hados_request *request, struct hados_response *response) {
	if (checkDataDir(context->data_dir, response) != HADOS_SUCCESS)
		return response->status;
	char *path = checkPath(request, response);
	if (response->status != HADOS_SUCCESS)
		return response->status;
	hados_context_set_file_path(context, path);
	struct stat st;
	if (stat(context->currentFilePath, &st) != 0) {
		if (errno == ENOENT)
			return hados_response_set_status(response, HADOS_OBJECT_NOT_FOUND,
					"Object/file not found");
		return hados_response_set_errno(response);
	}
	if (unlink(context->currentFilePath) != 0)
		return hados_response_set_errno(response);
	return hados_response_set_success(response);
}

static int hados_command_exists(struct hados_context *context,
		struct hados_request *request, struct hados_response *response) {
	if (checkDataDir(context->data_dir, response) != HADOS_SUCCESS)
		return response->status;
	char *path = checkPath(request, response);
	if (response->status != HADOS_SUCCESS)
		return response->status;
	hados_context_set_file_path(context, path);
	struct stat st;
	if (stat(context->currentFilePath, &st) != 0) {
		if (errno == ENOENT)
			return hados_response_set_status(response, HADOS_OBJECT_NOT_FOUND,
					"Object/file not found");
		return hados_response_set_errno(response);
	}
	return hados_response_set_status(response, HADOS_OBJECT_FOUND,
			"Object/file exists");
}

void hados_command_dispatch(struct hados_context *context,
		struct hados_request *request, struct hados_response *response) {

	if (request->command == NULL )
		return;

	if (strcmp(request->command, "put") == 0) {
		hados_command_put(context, request, response);
		if (response->status == HADOS_SUCCESS)
			hados_external_put_if_exists(context);
	} else if (strcmp(request->command, "get") == 0) {
		hados_command_get(context, request, response);
	} else if (strcmp(request->command, "delete") == 0) {
		hados_command_delete(context, request, response);
	} else if (strcmp(request->command, "exists") == 0) {
		hados_command_exists(context, request, response);
	} else {
		hados_response_set_status(response, HADOS_UNKNOWN_COMMAND,
				"Unknown command");
	}
}
