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

struct hados_parameter {
	char *key;
	char *value;
};

typedef struct hados_parameter hados_parameter;

//defined in hados.c
char* hados_getParameter(struct hados_parameter *parameters, const char* key);

//define in commands.c
int hados_put(const char *data_dir, struct hados_parameter *parameters);
int hados_get(const char *data_dir, struct hados_parameter *parameters);
int hados_delete(const char *data_dir, struct hados_parameter *parameters);

#define HADOS_MAX_PATH_LENGTH				2048

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

