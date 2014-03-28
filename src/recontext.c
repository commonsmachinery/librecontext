/*
 * librecontext - a lightweight metadata handling library
 *
 * Copyright 2014 Commons Machinery http://commonsmachinery.se/
 * Authors: Artem Popov <artfwo@commonsmachinery.se>
 *
 * Distributed under the MIT license, please see LICENSE in the top dir.
 */

#include <stdlib.h>

#include "recontext.h"

static librdf_world *world = NULL;

struct recontext_s {
    librdf_storage  *storage;
    librdf_model    *model;
    librdf_parser   *parser;
};

void recontext_init()
{
    world = librdf_new_world();
    librdf_world_open(world);
}

void recontext_cleanup()
{
    librdf_free_world(world);
}

recontext* recontext_new()
{
    recontext* rc;

    rc = malloc(sizeof(recontext));
    rc->storage = librdf_new_storage(world, "memory", NULL, NULL);
    rc->model = librdf_new_model(world, rc->storage, NULL);
    rc->parser = librdf_new_parser (world, "rdfxml", NULL, NULL);

    return rc;
}

recontext* recontext_new_from_string_with_uri(const char *rdf_xml, const char *uri_str)
{
    recontext* rc;
    librdf_uri *uri;

    rc = recontext_new();

    uri = librdf_new_uri (world, (const unsigned char *) uri_str);
    librdf_parser_parse_string_into_model (rc->parser, rdf_xml, uri, rc->model);

    return rc;
}

recontext* recontext_new_from_string(const char *rdf_xml)
{
    recontext* rc;

    rc = recontext_new_from_string_with_uri(rdf_xml, "about:this");

    return rc;
}
