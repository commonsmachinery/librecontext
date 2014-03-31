/*
 * librecontext - a lightweight metadata handling library
 *
 * Copyright 2014 Commons Machinery http://commonsmachinery.se/
 * Authors: Artem Popov <artfwo@commonsmachinery.se>
 *
 * Distributed under the MIT license, please see LICENSE in the top dir.
 */

#ifndef __RECONTEXT_GEXIV2_H__
#define __RECONTEXT_GEXIV2_H__

#include "recontext.h"
#include <gexiv2.h>

void recontext_write_exiv2(recontext *rc, GExiv2Metadata *metadata);

#endif /* __RECONTEXT_GEXIV2_H__ */
