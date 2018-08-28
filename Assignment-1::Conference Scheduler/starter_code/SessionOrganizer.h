/* 
 * File:   SessionOrganizer.h
 * Author: Kapil Thakkar
 *
 */

#ifndef SESSIONORGANIZER_H
#define	SESSIONORGANIZER_H

#include <string>
#include <iostream>
#include <fstream>
#include <vector>


#include "Conference.h"
#include "Track.h"
#include "Session.h"

using namespace std;


/**
 * SessionOrganizer reads in a similarity matrix of papers, and organizes them
 * into sessions and tracks.
 * 
 * @author Kapil Thakkar
 *
 */
class SessionOrganizer {
private:
    double ** distanceMatrix;

    int parallelTracks ;
    int papersInSession ;
    int sessionsInTrack ;

    Conference *conference;

    double processingTimeInMinutes ;
    double tradeoffCoefficient ; // the tradeoff coefficient


public:
    SessionOrganizer();
    SessionOrganizer(string filename);
    
    
    /**
     * Read in the number of parallel tracks, papers in session, sessions
     * in a track, and the similarity matrix from the specified filename.
     * @param filename is the name of the file containing the matrix.
     * @return the similarity matrix.
     */
    void readInInputFile(string filename);
    
    //deep copy
    Conference* deepCopyConference(Conference* c);
    // swap one state to rest all
    vector<Conference> swapPapersWith(int p, tuple<int, int, int> i, Conference* c);
    // Get the neighbours for a state (Conference object)
    vector<Conference> getNeighbours(Conference* c);
    // select Neighbour
    Conference selectNeighbour(vector<Conference> vC);

    //get a random state
    Conference* getRandomState(Conference* cf);

    /**
     * Organize the papers according to some algorithm.
     */
    void organizePapers(double b, char * filename);
    /**
     * Get the distance matrix.
     * @return the distance matrix.
     */
    double** getDistanceMatrix();
    /**
     * Score the organization.
     * @return the score.
     */
    double scoreOrganization();
    double scoreOrganization(Conference c);
    
    // print Conference state on console
    void printOnConsole(Conference temp);

    void printSessionOrganiser(char *);

    void printConference (char * filename, Conference temp);

};

#endif	/* SESSIONORGANIZER_H */
