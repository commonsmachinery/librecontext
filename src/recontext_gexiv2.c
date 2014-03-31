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
#include "recontext_gexiv2.h"

static void
recontext_metadata_append_tag_value(GExiv2Metadata *metadata, const gchar *tagname, gchar *value)
{
    gchar** values;
    gchar** temp;
    guint length = 1;
    guint i;

    values = gexiv2_metadata_get_tag_multiple(metadata, tagname);
    while (values[length - 1] != NULL) length++;

    temp = g_new (gchar*, length + 1);

    for (i = 0; i < length + 1; i++) {
        temp[i] = values[i];
    }

    temp[length - 1] = g_strdup(value);
    temp[length] = NULL;

    gexiv2_metadata_set_tag_multiple(metadata, tagname, (const gchar **) temp);
}

void
recontext_write_exiv2(recontext *rc, GExiv2Metadata *metadata)
{
    gchar *source_query_string = \
        "PREFIX dc: <http://purl.org/dc/elements/1.1/>                  "
        "PREFIX dcterms: <http://purl.org/dc/terms/>                    "
        "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>      "
        "SELECT ?subject ?label WHERE {                                 "
        "    {                                                          "
        "        { ?subject dc:source ?label . }                        "
        "        UNION                                                  "
        "        { ?subject dcterms:source ?label . }                   "
        "        FILTER(isLiteral(?label) || isURI(?label))             "
        "    }                                                          "
        "    UNION                                                      "
        "    {                                                          "
        "        { ?subject dc:source ?node . }                         "
        "        UNION                                                  "
        "        { ?subject dcterms:source ?node . }                    "
        "        FILTER(isBlank(?node))                                 "
        "        ?node a rdf:Alt .                                      "
        "        {                                                      "
        "            SELECT ?label WHERE {                              "
        "                ?node rdf:_1 ?label .                          "
        "                FILTER(isLiteral(?label) || isURI(?label))     "
        "            }                                                  "
        "        }                                                      "
        "    }                                                          "
        "    UNION                                                      "
        "    {                                                          "
        "        { ?subject dc:source ?node . }                         "
        "        UNION                                                  "
        "        { ?subject dcterms:source ?node . }                    "
        "        FILTER(isBlank(?node))                                 "
        "        ?node a rdf:Seq .                                      "
        "        {                                                      "
        "            SELECT ?label WHERE {                              "
        "                ?node ?pred ?label                             "
        "                FILTER(isLiteral(?label) || isURI(?label))     "
        "                FILTER(?label != rdf:Seq)                      "
        "            }                                                  "
        "        }                                                      "
        "    }                                                          "
        "    UNION                                                      "
        "    {                                                          "
        "        { ?subject dc:source ?node . }                         "
        "        UNION                                                  "
        "        { ?subject dcterms:source ?node . }                    "
        "        FILTER(isBlank(?node))                                 "
        "        ?node a rdf:Bag .                                      "
        "        {                                                      "
        "            SELECT ?label WHERE {                              "
        "                ?node ?pred ?label                             "
        "                FILTER(isLiteral(?label) || isURI(?label))     "
        "                FILTER(?label != rdf:Bag)                      "
        "            }                                                  "
        "        }                                                      "
        "    }                                                          "
        "}                                                              ";

    gchar *creator_query_string = \
        "PREFIX dc: <http://purl.org/dc/elements/1.1/>                  "
        "PREFIX dcterms: <http://purl.org/dc/terms/>                    "
        "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>      "
        "SELECT ?subject ?label WHERE {                                 "
        "    {                                                          "
        "        { ?subject dc:creator ?label . }                       "
        "        UNION                                                  "
        "        { ?subject dcterms:creator ?label . }                  "
        "        FILTER(isLiteral(?label) || isURI(?label))             "
        "    }                                                          "
        "    UNION                                                      "
        "    {                                                          "
        "        { ?subject dc:creator ?node . }                        "
        "        UNION                                                  "
        "        { ?subject dcterms:creator ?node . }                   "
        "        FILTER(isBlank(?node))                                 "
        "        ?node a rdf:Alt .                                      "
        "        {                                                      "
        "            SELECT ?label WHERE {                              "
        "                ?node rdf:_1 ?label .                          "
        "                FILTER(isLiteral(?label) || isURI(?label))     "
        "            }                                                  "
        "        }                                                      "
        "    }                                                          "
        "    UNION                                                      "
        "    {                                                          "
        "        { ?subject dc:creator ?node . }                        "
        "        UNION                                                  "
        "        { ?subject dcterms:creator ?node . }                   "
        "        FILTER(isBlank(?node))                                 "
        "        ?node a rdf:Seq .                                      "
        "        {                                                      "
        "            SELECT ?label WHERE {                              "
        "                ?node ?pred ?label                             "
        "                FILTER(isLiteral(?label) || isURI(?label))     "
        "                FILTER(?label != rdf:Seq)                      "
        "            }                                                  "
        "        }                                                      "
        "    }                                                          "
        "    UNION                                                      "
        "    {                                                          "
        "        { ?subject dc:creator ?node . }                        "
        "        UNION                                                  "
        "        { ?subject dcterms:creator ?node . }                   "
        "        FILTER(isBlank(?node))                                 "
        "        ?node a rdf:Bag .                                      "
        "        {                                                      "
        "            SELECT ?label WHERE {                              "
        "                ?node ?pred ?label                             "
        "                FILTER(isLiteral(?label) || isURI(?label))     "
        "                FILTER(?label != rdf:Bag)                      "
        "            }                                                  "
        "        }                                                      "
        "    }                                                          "
        "}                                                              ";

    librdf_query *query;
    librdf_query_results *results;

    // iterate through sources
    gexiv2_metadata_clear_tag(metadata, "Xmp.dc.source");

    query = librdf_new_query(rc->world, "sparql", NULL,
                             (const unsigned char *) source_query_string, NULL);
    results = librdf_model_query_execute(rc->model, query);

    while (!librdf_query_results_finished(results))
    {
        librdf_node *node = librdf_query_results_get_binding_value_by_name(results, "label");
        gchar *value = NULL;

        if (librdf_node_is_literal(node))
            value = g_strdup((const gchar *) librdf_node_get_literal_value(node));
        else if (librdf_node_is_resource(node))
            value = g_strdup((const gchar *) librdf_uri_as_string(librdf_node_get_uri(node)));
        else if (librdf_node_is_blank(node))
            g_printerr("shouldn't happen. blank nodes are omitted in the query\n");

        if (value != NULL) {
            recontext_metadata_append_tag_value(metadata, "Xmp.dc.source", value);
            g_free(value);
        }

        librdf_query_results_next(results);
    }

    librdf_free_query_results(results);
    librdf_free_query(query);

    // iterate through creators
    gexiv2_metadata_clear_tag(metadata,  "Xmp.dc.creator");

    query = librdf_new_query(rc->world, "sparql", NULL,
                             (const unsigned char *) creator_query_string, NULL);
    results = librdf_model_query_execute(rc->model, query);

    while (!librdf_query_results_finished(results))
    {
        librdf_node *node = librdf_query_results_get_binding_value_by_name(results, "label");
        gchar *value = NULL;

        if (librdf_node_is_literal(node))
            value = g_strdup((const gchar *) librdf_node_get_literal_value(node));
        else if (librdf_node_is_resource(node))
            value = g_strdup((const gchar *) librdf_uri_as_string(librdf_node_get_uri(node)));
        else if (librdf_node_is_blank(node))
            g_printerr("shouldn't happen. blank nodes are omitted in the query\n");

        if (value != NULL) {
            recontext_metadata_append_tag_value(metadata, "Xmp.dc.creator", value);
            g_free(value);
        }

        librdf_query_results_next(results);
    }

    librdf_free_query_results(results);
    librdf_free_query(query);
}
