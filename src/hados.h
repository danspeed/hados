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
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/statvfs.h>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include "fcgiapp.h"
#include <curl/curl.h>
#include "json.h"

typedef struct hados_context hados_context;

struct hados_object {
	char *filepath;
	char *filename;
	hados_context *context;
};

/**
 * Contains the information used to create the response. Handling HTTP result code and the JSON returned.
 */
struct hados_response {
	int status; // The status code
	int http_status; // The HTTP code that will be returned
	char *moreheader; // Any additional formatted HTTP header
	char *message; // The JSON message
	char *morejson; // Any additional JSON formated message
	hados_context *context;
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

struct hados_context {
	// Application global
	char *node; // The public URL of myself
	int nodesNumber; // Number of nodes in the cluster
	char *nodes; // The string defining the list of nodes
	char **nodeArray; // Node of the cluster
	char *data_dir; // The directory where the data are stored
	char *file_dir; // The directory where the files are stored
	char *temp_dir; // The directory where the files are stored
	// Transaction global
	FCGX_Request fcgxRequest; //Fast CGI
	struct hados_request request; // Users request
	struct hados_response response; // Users response
	struct hados_object object; // The current file object
	long bytes_received;
};

struct hados_nodes {
	int length;
	char *array;
};

/**
 * Hold the information used to request another node
 */
struct hados_external {
	char *body;
	size_t size;
	int hados_status;
	json_value* json;
	struct hados_context *context;
};

/**
 * Define a temp file stored in the temp directory of hados
 */
struct hados_tempfile {
	char *path;
};

/**
 * Define a file with its type
 */
struct hados_fileitem {
	char *name;
	char *type;
};

/**
 * An array of fileitem
 */
struct hados_fileitem_array {
	size_t length;
	struct hados_fileitem *fileitems;
};

// defined in object.c

void hados_object_init(struct hados_object *object,
		struct hados_context *context);
int hados_object_load(struct hados_object *object);
void hados_object_free(struct hados_object *object);

// defined in context.c

void hados_context_init(struct hados_context *context);
void hados_context_transaction_init(struct hados_context *context);
void hados_context_transaction_free(struct hados_context *context);
const char* hados_context_get_env(struct hados_context *context,
		const char* param);
char* hados_context_get_env_dup(struct hados_context *context,
		const char* param);
int hados_context_printf(const struct hados_context *context,
		const char *format, ...);
int hados_context_error_printf(struct hados_context *context,
		const char *format, ...);
void hados_context_free(struct hados_context *context);
int hados_context_set_object(struct hados_context *context);

//defined in request.c

void hados_request_init(struct hados_request *request);
void hados_request_free(struct hados_request *request);
void hados_request_load(struct hados_request *request, const char* queryString);
char* hados_request_getvalue(struct hados_request *request, const char* key);

//defined in response.c

void hados_response_init(struct hados_response *response,
		struct hados_context *context);
void hados_response_free(struct hados_response *response);
int hados_response_set_status(struct hados_response *response, int status,
		const char* message);
void hados_response_more_json(struct hados_response *response,
		const char* morejson);
void hados_response_more_header(struct hados_response *response,
		const char* header, const char* value);
int hados_response_set_errno(struct hados_response *response);
int hados_response_set_success(struct hados_response *response);
void hados_response_write(struct hados_response *response);

//define in commands.c

void hados_command_dispatch(struct hados_context *context);

//defined in external.c
void hados_external_init(struct hados_external *external,
		struct hados_context *context);
void hados_external_free(struct hados_external *external);
char* hados_external_url(struct hados_external *external, const char* node_url,
		const char *cmd, const char *path);
int hados_external_exists(struct hados_external *external, const char* node_url,
		const char* path);
int hados_external_delete(struct hados_external *external, const char* node_url,
		const char* path);
int hados_external_list(struct hados_external *external, const char* node_url,
		const char* path);
int hados_external_put(struct hados_external *external,
		struct hados_tempfile *tempfile, const char* node_url, const char*path);
json_value* hados_external_get_json(struct hados_external *external);

//define in utils.c

char* hados_utils_concat_path(const char *root_dir, const char *file_name,
		char *buffer);
char* hados_utils_strcat(char* str, const char* add);
int hados_utils_mkdirs(const char *file_path, struct hados_response *response);
int hados_utils_mkdir_if_not_exists(struct hados_context *context,
		const char *dir_path);
json_value* hados_utils_json_get(json_value* json, const char* name);
char *hados_utils_json_get_string(json_value* json, const char* name);
json_value* hados_utils_json_get_array(json_value* json, const char* name);

// define in tempfile.c

int hados_tempfile_new(struct hados_tempfile *tempfile,
		struct hados_context *context);
void hados_tempfile_free(struct hados_tempfile *tempfile);
int hados_tempfile_upload(struct hados_tempfile *tempfile,
		struct hados_context *context);

// defined in fileitem.c

int hados_fileitem_cmp(const void *v1, const void *v2);
void hados_fileitem_to_json(struct hados_fileitem *fileitem, char* s,
		size_t length);
void hados_fileitem_static(struct hados_fileitem *fileitem, struct dirent *ep);
void hados_fileitem_array_init(struct hados_fileitem_array *array);
void hados_fileitem_array_free(struct hados_fileitem_array *array);
void hados_fileitem_array_load(struct hados_fileitem_array *array,
		json_value *json);
void hados_fileitem_array_sort(struct hados_fileitem_array *array);

// defined in nodes.c

void hados_nodes_init(struct hados_nodes *nodes, int length);
void hados_nodes_free(struct hados_nodes *nodes);
void hados_nodes_set(struct hados_nodes *nodes, int pos, char val);

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
#define HADOS_PATH_IS_NOT_A_DIRECTORY		100009
#define HADOS_OBJECT_NOT_FOUND				404
#define HADOS_OBJECT_FOUND					200
#define HADOS_REDIRECT						302

// Hados HTTP header
#define HADOS_HEADER_STATUS		"X-Hados-Status"
