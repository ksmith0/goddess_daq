#include "GoddessAnalysis.h"
#include <sys/times.h>

using std::string;

// --------- Initialization and Utilities. Should not be touched -------------------------- //


GoddessAnalysis* gA = 0;

TChain* uChain = 0;

std::map<string, TObject*> histsMap;

unsigned long long int eventNbr = 0;
unsigned long long int totEvents = 0;

void LoadTrees()
{
    std::cout << "To initialize the chain of runs, type:\n   LoadTrees( (string) treeName, (string) fileName1, (string) fileName2, (string) ... )\n\n";
}

template<typename First, typename... Rest> void LoadTrees ( string treeName, First fileName1, Rest... fileNameRest )
{
    gA = new GoddessAnalysis();

    gA->InitUserAnalysis ( treeName, fileName1, fileNameRest... );

    uChain = gA->userChain;
    totEvents = uChain->GetEntries();
}

void PrintProgress ( unsigned long long int maxEvents_ )
{
    if ( eventNbr % 10000 == 0 )
    {
        std::cout << "Treated " << std::setw ( 11 ) << eventNbr << " / " << std::setw ( 11 ) << std::left << maxEvents_;
        std::cout << " ( " << std::setw ( 5 ) << std::setprecision ( 2 ) << std::fixed << ( float ) eventNbr/maxEvents_ * 100. << " % )\r" << std::flush;
    }
}

TH1F* MakeNewHist ( string name, string title, unsigned int nBinsX, int minX, int maxX )
{
    TH1F* newHist = new TH1F ( name.c_str(), title.c_str(), nBinsX, minX, maxX );
    histsMap[title] = newHist;

    return newHist;
}

TH2F* MakeNewHist ( string name, string title, unsigned int nBinsX, int minX, int maxX, unsigned int nBinsY, int minY, int maxY )
{
    TH2F* newHist = new TH2F ( name.c_str(), title.c_str(), nBinsX, minX, maxX, nBinsY, minY, maxY );
    histsMap[title] = newHist;

    return newHist;
}


// ------------- Write your macros here ----------------------------------- //
// ------------- Use the variable uChain to process your root files ------- //


// ------------- Particles Energy vs. Angle ----------- //

TH2F* hEvsA;
TH2F* hEvsA_SX3U_tot;
TH2F* hEvsA_QQQ5U_tot;
TH2F* hEvsA_SX3U[12];
TH2F* hEvsA_QQQ5U[4];

void InitEvsAHist ( unsigned int nBinsX, unsigned int minX, unsigned int maxX, unsigned int nBinsY, unsigned int minY, unsigned int maxY )
{
    char* hname = new char[500];

    hEvsA = MakeNewHist ( "Energy_vs_Angle", "Energy vs. Angle", nBinsX, minX, maxX, nBinsY, minY, maxY );
    hEvsA_SX3U_tot = MakeNewHist ( "Energy_vs_Angle_SX3U", "Energy vs. Angle SX3 Upstream", nBinsX, minX, maxX, nBinsY, minY, maxY );
    hEvsA_QQQ5U_tot = MakeNewHist ( "Energy_vs_Angle_QQQ5U", "Energy vs. Angle QQQ5 Upstream", nBinsX, minX, maxX, nBinsY, minY, maxY );

    for ( int i = 0; i < 12; i++ )
    {
        sprintf ( hname, "Energy vs Angle SX3 U%d", i );

        hEvsA_SX3U[i] = MakeNewHist ( hname, hname, nBinsX, minX, maxX, nBinsY, minY, maxY );
    }

    for ( int i = 0; i < 4; i++ )
    {
        sprintf ( hname, "Energy vs Angle QQQ5 U%d", i );

        hEvsA_QQQ5U[i] = MakeNewHist ( hname, hname, 32, -1, 33, nBinsY, minY, maxY );
    }
}

