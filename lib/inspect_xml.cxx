/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <rpm/header.h>
#include <rpm/rpmtag.h>

#include "inspect.h"
#include "rpminspect.h"

#ifndef _HAVE_XMLSETGENERICERRORFUNC
/*
 * By default, libxml will send error messages to stderr.  Turn that off for
 * our purposes.
 */
static void xml_silence_errors(void *ctx __attribute__((unused)), const char *msg __attribute__((unused)), ...)
{
    return;
}
#endif

/*
 * Return true if the given file is a well-formed XML document, false otherwise.
 * This function first tries with DTD validation.  Failing that it tries to just
 * check the XML.  The tests get less and less strict.
 */
static bool is_xml_well_formed(const char *path, size_t prefixlen, char **errors)
{
    static bool initialized = false;
#ifndef _HAVE_XMLSETGENERICERRORFUNC
    static xmlGenericErrorFunc silence = xml_silence_errors;
#endif
    xmlParserCtxtPtr ctxt;
    xmlDocPtr doc;
    int opts = XML_PARSE_NOERROR | XML_PARSE_NOWARNING | XML_PARSE_RECOVER | XML_PARSE_NONET;
    char *line = NULL;
    bool result = true;

    if (!initialized) {
#ifdef _HAVE_XMLSETGENERICERRORFUNC
        xmlSetGenericErrorFunc(NULL, NULL);
#else
        initGenericErrorDefaultFunc(&silence);
#endif
        LIBXML_TEST_VERSION
        initialized = true;
    }

    ctxt = xmlNewParserCtxt();
    assert(ctxt != NULL);
    doc = xmlCtxtReadFile(ctxt, path, NULL, opts | XML_PARSE_DTDVALID);

    /* try again if no DTD specified */
    if (!ctxt->valid && ctxt->errNo == XML_DTD_NO_DTD) {
        xmlFreeDoc(doc);
        doc = xmlCtxtReadFile(ctxt, path, NULL, opts);
    }

    if (ctxt->wellFormed && ctxt->errNo == XML_ERR_NONE) {
        /* well-formed documents */
        result = true;
    } else if (ctxt->errNo == XML_ERR_UNDECLARED_ENTITY || ctxt->errNo == XML_WAR_UNDECLARED_ENTITY) {
        /* an unparsed entity is ok in this check */
        result = true;
    } else {
        /* any other non-zero error code */
        result = false;
    }

    /* capture validity output */
    if (!ctxt->valid && errors != NULL && ctxt->lastError.message) {
        ctxt->lastError.message[strcspn(ctxt->lastError.message, "\r\n")] = '\0';
        *errors = strdup(ctxt->lastError.message);

        if (ctxt->lastError.file) {
            xasprintf(&line, "%d", ctxt->lastError.line);
            assert(line != NULL);
            *errors = strappend(*errors, ctxt->lastError.file + prefixlen, " on line ", line, NULL);
            free(line);
        }

        if (ctxt->lastError.str1) {
            *errors = strappend(*errors, "\n%s", ctxt->lastError.str1, NULL);
        }

        if (ctxt->lastError.str2) {
            *errors = strappend(*errors, "\n%s", ctxt->lastError.str2, NULL);
        }

        if (ctxt->lastError.str3) {
            *errors = strappend(*errors, "\n%s", ctxt->lastError.str3, NULL);
        }
    }

    if (doc != NULL) {
        xmlFreeDoc(doc);
    }

    xmlFreeParserCtxt(ctxt);
    return result;
}

