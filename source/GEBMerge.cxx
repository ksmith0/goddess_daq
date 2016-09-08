#include <stdlib.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <assert.h>
#include <zlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>

#include "sys/sysinfo.h"

#include "gdecomp.h"
#include "GTMerge.h"
#include "MergeManager.h"

using std::string;

MSTAT* nstat;
CONTROL control;

bool printDebug = false;
bool printBytesCount = false;
bool printProgress = true;

// MergeManager* MergeManager::s_instance = nullptr;

#if (USEZLIB == 0 )
int bread ( std::ifstream inFile, char *val, int *pos, int *buf, int *bufsiz )
#else
int bread ( gzFile in, int *val, int *pos, int *buf, int *bufsiz )
#endif
{
    /* buffered inter read */

    /* declarations */

    int siz;

    /* read new buffer */

    if ( *pos == 0 )
    {

        /* read in a buffer of data */

#if (USEZLIB == 0 )

        inFile.read ( ( char * ) buf, *bufsiz * sizeof ( int ) );

#else

        siz = gzread ( in, ( char * ) buf, *bufsiz * sizeof ( int ) );

#endif

        *bufsiz = siz / sizeof ( int );

        if ( siz <= 0 )
            return ( -1 );

    };

    /* return a value */

    *val = * ( buf + ( *pos ) );

    ( *pos ) ++;

    /* force buffer read the next time? */

    if ( *pos == *bufsiz )
        *pos = 0;

    /* done, report we got 4 bytes like 'read' would */

    return ( sizeof ( int ) );

}

EVENT* GTGetDiskEv ( InDataInfo* inFile, EVENT* bufEvent, bool printInfo )
{
    if ( printDebug ) std::cerr << "File is " << ( inFile->istream->is_open() ? "open" : "NOT open!" ) << "\n";
    /* returns: */
    /*   0: success */
    /*   1: header */
    /*   2: read trouble; end of file */
    /*   5: bad test pattern */

    /* declarations */

    MergeManager* theMergeManager = MergeManager::sinstance();

    if ( printDebug ) std::cerr << "Retreived theMergeManager...\n";

    int siz, i1;
    int storeNo = theMergeManager->Event.size();

    /* attempt to read a GEB header */

#if (USEBREAD)

    siz = bread ( inFile->istream, ( char * ) bufEvent->gd, &bread_pos[storeNo], bread_buf[storeNo], &bread_bufsiz[storeNo] );

    if ( siz != sizeof ( GEBDATA ) )
    {
        printf ( "failed to read %lu bytes for header, got %i\n", sizeof ( GEBDATA ), siz );
        return nullptr;
    }

    nstat->inbytes += siz;
    control.filesiz[storeNo] += siz;

#else

    if ( printDebug ) std::cerr << "*** Reading the header...\n";

    ( * ( inFile->istream ) ).read ( ( char* ) bufEvent->gd, sizeof ( GEBDATA ) );

    if ( printDebug )
    {
        std::cerr << "=> Header read, now checking it...\n";
        std::cerr << "   - type is " << bufEvent->gd->type << " / length is " << bufEvent->gd->length << "\n";
        std::cerr << "   - timestamp is " << bufEvent->gd->timestamp << "\n";
    }

    if ( ( inFile->istream )->gcount() != sizeof ( GEBDATA ) || ( inFile->istream )->eof() )
    {
        if ( inFile->istream->eof() )
        {
            std::cerr << "\n\n\n";
            std::cerr << "The file " << inFile->fileName << " (#" << inFile->fileNum << ") do not have anymore data" << ( inFile->istream->gcount() == 0 ? "" : " after reading the header" ) << "...\n";
            std::cerr << "--------> Last timestamp was " << bufEvent->gd->timestamp << "\n";
        }

        printf ( "failed to read %lu bytes for header, got %i\n", sizeof ( GEBDATA ), ( inFile->istream )->gcount() );

        theMergeManager->RemoveFromInputList ( inFile->fileName );

        if ( inFile->istream->eof() )
        {
            std::cerr << "The file has been removed from the list of files to treat... (files remaining: " << theMergeManager->inData->size() << ")\n\n";
        }

        return nullptr;
    }

    if ( printDebug ) std::cerr << "Header looks fine... Filling some stats...\n";

    theMergeManager->readBytesCount += sizeof ( GEBDATA );

    nstat->inbytes += ( inFile->istream )->gcount();
    control.filesiz[storeNo] += ( inFile->istream )->gcount();

#endif

    if ( printDebug ) std::cerr << "Header stats filled...\n";

    if ( printInfo ) printf ( "\ngot initial header, TS=%lli for storeNo=%i\n", bufEvent->gd->timestamp, storeNo );

    /* attempt to read payload */

    i1 = bufEvent->gd->length;

//     bufEvent->payload = new char[i1];

#if (USEBREAD)

    siz = bread ( inData, ( char * ) bufEvent->payload, &bread_pos[storeNo], bread_buf[storeNo], &bread_bufsiz[storeNo] );

    if ( siz != i1 )
    {
        printf ( "failed to read %i bytes for payload, got %i\n", i1, siz );
        return nullptr;
    }

    nstat->inbytes += siz;
    control.filesiz[storeNo] += siz;
    control.fileEventsRead[storeNo]++;

#else

    if ( printDebug ) std::cerr << "Reading the payload...\n";

    ( inFile->istream )->read ( ( char * ) bufEvent->payload, i1 );

    if ( printDebug ) std::cerr << "Payload read... now checking it...\n";

    if ( ( inFile->istream )->gcount() != i1 || ( inFile->istream )->eof() )
    {
        if ( inFile->istream->eof() )
        {
            std::cout << "\n\n\n";
            std::cout << "The file " << inFile->fileName << " (#" << inFile->fileNum << ") do not have anymore data" << ( inFile->istream->gcount() == 0 ? "" : " after reading the payload" ) << "...\n";
            std::cout << "--------> Last timestamp was " << bufEvent->gd->timestamp << "\n";
        }

        printf ( "failed to read %i bytes for payload, got %i\n", i1, siz );
        theMergeManager->RemoveFromInputList ( inFile->fileName );

        if ( inFile->istream->eof() )
        {
            std::cout << "The file has been removed from the list of files to treat... (files remaining: " << theMergeManager->inData->size() << ")\n\n";
        }

        return nullptr;
    }

    if ( printDebug ) std::cerr << "Payload seems fine... now filling some stats...\n";

    theMergeManager->readBytesCount += i1;

    nstat->inbytes += ( inFile->istream )->gcount();
    control.filesiz[storeNo] += ( inFile->istream )->gcount();
    control.fileEventsRead[storeNo]++;

    if ( printDebug ) std::cerr << "Payload stats filled...\n";

#endif

    if ( printInfo ) printf ( "read initial payload of siz=%i into  storeNo=%i\n", siz, storeNo );

    /* done */

    if ( printDebug ) std::cerr << "Done with GTGetDiskEv... Exiting...\n";

    return bufEvent;
}

