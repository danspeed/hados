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

#include "fcgi_config.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/statvfs.h>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include "fcgiapp.h"

struct hados_object {
	char *filepath;
	char *filename;
};

/**
 * Contains the information used to create the response. Handling HTTP result code and the JSON returned.
 */
struct hados_response {
	int status; // The status code
	int http_status; // The HTTP code that will be returned
	char *message; // The JSON message
	char *morejson; // Any additional JSON formated message
};

struct hados_context {
	FCGX_Request fcgxRequest;
	char *node; // The public URL of myself
	int nodesNumber; // Number of nodes in the cluster
	char *nodes; // The string defining the list of nodes
	char **nodeArray; // Node of the cluster
	char *data_dir; // The directory where the data are stored
	char *file_dir; // The directory where the files are stored
	char *temp_dir; // The directory where the files are stored
	struct hados_object object; // The current file object
	long bytes_received;
};

struct hados_request {
	int count;
	char *queryString;
	char **keyvalue;
	char **key;
	char **value;
	char *command;
	char *paramPath;
	struct timeval requestTime;
};

struct hados_tempfile {
	char *path;
	FILE *fd;
};

// defined in object.c

void hados_object_init(struct hados_object *object);
int hados_object_load(struct hados_object *hados_object,
		struct hados_context *context, struct hados_request *request,
		struct hados_response *response);
void hados_object_free(struct hados_object *hados_object);

// defined in context.c

void hados_context_init(struct hados_context *context);
void hados_context_load(struct hados_context *context);
const char* hados_context_get_env(struct hados_context *context,
		const char* param);
char* hados_context_get_env_dup(struct hados_context *context,
		const char* param);
int hados_context_printf(struct hados_context *context, const char *format, ...);
int hados_context_error_printf(struct hados_context *context,
		const char *format, ...);
void hados_context_free(struct hados_context *context);
int hados_context_set_object(struct hados_context *context,
		struct hados_request *request, struct hados_response *response);

//defined in request.c

void hados_request_init(struct hados_request *request);
void hados_request_free(struct hados_request *request);
void hados_request_load(struct hados_request *request,
		struct hados_context *context);
char* hados_request_getvalue(struct hados_request *request, const char* key);

//defined in response.c

void hados_response_init(struct hados_response *response);
void hados_response_free(struct hados_response *response);
int hados_response_set_status(struct hados_response *response, int status,
		const char* message);
void hados_response_more_json(struct hados_response *response,
		const char* morejson);
int hados_response_set_errno(struct hados_response *response);
int hados_response_set_success(struct hados_response *response);
void hados_response_write(struct hados_response *response,
		struct hados_context *context, struct hados_request *request);

//define in commands.c

void hados_command_dispatch(struct hados_context *context,
		struct hados_request *request, struct hados_response *response);

//defined in external.c

char* hados_external_url(const char* node_url, const char *cmd,
		const char *path);
int hados_external_exists(struct hados_response *response, const char* url,
		const char* path);
int hados_external_delete(struct hados_response *response, const char* node_url,
		const char* path);

//define in utils.c

char* hados_utils_concat_path(const char *root_dir, const char *file_name,
		char *buffer);
char* hados_utils_strcat(char* str, const char* add);
int hados_utils_mkdirs(const char *file_path, struct hados_response *response);
int hados_utils_mkdir_if_not_exists(struct hados_context *context,
		const char *dir_path);

// define in tempfile.c
int hados_tempfile_new(struct hados_tempfile *tempfile,
		struct hados_context *context, struct hados_response *response);
void hados_tempfile_free(struct hados_tempfile *tempfile);
int hados_tempfile_upload(struct hados_tempfile *tempfile,
		struct hados_context *context, struct hados_response *response);

// Constants

#define HADOS_MAX_PATH_LENGTH				2048

// Status & errors

#define HADOS_SUCCESS						0
#define HADOS_BINARY_RESULT					777777
#define HADOS_UNKNOWN_COMMAND				888888
#define HADOS_INTERNAL_ERROR				999999
#define HADOS_DATADIR_NOT_SET				100001
#define HADOS_FILEDIR_NOT_SET				100002
#define HADOS_PATH_IS_MISSING				100004
#define HADOS_PATH_TOO_LONG					100005
#define HADOS_WRONG_CHARACTER_IN_PATH		100006
#define HADOS_NO_CONTENT_LENGTH_GIVEN		100007
#define HADOS_NOT_ENOUGH_BYTES_RECEIVED		100008
#define HADOS_OBJECT_NOT_FOUND				404
#define HADOS_OBJECT_FOUND					200
#define HADOS_REDIRECT						302

