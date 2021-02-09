/*-----------------------------------
 *| Assignment 1					|
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
#include "GPXParser.h"
#include "GPXHelpers.h"
#include "LinkedListAPI.h"

GPXdoc *createGPXdoc(char *fileName) {
	GPXdoc *newDoc = (GPXdoc *)malloc(sizeof(GPXdoc));

	strcpy(newDoc->namespace, "namespace");
	newDoc->version = 0.0;
	newDoc->creator = (char *)malloc(sizeof(char));
	newDoc->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
	newDoc->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
	newDoc->tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);

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

	if (strcmp((char *)root->name, "gpx") == 0) {
		if (strcmp((char *)root->ns->href, "") != 0) {
        	strcpy(newDoc->namespace, (char *)root->ns->href);
    	} else {
    		deleteGPXdoc(newDoc);
			xmlFreeDoc(doc);
			return NULL;
    	}
    }

    // Gets the data on attributes (version and creator)
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
	char *docString = (char *)malloc(1024 * sizeof(char));
	int length = 0;
	length += sprintf(docString + length, "Namespace: %s\nVersion: %0.1f\nCreator: %s\n\n", doc->namespace, doc->version, doc->creator);
	
	ListIterator wptIter = createIterator(doc->waypoints);
	ListIterator rteIter = createIterator(doc->routes);
	ListIterator trkIter = createIterator(doc->tracks);

	Waypoint *wpt = (Waypoint *)nextElement(&wptIter);
	while (wpt != NULL) {
		char *temp = waypointToString(wpt);
		length += sprintf(docString + length, "%s", temp);
		free(temp);
		wpt = (Waypoint *)nextElement(&wptIter);
	}

	Route *rte = (Route *)nextElement(&rteIter);
	while (rte != NULL) {
		char *temp = routeToString(rte);
		length += sprintf(docString + length, "%s", temp);
		free(temp);
		rte = (Route *)nextElement(&rteIter);
	}

	Track *trk = (Track *)nextElement(&trkIter);
	while (trk != NULL) {
		char *temp = trackToString(trk);
		length += sprintf(docString + length, "%s", temp);
		free(temp);
		trk = (Track *)nextElement(&trkIter);
	}
	//docString[strlen(docString)] = '\0';
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
		if (wpt->name[0] != '\0') {
			length += sprintf(wptString + length, "lat=%f lon=%f\nName=%s\n\n", wpt->latitude, wpt->longitude, wpt->name);
		} else {
			length += sprintf(wptString + length, "lat=%f lon=%f\n", wpt->latitude, wpt->longitude);
		}
		ListIterator otherIter = createIterator(wpt->otherData);
		GPXData *gpx = (GPXData *)nextElement(&otherIter);
		while (gpx != NULL) {
			char *temp = gpxDataToString(gpx);
			length += sprintf(wptString + length, "%s", temp);
			free(temp);
			gpx = (GPXData *)nextElement(&otherIter);
		}
	}
	//sprintf(wptString + strlen(wptString), "\n");
	//wptString[strlen(wptString)] = '\0';
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
		while (wpt != NULL) {
			char *temp = waypointToString(wpt);
			length += sprintf(rteString +length, "%s", temp);
			free(temp);
			wpt = (Waypoint *)nextElement(&rtept);
		}

		ListIterator otherIter = createIterator(rte->otherData);
		GPXData *gpx = (GPXData *)nextElement(&otherIter);
		while (gpx != NULL) {
			char *temp = gpxDataToString(gpx);
			length += sprintf(rteString + length, "%s", temp);
			free(temp);
			gpx = (GPXData *)nextElement(&otherIter);
		}
	}
	//sprintf(rteString + strlen(rteString), "\n");
	//rteString[strlen(rteString)] = '\0';
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

	// Prints first element without adding strlen(trksegString) to avoid printing unknown characters
	int length = 0;
	while (wpt != NULL) {
		char *temp = waypointToString(wpt);
		length += sprintf(trksegString + length, "%s", temp);
		free(temp);
		wpt = (Waypoint *)nextElement(&wptIter);
	}
	//trksegString[strlen(trksegString)] = '\0';
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
		while (trkseg != NULL) {
			char *temp = trackSegmentToString(trkseg);
			length += sprintf(trkString + length, "%s", temp);
			free(temp);
			trkseg = (TrackSegment *)nextElement(&trksegIter);
		}

		ListIterator otherIter = createIterator(trk->otherData);
		GPXData *gpx = (GPXData *)nextElement(&otherIter);
		while (gpx != NULL) {
			char *temp = gpxDataToString(gpx);
			length += sprintf(trkString + length, "%s", temp);
			free(temp);
			gpx = (GPXData *)nextElement(&otherIter);
		}
	}
	//sprintf(trkString + strlen(trkString), "\n");
	//trkString[strlen(trkString)] = '\0';
	return trkString;
}

int compareTracks(const void *first, const void *second) {
	return -1;
}

int getNumWaypoints(const GPXdoc *doc) {
	return 0;
}

int getNumRoutes(const GPXdoc* doc) {
	return 0;
}

int getNumTracks(const GPXdoc* doc) {
	return 0;
}

int getNumSegments(const GPXdoc* doc) {
	return 0;
}

int getNumGPXData(const GPXdoc* doc) {
	return 0;
}

Waypoint* getWaypoint(const GPXdoc* doc, char* name) {
	return NULL;
}

Track* getTrack(const GPXdoc* doc, char* name) {
	return NULL;
}

Route* getRoute(const GPXdoc* doc, char* name) {
	return NULL;
}