void FillEvsAHist ( SiDataBase* siData_ )
{
    float energy = siData_->ESumLayer ( 1, false );
    float angle = siData_->Angle ( 1 );

    unsigned int sector = siData_->sector;
    unsigned int strip = siData_->StripMaxLayer ( 1, false );

    if ( angle != 0 && energy > 0 )
    {
        hEvsA->Fill ( angle, energy );

        if ( siData_->isUpstream )
        {
            if ( siData_->isBarrel )
            {
                hEvsA_SX3U_tot->Fill ( angle, energy );
                hEvsA_SX3U[sector]->Fill ( angle, energy );
            }
            else
            {
                hEvsA_QQQ5U_tot->Fill ( strip, energy );
                hEvsA_QQQ5U[sector]->Fill ( strip, energy );
            }
        }
    }
}


// ------------- Q-Value spectra ----------- //

TH1F* hQval_tot;
TH1F* hQval_SX3U_tot;
TH1F* hQval_QQQ5U_tot;
TH1F* hQval_SX3U[12];
TH1F* hQval_QQQ5U[4];


void InitQvalHist ( unsigned int nBinsX, unsigned int minX, unsigned int maxX )
{
    char* hname = new char[500];

    hQval_tot = MakeNewHist ( "Qvalue_tot", "Q-value tot", nBinsX, minX, maxX );
    hQval_SX3U_tot = MakeNewHist ( "Qvalue_tot_SX3U", "Q-value SX3s Upstream", nBinsX, minX, maxX );
    hQval_QQQ5U_tot = MakeNewHist ( "Qvalue_tot_QQQ5U", "Q-value QQQ5s Upstream", nBinsX, minX, maxX );

    for ( int i = 0; i < 12; i++ )
    {
        sprintf ( hname, "Qvalue SX3 U%d", i );

        hQval_SX3U[i] = MakeNewHist ( hname, hname, nBinsX, minX, maxX );
    }

    for ( int i = 0; i < 4; i++ )
    {
        sprintf ( hname, "Qvalue QQQ5 U%d", i );

        hQval_QQQ5U[i] = MakeNewHist ( hname, hname, nBinsX, minX, maxX );
    }
}

void FillQvalHist ( SiDataBase* siData_, float massBeam, float massTarget, float massRecoil, float massEjec, float beamEk )
{
    float energy = siData_->ESumLayer ( 1, false );
    float angle = siData_->Angle ( 1 );
    float qval = siData_->QValue ( massBeam, beamEk, massTarget, massEjec );

    unsigned int sector = siData_->sector;

    if ( angle != 0 && energy > 0 )
    {
        hQval_tot->Fill ( qval );

        if ( siData_->isUpstream )
        {
            if ( siData_->isBarrel )
            {
                hQval_SX3U_tot->Fill ( qval );
                hQval_SX3U[sector]->Fill ( qval );
            }
            else
            {
                hQval_QQQ5U_tot->Fill ( qval );
                hQval_QQQ5U[sector]->Fill ( qval );
            }
        }
    }
}

// ------------- Time difference between GammaSphere and ORRUBA ----------- //

TH1F* dTGsSX3D[12];
TH1F* dTGsSX3U[12];
TH1F* dTGsQQQ5D[4];
TH1F* dTGsQQQ5U[4];


TH1F* dTGsNoBGOSX3D[12];
TH1F* dTGsNoBGOSX3U[12];
TH1F* dTGsNoBGOQQQ5D[4];
TH1F* dTGsNoBGOQQQ5U[4];

