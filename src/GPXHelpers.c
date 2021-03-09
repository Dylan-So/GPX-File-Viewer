/*-----------------------------------
 *| Assignment 2                    |
 *|                                 |
 *| AUTHOR: Dylan So                |
 *| Student ID: 1091854             |
 *| School: University of Guelph    |
 *| Course: CIS2750                 |
 *-----------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/xmlschemastypes.h>
#include "GPXHelpers.h"


// FUNCTIONS getWaypointData, getRouteData, getTrackData CONTAINS SAMPLE CODE FROM PROVIDED the libXmlExample.c FILE
// TO PARSE AND RETRIEVE DATA FROM THE XML FILE

// A1
GPXdoc *populateGPX(GPXdoc *doc, xmlNode *root) {
    xmlNode *curNode = NULL;

    for (curNode = root; curNode != NULL; curNode = curNode->next) {
        char *name = (char *)curNode->name;

        // Gets the data required depending on specific nodes
        if (strcmp(name, "wpt") == 0) {
            Waypoint *newWaypoint = getWaypointData(curNode);
            if (newWaypoint == NULL) {
                return NULL;
            }
            insertBack(doc->waypoints, (void *)newWaypoint);
        } else if (strcmp(name, "rte") == 0) {
            Route *newRoute = getRouteData(curNode);
            if (newRoute == NULL) {
                return NULL;
            }
            insertBack(doc->routes, (void *)newRoute);
        } else if (strcmp(name, "trk") == 0) {
            Track *newTrack = getTrackData(curNode);
            if (newTrack == NULL) {
                return NULL;
            }
            insertBack(doc->tracks, (void *)newTrack);
        }
    }

    return doc;
}

Waypoint *getWaypointData(xmlNode *curNode) {
    Waypoint *newWaypoint = (Waypoint *)malloc(sizeof(Waypoint));

    // Initialize all Waypoint variables
    newWaypoint->name=(char *)malloc(sizeof(char));
    newWaypoint->name[0] = '\0';
    newWaypoint->longitude = 0.0;
    newWaypoint->latitude = 0.0;
    newWaypoint->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);


    // Gets the longitude and latitude of the waypoint
    xmlAttr *attr;
    for (attr = curNode->properties; attr != NULL; attr = attr->next) {
        char *attrName = (char *)attr->name;
        char *cont = (char *)(attr->children->content);
        if (strcmp(attrName, "lat") == 0) {
            newWaypoint->latitude = atof(cont);
        } else if (strcmp(attrName, "lon") == 0) {
            newWaypoint->longitude = atof(cont);
        }
    }

    xmlNode *copy = NULL;
    for (copy = curNode->children; copy != NULL; copy = copy->next) {
        char *nodeName = (char *)copy->name;

        // If the node is a name it will copy the data into the appropiate places
        if (strcmp(nodeName, "name") == 0) {
            if (copy->children->content != NULL) {
                newWaypoint->name = (char *)realloc(newWaypoint->name, strlen((char *)copy->children->content) + 1);
                strcpy(newWaypoint->name, (char *)copy->children->content);
            }
        } else {
            if (copy->children != NULL) {
                GPXData *newData = (GPXData *)malloc(sizeof(GPXData) + strlen((char *)copy->children->content) + 1);
                if (copy->children->content[0] == '\0') {
                    deleteGpxData(newData);
                    return NULL;
                }
                strcpy(newData->name, nodeName);
                strcpy(newData->value, (char *)copy->children->content);
                insertBack(newWaypoint->otherData, (void *)newData);
            }
        }
    }

    return newWaypoint;
}

Route *getRouteData(xmlNode *curNode) {
    Route *newRoute = (Route *)malloc(sizeof(Route));

    // Initialize all variables in a route
    newRoute->name = (char *)malloc(sizeof(char));
    newRoute->name[0] = '\0';
    newRoute->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    newRoute->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

    xmlNode *copy = NULL;

    for (copy = curNode->children; copy != NULL; copy = copy->next) {
        char *nodeName = (char *)copy->name;

        /// If the node is a name or rtept, then it will copy the data into the appropiate places
        if (strcmp(nodeName, "name") == 0) {
            if (copy->children->content != NULL) {
                newRoute->name = (char *)realloc(newRoute->name, strlen((char *)copy->children->content) + 1);
                strcpy(newRoute->name, (char *)copy->children->content);
            } else {
                newRoute->name[0] = '\0';
            }
        } else if (strcmp(nodeName, "rtept") == 0) {
            Waypoint *newWaypoint = getWaypointData(copy);
            insertBack(newRoute->waypoints, (void *)newWaypoint);
        } else {
            if (copy->children != NULL) {
                GPXData *newData = (GPXData *)malloc(sizeof(GPXData) + strlen((char *)copy->children->content) + 1);
                strcpy(newData->name, nodeName);
                strcpy(newData->value, (char *)copy->children->content);
                insertBack(newRoute->otherData, (void *)newData);
            }
        }
    }

    return newRoute;
}

Track *getTrackData(xmlNode *curNode) {
    Track *newTrack = (Track *)malloc(sizeof(Track));

    // Initialize all variables in a Track
    newTrack->name = (char *)malloc(sizeof(char));
    newTrack->name[0] = '\0';
    newTrack->segments = initializeList(&trackSegmentToString, &deleteTrackSegment, &compareTrackSegments);
    newTrack->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

    xmlNode *copy = NULL;

    for (copy = curNode->children; copy != NULL; copy = copy->next) {
        char *nodeName = (char *)copy->name;

        // If the node is a name or rtept, then it will copy the data into the appropiate places
        if (strcmp(nodeName, "name") == 0) {
            if (copy->children->content != NULL) {
                newTrack->name = (char *)realloc(newTrack->name, strlen((char *)copy->children->content) + 1);
                strcpy(newTrack->name, (char *)copy->children->content);
            } else {
                newTrack->name[0] = '\0';
            }
        } else if (strcmp(nodeName, "trkseg") == 0) {
            TrackSegment *newSegment = getSegmentData(copy);
            insertBack(newTrack->segments, (void *)newSegment);
        } else {
            if (copy->children != NULL) {
                GPXData *newData = (GPXData *)malloc(sizeof(GPXData) + strlen((char *)copy->children->content) + 1);
                if (copy->children->content[0] == '\0') {
                    deleteGpxData(newData);
                    return NULL;
                }
                strcpy(newData->name, nodeName);
                strcpy(newData->value, (char *)copy->children->content);
                insertBack(newTrack->otherData, (void *)newData);
            }
        }
    }

    return newTrack;
}

TrackSegment *getSegmentData(xmlNode *curNode) {
    TrackSegment *newSegment = (TrackSegment *)malloc(sizeof(TrackSegment));

    newSegment->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);

    xmlNode *copy = NULL;

    for (copy = curNode->children; copy != NULL; copy = copy->next) {
        if (strcmp((char *)copy->name, "text")) {
            Waypoint *newWaypoint = getWaypointData(copy);
            if (newWaypoint == NULL) {
                freeList(newSegment->waypoints);
                free(newSegment);
                return NULL;
            }
            insertBack(newSegment->waypoints, (void *)newWaypoint);
        }
    }

    return newSegment;
}

int getNumElements(List *list) {
    if (list != NULL) {
        int count = 0;
        ListIterator listIter = createIterator(list);
        void *data = nextElement(&listIter);
        while (data != NULL) {
            count++;
            data = nextElement(&listIter);
        }
        return count;
    } else {
        return -1;
    }
}

int getNumWptList(List *list) {
    if (list != NULL) {
        int count = 0;
        ListIterator listIter = createIterator(list);
        Waypoint *wpt = (Waypoint *)nextElement(&listIter);
        while (wpt != NULL) {
            // Get the number of 'otherData' in a Waypoint
            if (strcmp(wpt->name, "") != 0) {
                count++;
            }
            int wptTemp = getNumElements(wpt->otherData);
            if (wptTemp != -1) {
                count += wptTemp;
            }
            wpt = (Waypoint *)nextElement(&listIter);
        }
        return count;
    } else {
        return -1;
    }
}

int getNumRteList(List *list) {
    if (list != NULL) {
        int count = 0;
        ListIterator listIter = createIterator(list);
        Route *rte = (Route *)nextElement(&listIter);
        while (rte != NULL) {
            // Get number of 'otherData' in a Waypoint
            if (strcmp(rte->name, "") != 0) {
                count++;
            }
            int rteTemp = getNumElements(rte->otherData);
            if (rteTemp != -1) {
                count += rteTemp;
            }

            // Get the number of 'otherData' in a Route's waypoints
            int rteptTemp = getNumWptList(rte->waypoints);
            if (rteptTemp != -1) {
                count += rteptTemp;
            }
            rte = (Route *)nextElement(&listIter);
        }
        return count;
    } else {
        return -1;
    }
}

int getNumTrkList(List *list) {
    if (list != NULL) {
        int count = 0;
        ListIterator listIter = createIterator(list);
        Track *trk = (Track *)nextElement(&listIter);
        while (trk != NULL) {
            // Get number of 'otherData' in a Track
            if (strcmp(trk->name, "") != 0) {
                count++;
            }
            int trkTemp = getNumElements(trk->otherData);
            if (trkTemp != -1) {
                count += trkTemp;
            }

            // Get number of 'otherData' in a TrackSegment
            ListIterator trksegIter = createIterator(trk->segments);
            TrackSegment *trkSeg = (TrackSegment *)nextElement(&trksegIter);
            while (trkSeg != NULL) {
                int segTemp = getNumWptList(trkSeg->waypoints);
                if (segTemp != -1) {
                    count += segTemp;
                }
                trkSeg = (TrackSegment *)nextElement(&trksegIter);
            }
            trk = (Track *)nextElement(&listIter);
        }
        return count;
    } else {
        return -1;
    }
}

int validateXmlTree(xmlDoc* doc, char* gpxSchemaFile) {
    int invalid = 1;
    xmlSchema *schema = NULL;
    xmlSchemaParserCtxt* schemaParser = NULL;
    xmlSchemaValidCtxt* validCtxt = NULL;

    LIBXML_TEST_VERSION

    schemaParser = xmlSchemaNewParserCtxt(gpxSchemaFile);
    xmlSchemaSetParserErrors(schemaParser, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    schema = xmlSchemaParse(schemaParser);
    xmlSchemaFreeParserCtxt(schemaParser);

    if (doc == NULL) {
        fprintf(stderr, "validateXmlTree: Provided xmlDoc is not valid\n");
        if(schema != NULL) {
            xmlSchemaFree(schema);
        }
        xmlSchemaCleanupTypes();
        xmlCleanupParser();
        return -1;
    } else {
        validCtxt = xmlSchemaNewValidCtxt(schema);
        xmlSchemaSetValidErrors(validCtxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
        invalid = xmlSchemaValidateDoc(validCtxt, doc);

        if (invalid > 0) {
            xmlFreeDoc(doc);
            fprintf(stderr, "validateXmlTree: Provided xmlDoc is not valid\n");
            return -1;
        } else if (invalid != 0) {
            xmlFreeDoc(doc);
            printf("Test\n");
            fprintf(stderr, "validateXmlTree: Validation process failed\n");
            return -1;
        }
        xmlSchemaFreeValidCtxt(validCtxt);
    }
    if(schema != NULL) {
        xmlSchemaFree(schema);
    }

    xmlSchemaCleanupTypes();
    xmlCleanupParser();

    return 1;
}

xmlDoc* GPXdocToTree(GPXdoc* gpxDoc) {
    xmlDoc* doc = NULL;
    xmlNode* root = NULL;

    LIBXML_TEST_VERSION;

    // Create the xmlDoc with version specified in GPXdoc
    char *version = malloc(10 * sizeof(char));
    sprintf(version, "%0.1f", gpxDoc->version);
    doc = xmlNewDoc(BAD_CAST "1.0");

    // Create and set the namespace for root node
    root = xmlNewNode(NULL, BAD_CAST "gpx");
    xmlNs* namespace = xmlNewNs(root, BAD_CAST gpxDoc->namespace, NULL);
    xmlSetNs(root, namespace);
    xmlNewProp(root, BAD_CAST "version", BAD_CAST version);
    xmlNewProp(root, BAD_CAST "creator", BAD_CAST gpxDoc->creator);

    addWptNodes(gpxDoc->waypoints, root, "wpt", namespace);
    addRteNodes(gpxDoc->routes, root, namespace);
    addTrkNodes(gpxDoc->tracks, root, namespace);

    xmlDocSetRootElement(doc, root);

    return doc;
}

void addWptNodes(List* waypoints, xmlNode* root, char *name, xmlNs* namespace) {
    ListIterator wptIter = createIterator(waypoints);
    Waypoint *wpt = (Waypoint *)nextElement(&wptIter);
    while (wpt != NULL) {
        // Adds the wpt node to the root node, along with its lat and lon
        xmlNode* root_child = xmlNewChild(root, namespace, BAD_CAST name, NULL);
        char *lat = malloc(15 * sizeof(char));
        char *lon = malloc(15 * sizeof(char));

        sprintf(lat, "%0.6f", wpt->latitude);
        sprintf(lon, "%0.6f", wpt->longitude);

        xmlNewProp(root_child, BAD_CAST "lat", BAD_CAST lat);
        xmlNewProp(root_child, BAD_CAST "lon", BAD_CAST lon);

        // Adds the name of a waypoint
        if (wpt->name[0] != '\0') {
            xmlNode* wptName = xmlNewChild(root_child, namespace, BAD_CAST "name", BAD_CAST wpt->name);
            xmlAddChild(root_child, wptName);
        }

        // Adds any other data included in a waypoint
        ListIterator othIter = createIterator(wpt->otherData);
        GPXData* oth = (GPXData *)nextElement(&othIter);
        while (oth != NULL) {
            xmlNode* otherData = xmlNewChild(root_child, namespace, BAD_CAST oth->name, BAD_CAST oth->value);
            xmlAddChild(root_child, otherData);
            oth = (GPXData *)nextElement(&othIter);
        }

        xmlAddChild(root, root_child);

        wpt = (Waypoint *)nextElement(&wptIter);
    }
}

void addRteNodes(List* routes, xmlNode* root, xmlNs* namespace) {
    ListIterator rteIter = createIterator(routes);
    Route *rte = (Route *)nextElement(&rteIter);
    while (rte != NULL) {
        // Adds the rte node to the root
        xmlNode* root_child = xmlNewChild(root, namespace, BAD_CAST "rte", NULL);

        // Adds the name of a route
        xmlNode* rteName = xmlNewChild(root_child, namespace, BAD_CAST "name", BAD_CAST rte->name);
        xmlAddChild(root_child, rteName);

        // Adds any other data included in a waypoint
        ListIterator othIter = createIterator(rte->otherData);
        GPXData* oth = (GPXData *)nextElement(&othIter);
        while (oth != NULL) {
            xmlNode* otherData = xmlNewChild(root_child, namespace, BAD_CAST oth->name, BAD_CAST oth->value);
            xmlAddChild(root_child, otherData);
            oth = (GPXData *)nextElement(&othIter);
        }

        addWptNodes(rte->waypoints, root_child, "rtept", namespace);

        xmlAddChild(root, root_child);

        rte = (Route *)nextElement(&rteIter);
    }
}

void addTrkNodes(List* tracks, xmlNode* root, xmlNs* namespace) {
    ListIterator trkIter = createIterator(tracks);
    Track *trk = (Track *)nextElement(&trkIter);
    while (trk != NULL) {
        // Adds the trk node to the root
        xmlNode* root_child = xmlNewChild(root, namespace, BAD_CAST "trk", NULL);

        // Adds the name of a track
        xmlNode* trkName = xmlNewChild(root_child, namespace, BAD_CAST "name", BAD_CAST trk->name);
        xmlAddChild(root_child, trkName);

        // Adds any other data included in a track
        ListIterator othIter = createIterator(trk->otherData);
        GPXData* oth = (GPXData *)nextElement(&othIter);
        while (oth != NULL) {
            xmlNode* otherData = xmlNewChild(root_child, namespace, BAD_CAST oth->name, BAD_CAST oth->value);
            xmlAddChild(root_child, otherData);
            oth = (GPXData *)nextElement(&othIter);
        }

        // Adds the track segments to the trk node
        xmlNode* trkseg_child = xmlNewChild(root_child, namespace, BAD_CAST "trkseg", NULL);
        ListIterator trksegIter = createIterator(trk->segments);
        TrackSegment* trkseg = (TrackSegment *)nextElement(&trksegIter);
        while (trkseg != NULL) {
            addWptNodes(trkseg->waypoints, trkseg_child, "trkpt", namespace);
            trkseg = (TrackSegment *)nextElement(&trksegIter);
        }

        xmlAddChild(root, root_child);

        trk = (Track *)nextElement(&trkIter);
    }
}