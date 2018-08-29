/* 
 * File:   SessionOrganizer.cpp
 * Author: Kapil Thakkar
 * 
 */

#include "SessionOrganizer.h"
#include "Util.h"
#include <vector>
#include <tuple>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cmath>
#include <algorithm>
#include <random>
#include <chrono>
#include <ctime>
using namespace std;

SessionOrganizer::SessionOrganizer ( )
{
    parallelTracks = 0;
    papersInSession = 0;
    sessionsInTrack = 0;
    processingTimeInMinutes = 0;
    tradeoffCoefficient = 1.0;
}

SessionOrganizer::SessionOrganizer ( string filename )
{
    readInInputFile ( filename );
    conference = new Conference ( parallelTracks, sessionsInTrack, papersInSession );
}


Conference* SessionOrganizer::getRandomState(Conference* cf){
    int papers = cf->getParallelTracks() * cf ->getSessionsInTrack()
            * cf->getPapersInSession();
    //vector with numbers in a range
    vector<int> v(papers);
    iota(begin(v), end(v), 0);
    //shuffle
    auto rng = default_random_engine {};
    // obtain a time-based seed:
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(begin(v), end(v), default_random_engine(seed));
    
    int paperCounter=0;
    for ( int i = 0; i < cf->getSessionsInTrack ( ); i++ ){
        for ( int j = 0; j < cf->getParallelTracks ( ); j++ ){
            for ( int k = 0; k < cf->getPapersInSession ( ); k++ ){
                cf->setPaper( j, i, k, v.at(paperCounter));
                paperCounter++;
            }
        }
    }
    return cf;
}

Conference* SessionOrganizer::deepCopyConference(Conference* confForCopy){
    Conference* copiedConf = new Conference( confForCopy->getParallelTracks(),
                             confForCopy->getSessionsInTrack(), confForCopy->getPapersInSession());

    for ( int _i = 0; _i < confForCopy->getSessionsInTrack(); _i++){
        for ( int _j = 0; _j < confForCopy->getParallelTracks(); _j++){
            for ( int _k = 0; _k < confForCopy->getPapersInSession(); _k++){
                copiedConf->setPaper(_j, _i, _k, confForCopy->getTrack(_j).getSession(_i).getPaper(_k));
            }
        }
    }

    return copiedConf;
}

void SessionOrganizer::organizePapers (double begin, char * filename)
{
    int restart=0;
    Conference conf;
    vector<Conference> bestConf;
    double goodestScore = 0, confScore, swapScoring, totalTime=0, startTime=0, averageTime=0;
    int iteration=0;
    int p = conference->getPapersInSession(); //k
    int k = conference->getParallelTracks(); //j
    int t = conference->getSessionsInTrack(); //i
    int maxSwaps = min((double)(40+(.00001*p*p*k*k*t*t)), (double)100);

    double currentTime = (clock() - begin) / (double)CLOCKS_PER_SEC;

    while(currentTime < 60*processingTimeInMinutes - 5){
        startTime = (clock() - begin) / (double) CLOCKS_PER_SEC;

        conference = getRandomState(conference);
        conf = *conference;

        confScore = this->scoreOrganization(conf);

        if(confScore > goodestScore){
            goodestScore = confScore;

            Conference* conff = deepCopyConference(&conf);
            bestConf.push_back(*conff);
            if(bestConf.size()>1){
                bestConf.erase(bestConf.begin());
            }
        }

        srand(time(NULL));
        int wrongSwap = 0;
        int stateChange = 0;

        while(wrongSwap<maxSwaps){
            tuple<int, int, int> randHost = make_tuple(rand() % t, rand() % k, rand() % p);
            int randHostNum = conf.getTrack(get<1>(randHost)).getSession(get<0>(randHost)).getPaper(get<2>(randHost));
            
            tuple<int, int, int> randGuest = make_tuple(rand() % t, rand() % k, rand() % p);
            int randGuestNum = conf.getTrack(get<1>(randGuest)).getSession(get<0>(randGuest)).getPaper(get<2>(randGuest));

            conf.setPaper(get<1>(randHost), get<0>(randHost), get<2>(randHost), randGuestNum);
            conf.setPaper(get<1>(randGuest), get<0>(randGuest), get<2>(randGuest), randHostNum);
            swapScoring = this->scoreOrganization(conf);

            if(swapScoring > confScore){
                confScore = swapScoring;
                stateChange++;
                wrongSwap = 0;
            }
            else{
                conf.setPaper(get<1>(randHost), get<0>(randHost), get<2>(randHost), randHostNum);
                conf.setPaper(get<1>(randGuest), get<0>(randGuest), get<2>(randGuest), randGuestNum);
                wrongSwap++;
            }   
        }

        // cout <<"[!] state change: " <<stateChange <<endl;
        if(this->scoreOrganization(conf) > goodestScore){
            goodestScore = this->scoreOrganization(conf);
            // cout<<"[*] Highest score in the iteration: "<<goodestScore<<endl;
            Conference* conff = deepCopyConference(&conf);
            bestConf.push_back(*conff);
            if(bestConf.size()>1){
                bestConf.erase(bestConf.begin());
            }
        }

        iteration++;
        currentTime = (clock() - begin) / (double)CLOCKS_PER_SEC;
        totalTime += currentTime - startTime;
        averageTime = totalTime / iteration;
        // cout<<averageTime<<endl;

        if(currentTime > (60*processingTimeInMinutes - 2*averageTime))
        {
            printConference(filename, bestConf.at(bestConf.size()-1));
        }
        if(currentTime > 60*processingTimeInMinutes - 1.1*averageTime -2){
           break; 
        }
    }

    // cout <<"\n---------FINAL--------\n";
    // for(Conference c: bestConf){
    //     printOnConsole(c);
    //     cout<<this->scoreOrganization(c)<<endl;
    // }
    printConference(filename, bestConf.at(bestConf.size()-1));
    // cout<<this->scoreOrganization(bestConf.at(bestConf.size()-1));
}