int main ( int argc, char **argv )
{
//     std::cerr << "Entering main function of GEBMerge.cxx...\n";

    char esc ( 27 );

    struct sysinfo memInfo;

    /* declarations */

    MergeManager* theMergeManager = MergeManager::sinstance();
    std::vector<InDataInfo*>* inData = theMergeManager->inData;
    std::ofstream* outData = &theMergeManager->outData;

#if(USEZLIB==1)

    gzFile* zFile = theMergeManager->zFile;

#endif

    gzFile zoutData;
    int maxNoEvents = 0, nPoolEvents = 0;
    FILE *fp;
    int i1, i2;
    int st, nn, argcoffset = 0;
    float ehigain[NGE + 1], ehioffset[NGE + 1], r1;
    char str[STRLEN], str1[512];
    unsigned int seed = 0;
    struct tms timesThen;
    int i7, i8, reportinterval;
    int echo = 0, nni, nret;
    int size;
    int wosize = 0;
    int nprint;

    /* help */

    if ( argc == 1 )
    {
        printf ( "use: GEBMerge chatfile outfile     file1  file2  file3  file4 .....\n" );
        printf ( "eg., GEBMerge gtmerge.chat c.gtd   t1.gtd t2.gtd t3.gtd t4.gtd\n" );
        exit ( 0 );
    };


    /* initialize random number generator etc */

    get_a_seed ( &seed );
    srand ( seed );
    nstat = ( MSTAT * ) calloc ( 1, sizeof ( MSTAT ) );
    bzero ( ( char * ) &control, sizeof ( CONTROL ) );
    bzero ( ( char * ) nstat, sizeof ( MSTAT ) );

    control.dtsfabort = 5;
    control.dtsbabort = 5;

    for ( int i = 0; i < NCHANNELS; i++ )
        nstat->id_hit[i] = 0;

    for ( int i = 0; i <= NGE; i++ )
    {
        ehigain[i] = 1;
        ehioffset[i] = 0;
    };

    /* open chat file */

    std::ifstream chatFile ( argv[1] );

    if ( !chatFile.is_open() )
    {
        printf ( "error: could not open chat file: <%s>\n", argv[1] );
        exit ( 0 );
    }

    printf ( "chat file: <%s> open\n", argv[1] );
    printf ( "\n" );
    fflush ( stdout );

    if ( printDebug ) std::cerr << "Reading chatfile...\n";

    /* read chatfile content and act */

    string readLine;

    nn = 0;
    while ( std::getline ( chatFile, readLine ) )
    {
        if ( readLine.empty() ) continue;

        if ( echo )
            printf ( "chat->%s", readLine.c_str() );
        fflush ( stdout );

        /* attemp to interpret the line */

        if ( readLine.find ( "echo" ) != string::npos )
        {
            echo = 1;
            if ( echo )
                printf ( "will echo command lines\n" );
            fflush ( stdout );

        }
        else if ( readLine[0] == 35 )
        {
            /* '#' comment line, do nothing */
            nni--;                /* don't count as instruction */

        }
        else if ( readLine[0] == 59 )
        {
            /* ';' comment line, do nothing */
            nni--;                /* don't count as instruction */

        }
        else if ( readLine[0] == 10 )
        {
            /* empty line, do nothing */
            nni--;                /* don't count as instruction */

        }
        else if ( readLine.find ( "maxNoEvents" ) != string::npos )
        {
            nret = sscanf ( readLine.c_str(), "%s %i", str1, &maxNoEvents );
            CheckNoArgs ( nret, 2, readLine );
        }
        else if ( readLine.find ( "reportinterval" ) != string::npos )
        {
            nret = sscanf ( readLine.c_str(), "%s %i", str1, &reportinterval );
            CheckNoArgs ( nret, 2, readLine );
        }
        else if ( readLine.find ( "chunksiz" ) != string::npos )
        {
            nret = sscanf ( readLine.c_str(), "%s %i", str1, &control.chunksiz );
            CheckNoArgs ( nret, 2, readLine );
        }
        else if ( readLine.find ( "bigbufsize" ) != string::npos )
        {
            nret = sscanf ( readLine.c_str(), "%s %i", str1, &size );
            CheckNoArgs ( nret, 2, readLine );
            assert ( size <= MAXBIGBUFSIZ );
            r1 = ( size * sizeof ( EVENT ) + ( size + 1 ) * sizeof ( int ) ) / 1024.0 / 1024.0;
            printf ( "sizeof(EVENT)= %lu\n", sizeof ( EVENT ) );
            printf ( "will use a bigbuffer size of %i, or %7.3f MBytes\n", size, r1 );
        }
        else if ( readLine.find ( "nprint" ) != string::npos )
        {
            nret = sscanf ( readLine.c_str(), "%s %i", str1, &nprint );
            CheckNoArgs ( nret, 2, readLine );
            printf ( "will print information for first %i events\n", nprint );
        }
        else if ( readLine.find ( "wosize" ) != string::npos )
        {
            nret = sscanf ( readLine.c_str(), "%s %f", str1, &r1 );
            r1 = ( r1 / 100.0 * size );
            wosize = ( int ) r1;
            CheckNoArgs ( nret, 2, readLine );
            printf ( "will use a bigbuffer wosize of %i\n", wosize );
            assert ( wosize <= size );
        }
        else if ( readLine.find ( "startTS" ) != string::npos )
        {
            nret = sscanf ( readLine.c_str(), "%s %llu %llu ", str1, &control.startTS_lo, &control.startTS_hi );
            CheckNoArgs ( nret, 3, readLine );
            printf ( "startTS from %lli to %lli\n", control.startTS_lo, control.startTS_hi );
            control.startTS = 1;
        }
        else if ( readLine.find ( "zzipout" ) != string::npos )
        {
            nret = sscanf ( readLine.c_str(), "%s", str1 );
            CheckNoArgs ( nret, 1, readLine );
            control.zzipout = 1;
            printf ( "will zzip output, %i\n", control.zzipout );
            fflush ( stdout );

        }
        else if ( readLine.find ( "TSlistelen" ) != string::npos )
        {
            nret = sscanf ( readLine.c_str(), "%s %i %i %i", str1, &control.TSlistelen, &control.TSlist_lo, &control.TSlist_hi );
            CheckNoArgs ( nret, 4, readLine );
        }
        else if ( readLine.find ( "dts_min" ) != string::npos )
        {
            nret = sscanf ( readLine.c_str(), "%s %lli", str1, &control.dts_min );
            CheckNoArgs ( nret, 2, readLine );
            printf ( "control.dts_min=%lli\n", control.dts_min );
        }
        else if ( readLine.find ( "dts_max" ) != string::npos )
        {
            nret = sscanf ( readLine.c_str(), "%s %lli", str1, &control.dts_max );
            CheckNoArgs ( nret, 2, readLine );
            printf ( "control.dts_max=%lli\n", control.dts_max );
        }
        else if ( readLine.find ( "dtsfabort" ) != string::npos )
        {
            nret = sscanf ( readLine.c_str(), "%s %i", str1, &control.dtsfabort );
            CheckNoArgs ( nret, 2, readLine );
        }
        else if ( readLine.find ( "dtsbabort" ) != string::npos )
        {
            nret = sscanf ( readLine.c_str(), "%s %i", str1, &control.dtsbabort );
            CheckNoArgs ( nret, 2, readLine );
        }
        else
        {
            /* --------------------------- */
            /* chatscript read error point */
            /* --------------------------- */

            printf ( "line %2.2i in chat script, option :%s \n__not understood\n", nn, readLine.c_str() );
            printf ( "%i\n", readLine[0] );
            printf ( "aborting\n" );
            fflush ( stdout );
            exit ( 0 );
        }

        /* read next line in chat script */

        nn++;                     /* line counter */
        nni++;                    /* instruction counter */
    }

    chatFile.close();

    /* extract and repeat parameters */

    printf ( "%s: will produce a max of %i events\n", argv[0], maxNoEvents );

    printf ( "%s: will write combined data to file \"%s\"\n", argv[0], argv[2] );

    /* offset for data file name reads, find  nfiles etc */

    argcoffset = 3;
    const int nfiles = ( argc - argcoffset );
    printf ( "%s: we have %i datafiles to combine\n", argv[0], nfiles );
    fflush ( stdout );

    nPoolEvents = nfiles;
    printf ( "%s: will keep a pool of %i events to combine from \n", argv[0], nPoolEvents );
    assert ( nPoolEvents < MAXCOINEV );

    /* just to be sure */

    assert ( nfiles == nPoolEvents );

    /************************/
    /* open all input files */
    /************************/

    std::ifstream ifstreamArray[nfiles];

    unsigned long long int totBytesCount = 0;

    for ( int i = 0; i < nfiles; i++ )
    {
        nn = i + argcoffset;

#if(USEZLIB==0)

        ifstreamArray[i].open ( argv[nn], std::ios_base::in );

        ifstreamArray[i].seekg ( 0, ifstreamArray[i].end );
        totBytesCount += ifstreamArray[i].tellg();
        ifstreamArray[i].seekg ( 0, ifstreamArray[i].beg );

        InDataInfo* newInDataInfo = new InDataInfo ( ifstreamArray[i] );

        if ( !newInDataInfo->istream->is_open() )
        {
            printf ( "could not open input data file [%i] %s, quit!\n", nn, argv[nn] );
            exit ( 1 );
        }

        newInDataInfo->fileName = argv[nn];
        newInDataInfo->fileNum = i;

        inData->push_back ( newInDataInfo );

//         std::cerr << "Pushed file " << theMergeManager->inData->at ( theMergeManager->inData->size()-1 ).fileName << " (file is " << ( theMergeManager->inData->at ( theMergeManager->inData->size()-1 ).istream->is_open() ? "open" : "NOT open!" ) << ")\n";

#else

        zFile[i] = gzdopen ( argv[nn], "r" );

        if ( zFile[i] == NULL )
        {
            printf ( "could not open input data file [%i] %s, quit!\n", nn, argv[nn] );
            exit ( 1 );
        }

#endif

        printf ( "%s: input data file \"%s\", number %i, is open\n", argv[0], argv[nn], i );
        fflush ( stdout );
        control.nOpenFiles++;
        control.fileActive[i] = 1;
    }

    if ( printDebug )
    {
        for ( unsigned int j = 0; j < theMergeManager->inData->size(); j++ )
        {
            if ( !theMergeManager->inData->at ( j )->istream->is_open() )
            {
                std::cerr << theMergeManager->inData->at ( j )->fileName << " is NOT open!!\n";
            }
        }
    }

#if (USEBREAD)

    for ( int i = 0; i < nfiles; i++ )
    {
        bread_buf[i] = ( int * ) calloc ( BREAD_BUFSIZE, sizeof ( int ) );
        bread_pos[i] = 0;
        bread_bufsiz[i] = BREAD_BUFSIZE;
    }

#endif


    /* ----------- */
    /* output file */
    /* ----------- */

    /*                        + name of data file */
    /*                        |                   */
    char outName[512];
    sprintf ( outName, "%s_%3.3i", argv[2], control.chunkno );

    if ( control.zzipout == 0 )
    {
        outData->open ( outName, std::ios_base::out | std::ios_base::binary );

        if ( !outData->is_open() )
        {
            printf ( "could not open output data file %s, quit!\n", outName );
            exit ( 1 );
        }
    }
    else
    {
        int gzdfd = open ( outName, O_WRONLY | O_CREAT | O_TRUNC, PMODE );
        zoutData = gzdopen ( gzdfd, "w" );

        if ( zoutData == NULL )
        {
            printf ( "could not open output data file %s, quit!\n", outName );
            exit ( 1 );
        }
    }

    printf ( "%s: output data file \"%s\" is open\n", argv[0], outName );
    fflush ( stdout );

// #if(0)
//     /* write output header file */
//
//     bzero ( ( char * ) &outheader, sizeof ( DGSHEADER ) );
//     outheader.id = 1000;
//     if ( control.zzipout == 0 )
//         siz = write ( outData, ( char * ) &outheader, sizeof ( DGSHEADER ) );
//     else
//         siz = gzwrite ( zoutData, ( char * ) &outheader, sizeof ( DGSHEADER ) );
//
//     printf ( "header written to output file\n" );
// #endif

    /* -------------------- */
    /* read in the map file */
    /* -------------------- */

    if ( printDebug ) std::cerr << "Reading or creating the map.dat...\n";

    for ( int i = 0; i < NCHANNELS; i++ )
    {
        theMergeManager->tlkup[i] = NOTHING;
        theMergeManager->tid[i] = NOTHING;
    }

    fp = fopen ( "map.dat", "r" );
    if ( fp == NULL )
    {
        printf ( "need a \"map.dat\" file to run\n" );
        int sysRet = system ( "./mkMap > map.dat" );
        if ( sysRet == -1 )
        {
            std::cerr << "ERROR WHILE MAKING map.dat !!!!!!!!!" << std::endl;
            return -1;
        }
        printf ( "just made you one...\n" );
        fp = fopen ( "map.dat", "r" );
        assert ( fp != NULL );
    };

    printf ( "\nmapping\n" );

    i2 = fscanf ( fp, "\n%i %i %i %s", &i1, &i7, &i8, str );
    printf ( "Successfully read %i items: %i %i %i %s\n", i2, i1, i7, i8, str );
    while ( i2 == 4 )
    {
        theMergeManager->tlkup[i1] = i7;
        theMergeManager->tid[i1] = i8;
        i2 = fscanf ( fp, "\n%i %i %i %s", &i1, &i7, &i8, str );
        if ( i2 == 4 ) printf ( "Successfully read %i items: %i %i %i %s\n", i2, i1, i7, i8, str );
    };
    fclose ( fp );


    /* start timer */

    times ( ( struct tms * ) &timesThen );

    /* -------------------------------------------- */
    /* read until we have filled our pool of events */
    /* -------------------------------------------- */

    if ( printDebug ) std::cerr << "Starting the merge procedure... (theMergeManager->inData size is " << theMergeManager->inData->size() << ")\n";

    bool firstExec = true;

    unsigned long long int loopCounter = 0, evCounter = 0;

    unsigned int maxEventMapSize = 200;

    EVENT* EventsCA[maxEventMapSize+1];

    std::vector<unsigned int> unusedEventsCAKeys;
    unusedEventsCAKeys.clear();

    for ( int i = 0; i < maxEventMapSize+1; i++ )
    {
        EventsCA[i] = new EVENT();
        EventsCA[i]->key = i;

        unusedEventsCAKeys.push_back ( i );
    }

    EVENT* ofEventsCA[nfiles];

    std::pair<InDataInfo*, EVENT*>* newOfEv[nfiles];

    for ( int i = 0; i < nfiles; i++ )
    {
        ofEventsCA[i] = new EVENT();

        for ( int j = 0; j < maxEventMapSize+1; j++ )
        {
            if ( ofEventsCA[i] == EventsCA[j] ) std::cerr << "WARNING!!!! The Buffer Event #" << j << " and the Overflow Event #" << i << " have the same address somehow!!!!!!!!!!!!!!\n";
            if ( ofEventsCA[i]->gd == EventsCA[j]->gd ) std::cerr << "WARNING!!!! The headers for Buffer Event #" << j << " and the Overflow Event #" << i << " have the same address somehow!!!!!!!!!!!!!!\n";
            if ( ofEventsCA[i]->payload == EventsCA[j]->payload ) std::cerr << "WARNING!!!! The payloads for Buffer Event #" << j << " and the Overflow Event #" << i << " have the same address somehow!!!!!!!!!!!!!!\n";
        }

        newOfEv[i] = new std::pair<InDataInfo*, EVENT*>;
    }

    unsigned long long int dTWarning = 1e9;

    theMergeManager->readBytesCount = 0;

    unsigned long long int ignoredBytesCount = 0;

    std::map<unsigned long long int, unsigned long long int> tsOutputPosMap;

    unsigned long long int prevBytesDiff = 0;

    while ( theMergeManager->inData->size() > 0  || theMergeManager->overflowEvent.size() > 0 )
    {
//         if ( loopCounter > 1300000 ) printDebug = true;

        if ( printProgress && ( loopCounter%10000 == 0 || loopCounter == 1 ) )
        {
            auto ofInfo = theMergeManager->GetSizeAndBytesCount ( true );
            unsigned int ofEvSize = ofInfo.first;
            unsigned long long int ofBytesCount = ofInfo.second;

            auto writeBufInfo = theMergeManager->GetSizeAndBytesCount ( false );
            unsigned int writeBufSize = writeBufInfo.first;
            unsigned long long int bufferBytesCount = writeBufInfo.second;

            sysinfo ( &memInfo );

            long long totalPhysMem = memInfo.totalram;
            totalPhysMem *= memInfo.mem_unit;

            long long physMemUsed = memInfo.totalram - memInfo.freeram;
            physMemUsed *= memInfo.mem_unit;

            unsigned long long int outSize = outData->tellp();

            long long int bytesDiff = ( long long int ) ( theMergeManager->readBytesCount - outSize - bufferBytesCount - ofBytesCount - ignoredBytesCount );

            if ( bytesDiff != 0 && prevBytesDiff != bytesDiff )
            {
                std::cerr << "\n\n/!\\WARNING/!\\ Amount of bytes read differs from amount of bytes treated !!! (diff = " << bytesDiff << ")\n\n";

                prevBytesDiff = bytesDiff;
            }

            std::cerr << "Loop #" << loopCounter << ": " << theMergeManager->readBytesCount << " bytes read (" <<  ofBytesCount << " + " << bufferBytesCount << " in buffer / " << ignoredBytesCount << " ignored) out of " << totBytesCount;
            std::cerr << " ( " << std::setprecision ( 4 ) << ( float ) theMergeManager->readBytesCount/totBytesCount * 100. << "% )";
            std::cerr << "... Output file size: " << outSize << " bytes (diff = " << bytesDiff << " bytes)...\n";
            std::cerr<< evCounter << " events treated / " << writeBufSize << " events currently waiting in the write buffer / ";
            std::cerr << ofEvSize << " awaiting treatment for " << theMergeManager->overflowEvent.size() << " map entries / ";
            std::cerr << theMergeManager->inData->size() << " files left in the queue\n";
            std::cerr << "Memory used: " << physMemUsed/1000000 << " MB (" << totalPhysMem/1000000 << "MB total)" << esc << "[1A" << esc << "[1A" << "\r" << std::flush;
        }

        if ( printDebug )
        {
            for ( int i = 0; i < nfiles; i++ )
            {
                for ( int j = 0; j < maxEventMapSize+1; j++ )
                {
                    if ( ofEventsCA[i] == EventsCA[j] ) std::cerr << std::flush << "WARNING!!!! The Buffer Event #" << j << " and the Overflow Event #" << i << " have the same address somehow!!!!!!!!!!!!!!\n";
                    if ( ofEventsCA[i]->gd == EventsCA[j]->gd ) std::cerr << std::flush << "WARNING!!!! The headers for Buffer Event #" << j << " and the Overflow Event #" << i << " have the same address somehow!!!!!!!!!!!!!!\n";
                    if ( ofEventsCA[i]->payload == EventsCA[j]->payload ) std::cerr << std::flush << "WARNING!!!! The payloads for Buffer Event #" << j << " and the Overflow Event #" << i << " have the same address somehow!!!!!!!!!!!!!!\n";
                }
            }

//             std::cerr << "About to clear theMergeManager->Event... Size of theMergeManager->Event is " << theMergeManager->Event.size() << "\n";
        }

//         theMergeManager->Event.clear();

        if ( printDebug )
        {
            std::cerr << "Size of theMergeManager->overflowEvent is " << theMergeManager->overflowEvent.size() << "\n";
            std::cerr << "-*-*-*-*-*--*-*-*-*-*--*-*-*-*-*--*-*-*-*-*--*-*-*-*-*--*-*-*-*-*--*-*-*-*-*--*-*-*-*-*--*-*-*-*-*-\n";
            std::cerr << "-*-*-*-*-*--*-*-*-*-*-  LOOP # " << loopCounter << " / EVENT #" << evCounter << " -*-*-*-*-*--*-*-*-*-*--*-*-*-*-*-\n";
            std::cerr << "-*-*-*-*-*--*-*-*-*-*--*-*-*-*-*--*-*-*-*-*--*-*-*-*-*--*-*-*-*-*--*-*-*-*-*--*-*-*-*-*--*-*-*-*-*-\n";
        }

        if ( theMergeManager->overflowEvent.size()  == 0 )
        {
            if ( theMergeManager->inData->size() == 0 )
            {
                if ( printDebug ) std::cerr << "No more file to treat and event buffer is empty... Terminating GEBMerge...\n";

                break;
            }

            if ( printDebug ) std::cerr << "-*-*-*-*-*- Fill the first pool of events -*-*-*-*-*-*-\n";

            for ( unsigned int i = 0; i < theMergeManager->inData->size(); i++ )
            {
                if ( printDebug ) std::cerr << "------------ (Overflow Event) Entering GTGetDiskEv for " << theMergeManager->inData->at ( i )->fileName << " (file # " << i << ")\n";

                int fnum = theMergeManager->inData->at ( i )->fileNum;

                EVENT* newEv = GTGetDiskEv ( theMergeManager->inData->at ( i ), ofEventsCA[fnum], false );

                if ( newEv != NULL )
                {
                    if ( firstExec )
                    {
                        firstExec = false;

                        theMergeManager->inData->at ( i )->firstTimestamp = newEv->gd->timestamp;
                    }

                    newOfEv[fnum]->first = theMergeManager->inData->at ( i );
                    newOfEv[fnum]->second = newEv;

                    if ( theMergeManager->overflowEvent.find ( newEv->gd->timestamp ) == theMergeManager->overflowEvent.end() )
                    {
                        std::vector<std::pair<InDataInfo*, EVENT*>*>* ofEvMapEntry = new std::vector<std::pair<InDataInfo*, EVENT*>*>;
                        ofEvMapEntry->clear();

                        ofEvMapEntry->push_back ( newOfEv[fnum] );
                        theMergeManager->overflowEvent[newEv->gd->timestamp] = ofEvMapEntry;
                    }
                    else
                    {
                        if ( printDebug ) std::cerr << "!!!! Timestamp already present in the map. Pushing it back as a vector...\n";

                        theMergeManager->overflowEvent[newEv->gd->timestamp]->push_back ( newOfEv[fnum] );
                    }
                }
            }

            if ( printDebug ) std::cerr << "++++++++++++++++++++ First pool filled! +++++++++++++++++++++++\n";
        }

        if ( printBytesCount )
        {
            std::cerr << ".......................... Loop #" << loopCounter << " ...........................\n";
            std::cerr << "Loop Entrance...";
            std::cerr << "\nBytes read: " << theMergeManager->readBytesCount << " / Buffer byte count: " << theMergeManager->GetSizeAndBytesCount ( true ).second;
            std::cerr << " + " << theMergeManager->GetSizeAndBytesCount ( false ).second;
            std::cerr << " / Bytes written: " << outData->tellp() << " / Bytes ignored: " << ignoredBytesCount;
            std::cerr << " (diff = " << ( long long int ) ( theMergeManager->readBytesCount - theMergeManager->GetSizeAndBytesCount ( true ).second  - theMergeManager->GetSizeAndBytesCount ( false ).second - outData->tellp() - ignoredBytesCount ) << ")\n";
        }

        if ( printDebug ) std::cerr << "size of theMergeManager->inData is " << theMergeManager->inData->size() << "\n";

        unsigned int bufEvtSize = 0;

        for ( auto bufEvtItr = theMergeManager->Event.begin(); bufEvtItr != theMergeManager->Event.end(); bufEvtItr++ )
        {
            bufEvtSize += bufEvtItr->second->size();
        }

        if ( printDebug )
        {
            int ofEvSize = 0;

            for ( auto ofItr = theMergeManager->overflowEvent.begin(); ofItr != theMergeManager->overflowEvent.end(); ofItr++ )
            {
                ofEvSize += ofItr->second->size();
            }

            std::cerr << "size of theMergeManager->overflowEvent is " << theMergeManager->overflowEvent.size() << " (" << ofEvSize << " elements)\n";

            std::cerr << "size of theMergeManager->Event is " << theMergeManager->Event.size() << " (" << bufEvtSize << " elements / " << unusedEventsCAKeys.size() << " unused keys)\n";
        }

        auto itr = theMergeManager->overflowEvent.begin();

        auto nextItr = itr;
        nextItr++;

        unsigned long long int longestTs = -1;

        unsigned long long int ts, nextTs;

        ts = itr->first;
        nextTs = ( nextItr != theMergeManager->overflowEvent.end() ) ? nextItr->first : longestTs;

        bool forceWrite = false;

        if ( itr->second->size() > 1 )
        {
            if ( bufEvtSize+itr->second->size() > maxEventMapSize )
            {
                forceWrite = true;
            }
            else
            {
                if ( printDebug )
                {
                    std::cerr << "!!!!!!!!!!!!!!!!!!!!!! Encountered a timestamp entry with multiple events... ( TS = " << ts << ")\n";
                    std::cerr << "!!!!!!!!!!!!!!!!!!!!!! Multiplicity is " << itr->second->size() << "\n";
                }

                for ( unsigned m = 0; m < itr->second->size(); m++ )
                {
                    unsigned int evtCAKey = * ( unusedEventsCAKeys.begin() );
                    unusedEventsCAKeys.erase ( unusedEventsCAKeys.begin() );

                    InDataInfo* input = itr->second->at ( m )->first;
                    * ( EventsCA[evtCAKey]->gd ) = * ( itr->second->at ( m )->second->gd );
                    memcpy ( EventsCA[evtCAKey]->payload, itr->second->at ( m )->second->payload, EventsCA[evtCAKey]->gd->length );


                    int fnum = input->fileNum ;

                    if ( printDebug ) std::cerr << "Copying entry from file #" << fnum << " to the list of events to be written on file: TS = " << EventsCA[evtCAKey]->gd->timestamp << ")\n";

                    if ( theMergeManager->Event.find ( EventsCA[evtCAKey]->gd->timestamp ) == theMergeManager->Event.end() )
                    {
                        std::vector<EVENT*>* evMapEntry = new std::vector<EVENT*>;
                        evMapEntry->clear();

                        evMapEntry->push_back ( EventsCA[evtCAKey] );
                        theMergeManager->Event[EventsCA[evtCAKey]->gd->timestamp] = evMapEntry;
                    }
                    else
                    {
                        if ( printDebug ) std::cerr << "!!!! Timestamp already present in the map. Pushing it back as a vector...\n";

                        theMergeManager->Event[EventsCA[evtCAKey]->gd->timestamp]->push_back ( EventsCA[evtCAKey] );
                    }

                    if ( printDebug )
                    {
                        std::cerr << "------------ (Overflow Event) Entering GTGetDiskEv for " << input->fileName << " (file # " << input->fileNum << "\n";
                    }

                    EVENT* newEv = GTGetDiskEv ( input, ofEventsCA[input->fileNum], false );

                    if ( EventsCA[evtCAKey] == ofEventsCA[input->fileNum] ) std::cerr << "WARNING!!!! The Buffer Event #" << m << " and the Overflow Event #" << input->fileNum << " have the same address somehow!!!!!!!!!!!!!!\n";

                    auto lastEvtItr = theMergeManager->Event.end();
                    lastEvtItr--;

                    if ( newEv != NULL )
                    {
                        if ( ( newEv->gd->timestamp > lastEvtItr->first ) && ( newEv->gd->timestamp - lastEvtItr->first > dTWarning ) )
                        {
                            std::cerr << "\n\n\n\nWeird Timestamp at loop #" << loopCounter << " / event #" << evCounter << " (file: " << input->fileName << ") => Previous: " << lastEvtItr->first << " --- Current: " << newEv->gd->timestamp << " ...\n";

                            ignoredBytesCount += sizeof ( GEBDATA ) + newEv->gd->length;

                            newEv = GTGetDiskEv ( input, ofEventsCA[input->fileNum], false );

                            std::cerr << "Ignored this event... Next event timestamp: " << newEv->gd->timestamp << "...\n";

                            if ( newEv->gd->timestamp - lastEvtItr->first > dTWarning )
                            {
                                std::cerr << "The timestamp difference is still pretty high... You might want to look into the raw files...\n";
                            }

                            std::cerr << "\n";
                        }

                        newOfEv[fnum]->first = input;
                        newOfEv[fnum]->second = newEv;

                        if ( theMergeManager->overflowEvent.find ( newEv->gd->timestamp ) == theMergeManager->overflowEvent.end() )
                        {
                            std::vector<std::pair<InDataInfo*, EVENT*>*>* ofEvMapEntry = new std::vector<std::pair<InDataInfo*, EVENT*>*>;
                            ofEvMapEntry->clear();

                            ofEvMapEntry->push_back ( newOfEv[fnum] );
                            theMergeManager->overflowEvent[newEv->gd->timestamp] = ofEvMapEntry;
                        }
                        else
                        {
                            if ( printDebug ) std::cerr << "!!!! Timestamp already present in the map. Pushing it back as a vector...\n";
                            theMergeManager->overflowEvent[newEv->gd->timestamp]->push_back ( newOfEv[fnum] );
                        }
                    }
                }

                itr->second->clear();
                delete itr->second;
                theMergeManager->overflowEvent.erase ( itr );
            }
        }

        else if ( itr->second->size() == 1 )
        {
            unsigned int evtCAKey = * ( unusedEventsCAKeys.begin() );
            unusedEventsCAKeys.erase ( unusedEventsCAKeys.begin() );

            InDataInfo* input = itr->second->at ( 0 )->first;
            * ( EventsCA[evtCAKey]->gd ) = * ( itr->second->at ( 0 )->second->gd );
            memcpy ( EventsCA[evtCAKey]->payload, itr->second->at ( 0 )->second->payload, EventsCA[evtCAKey]->gd->length );

            int fnum = input->fileNum;

            if ( printDebug ) std::cerr << "Copying entry from file #" << fnum << " to the list of events to be written on file: TS = " << EventsCA[evtCAKey]->gd->timestamp << ")\n";

            if ( theMergeManager->Event.find ( ts ) == theMergeManager->Event.end() )
            {
                std::vector<EVENT*>* evMapEntry = new std::vector<EVENT*>;
                evMapEntry->clear();

                evMapEntry->push_back ( EventsCA[evtCAKey] );
                theMergeManager->Event[ts] = evMapEntry;
            }
            else
            {
                if ( printDebug ) std::cerr << "!!!! Timestamp already present in the map. Pushing it back as a vector...\n";
                theMergeManager->Event[ts]->push_back ( EventsCA[evtCAKey] );
            }

            int iteration = bufEvtSize+1;

            while ( ts < nextTs && iteration <= maxEventMapSize )
            {
                if ( printDebug ) std::cerr << "x"<< iteration << "------------ (Buffered Event) Entering GTGetDiskEv for " << input->fileName << " (file # " << input->fileNum << " / Event buffer size = " << theMergeManager->Event.size() << ")\n";

                auto lastEvtItr = theMergeManager->Event.end();
                lastEvtItr--;

                evtCAKey = * ( unusedEventsCAKeys.begin() );

                EVENT* newEv = GTGetDiskEv ( input, EventsCA[evtCAKey], false );

                if ( newEv != NULL )
                {
                    if ( ( newEv->gd->timestamp > lastEvtItr->first ) && ( newEv->gd->timestamp - lastEvtItr->first > dTWarning ) )
                    {
                        std::cerr << "\n\n\n\nWeird Timestamp at loop #" << loopCounter << " / event #" << evCounter << " (file " << input->fileName << ") => Previous: " << lastEvtItr->first << " --- Current: " << newEv->gd->timestamp << " ...\n";

                        ignoredBytesCount += sizeof ( GEBDATA ) + newEv->gd->length;

                        newEv = GTGetDiskEv ( input, EventsCA[evtCAKey], false );

                        std::cerr << "Ignored this event... Next event timestamp: " << newEv->gd->timestamp << "...\n";

                        if ( newEv->gd->timestamp - lastEvtItr->first > dTWarning )
                        {
                            std::cerr << "The timestamp difference is still pretty high... You might want to look into the raw files...\n";
                        }

                        std::cerr << "\n";
                    }

                    ts = newEv->gd->timestamp;

                    if ( ts < nextTs && iteration < maxEventMapSize )
                    {
                        unusedEventsCAKeys.erase ( unusedEventsCAKeys.begin() );

                        if ( printDebug )
                        {
                            std::cerr << "-*\\_/*-> Timestamp is smaller than the one from the next file (file #";
                            std::cerr << ( ( nextItr != theMergeManager->overflowEvent.end() ) ? nextItr->second->at ( 0 )->first->fileNum : -1 ) << "): " << ts << " < " << nextTs << ")\n";
                        }

                        if ( theMergeManager->Event.find ( newEv->gd->timestamp ) == theMergeManager->Event.end() )
                        {
                            std::vector<EVENT*>* evMapEntry = new std::vector<EVENT*>;
                            evMapEntry->clear();

                            evMapEntry->push_back ( newEv );
                            theMergeManager->Event[newEv->gd->timestamp] = evMapEntry;
                        }
                        else
                        {
                            if ( printDebug ) std::cerr << "!!!! Timestamp already present in the map. Pushing it back as a vector...\n";
                            theMergeManager->Event[newEv->gd->timestamp]->push_back ( newEv );
                        }
                    }
                    else
                    {
                        if ( printDebug )
                        {
                            std::cerr << "-*\\_/*-> Timestamp is bigger than the one from the next file #";
                            std::cerr << ( ( nextItr != theMergeManager->overflowEvent.end() ) ? nextItr->second->at ( 0 )->first->fileNum : -1 ) << "): " << ts << " > " << nextTs << "\n";
                        }

                        itr->second->clear();
                        delete itr->second;
                        theMergeManager->overflowEvent.erase ( itr );

                        * ( ofEventsCA[input->fileNum]->gd ) = * ( newEv->gd );
                        memcpy ( ofEventsCA[input->fileNum]->payload, newEv->payload, newEv->gd->length );

                        newOfEv[fnum]->first = input;
                        newOfEv[fnum]->second = ofEventsCA[input->fileNum];

                        if ( theMergeManager->overflowEvent.find ( newEv->gd->timestamp ) == theMergeManager->overflowEvent.end() )
                        {
                            std::vector<std::pair<InDataInfo*, EVENT*>*>* ofEvMapEntry = new std::vector<std::pair<InDataInfo*, EVENT*>*>;
                            ofEvMapEntry->clear();

                            ofEvMapEntry->push_back ( newOfEv[fnum] );
                            theMergeManager->overflowEvent[newEv->gd->timestamp] = ofEvMapEntry;
                        }
                        else
                        {
                            if ( printDebug ) std::cerr << "!!!! Timestamp already present in the map. Pushing it back as a vector...\n";

                            theMergeManager->overflowEvent[newEv->gd->timestamp]->push_back ( newOfEv[fnum] );
                        }
                    }

                    iteration++;
                }
                else
                {
                    itr->second->clear();
                    delete itr->second;
                    theMergeManager->overflowEvent.erase ( itr );
                    break;
                }
            }
        }

        if ( printBytesCount )
        {
            std::cerr << "\nTreatment done...";
            std::cerr << "\nBytes read: " << theMergeManager->readBytesCount << " / Buffer byte count: " << theMergeManager->GetSizeAndBytesCount ( true ).second;
            std::cerr << " + " << theMergeManager->GetSizeAndBytesCount ( false ).second;
            std::cerr << " / Bytes written: " << outData->tellp();
            std::cerr << " / Events to write: " << theMergeManager->GetSizeAndBytesCount ( false ).second << " / Bytes ignored: " << ignoredBytesCount;
            std::cerr << " (diff = " << ( long long int ) ( theMergeManager->readBytesCount - theMergeManager->GetSizeAndBytesCount ( true ).second  - outData->tellp() - theMergeManager->GetSizeAndBytesCount ( false ).second - ignoredBytesCount ) << ")\n";
        }

//         if ( theMergeManager->Event.size() > maxEventMapSize/2 )
//         if ( bufEvtSize > maxEventMapSize/2 )
        if ( bufEvtSize > maxEventMapSize/2 || forceWrite )
        {
            if ( printDebug )
            {
                int evSize = 0;

                for ( auto ofItr = theMergeManager->Event.begin(); ofItr != theMergeManager->Event.end(); ofItr++ )
                {
                    evSize += ofItr->second->size();
                }

                std::cerr << "Writing buffered events to file... (" << theMergeManager->Event.size() << " elements / " << evSize << " events)\n";
            }

            int nEvtToWrite = std::min ( maxEventMapSize/3, bufEvtSize );

//             for ( unsigned int l = 0; l < maxEventMapSize/4; l++ )
//             {
//                 auto readItr = theMergeManager->Event.begin();
//
//                 for ( unsigned m = 0; m < readItr->second->size(); m++ )
//                 {
//                     evCounter++;
//
//                     unsigned long long int evtTs = readItr->second->at ( m )->gd->timestamp;
//                     int evtLength = readItr->second->at ( m )->gd->length;
//
//                     theMergeManager->outData.write ( ( char* ) readItr->second->at ( m )->gd, sizeof ( GEBDATA ) );
//                     theMergeManager->outData.write ( ( char* ) readItr->second->at ( m )->payload, evtLength );
//
//                     unusedEventsCAKeys.push_back ( readItr->second->at ( m )->key );
//                 }
//
//                 theMergeManager->Event.erase ( readItr );
//             }

//             while(theMergeManager->Event.size() > maxEventMapSize/3)
//             {
//                 auto readItr = theMergeManager->Event.begin();
//
//                 for ( unsigned m = 0; m < readItr->second->size(); m++ )
//                 {
//                     evCounter++;
//
//                     unsigned long long int evtTs = readItr->second->at ( m )->gd->timestamp;
//                     int evtLength = readItr->second->at ( m )->gd->length;
//
//                     theMergeManager->outData.write ( ( char* ) readItr->second->at ( m )->gd, sizeof ( GEBDATA ) );
//                     theMergeManager->outData.write ( ( char* ) readItr->second->at ( m )->payload, evtLength );

//                     unusedEventsCAKeys.push_back ( readItr->second->at ( m )->key );
//                 }
//
//                 theMergeManager->Event.erase ( readItr );
//             }

            while ( nEvtToWrite > 0 )
            {
                auto readItr = theMergeManager->Event.begin();

                for ( unsigned m = 0; m < readItr->second->size(); m++ )
                {
                    evCounter++;

                    unsigned long long int evtTs = readItr->second->at ( m )->gd->timestamp;
                    int evtLength = readItr->second->at ( m )->gd->length;

                    theMergeManager->outData.write ( ( char* ) readItr->second->at ( m )->gd, sizeof ( GEBDATA ) );
                    theMergeManager->outData.write ( ( char* ) readItr->second->at ( m )->payload, evtLength );

                    unusedEventsCAKeys.push_back ( readItr->second->at ( m )->key );

                    nEvtToWrite--;
                }

                theMergeManager->Event.erase ( readItr );
            }

            if ( printDebug ) std::cerr << "Done writing the buffered events to file...\n";

            if ( printBytesCount )
            {
                std::cerr << "\nWriting Step Done...";
                std::cerr << "\nBytes read: " << theMergeManager->readBytesCount << " / Buffer byte count: " << theMergeManager->GetSizeAndBytesCount ( true ).second;
                std::cerr << " + " << theMergeManager->GetSizeAndBytesCount ( false ).second;
                std::cerr << " / Bytes written: " << outData->tellp() << " / Bytes ignored: " << ignoredBytesCount;
                std::cerr << " (diff = " << ( long long int ) ( theMergeManager->readBytesCount - theMergeManager->GetSizeAndBytesCount ( true ).second  - theMergeManager->GetSizeAndBytesCount ( false ).second - outData->tellp() - ignoredBytesCount ) << ")\n";
            }

//             for ( auto evtItr = theMergeManager->Event.begin(); evtItr != theMergeManager->Event.end(); evtItr++ )
//             {
//                 unsigned int itrNum = std::distance ( theMergeManager->Event.begin(), evtItr );
//
//                 for ( unsigned m = 0; m < evtItr->second->size(); m++ )
//                 {
//                     * ( EventsCA[itrNum+m]->gd ) = * ( evtItr->second->at ( m )->gd );
//                     memcpy ( EventsCA[itrNum+m]->payload, evtItr->second->at ( m )->payload, evtItr->second->at ( m )->gd->length );
//
//                     evtItr->second->at ( m ) = EventsCA[itrNum+m];
//                 }
//             }
//
//             if ( printDebug ) std::cerr << "Reorganized the buffered events... Will start a new loop...\n";

//             if ( printBytesCount )
//             {
//                 std::cerr << "\Reorganization Step Done...";
//                 std::cerr << "\nBytes read: " << theMergeManager->readBytesCount << " / Buffer byte count: " << theMergeManager->GetSizeAndBytesCount ( true ).second;
//                 std::cerr << " + " << theMergeManager->GetSizeAndBytesCount ( false ).second;
//                 std::cerr << " / Bytes written: " << outData->tellp() << " / Bytes ignored: " << ignoredBytesCount;
//                 std::cerr << " (diff = " << ( long long int ) ( theMergeManager->readBytesCount - theMergeManager->GetSizeAndBytesCount ( true ).second  - theMergeManager->GetSizeAndBytesCount ( false ).second - outData->tellp() - ignoredBytesCount ) << ")\n";
//             }
        }

        else if ( theMergeManager->Event.size() == 0 )
        {
            break;
        }

        loopCounter++;
    }

    theMergeManager->outData.close();

    std::cerr << "\n\n\n\nDone Merging the files... Read a total of " << theMergeManager->readBytesCount << " out of " << totBytesCount << " bytes...\n\n";

    return 0;

}














