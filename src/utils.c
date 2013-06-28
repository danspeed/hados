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
 Name        : utils.c
 Author      : Emmanuel Keller
 Version     :
 Copyright   : Jaeksoft / Emmanuel Keller
 ============================================================================
 */

#include "fcgi_stdio.h"
#include <stdlib.h>
#include <string.h>

char* hados_concat_path(const char *root_dir, const char *file_name,
		char *buffer) {
	strcpy(buffer, root_dir);
	if (file_name == NULL)
		return buffer;
	if (strlen(file_name) == 0)
		return buffer;
	if (file_name[0] != '/')
		strcat(buffer, "/");
	strcat(buffer, file_name);
	return buffer;
}