void InitdTGsORRUBAHists ( unsigned int nBinsX = 1000, unsigned int minX = -500, unsigned int maxX = 500 )
{
    for ( int i = 0; i < 12; i++ )
    {
        char* hname = new char[500];

        sprintf ( hname, "dT GS SX3 U%d", i );

        dTGsSX3U[i] = MakeNewHist ( hname, hname, nBinsX, minX, maxX );

        sprintf ( hname, "dT GS SX3 D%d", i );

        dTGsSX3D[i] = MakeNewHist ( hname, hname, nBinsX, minX, maxX );

        sprintf ( hname, "dT GS No BGO SX3 U%d", i );

        dTGsNoBGOSX3U[i] = MakeNewHist ( hname, hname, nBinsX, minX, maxX );

        sprintf ( hname, "dT GS No BGO SX3 D%d", i );

        dTGsNoBGOSX3D[i] = MakeNewHist ( hname, hname, nBinsX, minX, maxX );
    }

    for ( int i = 0; i < 4; i++ )
    {
        char* hname = new char[500];

        sprintf ( hname, "dT GS QQQ5 U%d", i );

        dTGsQQQ5U[i] = MakeNewHist ( hname, hname, nBinsX, minX, maxX );

        sprintf ( hname, "dT GS QQQ5 D%d", i );

        dTGsQQQ5D[i] = MakeNewHist ( hname, hname, nBinsX, minX, maxX );

        sprintf ( hname, "dT GS No BGO QQQ5 U%d", i );

        dTGsNoBGOQQQ5U[i] = MakeNewHist ( hname, hname, nBinsX, minX, maxX );

        sprintf ( hname, "dT GS No BGO QQQ5 D%d", i );

        dTGsNoBGOQQQ5D[i] = MakeNewHist ( hname, hname, nBinsX, minX, maxX );
    }
}

void FilldTGammaORRUBA ( SiDataBase* siData_, GamData* gamData_ )
{
    if ( gamData_->type == 2 ) return;

    unsigned long long int orrubaTs = siData_->TimestampMaxLayer ( 1, false );

    int sector = siData_->sector;

    unsigned long long int gsTs = gamData_->time;

    if ( siData_->isBarrel )
    {
        if ( siData_->isUpstream )
        {
            dTGsSX3U[sector]->Fill ( gsTs - orrubaTs );
        }
        else
        {
            dTGsSX3D[sector]->Fill ( gsTs - orrubaTs );
        }
    }
    else
    {
        if ( siData_->isUpstream )
        {
            dTGsQQQ5U[sector]->Fill ( gsTs - orrubaTs );
        }
        else
        {
            dTGsQQQ5D[sector]->Fill ( gsTs - orrubaTs );
        }
    }


    return;
}


void FilldTNoBGOGammaORRUBA ( SiDataBase* siData_, GamData* gamData_ )
{
    unsigned long long int orrubaTs = siData_->TimestampMaxLayer ( 1, false );

    int sector = siData_->sector;

    unsigned long long int gsTs = gamData_->time;

    if ( siData_->isBarrel )
    {
        if ( siData_->isUpstream )
        {
            dTGsNoBGOSX3U[sector]->Fill ( gsTs - orrubaTs );
        }
        else
        {
            dTGsNoBGOSX3D[sector]->Fill ( gsTs - orrubaTs );
        }
    }
    else
    {
        if ( siData_->isUpstream )
        {
            dTGsNoBGOQQQ5U[sector]->Fill ( gsTs - orrubaTs );
        }
        else
        {
            dTGsNoBGOQQQ5D[sector]->Fill ( gsTs - orrubaTs );
        }
    }


    return;
}

// -------------------- GammaSphere gated by ORRUBA ------------------------ //

TH1F* gsGatedSX3U;
TH1F* gsGatedSX3U_analog;
TH1F* gsGatedSX3U_digital;

TH1F* gsGatedSX3D;
TH1F* gsGatedSX3D_analog;
TH1F* gsGatedSX3D_digital;

TH1F* gsGatedQQQ5U;
TH1F* gsGatedQQQ5U_analog;
TH1F* gsGatedQQQ5U_digital;

TH1F* gsGatedQQQ5D;
TH1F* gsGatedQQQ5D_analog;
TH1F* gsGatedQQQ5D_digital;


