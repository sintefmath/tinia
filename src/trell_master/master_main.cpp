#include <iostream>
#include <cstdlib>

#include "Master.hpp"

int
main( int argc, char** argv )
{
    if( argc != 4 ) {
        std::cerr << "arc != 4, exiting.\n";
        exit( EXIT_FAILURE );
    }

    Trell::Master m(true, argv[3] );
    m.run( 3, argv );
    exit( EXIT_SUCCESS );
}
