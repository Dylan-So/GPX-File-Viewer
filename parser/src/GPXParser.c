/*-----------------------------------
 *| Assignment 2					|
 *|									|
 *| AUTHOR: Dylan So 				|
 *| Student ID: 1091854				|
 *| School: University of Guelph	|
 *| Course: CIS2750					|
 *-----------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemastypes.h>
#include <math.h>
#include "GPXParser.h"
#include "GPXHelpers.h"
#include "LinkedListAPI.h"

// A1
GPXdoc *createGPXdoc(char *fileName) {
	GPXdoc *newDoc = (GPXdoc *)malloc(sizeof(GPXdoc));

	// Initialize all the variables in a GPXdoc
	strcpy(newDoc->namespace, "namespace");
	newDoc->version = 0.0;
	newDoc->creator = (char *)malloc(sizeof(char));
	newDoc->creator[0] = '\0';
	newDoc->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
	newDoc->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
	newDoc->tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);

	// The code below contains sample code from the provided libXmlExample.c file
	LIBXML_TEST_VERSION

	xmlDoc *doc = NULL;
	xmlNode *root = NULL;

	doc = xmlReadFile(fileName, NULL, 0);

	if (doc == NULL) {
		deleteGPXdoc(newDoc);
		xmlFreeDoc(doc);
		return NULL;
	}

	root = xmlDocGetRootElement(doc);

	// Gets the namespace of the xml file
	if (strcmp((char *)root->name, "gpx") == 0) {
		if (strcmp((char *)root->ns->href, "") != 0) {
        	strcpy(newDoc->namespace, (char *)root->ns->href);
    	} else {
    		deleteGPXdoc(newDoc);
			xmlFreeDoc(doc);
			return NULL;
    	}
    }

    // Gets the version and creator of the xml file
    xmlAttr *attr;
    for (attr = root->properties; attr != NULL; attr = attr->next) {
        char *attrName = (char *)attr->name;
        char *cont = (char *)(attr->children->content);

        if (strcmp(attrName, "version") == 0) {
            newDoc->version = atof(cont);
        } else if (strcmp(attrName, "creator") == 0) {
        	if (strcmp(cont, "") != 0) {
	            newDoc->creator = (char *)realloc(newDoc->creator, strlen(cont) + 1);
	            strcpy(newDoc->creator, cont);
	        } else {
	        	deleteGPXdoc(newDoc);
				xmlFreeDoc(doc);
				return NULL;
	        }
        }
    }

    // Populates the GPXdoc with the waypoints, routes, and tracks in the xml file
	newDoc = populateGPX(newDoc, root->children);

	if (newDoc == NULL) {
		deleteGPXdoc(newDoc);
		xmlFreeDoc(doc);
		xmlCleanupParser();
		return NULL;
	}

	xmlFreeDoc(doc);
	xmlCleanupParser();

	return newDoc;
}

char *GPXdocToString(GPXdoc *doc) {
	if (doc == NULL) {
		return "";
	}

	char *docString = (char *)malloc(1024 * sizeof(char));
	int length = 0;
	length += sprintf(docString + length, "Namespace: %s\nVersion: %0.1f\nCreator: %s\n\n", doc->namespace, doc->version, doc->creator);
	
	ListIterator wptIter = createIterator(doc->waypoints);
	ListIterator rteIter = createIterator(doc->routes);
	ListIterator trkIter = createIterator(doc->tracks);

	// Concatenates the string of all the waypoints to docString
	Waypoint *wpt = (Waypoint *)nextElement(&wptIter);
	while (wpt != NULL) {
		char *temp = waypointToString(wpt);
		length += sprintf(docString + length, "%s", temp);
		free(temp);
		wpt = (Waypoint *)nextElement(&wptIter);
	}

	// Concatenates the string of all the routes to docString
	Route *rte = (Route *)nextElement(&rteIter);
	while (rte != NULL) {
		char *temp = routeToString(rte);
		length += sprintf(docString + length, "%s", temp);
		free(temp);
		rte = (Route *)nextElement(&rteIter);
	}

	// Concatenates the string of all the tracks to docString
	Track *trk = (Track *)nextElement(&trkIter);
	while (trk != NULL) {
		char *temp = trackToString(trk);
		length += sprintf(docString + length, "%s", temp);
		free(temp);
		trk = (Track *)nextElement(&trkIter);
	}
	docString[strlen(docString)] = '\0';
	return docString;
}

void deleteGPXdoc(GPXdoc *doc) {
	if (doc == NULL) {
		return;
	}
	free(doc->creator);
	freeList(doc->waypoints);
	freeList(doc->routes);
	freeList(doc->tracks);
	free(doc);
}

void deleteGpxData(void *data) {
	GPXData *oldData = (GPXData *)data;
	free(oldData);
}

char *gpxDataToString(void *data) {
	char *gpxString = (char *)malloc(512 * sizeof(char));
	GPXData *gpx = (GPXData *)data;
	if (gpx != NULL) {
		sprintf(gpxString, "\tName=%s\n\tValue=%s\n\n", gpx->name, gpx->value);
	}
	gpxString[strlen(gpxString)] = '\0';
	return gpxString;
}

int compareGpxData(const void *first, const void *second) {
	return -1;
}

void deleteWaypoint(void *data) {
	Waypoint *oldPoint = (Waypoint *)data;
	free(oldPoint->name);
	freeList(oldPoint->otherData);
	free(oldPoint);
}

char *waypointToString(void *data) {
	char *wptString = (char *)malloc(512 * sizeof(char));
	Waypoint *wpt = (Waypoint *)data;
	int length = 0;
	if (wpt != NULL) {
		// String of the core information of a waypoint (name, lat, lon)
		if (wpt->name[0] != '\0') {
			length += sprintf(wptString + length, "lat=%f lon=%f\nName=%s\n\n", wpt->latitude, wpt->longitude, wpt->name);
		} else {
			length += sprintf(wptString + length, "lat=%f lon=%f\n", wpt->latitude, wpt->longitude);
		}
		ListIterator otherIter = createIterator(wpt->otherData);
		GPXData *gpx = (GPXData *)nextElement(&otherIter);
		// String of the otherData in the waypoint
		while (gpx != NULL) {
			char *temp = gpxDataToString(gpx);
			length += sprintf(wptString + length, "%s", temp);
			free(temp);
			gpx = (GPXData *)nextElement(&otherIter);
		}
	}
	wptString[strlen(wptString)] = '\0';
	return wptString;
}

int compareWaypoints(const void *first, const void *second) {
	return -1;
}

void deleteRoute(void *data) {
	Route *oldRoute = (Route *)data;
	free(oldRoute->name);
	freeList(oldRoute->waypoints);
	freeList(oldRoute->otherData);
	free(oldRoute);
}

char *routeToString(void *data) {
	char *rteString = malloc(512 * sizeof(char));
	Route *rte = (Route *)data;
	int length = 0;
	if (rte != NULL) {
		if (strcmp(rte->name, "") != 0) {
			length += sprintf(rteString + length, "Name: %s\n", rte->name);
		}
		ListIterator rtept = createIterator(rte->waypoints);
		Waypoint *wpt = (Waypoint *)nextElement(&rtept);
		// String of all the route points in the route
		while (wpt != NULL) {
			char *temp = waypointToString(wpt);
			length += sprintf(rteString +length, "%s", temp);
			free(temp);
			wpt = (Waypoint *)nextElement(&rtept);
		}

		// String of all the otherData in the route
		ListIterator otherIter = createIterator(rte->otherData);
		GPXData *gpx = (GPXData *)nextElement(&otherIter);
		while (gpx != NULL) {
			char *temp = gpxDataToString(gpx);
			length += sprintf(rteString + length, "%s", temp);
			free(temp);
			gpx = (GPXData *)nextElement(&otherIter);
		}
	}
	rteString[strlen(rteString)] = '\0';
	return rteString;
}

int compareRoutes(const void *first, const void *second) {
	return -1;
}

void deleteTrackSegment(void *data) {
	TrackSegment *oldSeg = (TrackSegment *)data;
	freeList(oldSeg->waypoints);
	free(oldSeg);
}

char *trackSegmentToString(void *data) {
	char *trksegString = malloc(512 * sizeof(char));
	TrackSegment *trkSeg = (TrackSegment *)data;
	ListIterator wptIter = createIterator(trkSeg->waypoints);
	Waypoint *wpt = (Waypoint *)nextElement(&wptIter);

	// String of all waypoints in a TrackSegment
	int length = 0;
	while (wpt != NULL) {
		char *temp = waypointToString(wpt);
		length += sprintf(trksegString + length, "%s", temp);
		free(temp);
		wpt = (Waypoint *)nextElement(&wptIter);
	}
	trksegString[strlen(trksegString)] = '\0';
	return trksegString;
}

int compareTrackSegments(const void *first, const void *second) {
	return -1;
}

void deleteTrack(void *data) {
	Track *oldTrack = (Track *)data;
	free(oldTrack->name);
	freeList(oldTrack->segments);
	freeList(oldTrack->otherData);
	free(oldTrack);
}

char *trackToString(void *data) {
	char *trkString = (char *)malloc(512 * sizeof(char));
	Track *trk = (Track *)data;
	int length = 0;
	if (trk != NULL) {
		if (strcmp(trk->name, "") != 0) {
			length += sprintf(trkString + length, "Name: %s\n", trk->name);
		}
		ListIterator trksegIter = createIterator(trk->segments);
		TrackSegment *trkseg = (TrackSegment *)nextElement(&trksegIter);
		// String of all TrackSegments
		while (trkseg != NULL) {
			char *temp = trackSegmentToString(trkseg);
			length += sprintf(trkString + length, "%s", temp);
			free(temp);
			trkseg = (TrackSegment *)nextElement(&trksegIter);
		}

		// String of all otherData in the Track
		ListIterator otherIter = createIterator(trk->otherData);
		GPXData *gpx = (GPXData *)nextElement(&otherIter);
		while (gpx != NULL) {
			char *temp = gpxDataToString(gpx);
			length += sprintf(trkString + length, "%s", temp);
			free(temp);
			gpx = (GPXData *)nextElement(&otherIter);
		}
	}
	trkString[strlen(trkString)] = '\0';
	return trkString;
}

int compareTracks(const void *first, const void *second) {
	return -1;
}

int getNumWaypoints(const GPXdoc *doc) {
	if (doc != NULL) {
		int count = 0;
		ListIterator wptIter = createIterator(doc->waypoints);
		Waypoint *wpt = (Waypoint *)nextElement(&wptIter);
		// Loops until encountering a NULL element, counts the number of non-NULL elements
		while (wpt != NULL) {
			count++;
			wpt = (Waypoint *)nextElement(&wptIter);
		}
		return count;
	} else {
		return -1;
	}
}

int getNumRoutes(const GPXdoc* doc) {
	if (doc != NULL) {
		int count = 0;
		ListIterator rteIter = createIterator(doc->routes);
		Route *rte = (Route *)nextElement(&rteIter);
		// Loops until encountering a NULL element, counts the number of non-NULL elements
		while (rte != NULL) {
			count++;
			rte = (Route *)nextElement(&rteIter);
		}
		return count;
	} else {
		return -1;
	}
}

int getNumTracks(const GPXdoc* doc) {
	if (doc != NULL) {
		int count = 0;
		ListIterator trkIter = createIterator(doc->tracks);
		Track *trk = (Track *)nextElement(&trkIter);
		// Loops until encountering a NULL element, counts the number of non-NULL elements
		while (trk != NULL) {
			count++;
			trk = (Track *)nextElement(&trkIter);
		}
		return count;
	} else {
		return -1;
	}
}

int getNumSegments(const GPXdoc* doc) {
	if (doc != NULL) {
		int count = 0;
		ListIterator trkIter = createIterator(doc->tracks);
		Track *trk = (Track *)nextElement(&trkIter);
		// Loops until encountering a NULL element, counts the number of non-NULL elements
		while (trk != NULL) {
			ListIterator segIter = createIterator(trk->segments);
			TrackSegment *trkSeg = (TrackSegment *)nextElement(&segIter);
			while (trkSeg != NULL) {
				count++;
				trkSeg = (TrackSegment *)nextElement(&segIter);
			}
			trk = (Track *)nextElement(&trkIter);
		}
		return count;
	} else {
		return -1;
	}
}

int getNumGPXData(const GPXdoc* doc) {
	if (doc != NULL) {
		int count = 0;
		int temp = 0;

		// Adds all the number of 'otherData' in all waypoints, routes, tracks
		temp = getNumWptList(doc->waypoints);
		if (temp != -1) {
			count += temp;
		}

		temp = getNumRteList(doc->routes);
		if (temp != -1) {
			count += temp;
		}

		temp = getNumTrkList(doc->tracks);
		if (temp != -1) {
			count += temp;
		}
		return count;
	} else {
		return -1;
	}
}

Waypoint *getWaypoint(const GPXdoc* doc, char* name) {
	if (doc != NULL) {
		ListIterator wptIter = createIterator(doc->waypoints);
		Waypoint *wpt = (Waypoint *)nextElement(&wptIter);
		// Checks each element for a name matching the key, return a pointer to that Waypoint,
		// otherwise return NULL
		while (wpt != NULL) {
			if (strcmp(wpt->name, name) == 0) {
				return wpt;
			}
			wpt = (Waypoint *)nextElement(&wptIter);
		}
		return NULL;
	} else {
		return NULL;
	}
}

Route *getRoute(const GPXdoc* doc, char* name) {
	if (doc != NULL) {
		ListIterator rteIter = createIterator(doc->routes);
		Route *rte = (Route *)nextElement(&rteIter);
		// Checks each element for a name matching the key, return a pointer to that Waypoint,
		// otherwise return NULL
		while (rte != NULL) {
			if (strcmp(rte->name, name) == 0) {
				return rte;
			}
			rte = (Route *)nextElement(&rteIter);
		}
		return NULL;
	} else {
		return NULL;
	}
}

Track *getTrack(const GPXdoc* doc, char* name) {
	if (doc != NULL) {
		ListIterator trkIter = createIterator(doc->tracks);
		Track *trk = (Track *)nextElement(&trkIter);
		// Checks each element for a name matching the key, return a pointer to that Waypoint,
		// otherwise return NULL
		while (trk != NULL) {
			if (strcmp(trk->name, name) == 0) {
				return trk;
			}
			trk = (Track *)nextElement(&trkIter);
		}
		return NULL;
	} else {
		return NULL;
	}
}

// A2
GPXdoc* createValidGPXdoc(char* fileName, char* gpxSchemaFile) {
    if (fileName == NULL || gpxSchemaFile == NULL || fileName[0] == '\0' || gpxSchemaFile[0] == '\0') {
    	return NULL;
    }

    GPXdoc *newDoc = (GPXdoc *)malloc(sizeof(GPXdoc));

	// Initialize all the variables in a GPXdoc
	strcpy(newDoc->namespace, "namespace");
	newDoc->version = 0.0;
	newDoc->creator = (char *)malloc(sizeof(char));
	newDoc->creator[0] = '\0';
	newDoc->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
	newDoc->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
	newDoc->tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);

	// The code below contains sample code from the provided libXmlExample.c file
	LIBXML_TEST_VERSION

	xmlDoc *doc = NULL;
	xmlNode* root = NULL;

	// Checks if provided gpx file is valid

	doc = xmlReadFile(fileName, NULL, 0);

	// Validates the xmlDoc before parsing the rest of the doc
	if (validateXmlTree(doc, gpxSchemaFile) == -1) {
		deleteGPXdoc(newDoc);
		xmlFreeDoc(doc);
		xmlCleanupParser();
		return NULL;
	}

	root = xmlDocGetRootElement(doc);

	// Gets the namespace of the xml file
	if (strcmp((char *)root->name, "gpx") == 0) {
		if (strcmp((char *)root->ns->href, "") != 0) {
        	strcpy(newDoc->namespace, (char *)root->ns->href);
    	} else {
    		deleteGPXdoc(newDoc);
			xmlFreeDoc(doc);
			return NULL;
    	}
    }

    // Gets the version and creator of the xml file
    xmlAttr *attr;
    for (attr = root->properties; attr != NULL; attr = attr->next) {
        char *attrName = (char *)attr->name;
        char *cont = (char *)(attr->children->content);

        if (strcmp(attrName, "version") == 0) {
            newDoc->version = atof(cont);
        } else if (strcmp(attrName, "creator") == 0) {
        	if (strcmp(cont, "") != 0) {
	            newDoc->creator = (char *)realloc(newDoc->creator, strlen(cont) + 1);
	            strcpy(newDoc->creator, cont);
	        } else {
	        	deleteGPXdoc(newDoc);
				xmlFreeDoc(doc);
				return NULL;
	        }
        }
    }

    // Populates the GPXdoc with the waypoints, routes, and tracks in the xml file
	newDoc = populateGPX(newDoc, root->children);

	if (newDoc == NULL) {
		deleteGPXdoc(newDoc);
		xmlFreeDoc(doc);
		xmlCleanupParser();
		return NULL;
	}

	xmlFreeDoc(doc);
	xmlCleanupParser();

	return newDoc;
}

bool writeGPXdoc(GPXdoc* doc, char* fileName) {
	if (doc == NULL || fileName == NULL || fileName[0] == '\0') {
		return false;
	}
	xmlDoc* tree = GPXdocToTree(doc);
	// Saves the GPXdoc as an xml file
	// Reference: http://www.xmlsoft.org/examples/tree2.c
	xmlSaveFormatFileEnc(fileName, tree, "UTF-8", 1);
	xmlFreeDoc(tree);
    xmlCleanupParser();
	return true;
}

bool validateGPXDoc(GPXdoc* doc, char* gpxSchemaFile) {
	if (doc == NULL || gpxSchemaFile == NULL || gpxSchemaFile[0] == '\0') {
    	return false;
	}

	// Validates the GPX doc using similar code from example given in assignment description
	// Reference: https://knol2share.blogspot.com/2009/05/validate-xml-against-xsd-in-c.html
	xmlDoc* tree = GPXdocToTree(doc);
	int invalid = validateXmlTree(tree, gpxSchemaFile);
	if (invalid == -1) {
		xmlFreeDoc(tree);
		return false;
	} else {
		xmlFreeDoc(tree);
		return true;
	}
}

float round10(float len) {
	float remainder = fmod(len, 10.0);
	// Gets the remainder of the length when divided by 10, if greater than 4, round up, else round down
	if (remainder == 0) {
		return len;
	} else if (remainder >= 5) {
		return (len + (10 - remainder));
	} else {
		return (len - remainder);
	}
}

float getRouteLen(const Route *rt) {
	if (rt == NULL) {
		return 0;
	}

	float totalLength = 0.0;
	ListIterator rteptIter = createIterator(rt->waypoints);
	Waypoint* rtept = (Waypoint *)nextElement(&rteptIter);
	Waypoint* prev = NULL;
	// Gets the length of the route by adding distance between all points (using Haversine Formula)
	while (rtept != NULL) {
		prev = rtept;
		rtept = (Waypoint *)nextElement(&rteptIter);
		totalLength += getPointDistance(prev, rtept);
	}
	return totalLength;
}

float getTrackLen(const Track *tr) {
	if (tr == NULL) {
		return 0;
	}

	float totalLength = 0.0;
	ListIterator trksegIter = createIterator(tr->segments);
	TrackSegment* trkseg = (TrackSegment *)nextElement(&trksegIter);
	Waypoint* prev = NULL;
	// Gets the length of the track by adding distance between all points and track segments
	// Uses Haversine Formula
	while (trkseg != NULL) {
		ListIterator trkptIter = createIterator(trkseg->waypoints);
		Waypoint* trkpt = (Waypoint *)nextElement(&trkptIter);
		if (prev != NULL) {
			totalLength += getPointDistance(trkpt, prev);
		}
		while (trkpt != NULL) {
			prev = trkpt;
			trkpt = (Waypoint *)nextElement(&trkptIter);
			totalLength += getPointDistance(prev, trkpt);
		}
		trkseg = (TrackSegment *)nextElement(&trksegIter);
	}
	return totalLength;
}

int numRoutesWithLength(const GPXdoc* doc, float len, float delta) {
	if (doc == NULL || len < 0 || delta < 0) {
		return 0;
	}

	int rteCount = 0;
	ListIterator rteIter = createIterator(doc->routes);
	Route* rte = (Route *)nextElement(&rteIter);
	while (rte != NULL) {
		float routeLen = getRouteLen(rte);
		if (getDistanceDiff(routeLen, len) <= (delta)) {
			rteCount++;
		}
		rte = (Route *)nextElement(&rteIter);
	}
	return rteCount;
}

int numTracksWithLength(const GPXdoc* doc, float len, float delta) {
	if (doc == NULL || len < 0 || delta < 0) {
		return 0;
	}

	int trkCount = 0;
	ListIterator trkIter = createIterator(doc->tracks);
	Track* trk = (Track *)nextElement(&trkIter);
	while (trk != NULL) {
		float trackLen = getTrackLen(trk);
		if (getDistanceDiff(trackLen, len) <= (delta)) {
			trkCount++;
		}
		trk = (Track *)nextElement(&trkIter);
	}
	return trkCount;
}

bool isLoopRoute(const Route* route, float delta) {
	if (route == NULL) {
		return false;
	}

	// If delta is negative or there are less than 4 waypoints then a loop is not possible
	if (delta < 0 || getNumElements(route->waypoints) < 4) {
		return false;
	}

	// Gets the first and last points of the route
	Waypoint *firstPt = (Waypoint *)getFromFront(route->waypoints);
	Waypoint *lastPt = (Waypoint *)getFromBack(route->waypoints);

	// Calculate the distance of the 2 points and determines if the route is a loop
	float ptDistance = getPointDistance(firstPt, lastPt);
	if (ptDistance <= delta) {
		return true;
	} else {
		return false;
	}
}

bool isLoopTrack(const Track *tr, float delta) {
	if (tr == NULL) {
		return false;
	}

	ListIterator tempIter = createIterator(tr->segments);
	TrackSegment *tempSeg = (TrackSegment *)nextElement(&tempIter);

	int totalPts = 0;
	while (tempSeg != NULL) {
		totalPts += getNumElements(tempSeg->waypoints);
		tempSeg = (TrackSegment *)nextElement(&tempIter);
	}

	// If delta is negative or there are less than 4 points then a loop is not possible
	if (delta < 0 || totalPts < 4) {
		return false;
	}

	ListIterator trksegIter = createIterator(tr->segments);
	TrackSegment* trkseg = (TrackSegment *)nextElement(&trksegIter);
	TrackSegment* lastSeg = NULL;
	// Gets the first point of the first track segment with at least 1 point
	Waypoint* firstPt = (Waypoint *)getFromFront(trkseg->waypoints);
	while (getNumElements(trkseg->waypoints) == 0) {
		trkseg = (TrackSegment *)nextElement(&trksegIter);
		if (trkseg == NULL) {
			break;
		}
	}
	if (trkseg != NULL) {
		firstPt = (Waypoint *)getFromFront(trkseg->waypoints);
	}

	while (trkseg != NULL) {
		// Make sure that the track segments has waypoints
		if (getNumElements(trkseg->waypoints) > 0) {
			lastSeg = trkseg;
		}
		trkseg = (TrackSegment *)nextElement(&trksegIter);
	}
	Waypoint *lastPt = (Waypoint *)getFromBack(lastSeg->waypoints);
	float ptDistance = getPointDistance(firstPt, lastPt);
	if (ptDistance <= delta) {
		return true;
	} else {
		return false;
	}
}

List* getRoutesBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta) {
	if (doc == NULL) {
		return NULL;
	}
	List* routes = initializeList(&routeToString, &deleteNothing, &compareRoutes);
	Waypoint sourcePt = {"", sourceLong, sourceLat, NULL};
	Waypoint destPt = {"", destLong, destLat, NULL};
	ListIterator rteIter = createIterator(doc->routes);
	Route* rte = (Route *)nextElement(&rteIter);
	while (rte != NULL) {
		// Gets the first and last waypoints
		Waypoint *firstPt = (Waypoint *)getFromFront(rte->waypoints);
		Waypoint *lastPt = (Waypoint *)getFromBack(rte->waypoints);
		// Calculates the distance between first point and the source and last point and the destination
		float sourceDist = getPointDistance(&sourcePt, firstPt);
		float destDist = getPointDistance(&destPt, lastPt);
		if (sourceDist <= delta && destDist <= delta) {
			insertBack(routes, rte);
		}
		rte = (Route *)nextElement(&rteIter);
	}
	if (getNumElements(routes) < 1) {
		// If there are no routes between then free list and return
		freeList(routes);
		return NULL;
	} else {
		return routes;
	}
}

List* getTracksBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta) {
	if (doc == NULL) {
		return NULL;
	}
	List* tracks = initializeList(&trackToString, &deleteNothing, &compareTracks);
	Waypoint sourcePt = {"", sourceLong, sourceLat, NULL};
	Waypoint destPt = {"", destLong, destLat, NULL};
	ListIterator trkIter = createIterator(doc->tracks);
	Track* trk = (Track *)nextElement(&trkIter);
	Waypoint *firstPt = NULL;
	Waypoint *lastPt = NULL;
	while (trk != NULL) {
		ListIterator trksegIter = createIterator(trk->segments);
		TrackSegment* trkseg = (TrackSegment *)nextElement(&trksegIter);
		// Gets the first track segment with at least 1 waypoint
		while (getNumElements(trkseg->waypoints) == 0) {
			trkseg = (TrackSegment *)nextElement(&trksegIter);
		}
		firstPt = (Waypoint *)getFromFront(trkseg->waypoints);
		lastPt = (Waypoint *)getFromBack(trkseg->waypoints);
		trkseg = (TrackSegment *)nextElement(&trksegIter);
		// Loop to get the last valid point of the last valid track segment
		while (trkseg != NULL) {
			if (trkseg->waypoints != NULL && getNumElements(trkseg->waypoints) != 0) {
				lastPt = (Waypoint *)getFromBack(trkseg->waypoints);
			}
			trkseg = (TrackSegment *)nextElement(&trksegIter);
		}
		float sourceDist = getPointDistance(firstPt, &sourcePt);
		float destDist = getPointDistance(lastPt, &destPt);
		if (sourceDist <= delta && destDist <= delta) {
			insertBack(tracks, trk);
		}
		trk = (Track *)nextElement(&trkIter);
	}
	if (getNumElements(tracks) < 1) {
		// If there are no tracks between then free the list and return
		freeList(tracks);
		return NULL;
	} else {
		return tracks;
	}
}

char* routeToJSON(const Route *rt) {
	if (rt == NULL) {
		return "{}";
	}

	char *name = rt->name;
	if (name[0] == '\0') {
		name = "None";
	}
	int numPoints = getNumElements(rt->waypoints);
	float routeLen = round10(getRouteLen(rt));
	size_t stringLen = 0;
	char loopBool[6] = "";
	// Calculate length of string according to whether the track is a loop or not
	if (isLoopRoute(rt, 10)) {
		strcpy(loopBool, "true");
		stringLen = snprintf(NULL, 0, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%0.1f,\"loop\":%s}", name, numPoints, routeLen, loopBool);
	} else {
		strcpy(loopBool, "false");
		stringLen = snprintf(NULL, 0, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%0.1f,\"loop\":%s}", name, numPoints, routeLen, loopBool);
	}
	// Allocate enough memory and construct the string
	char *rteJSON = malloc((stringLen + 1) * sizeof(char));
	sprintf(rteJSON, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%0.1f,\"loop\":%s}", name, numPoints, routeLen, loopBool);
	return rteJSON;
}

char* trackToJSON(const Track *tr) {
	if (tr == NULL) {
		return "{}";
	}

	char* name = tr->name;
	if (name[0] == '\0') {
		name = "None";
	}
	float trackLen = round10(getTrackLen(tr));
	size_t stringLen = 0;
	char loopBool[6];
	// Calculate length of string according to whether the track is a loop or not
	if (isLoopTrack(tr, 10)) {
		strcpy(loopBool, "true");
		stringLen = snprintf(NULL, 0, "{\"name\":\"%s\",\"len\":%0.1f,\"loop\":%s}", name, trackLen, loopBool);
	} else {
		strcpy(loopBool, "false");
		stringLen = snprintf(NULL, 0, "{\"name\":\"%s\",\"len\":%0.1f,\"loop\":%s}", name, trackLen, loopBool);
	}
	// Allocate enough memory and construct the string
	char* trkJSON = malloc((stringLen + 1) * sizeof(char));
	sprintf(trkJSON, "{\"name\":\"%s\",\"len\":%0.1f,\"loop\":%s}", name, trackLen, loopBool);
	return trkJSON;
}

char* routeListToJSON(const List *list) {
	if (list == NULL) {
		return "[]";
	}

	ListIterator rteIter = createIterator((List *)list);
	Route* rte = (Route *)nextElement(&rteIter);
	// Get the length of the string to malloc
	size_t stringLen = snprintf(NULL, 0, "[");
	// Adds 1 more character for the closing bracket ']' if list is null 
	if (rte == NULL) {
		stringLen += 1;
	}
	// First loop is responsible for calculating the amount of memory to allocate
	while (rte != NULL) {
		char* rteJson = routeToJSON(rte);
		stringLen += snprintf(NULL, 0, "%s", rteJson);
		free(rteJson);
		rte = (Route *)nextElement(&rteIter);
		if (rte == NULL) {
			stringLen += snprintf(NULL, 0, "]");
		} else {
			stringLen += snprintf(NULL, 0, ",");
		}
	}
	char *rteListJSON = malloc((stringLen + 1) * sizeof(char));
	// Reset iterator and construct the string
	rteIter = createIterator((List *)list);
	rte = (Route *)nextElement(&rteIter);
	int length = 0;
	length += sprintf(rteListJSON + length, "[");
	// Second loop is responsible for constructing the string itself, using the routeToJSON() function
	while (rte != NULL) {
		char* rteJson = routeToJSON(rte);
		length += sprintf(rteListJSON + length, "%s", rteJson);
		free(rteJson);
		rte = (Route *)nextElement(&rteIter);
		if (rte != NULL) {
			length += sprintf(rteListJSON + length, ",");
		}
	}
	length += sprintf(rteListJSON + length, "]");
	return rteListJSON;
}

char* trackListToJSON(const List *list) {
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
		char* trkJson = trackToJSON(trk);
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
		char* trkJson = trackToJSON(trk);
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

char* GPXtoJSON(const GPXdoc* gpx) {
	if (gpx == NULL) {
		return "{}";
	}

	// Gets the number of characters needed to allocate memory for
	size_t stringLength = snprintf(NULL, 0, "{\"version\":%0.1f,\"creator\":\"%s\",\"numWaypoints\":%d,\"numRoutes\":%d,\"numTracks\":%d}",
									gpx->version, gpx->creator, getNumWaypoints(gpx), getNumRoutes(gpx), getNumTracks(gpx));
	// Allocate and construct string with correct JSON formatting
	char* gpxJson = malloc((stringLength + 1) * sizeof(char));
	sprintf(gpxJson, "{\"version\":%0.1f,\"creator\":\"%s\",\"numWaypoints\":%d,\"numRoutes\":%d,\"numTracks\":%d}",
						gpx->version, gpx->creator, getNumWaypoints(gpx), getNumRoutes(gpx), getNumTracks(gpx));
	return gpxJson;
}

void addWaypoint(Route *rt, Waypoint *pt) {
	if (rt == NULL || pt == NULL) {
		return;
	}

	insertBack(rt->waypoints, pt);
}

void addRoute(GPXdoc* doc, Route* rt) {
	if (doc == NULL || rt == NULL) {
		return;
	}

	insertBack(doc->routes, rt);
}

GPXdoc* JSONtoGPX(const char* gpxString) {
	if (gpxString == NULL) {
		return NULL;
	}

	GPXdoc *newDoc = (GPXdoc *)malloc(sizeof(GPXdoc));

	// Initialize all the variables in a GPXdoc
	strcpy(newDoc->namespace, "http://www.topografix.com/GPX/1/1");
	newDoc->version = 0.0;
	newDoc->creator = (char *)malloc(sizeof(char));
	newDoc->creator[0] = '\0';
	newDoc->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
	newDoc->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
	newDoc->tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);

	char creator[1024];
	sscanf(gpxString, "{\"version\":%lf,\"creator\":\"%[^\"]\"}", &newDoc->version, creator);
	newDoc->creator = (char *)realloc(newDoc->creator, strlen(creator) + 1);
	strcpy(newDoc->creator, creator);
	return newDoc;
}

Waypoint* JSONtoWaypoint(const char* gpxString) {
	if (gpxString == NULL) {
		return NULL;
	}

	Waypoint* newWpt = (Waypoint *)malloc(sizeof(Waypoint));
	newWpt->name = (char *)malloc(sizeof(char));
	newWpt->name[0] = '\0';
	newWpt->latitude = 0.0;
	newWpt->longitude = 0.0;
	newWpt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

	sscanf(gpxString, "{\"lat\":%lf,\"lon\":%lf", &newWpt->latitude, &newWpt->longitude);
	return newWpt;
}

Route* JSONtoRoute(const char* gpxString) {
	if (gpxString == NULL) {
		return NULL;
	}

	Route* newRte = (Route *)malloc(sizeof(Route));
	newRte->name = (char *)malloc(sizeof(char));
	newRte->name[0] = '\0';
	newRte->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
	newRte->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

	char name[1024];
	sscanf(gpxString, "{\"name\":\"%[^\"]\"}", name);
	newRte->name = (char *)realloc(newRte->name, strlen(name) + 1);
	strcpy(newRte->name, name);
	return newRte;
}