TH1F* gsNoBGOGatedSX3U;
TH1F* gsNoBGOGatedSX3U_analog;
TH1F* gsNoBGOGatedSX3U_digital;

TH1F* gsNoBGOGatedSX3D;
TH1F* gsNoBGOGatedSX3D_analog;
TH1F* gsNoBGOGatedSX3D_digital;

TH1F* gsNoBGOGatedQQQ5U;
TH1F* gsNoBGOGatedQQQ5U_analog;
TH1F* gsNoBGOGatedQQQ5U_digital;

TH1F* gsNoBGOGatedQQQ5D;
TH1F* gsNoBGOGatedQQQ5D_analog;
TH1F* gsNoBGOGatedQQQ5D_digital;

void InitGsGateORRUBAHists ( unsigned int nBinsX = 5000, unsigned int minX = 0, unsigned int maxX = 5000 )
{
    gsGatedSX3U = MakeNewHist ( "GammaSphere Gates SX3 Upstream", "GammaSphere Gates SX3 Upstream", nBinsX, minX, maxX );
    gsGatedSX3U_analog = MakeNewHist ( "GammaSphere Gates SX3 Upstream Analog", "GammaSphere Gates SX3 Upstream Analog", nBinsX, minX, maxX );
    gsGatedSX3U_digital = MakeNewHist ( "GammaSphere Gates SX3 Upstream Digital ", "GammaSphere Gates SX3 Upstream Digital", nBinsX, minX, maxX );

    gsGatedSX3D = MakeNewHist ( "GammaSphere Gates SX3 Downstream", "GammaSphere Gates SX3 Downstream", nBinsX, minX, maxX );
    gsGatedSX3D_analog = MakeNewHist ( "GammaSphere Gates SX3 Downstream Analog", "GammaSphere Gates SX3 Downstream Analog", nBinsX, minX, maxX );
    gsGatedSX3D_digital = MakeNewHist ( "GammaSphere Gates SX3 Downstream Digital ", "GammaSphere Gates SX3 Downstream Digital", nBinsX, minX, maxX );

    gsGatedQQQ5U = MakeNewHist ( "GammaSphere Gates QQQ5 Upstream", "GammaSphere Gates QQQ5 Upstream", nBinsX, minX, maxX );
    gsGatedQQQ5U_analog = MakeNewHist ( "GammaSphere Gates QQQ5 Upstream Analog", "GammaSphere Gates QQQ5 Upstream Analog", nBinsX, minX, maxX );
    gsGatedQQQ5U_digital = MakeNewHist ( "GammaSphere Gates QQQ5 Upstream Digital ", "GammaSphere Gates QQQ5 Upstream Digital", nBinsX, minX, maxX );

    gsGatedQQQ5D = MakeNewHist ( "GammaSphere Gates QQQ5 Downstream", "GammaSphere Gates QQQ5 Downstream", nBinsX, minX, maxX );
    gsGatedQQQ5D_analog = MakeNewHist ( "GammaSphere Gates QQQ5 Downstream Analog", "GammaSphere Gates QQQ5 Downstream Analog", nBinsX, minX, maxX );
    gsGatedQQQ5D_digital = MakeNewHist ( "GammaSphere Gates QQQ5 Downstream Digital ", "GammaSphere Gates QQQ5 Downstream Digital", nBinsX, minX, maxX );


    gsNoBGOGatedSX3U = MakeNewHist ( "GammaSphere No BGO Gates SX3 Upstream", "GammaSphere No BGO Gates SX3 Upstream", nBinsX, minX, maxX );
    gsNoBGOGatedSX3U_analog = MakeNewHist ( "GammaSphere No BGO Gates SX3 Upstream Analog", "GammaSphere No BGO Gates SX3 Upstream Analog", nBinsX, minX, maxX );
    gsNoBGOGatedSX3U_digital = MakeNewHist ( "GammaSphere No BGO Gates SX3 Upstream Digital ", "GammaSphere No BGO Gates SX3 Upstream Digital", nBinsX, minX, maxX );

    gsNoBGOGatedSX3D = MakeNewHist ( "GammaSphere No BGO Gates SX3 Downstream", "GammaSphere No BGO Gates SX3 Downstream", nBinsX, minX, maxX );
    gsNoBGOGatedSX3D_analog = MakeNewHist ( "GammaSphere No BGO Gates SX3 Downstream Analog", "GammaSphere No BGO Gates SX3 Downstream Analog", nBinsX, minX, maxX );
    gsNoBGOGatedSX3D_digital = MakeNewHist ( "GammaSphere No BGO Gates SX3 Downstream Digital ", "GammaSphere No BGO Gates SX3 Downstream Digital", nBinsX, minX, maxX );

    gsNoBGOGatedQQQ5U = MakeNewHist ( "GammaSphere No BGO Gates QQQ5 Upstream", "GammaSphere No BGO Gates QQQ5 Upstream", nBinsX, minX, maxX );
    gsNoBGOGatedQQQ5U_analog = MakeNewHist ( "GammaSphere No BGO Gates QQQ5 Upstream Analog", "GammaSphere No BGO Gates QQQ5 Upstream Analog", nBinsX, minX, maxX );
    gsNoBGOGatedQQQ5U_digital = MakeNewHist ( "GammaSphere No BGO Gates QQQ5 Upstream Digital ", "GammaSphere No BGO Gates QQQ5 Upstream Digital", nBinsX, minX, maxX );

    gsNoBGOGatedQQQ5D = MakeNewHist ( "GammaSphere No BGO Gates QQQ5 Downstream", "GammaSphere No BGO Gates QQQ5 Downstream", nBinsX, minX, maxX );
    gsNoBGOGatedQQQ5D_analog = MakeNewHist ( "GammaSphere No BGO Gates QQQ5 Downstream Analog", "GammaSphere No BGO Gates QQQ5 Downstream Analog", nBinsX, minX, maxX );
    gsNoBGOGatedQQQ5D_digital = MakeNewHist ( "GammaSphere No BGO Gates QQQ5 Downstream Digital ", "GammaSphere No BGO Gates QQQ5 Downstream Digital", nBinsX, minX, maxX );
}