void SessionOrganizer::readInInputFile ( string filename )
{
    vector<string> lines;
    string line;
    ifstream myfile ( filename.c_str () );
    if ( myfile.is_open ( ) )
    {
        while ( getline ( myfile, line ) )
        {
            //cout<<"Line read:"<<line<<endl;
            lines.push_back ( line );
        }
        myfile.close ( );
    }
    else
    {
        cout << "Unable to open input file";
        exit ( 0 );
    }

    if ( 6 > lines.size ( ) )
    {
        cout << "Not enough information given, check format of input file";
        exit ( 0 );
    }

    processingTimeInMinutes = atof ( lines[0].c_str () );
    papersInSession = atoi ( lines[1].c_str () );
    parallelTracks = atoi ( lines[2].c_str () );
    sessionsInTrack = atoi ( lines[3].c_str () );
    tradeoffCoefficient = atof ( lines[4].c_str () );

    int n = lines.size ( ) - 5;
    double ** tempDistanceMatrix = new double*[n];
    for ( int i = 0; i < n; ++i )
    {
        tempDistanceMatrix[i] = new double[n];
    }


    for ( int i = 0; i < n; i++ )
    {
        string tempLine = lines[ i + 5 ];
        string elements[n];
        splitString ( tempLine, " ", elements, n );

        for ( int j = 0; j < n; j++ )
        {
            tempDistanceMatrix[i][j] = atof ( elements[j].c_str () );
        }
    }
    distanceMatrix = tempDistanceMatrix;

    int numberOfPapers = n;
    int slots = parallelTracks * papersInSession*sessionsInTrack;
    if ( slots != numberOfPapers )
    {
        cout << "More papers than slots available! slots:" << slots << " num papers:" << numberOfPapers << endl;
        exit ( 0 );
    }
}

double** SessionOrganizer::getDistanceMatrix ( )
{
    return distanceMatrix;
}

void SessionOrganizer::printSessionOrganiser ( char * filename)
{
    conference->printConference ( filename);
}

