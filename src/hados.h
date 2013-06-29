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
 Name        : hados.h
 Author      : Emmanuel Keller
 Version     :
 Copyright   : Jaeksoft / Emmanuel Keller
 ============================================================================
 */
#pragma once

#include "fcgi_stdio.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <dirent.h>

// defined in context.c

struct hados_context {
	char *node; // The public URL of myself
	int nodesNumber; // Number of nodes in the cluster
	char *nodes; // The string defining the list of nodes
	char **nodeArray; // Node of the cluster
	char *data_dir; // The directory where the index are stored
	char *currentFilePath; // The current file
	char *paramPath; // The current path param
};

void hados_context_init(struct hados_context *context);
void hados_context_load(struct hados_context *context);
void hados_context_free(struct hados_context *context);
void hados_context_set_file_path(struct hados_context *context,
		const char* path);

//defined in request.c

struct hados_request {
	int count;
	char *queryString;
	char **keyvalue;
	char **key;
	char **value;
	char *command;
	struct timeval requestTime;
};

void hados_request_init(struct hados_request *request);
void hados_request_free(struct hados_request *request);
void hados_request_load(struct hados_request *request, const char* queryString);
char* hados_request_getvalue(struct hados_request *request, const char* key);

//defined in response.c

struct hados_response {
	int status;
	char *message;
};

void hados_response_init(struct hados_response *response);
void hados_response_free(struct hados_response *response);
int hados_response_set_status(struct hados_response *response, int status,
		const char* message);
int hados_response_set_errno(struct hados_response *response);
int hados_response_set_success(struct hados_response *response);
void hados_response_write(struct hados_response *response,
		struct hados_context *context, struct hados_request *request);

//define in commands.c

void hados_command_dispatch(struct hados_context *context,
		struct hados_request *request, struct hados_response *response);

//defined in external.c

struct MemoryStruct {
	char *memory;
	size_t size;
};

int hados_external_put_if_exists(struct hados_context *context);

//define in utils.c

char* hados_concat_path(const char *root_dir, const char *file_name,
		char *buffer);

// Constants

#define HADOS_MAX_PATH_LENGTH				2048

// Status & errors

#define HADOS_SUCCESS						0
#define HADOS_BINARY_RESULT					777777
#define HADOS_UNKNOWN_COMMAND				888888
#define HADOS_INTERNAL_ERROR				999999
#define HADOS_DATADIR_NOT_SET				100001
#define HADOS_DATADIR_DONT_EXISTS			100002
#define HADOS_DATADIR_IS_NOT_A_DIRECTORY	100003
#define HADOS_PATH_IS_MISSING				100004
#define HADOS_PATH_TOO_LONG					100005
#define HADOS_WRONG_CHARACTER_IN_PATH		100006
#define HADOS_OBJECT_NOT_FOUND				404404
#define HADOS_OBJECT_FOUND					200200

