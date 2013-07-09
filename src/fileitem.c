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
 Name        : fileitem.c
 Author      : Emmanuel Keller
 Version     :
 Copyright   : Jaeksoft / Emmanuel Keller
 ============================================================================
 */

#include "hados.h"

int hados_fileitem_cmp(const void *v1, const void *v2) {
	const struct hados_fileitem *o1 = v1;
	const struct hados_fileitem *o2 = v2;
	int i = strcmp(o1->name, o2->name);
	if (i != 0)
		return i;
	return strcmp(o1->type, o2->type);
}

void hados_fileitem_to_json(struct hados_fileitem *fileitem, char* s,
		size_t length) {
	snprintf(s, length, "{ \"name\":\"%s\", \"type\":\"%s\" }", fileitem->name,
			fileitem->type);
}

void hados_fileitem_init(struct hados_fileitem *fileitem, json_value *json) {
	fileitem->name = NULL;
	fileitem->type = NULL;
	if (json == NULL )
		return;
	fileitem->name = hados_utils_json_get_string(json, "name");
	if (fileitem->name != NULL )
		fileitem->name = strdup(fileitem->name);
	fileitem->type = hados_utils_json_get_string(json, "type");
	if (fileitem->type != NULL )
		fileitem->type = strdup(fileitem->type);
}

void hados_fileitem_static(struct hados_fileitem *fileitem, struct dirent *ep) {
	fileitem->name = ep->d_name;
	fileitem->type = DT_DIR == ep->d_type ? "dir" : "file";
}

void hados_fileitem_free(struct hados_fileitem *fileitem) {
	if (fileitem->name != NULL ) {
		free(fileitem->name);
		fileitem->name = NULL;
	}
	if (fileitem->type != NULL ) {
		free(fileitem->type);
		fileitem->type = NULL;
	}
}

void hados_fileitem_array_init(struct hados_fileitem_array *array) {
	array->length = 0;
	array->fileitems = NULL;
}

void hados_fileitem_array_free(struct hados_fileitem_array *array) {
	if (array->fileitems != NULL ) {
		int i = 0;
		for (i = 0; i < array->length; i++)
			hados_fileitem_free(&array->fileitems[i]);
		free(array->fileitems);
		array->fileitems = NULL;
		array->length = 0;
	}
}

/**
 * Fill the array with the file items provided by the json array.
 * If the array is not empty, the new items will be added.
 */
void hados_fileitem_array_load(struct hados_fileitem_array *array,
		json_value *json) {
	if (json->type != json_array)
		return;
	if (json->u.array.length == 0)
		return;
	int pos = array->length;
	array->length += json->u.array.length;
	if (array->fileitems == NULL )
		array->fileitems = malloc(
				sizeof(struct hados_fileitem) * array->length);
	else
		array->fileitems = realloc(array->fileitems,
				sizeof(struct hados_fileitem) * array->length);
	int i;
	for (i = 0; i < json->u.array.length; i++)
		hados_fileitem_init(&array->fileitems[pos++], json->u.array.values[i]);
}

void hados_fileitem_array_sort(struct hados_fileitem_array *array) {
	qsort(array->fileitems, array->length, sizeof(struct hados_fileitem),
			hados_fileitem_cmp);
}
