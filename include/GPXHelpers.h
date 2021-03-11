/*-----------------------------------
 *| Assignment 2					|
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

/**
 * @brief Populates the GPXdoc with all the waypoints, routes, 
 *        and tracks in the file.
 *
 * @param doc A pointer to the GPXdoc
 * @param root The root node
 *
 * @return A pointer to the GPXdoc.
 */
GPXdoc *populateGPX(GPXdoc *doc, xmlNode *root);

/**
 * @brief Gets data of a waypoint from the xmlNode.
 *
 * @param curNode The current node
 *
 * @return A pointer to the waypoint.
 */
Waypoint *getWaypointData(xmlNode *curNode);

/**
 * @brief Gets the data of a route from the xmlNode.
 *
 * @param curNode The current node
 *
 * @return A pointer to the route.
 */
Route *getRouteData(xmlNode *curNode);

/**
 * @brief Gets the data of a track from the xmlNode.
 *
 * @param curNode The current node
 *
 * @return A pointer to the track.
 */
Track *getTrackData(xmlNode *curNode);

/**
 * @brief Gets data of a TrackSegment from the xmlNode.
 *
 * @param curNode The current node
 *
 * @return A pointer to the TrackSegment.
 */
TrackSegment *getSegmentData(xmlNode *curNode);

/**
 * @brief Gets the number of elements in a list.
 *
 * @param list The list
 *
 * @return The number of elements.
 */
int getNumElements(List *list);

/**
 * @brief Gets the number of 'otherData' in a list of Waypoints.
 *
 * @param list The list
 *
 * @return The number of otherData.
 */
int getNumWptList(List *list);

/**
 * @brief Gets the number of 'otherData' in a list of Routes.
 *
 * @param list The list
 *
 * @return The number of otherData.
 */
int getNumRteList(List *list);

/**
 * @brief Gets number of 'otherData' in a list of Tracks.
 *
 * @param list The list
 *
 * @return The number of otherData.
 */
int getNumTrkList(List *list);

/**
 * @brief Checks if the xmlDoc provided is valid.
 *
 * @param doc 			The xmlDoc
 * @param gpxSchemaFile The gpx schema file
 *
 * @return -1 if invalid, 1 if valid
 */
int validateXmlTree(xmlDoc* doc, char* gpxSchemaFile);

xmlDoc* GPXdocToTree(GPXdoc* gpxDoc);

void addWptNodes(List* waypoints, xmlNode* root, char *name, xmlNs* namespace);
void addRteNodes(List* routes, xmlNode* root, xmlNs* namespace);
void addTrkNodes(List* tracks, xmlNode* root, xmlNs* namespace);
float getPointDistance(Waypoint *wp1, Waypoint *wp2);
float getDistanceDiff(float routeLen, float len);
void deleteNothing(void *);
#endif