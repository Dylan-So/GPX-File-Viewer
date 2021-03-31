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
            if (copy->children != NULL) {
                xmlChar* name = xmlNodeGetContent(copy->children);
                if (name != NULL) {
                    newWaypoint->name = (char *)realloc(newWaypoint->name, strlen((char *)name) + 1);
                    strcpy(newWaypoint->name, (char *)name);
                }
                xmlFree(name);
            }
        } else {
            if (copy->children != NULL) {
                xmlChar* value = xmlNodeGetContent(copy->children);
                if (value != NULL) {
                    GPXData *newData = (GPXData *)malloc(sizeof(GPXData) + strlen((char *)copy->children->content) + 1);
                    strcpy(newData->name, nodeName);
                    strcpy(newData->value, (char *)value);
                    insertBack(newWaypoint->otherData, (void *)newData);
                }
                xmlFree(value);
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
            if (copy->children != NULL) {
                xmlChar *name = xmlNodeGetContent(copy->children);
                if (name != NULL) {
                    newRoute->name = (char *)realloc(newRoute->name, strlen((char *)name) + 1);
                    strcpy(newRoute->name, (char *)name);
                }
                xmlFree(name);
            }
        } else if (strcmp(nodeName, "rtept") == 0) {
            Waypoint *newWaypoint = getWaypointData(copy);
            insertBack(newRoute->waypoints, (void *)newWaypoint);
        } else {
            if (copy->children != NULL) {
                GPXData *newData = (GPXData *)malloc(sizeof(GPXData) + strlen((char *)copy->children->content) + 1);
                xmlChar* value = xmlNodeGetContent(copy->children);
                if (value != NULL) {
                    strcpy(newData->name, nodeName);
                    strcpy(newData->value, (char *)value);
                    insertBack(newRoute->otherData, (void *)newData);
                }
                xmlFree(value);
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
            if (copy->children != NULL) {
                xmlChar* name = xmlNodeGetContent(copy->children);
                if (name != NULL) {
                    newTrack->name = (char *)realloc(newTrack->name, strlen((char *)name) + 1);
                    strcpy(newTrack->name, (char *)name);
                }
                xmlFree(name);
            }
        } else if (strcmp(nodeName, "trkseg") == 0) {
            TrackSegment *newSegment = getSegmentData(copy);
            insertBack(newTrack->segments, (void *)newSegment);
        } else {
            if (copy->children != NULL) {
                xmlChar* value = xmlNodeGetContent(copy->children);
                if (value != NULL) {
                    GPXData *newData = (GPXData *)malloc(sizeof(GPXData) + strlen((char *)copy->children->content) + 1);
                    strcpy(newData->name, nodeName);
                    strcpy(newData->value, (char *)value);
                    insertBack(newTrack->otherData, (void *)newData);
                }
                xmlFree(value);
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
List* getWaypointList(GPXdoc* doc) {
    if (doc == NULL) {
        return NULL;
    } else {
        return doc->waypoints;
    }
}

List* getRouteList(GPXdoc* doc) {
    if (doc == NULL) {
        return NULL;
    } else {
        return doc->routes;
    }
}

List* getTrackList(GPXdoc* doc) {
    if (doc == NULL) {
        return NULL;
    } else {
        return doc->tracks;
    }
}

int getTrkPts(const Track* trk) {
    if (trk != NULL) {
        int count = 0;
        ListIterator trksegIter = createIterator(trk->segments);
        TrackSegment *trkseg = (TrackSegment *)nextElement(&trksegIter);
        while (trkseg != NULL) {
            count += getNumElements(trkseg->waypoints);
            trkseg = (TrackSegment *)nextElement(&trksegIter);
        }
        return count;
    } else {
        return 0;
    }
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
        return 0;
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

// This function references code from: https://knol2share.blogspot.com/2009/05/validate-xml-against-xsd-in-c.html
// *Example given in the assignment description*
int validateXmlTree(xmlDoc* doc, char* gpxSchemaFile) {
    int invalid = 1;
    xmlSchema *schema = NULL;
    xmlSchemaParserCtxt* schemaParser = NULL;
    xmlSchemaValidCtxt* validCtxt = NULL;

    schemaParser = xmlSchemaNewParserCtxt(gpxSchemaFile);
    xmlSchemaSetParserErrors(schemaParser, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    schema = xmlSchemaParse(schemaParser);
    xmlSchemaFreeParserCtxt(schemaParser);
    
    LIBXML_TEST_VERSION

    if (doc == NULL) {
        fprintf(stderr, "validateXmlTree: Provided xmlDoc is not valid\n");
        xmlSchemaFreeValidCtxt(validCtxt);
        if(schema != NULL) {
            xmlSchemaFree(schema);
        }
        xmlSchemaCleanupTypes();
        return -1;
    } else {
        validCtxt = xmlSchemaNewValidCtxt(schema);
        xmlSchemaSetValidErrors(validCtxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
        invalid = xmlSchemaValidateDoc(validCtxt, doc);

        if (invalid > 0) {
            fprintf(stderr, "validateXmlTree: Provided xmlDoc is not valid\n");
            xmlSchemaFreeValidCtxt(validCtxt);
            if(schema != NULL) {
                xmlSchemaFree(schema);
            }
            xmlSchemaCleanupTypes();
            return -1;
        } else if (invalid != 0) {
            fprintf(stderr, "validateXmlTree: Validation process failed\n");
            xmlSchemaFreeValidCtxt(validCtxt);
            if(schema != NULL) {
                xmlSchemaFree(schema);
            }
            xmlSchemaCleanupTypes();
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
    char version[10];
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
    xmlCleanupParser();
    return doc;
}

void addWptNodes(List* waypoints, xmlNode* root, char *name, xmlNs* namespace) {
    ListIterator wptIter = createIterator(waypoints);
    Waypoint *wpt = (Waypoint *)nextElement(&wptIter);
    while (wpt != NULL) {
        // Adds the wpt node to the root node, along with its lat and lon
        xmlNode* root_child = xmlNewChild(root, namespace, BAD_CAST name, NULL);
        char lat[15];
        char lon[15];

        sprintf(lat, "%0.6f", wpt->latitude);
        sprintf(lon, "%0.6f", wpt->longitude);

        xmlNewProp(root_child, BAD_CAST "lat", BAD_CAST lat);
        xmlNewProp(root_child, BAD_CAST "lon", BAD_CAST lon);

        // Adds the elements in proper order to ensure the xml file is valid
        ListIterator othIter = createIterator(wpt->otherData);
        GPXData* oth = (GPXData *)nextElement(&othIter);
        while (oth != NULL) {
            if (strcmp(oth->name, "ele") == 0 || strcmp(oth->name, "time") == 0 || strcmp(oth->name, "magvar") == 0 || strcmp(oth->name, "geoidheight") == 0) {
                xmlNewChild(root_child, namespace, BAD_CAST oth->name, BAD_CAST oth->value);
            }
            oth = (GPXData *)nextElement(&othIter);
        }

        // Adds the name of a waypoint
        if (wpt->name[0] != '\0') {
            xmlNewChild(root_child, namespace, BAD_CAST "name", BAD_CAST wpt->name);
        }

        // Resets iterator and adds any other data included in a waypoint
        othIter = createIterator(wpt->otherData);
        oth = (GPXData *)nextElement(&othIter);
        while (oth != NULL) {
            if (strcmp(oth->name, "ele") != 0 && strcmp(oth->name, "time") != 0 && strcmp(oth->name, "magvar") != 0 && strcmp(oth->name, "geoidheight") != 0) {
                xmlNewChild(root_child, namespace, BAD_CAST oth->name, BAD_CAST oth->value);
            }
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
            xmlNewChild(root_child, namespace, BAD_CAST oth->name, BAD_CAST oth->value);
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
            xmlNewChild(root_child, namespace, BAD_CAST oth->name, BAD_CAST oth->value);
            oth = (GPXData *)nextElement(&othIter);
        }

        // Adds the track segments to the trk node
        ListIterator trksegIter = createIterator(trk->segments);
        TrackSegment* trkseg = (TrackSegment *)nextElement(&trksegIter);
        while (trkseg != NULL) {
            xmlNode* trkseg_child = xmlNewChild(root_child, namespace, BAD_CAST "trkseg", NULL);
            addWptNodes(trkseg->waypoints, trkseg_child, "trkpt", namespace);
            trkseg = (TrackSegment *)nextElement(&trksegIter);
        }
        xmlAddChild(root, root_child);

        trk = (Track *)nextElement(&trkIter);
    }
}

// This function uses the Haversine formula
// Reference: https://www.movable-type.co.uk/scripts/latlong.html
float getPointDistance(Waypoint *wp1, Waypoint *wp2) {
    if (wp1 == NULL || wp2 == NULL) {
        return 0.0;
    }

    float earthRadius = 6371000;
    float wp1LatRad = wp1->latitude * (M_PI / 180.0);
    float wp2LatRad = wp2->latitude * (M_PI / 180.0);
    float latDiff = (wp2->latitude - wp1->latitude) * (M_PI / 180.0);
    float lonDiff = (wp2->longitude - wp1->longitude) * (M_PI / 180.0);
    float a = (sin(latDiff / 2) * sin(latDiff / 2)) + (cos(wp1LatRad) * cos(wp2LatRad)) * (sin(lonDiff / 2) * sin(lonDiff / 2));
    float c = 2 * atan2(sqrt(a), sqrt(1-a));
    float distance = earthRadius * c;
    return distance;
}

float getDistanceDiff(float routeLen, float len) {
    if ((len - routeLen) < 0) {
        return ((len - routeLen) * -1);
    } else {
        return (len - routeLen);
    }
}

void deleteNothing(void* toDelete) {

}

char* trkToJSON(const Track *tr) {
	if (tr == NULL) {
		return "{}";
	}

	char* name = tr->name;
	if (name[0] == '\0') {
		name = "None";
	}
	float trackLen = round10(getTrackLen(tr));
    int numPoints = getTrkPts(tr);
	size_t stringLen = 0;
	char loopBool[6];
	// Calculate length of string according to whether the track is a loop or not
	if (isLoopTrack(tr, 10)) {
		strcpy(loopBool, "true");
		stringLen = snprintf(NULL, 0, "{\"name\":\"%s\",\"len\":%0.1f,\"numPoints\":%d,\"loop\":%s}", name, trackLen, numPoints, loopBool);
	} else {
		strcpy(loopBool, "false");
		stringLen = snprintf(NULL, 0, "{\"name\":\"%s\",\"len\":%0.1f,\"numPoints\":%d,\"loop\":%s}", name, trackLen, numPoints, loopBool);
	}
	// Allocate enough memory and construct the string
	char* trkJSON = malloc((stringLen + 1) * sizeof(char));
	sprintf(trkJSON, "{\"name\":\"%s\",\"len\":%0.1f,\"numPoints\":%d,\"loop\":%s}", name, trackLen, numPoints, loopBool);
	return trkJSON;
}

char* trkListToJSON(const List *list) {
	if (list == NULL) {
		return "[]";
	}

	ListIterator trkIter = createIterator((List *)list);
	Track* trk = (Track *)nextElement(&trkIter);
	size_t stringLen = snprintf(NULL, 0, "[");
	// Adds 1 more character for the closing bracket ']' if list is null 
	if (trk == NULL) {
		stringLen += 1;
	}
	// First loop is responsible for calculating the amount of memory to allocate
	while (trk != NULL) {
		char* trkJson = trkToJSON(trk);
		stringLen += snprintf(NULL, 0, "%s", trkJson);
		free(trkJson);
		trk = (Track *)nextElement(&trkIter);
		if (trk == NULL) {
			stringLen += snprintf(NULL, 0, "]");
		} else {
			stringLen += snprintf(NULL, 0, ",");
		}
	}
	// Second loop is responsible for constructing the string itself, using the trackToJSON() function
	char *trkListJSON = malloc((stringLen + 1) * sizeof(char));
	// Reset iterator and construct the string
	trkIter = createIterator((List *)list);
	trk = (Track *)nextElement(&trkIter);
	int length = 0;
	length += sprintf(trkListJSON + length, "[");
	while (trk != NULL) {
		char* trkJson = trkToJSON(trk);
		length += sprintf(trkListJSON + length, "%s", trkJson);
		free(trkJson);
		trk = (Track *)nextElement(&trkIter);
		if (trk != NULL) {
			length += sprintf(trkListJSON + length, ",");
		}
	}
	length += sprintf(trkListJSON + length, "]");
	return trkListJSON;
}

void setRouteName(Route* rte, char* name) {
    if (rte != NULL && name != NULL) {
        rte->name = realloc(rte->name, (strlen(name) + 1) * sizeof(char));
        strcpy(rte->name, name);
        rte->name[strlen(name)] = '\0';
    }
}

void setTrackName(Track* trk, char* name) {
    if (trk != NULL && name != NULL) {
        trk->name = realloc(trk->name, (strlen(name) + 1) * sizeof(char));
        strcpy(trk->name, name);
        trk->name[strlen(name)] = '\0';
    }
}

bool containsRoute(GPXdoc* doc, char* name) {
    Route* route = getRoute(doc, name);
    if (route != NULL) {
        return true;
    } else {
        return false;
    }
}