bool FillGsGateORRUBA ( SiDataBase* siData_, GamData* gamData_ )
{
    unsigned long long int orrubaTs = siData_->TimestampMaxLayer ( 1, false );

    int sector = siData_->sector;

    unsigned long long int gsTs = gamData_->time;

    double gsEn = ( double ) gamData_->en/3.;
    double siEn = siData_->ESumLayer ( 1, false );

    bool filled = false;

    bool isDigital = ( siData_->isBarrel && ( siData_->sector >=1 && siData_->sector <= 6 ) ) ||
                     ( !siData_->isBarrel && ( ( siData_->isUpstream && ( siData_->sector == 0 || siData_->sector == 1 ) ) || ( !siData_->isUpstream && siData_->sector == 2 ) ) );

    bool timestampOK = ( isDigital && gsTs - orrubaTs > 190 && gsTs - orrubaTs < 210 ) || ( !isDigital && gsTs - orrubaTs > 410 && gsTs - orrubaTs < 430 );

    bool energyOK = ( isDigital && ( ( !siData_->isBarrel && siEn > 5000 ) || ( siData_->isBarrel && ( ( siData_->isUpstream && siEn > 10000 ) || ( !siData_->isUpstream && siEn > 25000 ) ) ) ) ) ||
                    ( !isDigital && ( ( !siData_->isBarrel &&  siEn > 275 ) || ( siData_->isBarrel && ( ( siData_->isUpstream && siEn > 20 ) || ( !siData_->isUpstream && siEn > 320 ) ) ) ) );

//     if ( timestampOK && energyOK )
    if ( timestampOK )
    {
        if ( siData_->isBarrel )
        {
            if ( siData_->isUpstream )
            {
                gsGatedSX3U->Fill ( gsEn );

                if ( sector > 0 && sector < 7 ) gsGatedSX3U_digital->Fill ( gsEn );
                else gsGatedSX3U_analog->Fill ( gsEn );
            }
            else
            {
                gsGatedSX3D->Fill ( gsEn );

                if ( sector > 0 && sector < 7 ) gsGatedSX3D_digital->Fill ( gsEn );
                else gsGatedSX3D_analog->Fill ( gsEn );
            }

//                 filled = true;
        }
        else
        {
            if ( siData_->isUpstream )
            {
                gsGatedQQQ5U->Fill ( gsEn );

                if ( sector == 0 || sector == 1 ) gsGatedQQQ5U_digital->Fill ( gsEn );
                else gsGatedQQQ5U_analog->Fill ( gsEn );
            }
            else
            {
                gsGatedQQQ5D->Fill ( gsEn );

                if ( sector == 2 ) gsGatedQQQ5D_digital->Fill ( gsEn );
                else gsGatedQQQ5D_analog->Fill ( gsEn );
            }

//                 filled = true;
        }
    }

    return filled;
}

