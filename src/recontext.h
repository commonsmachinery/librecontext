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

struct recontext_s {
    librdf_world    *world;
    librdf_storage  *storage;
    librdf_model    *model;
    librdf_parser   *parser;

    char *main_subject;
};

typedef struct recontext_s recontext;

recontext*      recontext_new(const char *subject);
recontext*      recontext_new_from_string(const char *rdf_xml, const char *uri_str);
recontext*      recontext_new_from_file(const char *filename, const char *uri_str);
recontext*      recontext_new_from_xmp(const char *packet, const char *base_uri);

recontext*      recontext_extract(recontext* rc, char* subject, int remove);
void            recontext_merge(recontext *rc, recontext* other, const char *relation);

char*           recontext_serialize(recontext *rc);
const char*     recontext_get_main_subject (recontext *rc);

void            recontext_destroy(recontext *rc);

#endif /* __RECONTEXT_H__ */