static bool is_xml(const char *path)
{
    FILE *input;
    unsigned char buffer[32];
    unsigned char *xml_data;
    size_t bytes_read;

    const char xml_ascii_prelude[] = "<?xml version=";
    const char xml_utf16_le_prelude[] = "<\0?\0x\0m\0l\0 \0v\0e\0r\0s\0i\0o\0n\0=\0";
    const char xml_utf16_be_prelude[] = "\0<\0?\0x\0m\0l\0 \0v\0e\0r\0s\0i\0o\0n\0=";
    const char *xml_prelude;
    size_t min_size;

    input = fopen(path, "r");

    if (input == NULL) {
        return false;
    }

    /* Look for an optional byte-order marker, followed by "<?xml version=" */
    bytes_read = fread(buffer, 1, sizeof(buffer), input);

    if (ferror(input)) {
        fclose(input);
        return false;
    }

    fclose(input);
    xml_data = buffer;

    /* Look for a byte-order marker */
    /* The XML spec says everyone has to deal with at least utf-8 and utf-16, so handle those */
    if ((bytes_read >= 3) && (buffer[0] == 0xEF) && (buffer[1] == 0xBB) && (buffer[2] == 0xBF)) {
        /* utf-8? */
        xml_data += 3;
        bytes_read -= 3;
        xml_prelude = xml_ascii_prelude;
        min_size = sizeof(xml_ascii_prelude) - 1;
    } else if ((bytes_read >= 2) && (buffer[0] == 0xFE) && (buffer[1] == 0xFF)) {
        /* utf-16 LE? */
        xml_data += 2;
        bytes_read -= 2;
        xml_prelude = xml_utf16_le_prelude;
        min_size = sizeof(xml_utf16_le_prelude) - 1;
    } else if ((bytes_read >= 2) && (buffer[0] == 0xFF) && (buffer[1] == 0xFE)) {
        /* utf-16 BE? */
        xml_data += 2;
        bytes_read -= 2;
        xml_prelude = xml_utf16_be_prelude;
        min_size = sizeof(xml_utf16_be_prelude) - 1;
    } else {
        /* otherwise just assume something close enough to ascii */
        xml_prelude = xml_ascii_prelude;
        min_size = sizeof(xml_ascii_prelude) - 1;
    }

    return (bytes_read >= min_size) && (memcmp(xml_data, xml_prelude, min_size) == 0);
}

static bool xml_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    const char *pkg = NULL;
    struct result_params params;

    /* Skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Is this an XML file? */
    if (!file->fullpath || !S_ISREG(file->st.st_mode)) {
        return true;
    }

    if (!process_file_path(file, ri->xml_path_include, ri->xml_path_exclude)) {
        return true;
    }

    if (!is_xml(file->fullpath)) {
        return true;
    }

    if (strsuffix(file->fullpath, SVG_FILENAME_EXTENSION)) {
        /*
         * Skip SVG files which are XML, but don't specify a DTD
         * We don't validate other image files so we can probably do the
         * same for SVG.
         */
        return true;
    }

    /* package name is used for reporting */
    pkg = headerGetString(file->rpm_header, RPMTAG_NAME);

    /* Set up result parameters */
    init_result_params(&params);
    params.header = NAME_XML;
    params.remedy = get_remedy(REMEDY_XML);
    params.arch = get_rpm_header_arch(file->rpm_header);
    params.file = file->localpath;

    result = is_xml_well_formed(file->fullpath, strlen(file->fullpath) - strlen(file->localpath), &params.details);

    if (result && params.details) {
        xasprintf(&params.msg, _("%s is a well-formed XML file in %s on %s, but is not a valid XML file"), file->localpath, pkg, params.arch);
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
        params.verb = VERB_OK;
        add_result(ri, &params);
        free(params.msg);
        free(params.details);
    } else if (!result) {
        xasprintf(&params.msg, _("%s is not a well-formed XML file in %s on %s"), file->localpath, pkg, params.arch);
        params.severity = RESULT_VERIFY;
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.verb = VERB_FAILED;
        params.noun = _("${FILE} is not well-formed XML on ${ARCH}");
        add_result(ri, &params);
        free(params.msg);
        free(params.details);
    }

    return result;
}

bool inspect_xml(struct rpminspect *ri)
{
    bool result;
    struct result_params params;

    assert(ri != NULL);
    result = foreach_peer_file(ri, NAME_XML, xml_driver);

    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_XML;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