double SessionOrganizer::scoreOrganization ( )
{
    // Sum of pairwise similarities per session.
    double score1 = 0.0;
    for ( int i = 0; i < conference->getParallelTracks ( ); i++ )
    {
        Track tmpTrack = conference->getTrack ( i );
        for ( int j = 0; j < tmpTrack.getNumberOfSessions ( ); j++ )
        {
            Session tmpSession = tmpTrack.getSession ( j );
            for ( int k = 0; k < tmpSession.getNumberOfPapers ( ); k++ )
            {
                int index1 = tmpSession.getPaper ( k );
                for ( int l = k + 1; l < tmpSession.getNumberOfPapers ( ); l++ )
                {
                    int index2 = tmpSession.getPaper ( l );
                    score1 += 1 - distanceMatrix[index1][index2];
                }
            }
        }
    }

    // Sum of distances for competing papers.
    double score2 = 0.0;
    for ( int i = 0; i < conference->getParallelTracks ( ); i++ )
    {
        Track tmpTrack1 = conference->getTrack ( i );
        for ( int j = 0; j < tmpTrack1.getNumberOfSessions ( ); j++ )
        {
            Session tmpSession1 = tmpTrack1.getSession ( j );
            for ( int k = 0; k < tmpSession1.getNumberOfPapers ( ); k++ )
            {
                int index1 = tmpSession1.getPaper ( k );

                // Get competing papers.
                for ( int l = i + 1; l < conference->getParallelTracks ( ); l++ )
                {
                    Track tmpTrack2 = conference->getTrack ( l );
                    Session tmpSession2 = tmpTrack2.getSession ( j );
                    for ( int m = 0; m < tmpSession2.getNumberOfPapers ( ); m++ )
                    {
                        int index2 = tmpSession2.getPaper ( m );
                        score2 += distanceMatrix[index1][index2];
                    }
                }
            }
        }
    }
    double score = score1 + tradeoffCoefficient*score2;
    return score;
}


// score calculation with @params: conference object
double SessionOrganizer::scoreOrganization (Conference temp)
{
    Conference* c = &temp;
    // Sum of pairwise similarities per session.
    double score1 = 0.0;
    for ( int i = 0; i < c->getParallelTracks ( ); i++ )
    {
        Track tmpTrack = c->getTrack ( i );
        for ( int j = 0; j < tmpTrack.getNumberOfSessions ( ); j++ )
        {
            Session tmpSession = tmpTrack.getSession ( j );
            for ( int k = 0; k < tmpSession.getNumberOfPapers ( ); k++ )
            {
                int index1 = tmpSession.getPaper ( k );
                for ( int l = k + 1; l < tmpSession.getNumberOfPapers ( ); l++ )
                {
                    int index2 = tmpSession.getPaper ( l );
                    score1 += 1 - distanceMatrix[index1][index2];
                }
            }
        }
    }

    // Sum of distances for competing papers.
    double score2 = 0.0;
    for ( int i = 0; i < c->getParallelTracks ( ); i++ )
    {
        Track tmpTrack1 = c->getTrack ( i );
        for ( int j = 0; j < tmpTrack1.getNumberOfSessions ( ); j++ )
        {
            Session tmpSession1 = tmpTrack1.getSession ( j );
            for ( int k = 0; k < tmpSession1.getNumberOfPapers ( ); k++ )
            {
                int index1 = tmpSession1.getPaper ( k );

                // Get competing papers.
                for ( int l = i + 1; l < c->getParallelTracks ( ); l++ )
                {
                    Track tmpTrack2 = c->getTrack ( l );
                    Session tmpSession2 = tmpTrack2.getSession ( j );
                    for ( int m = 0; m < tmpSession2.getNumberOfPapers ( ); m++ )
                    {
                        int index2 = tmpSession2.getPaper ( m );
                        score2 += distanceMatrix[index1][index2];
                    }
                }
            }
        }
    }
    double score = score1 + tradeoffCoefficient*score2;
    return score;
}


void SessionOrganizer::printOnConsole(Conference temp){
    Conference* c = &temp;
    for ( int i = 0; i < c->getSessionsInTrack(); i++ ){
        for ( int j = 0; j < c->getParallelTracks(); j++ ){
            for ( int k = 0; k < c->getPapersInSession(); k++ ){
                cout <<c->getTrack(j).getSession(i).getPaper(k) <<" ";
            }
            if ( j != c->getParallelTracks() - 1 ){
                cout << "|";
            }
        }
        cout<<"\n";
    }
}

void SessionOrganizer::printConference(char * filename, Conference temp)
{
    Conference* c = &temp;
    ofstream ofile(filename);
    for ( int i = 0; i < c->getSessionsInTrack(); i++ ){
        for ( int j = 0; j < c->getParallelTracks(); j++ ){
            for ( int k = 0; k < c->getPapersInSession(); k++ ){
                ofile<< c->getTrack(j).getSession( i ).getPaper( k ) << " ";
            }
            if ( j != c->getParallelTracks() - 1 ){
                ofile<<"| ";
            }
        }
        ofile<<"\n";
    }
    ofile.close();
    cout<<"Organization written to ";
    printf("%s :)\n",filename);
}