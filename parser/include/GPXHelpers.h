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
 * @brief Get the Waypoint List object
 * 
 * @param doc 
 * @return List* 
 */
List* getWaypointList(GPXdoc* doc);

/**
 * @brief Get the Route List object
 * 
 * @param doc 
 * @return List* 
 */
List* getRouteList(GPXdoc* doc);

/**
 * @brief Get the Track List object
 * 
 * @param doc 
 * @return List* 
 */
List* getTrackList(GPXdoc* doc);

int getTrkPts(List* list);
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

/**
 * @brief Generates an xmlDoc from a GPXdoc struct
 * 
 * @param gpxDoc    A pointer to a GPXdoc struct
 * @return xmlDoc   A pointer to the xmlDoc generated
 */
xmlDoc* GPXdocToTree(GPXdoc* gpxDoc);

/**
 * @brief Adds waypoints from the waypoint list in xml format to the provided xmlNode
 * 
 * @param waypoints     The waypoint list
 * @param root          The node to add waypoint xmlNodes to
 * @param name          The name of the node (wpt, rtept, trkpt)
 * @param namespace     The namespace for the xml file
 */
void addWptNodes(List* waypoints, xmlNode* root, char *name, xmlNs* namespace);

/**
 * @brief Adds routes from the route list in xml format to the provided xmlNode
 * 
 * @param routes        The route list
 * @param root          The node to add route xmlNodes to
 * @param namespace     The namespace for the xml file
 */
void addRteNodes(List* routes, xmlNode* root, xmlNs* namespace);

/**
 * @brief Adds tracks from the track list in xml format to the provided xmlNode
 * 
 * @param tracks        The track list
 * @param root          The node to add track xmlNodes to
 * @param namespace     The namespace for the xml file
 */
void addTrkNodes(List* tracks, xmlNode* root, xmlNs* namespace);

/**
 * @brief Get the distnace between 2 points using the Haversine Formula. Reference: https://www.movable-type.co.uk/scripts/latlong.html
 * 
 * @param wp1       The first waypoint
 * @param wp2       The second waypoint
 * @return float    The distance as a float
 */
float getPointDistance(Waypoint *wp1, Waypoint *wp2);

/**
 * @brief Calculates the difference and returns the absolute value
 * 
 * @param routeLen      The first float
 * @param len           The second float
 * @return float        The absolute value of the distance
 */
float getDistanceDiff(float routeLen, float len);

/**
 * @brief A dummy delete function for creating a list in getRoutesBetween() and getTracksBetween()
 * 
 */
void deleteNothing(void *);
#endif