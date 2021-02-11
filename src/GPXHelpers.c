/*-----------------------------------
 *| Assignment 1                    |
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
#include "GPXHelpers.h"

// FUNCTIONS getWaypointData, getRouteData, getTrackData CONTAINS SAMPLE CODE FROM PROVIDED the libXmlExample.c FILE
// TO PARSE AND RETRIEVE DATA FROM THE XML FILE
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