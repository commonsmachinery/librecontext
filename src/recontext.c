/*
 * librecontext - a lightweight metadata handling library
 *
 * Copyright 2014 Commons Machinery http://commonsmachinery.se/
 * Authors: Artem Popov <artfwo@commonsmachinery.se>
 *
 * Distributed under the MIT license, please see LICENSE in the top dir.
 */

#include <stdlib.h>
#include <string.h>

#include <uuid/uuid.h>

#include <glib.h>
#include <glib/gstdio.h>

#include "recontext.h"

recontext*
recontext_new(const char *subject)
{
    recontext* rc;
    char uuid_str[37];
    char uuid_uri[45];

    rc = malloc(sizeof(recontext));
    rc->world = librdf_new_world();
    librdf_world_open(rc->world);
    rc->storage = librdf_new_storage(rc->world, "memory", NULL, NULL);
    rc->model = librdf_new_model(rc->world, rc->storage, NULL);
    rc->parser = librdf_new_parser(rc->world, "rdfxml", NULL, NULL);

    if (subject == NULL) {
        uuid_t uuid;
        uuid_generate(uuid);
        uuid_unparse(uuid, uuid_str);
        sprintf(uuid_uri, "urn:uuid:%s", uuid_str);
        rc->main_subject = uuid_uri;
    } else {
        rc->main_subject = malloc(strlen(subject) + 1);
        strcpy(rc->main_subject, subject);
    }

    return rc;
}

recontext*
recontext_new_from_string(const char *rdf_xml, const char *base_uri)
{
    recontext* rc;
    librdf_uri *uri;

    rc = recontext_new(base_uri);
    uri = librdf_new_uri(rc->world, (const unsigned char *) rc->main_subject);
    librdf_parser_parse_string_into_model(rc->parser, rdf_xml, uri, rc->model);

    return rc;
}

recontext*
recontext_new_from_file(const char *filename, const char *base_uri)
{
    int    error;
    int    result;
    char  *contents;
    gsize  length;
    recontext *rc;
    librdf_uri *uri;

    rc = recontext_new(base_uri);
    uri = librdf_new_uri(rc->world, (const unsigned char *) rc->main_subject);

    result = g_file_get_contents(filename, &contents, &length, NULL);

    if (result) {
        error = librdf_parser_parse_counted_string_into_model(rc->parser,
            (const unsigned char *) contents,
            length,
            uri,
            rc->model);
        if (error == 0) {
            return rc;
        } else {
            recontext_destroy(rc);
            return NULL;
        }
    }
}

recontext*
recontext_extract(recontext* rc, char* subject, int remove)
{
    recontext        *new;
    librdf_statement *query_statement;
    librdf_stream    *stream;

    new = recontext_new(subject);

    query_statement = librdf_new_statement_from_nodes(rc->world,
        librdf_new_node_from_uri_string(rc->world, (const unsigned char *) subject), NULL, NULL);

    stream = librdf_model_find_statements(rc->model, query_statement);

    while (!librdf_stream_end(stream)) {
        librdf_statement *statement = librdf_stream_get_object(stream);

        librdf_model_add_statement(new->model, statement);
        if (remove) {
            librdf_model_remove_statement(rc->model, statement);
        }

        librdf_stream_next(stream);
    }

    librdf_free_stream(stream);
    librdf_free_statement(query_statement);
    return new;
}

void
recontext_merge(recontext* rc, recontext *other, const char *relation)
{
    librdf_stream *stream;
    const char *real_relation;

    stream = librdf_model_as_stream(other->model);
    librdf_model_add_statements(rc->model, stream);
    librdf_free_stream(stream);

    if (relation == NULL) {
        real_relation = "http://purl.org/dc/elements/1.1/source";
    } else {
        real_relation = strdup(relation);
    }

    librdf_model_add (rc->model,
        librdf_new_node_from_uri_string(rc->world, (const unsigned char *) rc->main_subject),
        librdf_new_node_from_uri_string(rc->world, (const unsigned char *) real_relation),
        librdf_new_node_from_uri_string(rc->world, (const unsigned char *) other->main_subject));
}

