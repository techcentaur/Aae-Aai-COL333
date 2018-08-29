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


/** let's make these functions
1. One that takes a state and returns all possible neighbours
    take a conference object in first settled state
    swap first paper to every other paper and calculate score
    do that for all papers
    make a tuple with (change in score, conference variable)
    return it
**/

// deep copy Conference objects
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

// take input as state -> Conferenece variable
vector<Conference> SessionOrganizer::swapPapersWith(int _paperTrack1, tuple<int, int, int> indices, Conference* _conf){
    vector<Conference> swappedNeighbours;

    for ( int _i = 0; _i < _conf->getSessionsInTrack(); _i++){
        for ( int _j = get<1>(indices)+1; _j < _conf->getParallelTracks(); _j++){
            for ( int _k = 0; _k < _conf->getPapersInSession(); _k++){                    
                // CheckPoint:: check for indices replacement:
                // cout<<get<0>(indices)<<" "<<get<1>(indices)<<" "<<get<2>(indices)<<endl;
                // cout<<_i<<" "<<_j<<" "<<_k<<endl;

                Conference* newConf = deepCopyConference(_conf);
                int _paperTrack2 = newConf->getTrack(_j).getSession(_i).getPaper(_k);
                newConf->setPaper(_j, _i, _k, _paperTrack1);
                newConf->setPaper(get<1>(indices), get<0>(indices), get<2>(indices), _paperTrack2);
                swappedNeighbours.push_back(*newConf);
                // printOnConsole(*newConf);
            }
        }
    }
    // for(Conference cc: swappedNeighbours){
    //     printOnConsole(cc);
    //     cout<<endl;
    // }    
    return swappedNeighbours;
}

vector<Conference> SessionOrganizer::getNeighbours(Conference* conf){
    vector<Conference> neighbours;
    int paperTrack1 = 0;

    for ( int i = 0; i < conf->getSessionsInTrack(); i++){
        for ( int j = 0; j < conf->getParallelTracks(); j++){
            for ( int k = 0; k < conf->getPapersInSession(); k++){
                paperTrack1 = conf->getTrack(j).getSession(i).getPaper(k);
                tuple<int, int, int> indices = make_tuple(i, j, k);
                vector<Conference> swappedNeigh = swapPapersWith(paperTrack1, indices, conf);
                neighbours.insert(neighbours.end(), swappedNeigh.begin(), swappedNeigh.end());
            }
        }
    }
    // for(Conference cc: neighbours){
    //     printOnConsole(cc);
    // }  
    return neighbours;
}

/** 2. Select one neighbour based on some logic:
select randomly from top 10% that are more than
**/

Conference SessionOrganizer::selectNeighbour(vector<Conference> neighbours){
    vector<tuple<Conference, double>> ConfTupPaper; 
    for(Conference v: neighbours){
        ConfTupPaper.push_back(make_tuple(v, this->scoreOrganization(v)));
    }

    sort(begin(ConfTupPaper), end(ConfTupPaper), [](auto const &t1, auto const &t2){
        return get<1>(t1) > get<1>(t2);
    });

    srand(time(NULL));

    int randomNum = rand() % ((int)(ConfTupPaper.size()));

 // CheckPoint:: check for upto a certain neighbours
    int ind=0;
 //    cout<<"\nTop 5 neighbours:\n";
    while(ind<ConfTupPaper.size()){
        cout<<"[.] State: ";
        printOnConsole(get<0>(ConfTupPaper.at(ind)));
        cout<<"[.] Value: " <<get<1>(ConfTupPaper.at(ind))<<endl;
        ind++;
    }
    cout<<ConfTupPaper.size()<<endl;
    tuple<Conference, double> tupval = ConfTupPaper.at(0);

    return get<0>(tupval);
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


void SessionOrganizer::organizePapers (double begin, char * filename)
{
    int restart=0;
    Conference goodestConference, neighbs, confer;
    double goodestScore = 0, conferScore, neighbScore, localHighest, currentTime;

    currentTime = (double)(clock() - begin) / CLOCKS_PER_SEC;

    while(currentTime < 58*processingTimeInMinutes){

        conference = getRandomState(conference);
        confer = *conference;
        
         // Checkpoints: check random states
          // cout<<"[*] Random selected state: ";
          // printOnConsole(confer);
          // cout<<"[*] Random selected score " <<scoreOrganization(confer) <<"\n" <<endl;
        
        while(true){
            conferScore = this->scoreOrganization(confer);
            neighbs = this->selectNeighbour(this->getNeighbours(&confer));

            neighbScore = this->scoreOrganization(neighbs);

            if (neighbScore > conferScore){
                // cout <<"[*] Neighbour selected with score: " <<neighbScore <<"\n"; 
                confer = neighbs;
            }
            else{
                localHighest = this->scoreOrganization(confer);
                // cout <<"[*] Highest score in "<<restart<<" iteration is : "<<localHighest<<"\n\n";
                if(localHighest > goodestScore){
                    goodestScore = localHighest;
                    goodestConference = confer;
                }
                break;
            }
        }
        restart++;
        currentTime = (double)(clock() - begin) / CLOCKS_PER_SEC;
    }
    cout <<"\n---------FINAL--------\n";
    cout <<"[*] Score: " <<goodestScore <<endl;
    cout <<"[*] State: "<<endl;
    printOnConsole(goodestConference);
    cout <<"-----------/----------\n";

    printConference(filename, goodestConference);
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
        cout<<"\n";
        ofile<<"\n";
    }
    ofile.close();
    // cout<<"Organization written to ";
    // printf("%s :)\n",filename);

}