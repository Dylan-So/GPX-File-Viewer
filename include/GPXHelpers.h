/*-----------------------------------
 *| Assignment 1					|
 *|									|
 *| AUTHOR: Dylan So 				|
 *| Student ID: 1091854				|
 *| School: University of Guelph	|
 *| Course: CIS2750					|
 *-----------------------------------
*/

#ifndef _GPXHELPER_H
#define _GPXHELPER_H

#include "GPXParser.h"

GPXdoc *populateGPX(GPXdoc *doc, xmlNode *root);

Waypoint *getWaypointData(xmlNode *curNode);

Route *getRouteData(xmlNode *curNode);

Track *getTrackData(xmlNode *curNode);

TrackSegment *getSegmentData(xmlNode *curNode);

int getNumElements(List *list);

int getNumWptList(List *list);

int getNumRteList(List *list);

int getNumTrkList(List *list);

#endif