recontext*
recontext_new_from_xmp(const char *packet, const char *base_uri)
{
    recontext     *rc;
    char          *rdf_start;
    char          *rdf_end;
    char          *rdf;
    int            result;
    librdf_uri    *uri;

    rdf_start = g_strstr_len(packet, -1, "<rdf:RDF");
    rdf_end = g_strrstr(packet, "</rdf:RDF>") + 10;

    rdf = g_strndup(rdf_start, rdf_end - rdf_start);

    rc = recontext_new_from_string(rdf, base_uri);

    free(rdf);
    return rc;
}

char*
recontext_serialize(recontext *rc)
{
    librdf_uri *uri_dc;
    librdf_uri *uri_dcterms;
    librdf_uri *uri_cc;
    librdf_uri *uri_xhv;
    librdf_uri *uri_og;
    librdf_uri *feature_uri;
    librdf_uri *base_uri;
    librdf_node *feature_node;

    librdf_model *serialize_model;
    librdf_storage *serialize_storage;
    librdf_stream *orig_stream;

    librdf_serializer *serializer;
    unsigned char *result;

    uri_dc = librdf_new_uri(rc->world,
        (const unsigned char *) "http://purl.org/dc/elements/1.1/");
    uri_dcterms = librdf_new_uri(rc->world,
        (const unsigned char *) "http://purl.org/dc/terms/");
    uri_cc = librdf_new_uri(rc->world,
        (const unsigned char *) "http://creativecommons.org/ns#");
    uri_xhv = librdf_new_uri(rc->world,
        (const unsigned char *) "http://www.w3.org/1999/xhtml/vocab#");
    uri_og = librdf_new_uri(rc->world,
        (const unsigned char *) "http://ogp.me/ns#");

    // copy the original model
    serialize_storage = librdf_new_storage(rc->world, "memory", NULL, NULL);
    serialize_model = librdf_new_model(rc->world, serialize_storage, NULL);

    orig_stream = librdf_model_as_stream(rc->model);
    librdf_model_add_statements(serialize_model, orig_stream);
    librdf_free_stream(orig_stream);

    base_uri = librdf_new_uri(rc->world, (const unsigned char *) rc->main_subject);

    serializer = librdf_new_serializer(rc->world, "rdfxml-abbrev", NULL, NULL);

    librdf_serializer_set_feature(serializer,
        feature_uri = librdf_new_uri(rc->world, (const unsigned char *) "http://feature.librdf.org/raptor-relativeURIs"),
        feature_node = librdf_new_node_from_literal(rc->world, (const unsigned char *) "1", NULL, 0));
    librdf_free_uri(feature_uri);
    librdf_free_node(feature_node);

    librdf_serializer_set_feature(serializer,
        feature_uri = librdf_new_uri(rc->world, (const unsigned char *) "http://feature.librdf.org/raptor-writeBaseURI"),
        feature_node = librdf_new_node_from_literal(rc->world, (const unsigned char *) "0", NULL, 0));
    librdf_free_uri(feature_uri);
    librdf_free_node(feature_node);

    librdf_serializer_set_namespace(serializer, uri_dc, "dc");
    librdf_serializer_set_namespace(serializer, uri_dcterms, "dcterms");
    librdf_serializer_set_namespace(serializer, uri_cc, "cc");
    librdf_serializer_set_namespace(serializer, uri_xhv, "xhv");
    librdf_serializer_set_namespace(serializer, uri_og, "og");

    result = librdf_serializer_serialize_model_to_string(serializer, base_uri, serialize_model);

    librdf_free_model(serialize_model);
    librdf_free_serializer(serializer);
    librdf_free_uri(base_uri);
    librdf_free_uri(uri_dc);
    librdf_free_uri(uri_dcterms);
    librdf_free_uri(uri_cc);
    librdf_free_uri(uri_xhv);
    librdf_free_uri(uri_og);

    return (char *) result;
}

const char*
recontext_get_main_subject (recontext *rc)
{
    return rc->main_subject;
}

void recontext_destroy(recontext *rc)
{
    librdf_free_world(rc->world);
}