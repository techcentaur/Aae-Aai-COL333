/* 
 * File:   main.cpp
 * Author: Kapil Thakkar
 *
 */

#include <cstdlib>
#include <ctime>

#include "SessionOrganizer.h"

using namespace std;

/*
 * 
 */
int main ( int argc, char** argv )
{
    clock_t begin = clock();
    
    // Parse the input.
    if ( argc < 3 )
    {   
        cout<<"Missing arguments\n";
        cout<<"Correct format : \n";
        cout << "./main <input_filename> <output_filename>";
        exit ( 0 );
    }
    string inputfilename ( argv[1] );
    
    // Initialize the conference organizer.
    SessionOrganizer *organizer  = new SessionOrganizer( inputfilename );

    // Organize the papers into tracks based on similarity.
    organizer->organizePapers(begin, argv[2]);
    double finish = (clock()-begin) / (double) CLOCKS_PER_SEC;
    // cout<<"[*] Time Taken: "<< finish <<" seconds"<<endl;
    return 0;
}