bool FillGsNoBGOGateORRUBA ( SiDataBase* siData_, GamData* gamData_ )
{
    unsigned long long int orrubaTs = siData_->TimestampMaxLayer ( 1, false );

    int sector = siData_->sector;

    unsigned long long int gsTs = gamData_->time;

    double gsEn = ( double ) gamData_->en/3.;
    double siEn = siData_->ESumLayer ( 1, false );

    bool filled = false;

    bool isDigital = ( siData_->isBarrel && ( siData_->sector >=1 && siData_->sector <= 6 ) ) ||
                     ( !siData_->isBarrel && ( ( siData_->isUpstream && ( siData_->sector == 0 || siData_->sector == 1 ) ) || ( !siData_->isUpstream && siData_->sector == 2 ) ) );

    bool timestampOK = ( isDigital && gsTs - orrubaTs > 190 && gsTs - orrubaTs < 210 ) || ( !isDigital && gsTs - orrubaTs > 410 && gsTs - orrubaTs < 430 );

    bool energyOK = ( isDigital && ( ( !siData_->isBarrel && siEn > 5000 ) || ( siData_->isBarrel && ( ( siData_->isUpstream && siEn > 10000 ) || ( !siData_->isUpstream && siEn > 25000 ) ) ) ) ) ||
                    ( !isDigital && ( ( !siData_->isBarrel &&  siEn > 275 ) || ( siData_->isBarrel && ( ( siData_->isUpstream && siEn > 20 ) || ( !siData_->isUpstream && siEn > 320 ) ) ) ) );

//     if ( timestampOK && energyOK )
    if ( timestampOK )
    {
        if ( siData_->isBarrel )
        {
            if ( siData_->isUpstream )
            {
                gsNoBGOGatedSX3U->Fill ( gsEn );

                if ( sector > 0 && sector < 7 ) gsNoBGOGatedSX3U_digital->Fill ( gsEn );
                else gsNoBGOGatedSX3U_analog->Fill ( gsEn );
            }
            else
            {
                gsNoBGOGatedSX3D->Fill ( gsEn );

                if ( sector > 0 && sector < 7 ) gsNoBGOGatedSX3D_digital->Fill ( gsEn );
                else gsNoBGOGatedSX3D_analog->Fill ( gsEn );
            }

//                 filled = true;
        }
        else
        {
            if ( siData_->isUpstream )
            {
                gsNoBGOGatedQQQ5U->Fill ( gsEn );

                if ( sector == 0 || sector == 1 ) gsNoBGOGatedQQQ5U_digital->Fill ( gsEn );
                else gsNoBGOGatedQQQ5U_analog->Fill ( gsEn );
            }
            else
            {
                gsNoBGOGatedQQQ5D->Fill ( gsEn );

                if ( sector == 2 ) gsNoBGOGatedQQQ5D_digital->Fill ( gsEn );
                else gsNoBGOGatedQQQ5D_analog->Fill ( gsEn );
            }

//                 filled = true;
        }
    }

    return filled;
}

