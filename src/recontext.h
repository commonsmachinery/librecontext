/*
 * librecontext - a lightweight metadata handling library
 *
 * Copyright 2014 Commons Machinery http://commonsmachinery.se/
 * Authors: Artem Popov <artfwo@commonsmachinery.se>
 *
 * Distributed under the MIT license, please see LICENSE in the top dir.
 */

#ifndef __RECONTEXT_H__
#define __RECONTEXT_H__

#include <redland.h>

typedef struct recontext_s recontext;

void recontext_init();
void recontext_cleanup();

recontext* recontext_new();
recontext* recontext_new_from_string_with_uri(const char *rdf_xml, const char *uri_str);
recontext* recontext_new_from_string(const char *rdf_xml);

#endif /* __RECONTEXT_H__ */