// --------- Put here the functions you want to be processed ---------------- //

void FillUserHists ( unsigned long long int maxEvents = 0 )
{
    std::vector<SiDataBase>* siData = new std::vector<SiDataBase>();
    std::vector<GamData>* gamData = new std::vector<GamData>();

    uChain->SetBranchAddress ( "si", &siData );
    uChain->SetBranchAddress ( "gam", &gamData );

    InitdTGsORRUBAHists();
    InitGsGateORRUBAHists();
    InitEvsAHist ( 1800, 0, 180, 1000, 0, 10 );
    InitQvalHist ( 800, -20, 20 );

    eventNbr = 0;

    if ( maxEvents <= 0 || maxEvents > totEvents ) maxEvents = totEvents;

    while ( eventNbr < maxEvents )
    {
        PrintProgress ( maxEvents );

        uChain->GetEntry ( eventNbr );

        for ( unsigned int i = 0; i < siData->size(); i++ )
        {
            FillEvsAHist ( & ( siData->at ( i ) ) );
            FillQvalHist ( & ( siData->at ( i ) ), 134., 2., 135., 1., 1337. );

            bool noBGO = true;

            for ( unsigned int j = 0; j < gamData->size(); j++ )
            {
                if ( gamData->at ( j ).type == 1 )
                {
                    FilldTGammaORRUBA ( & ( siData->at ( i ) ), & ( gamData->at ( j ) ) );

                    FillGsGateORRUBA ( & ( siData->at ( i ) ), & ( gamData->at ( j ) ) );
                }
                else noBGO = false;
            }

            if ( noBGO )
            {
                for ( unsigned int j = 0; j < gamData->size(); j++ )
                {
                    FilldTNoBGOGammaORRUBA ( & ( siData->at ( i ) ), & ( gamData->at ( j ) ) );

                    FillGsNoBGOGateORRUBA ( & ( siData->at ( i ) ), & ( gamData->at ( j ) ) );
                }
            }
        }

//         for ( unsigned int j = 0; j < std::min ( gamData->size(), siData->size() ); j++ )
//         {
//             FillGsGateORRUBA ( & ( siData->at ( j ) ), & ( gamData->at ( j ) ) );
//         }

        eventNbr++;
    }

    std::cout << "\n\n";

    return;
}

// ----------- Handles the writing to a TFile ---------------------- //

void WriteUserHists ( string outName )
{
    string mode = "recreate";

    TFile* outRootFile = new TFile ( outName.c_str(), "read" );

    int userChoice;

    if ( outRootFile->IsOpen() )
    {
        std::cout << "File " << outName << " already exists...\n";
        std::cout << "Would you like to overwrite it [1] or update it [2]? ";
        std::cin >> userChoice;

        if ( userChoice == 2 )
        {
            mode = "update";
        }
        else if ( std::cin.fail() || userChoice < 1 || userChoice > 2 )
        {
            std::cout << "Invalid input... aborting...\n";
            return;
        }

        outRootFile->Close();

        std::cout << "\n";
    }

    outRootFile = new TFile ( outName.c_str(), mode.c_str() );

    for ( auto itr = histsMap.begin(); itr != histsMap.end(); itr++ )
    {
        TH1F* h1 = dynamic_cast<TH1F*> ( itr->second );
        TH2F* h2 = dynamic_cast<TH2F*> ( itr->second );

        if ( h1 != NULL )
        {
            h1->Write();
        }
        else if ( h2 != NULL )
        {
            h2->Write();
        }
    }

    outRootFile->Close();
}


