#include "GoddessCalib.h"

void GoddessCalib::GetCornersCoordinates ( TCanvas* can, bool isUpstream, unsigned short sector, unsigned short strip, std::string detectorType, double refEnergy1 )
{
    std::string calMapKey = detectorType;
    calMapKey += isUpstream ? " U" : " D";
    calMapKey += std::to_string ( sector );

    auto calRes = resStripsCalMap[calMapKey][strip];

    TList* list = can->GetListOfPrimitives();

    int listSize = list->GetSize();

    TLine* negLine = 0;
    TLine* topLine = 0;
    TLine* botLine = 0;

    double tempSlope = 0;
    TLine* tempLine = 0;

    for ( int i = 0; i < listSize; i++ )
    {
        TObject* child = list->At ( i );

        TLine* line = dynamic_cast<TLine*> ( child );

        if ( line == NULL ) continue;

        double x1 = line->GetX1();
        double y1 = line->GetY1();
        double x2 = line->GetX2();
        double y2 = line->GetY2();

        double slope = ( y2 - y1 ) / ( x2 - x1 );

//         std::cout << "Found a line in the list at position #" << i << std::endl;
//         std::cout << "X1 : " << x1 << "   /   Y1 : " << y1 << std::endl;
//         std::cout << "X2 : " << x2 << "   /   Y2 ; " << y2 << std::endl;
//
//         std::cout << "Slope = " << slope << std::endl;

        if ( slope < 0 ) negLine = line;
        else
        {
            if ( tempLine == NULL )
            {
                tempSlope = slope;
                tempLine = line;
            }
            else
            {
                if ( slope > tempSlope )
                {
                    topLine = line;
                    botLine = tempLine;
                }
                else
                {
                    botLine = line;
                    topLine = tempLine;
                }
            }
        }
    }

    // ---------------- Let's find the inteserction between top and bottom ---------------- //

    double slopeTop = -100, slopeBot = -10, slopeNeg = -10;
    double offTop = -100, offBot = -100, offNeg = -100;

    if ( topLine != NULL )
    {
        slopeTop = ( topLine->GetY2() - topLine->GetY1() ) / ( topLine->GetX2() - topLine->GetX1() );
        offTop = ( topLine->GetX1() * topLine->GetY2() - topLine->GetX2() * topLine->GetY1() ) / ( topLine->GetX1() - topLine->GetX2() );
    }

    if ( botLine != NULL )
    {
        slopeBot = ( botLine->GetY2() - botLine->GetY1() ) / ( botLine->GetX2() - botLine->GetX1() );
        offBot = ( botLine->GetX1() * botLine->GetY2() - botLine->GetX2() * botLine->GetY1() ) / ( botLine->GetX1() - botLine->GetX2() );
    }

    if ( negLine != NULL )
    {
        slopeNeg = ( negLine->GetY2() - negLine->GetY1() ) / ( negLine->GetX2() - negLine->GetX1() );
        offNeg = ( negLine->GetX1() * negLine->GetY2() - negLine->GetX2() * negLine->GetY1() ) / ( negLine->GetX1() - negLine->GetX2() );
    }

    double xIntersect = -100, yIntersect = -100;

    if ( topLine != NULL && botLine != NULL )
    {
        xIntersect = ( offTop - offBot ) / ( slopeBot - slopeTop );
        yIntersect = slopeTop * xIntersect + offTop;
    }

    double nearCorrFactor = -100;
    double energyCalCoeff = -100;

    if ( negLine != NULL ) nearCorrFactor = -slopeNeg;

    if ( calRes[0] == 0 )
    {
        calRes[0] = 1;
        calRes[1] = xIntersect;
        calRes[2] = yIntersect;
        calRes[3] = nearCorrFactor;
        calRes[4] = -100;
    }
    else
    {
        calRes[1] = xIntersect == -100 ? calRes[1] : xIntersect;
        calRes[2] = yIntersect == -100 ? calRes[2] : yIntersect;
        calRes[3] = nearCorrFactor == -100 ? calRes[3] : nearCorrFactor;
    }

    if ( negLine != NULL && calRes[0] == 1 && calRes[1] != -100 && calRes[2] != -100 )
    {
        energyCalCoeff = refEnergy1 / ( ( negLine->GetX1() - calRes[1] ) * ( -slopeNeg ) + negLine->GetY1()  - calRes[2] );
    }

    std::cout << "Intersection between top and bottom at ( " << xIntersect << " ; " << yIntersect << " )" << std::endl;
    std::cout << "Correction factor for the near strip = " << nearCorrFactor << std::endl;
    std::cout << "Slope for energy calibration = " << energyCalCoeff << std::endl;

    calRes[4] = energyCalCoeff == -100 ? calRes[4] : energyCalCoeff;

    return;
}

bool GoddessCalib::DumpFileToResCalMap ( std::string fileName )
{
    std::ifstream readFile ( fileName.c_str() );

    if ( !readFile.is_open() )
    {
        std::cerr << "Failed to open file " << fileName << " for previous calibration reading (if it did not exist before, the file has now been created)" << std::endl;
        return false;
    }

    std::string dump;

    std::string lineID, detID, stripID, dummy;

    std::cout << "Reading " << fileName << "..." << std::endl;

    while ( std::getline ( readFile, dump ) )
    {
        std::istringstream readLine;
        readLine.str ( dump );

        readLine >> lineID;

        if ( lineID == "SuperX3" )
        {
            readLine >> detID;

            detID = lineID + " " + detID;

            int posInFile = readFile.tellg();

            std::cout << "Found " << detID << " at position " << posInFile << " in input file..." << std::endl;

            std::getline ( readFile, dump );

            readLine.clear();
            readLine.str ( dump );

            readLine >> lineID;
        }


        auto itr = resStripsCalMap.find ( detID );

        if ( lineID == "Res." )
        {
            readLine >> dummy >> stripID;

            unsigned short stripNbr = std::stoul ( stripID.substr ( 1 ) );

            if ( ( itr != resStripsCalMap.end() && itr->second.find ( stripNbr ) == itr->second.end() ) || ! ( itr != resStripsCalMap.end() ) )
            {
                float xinter, yinter, slope_gm, slope_encal;

                readLine >> xinter >> yinter >> slope_gm >> slope_encal;

                auto readCal = resStripsCalMap[detID][stripNbr];

                readCal[0] = xinter == -100 ? readCal[0] : xinter;
                readCal[1] = yinter == -100 ? readCal[1] : yinter;
                readCal[2] = slope_gm == -100 ? readCal[2] : slope_gm;
                readCal[3] = slope_encal == -100 ? readCal[3] : slope_encal;
            }
        }
    }

    return true;
}

void GoddessCalib::WriteResCalResults ( std::string fileName, std::string mode )
{
    if ( mode == "update" )
    {
        if ( DumpFileToResCalMap ( fileName.c_str() ) ) remove ( fileName.c_str() );
    }

    std::ofstream outStream ( fileName.c_str() );

    outStream << "Columns are: \"Strip #\"      \"Offset X\"      \"Offset Y\"      \"Slope (gain match)\"     \"Slope (energy calibration)\"\n";

    for ( auto itr = resStripsCalMap.begin(); itr != resStripsCalMap.end(); itr++ )
    {
        outStream << "\n" << itr->first << "\n";

        for ( unsigned short i = 0; i < 4; i++ )
        {
            if ( itr->second.find ( i ) != itr->second.end() )
            {
                outStream << "Res. Strip #" << i << "   " << itr->second[i][1] << "   " << itr->second[i][2] << "   " << itr->second[i][3] << "   " << itr->second[i][4] << "\n";
            }
        }
    }

    return;
}

bool GoddessCalib::UpdateResParamsInConf ( std::string configFile, bool invertContactMidDet, std::string mode )
{
    std::ifstream readFile ( configFile.c_str() );

    std::string path = configFile.substr ( 0, configFile.find_last_of ( "/" ) );

    int foundExt = configFile.find ( "." );

    std::string newName = configFile;
    newName.insert ( foundExt, "_new" );

    std::ofstream outStream ( newName.c_str() );

    if ( !readFile.is_open() )
    {
        std::cerr << "Failed to open file " << configFile << std::endl;
        return false;
    }

    std::string dump;

    std::string lineID, detID, stripID, dummy;

    std::cout << "Reading " << configFile << "..." << std::endl;

    while ( std::getline ( readFile, dump ) )
    {
        std::istringstream readLine;
        readLine.str ( dump );

        readLine >> lineID;

        if ( lineID != "superX3" )
        {
            outStream << dump << "\n";
            continue;
        }

        else
        {
            outStream << dump << "\n";

            readLine >> dummy >> detID;

            int posInFile = readFile.tellg();

            std::cout << "Found a superX3 entry: " << detID << std::endl;

            std::getline ( readFile, dump );
            outStream << dump << "\n";

            std::getline ( readFile, dump );
            outStream << dump << "\n";

            std::string detKey = "SuperX3 " + detID.substr ( 0,detID.find ( "-" ) );

            if ( resStripsCalMap.find ( detKey ) != resStripsCalMap.end() )
            {
                for ( int i =0; i < 4; i++ )
                {
                    if ( resStripsCalMap[detKey].find ( i ) == resStripsCalMap[detKey].end() )
                    {
                        std::cout << "Replacing the parameters for strip #" << i << " with the new values..." << std::endl;

                        std::getline ( readFile, dump );
                        std::getline ( readFile, dump );
                        std::getline ( readFile, dump );

                        if ( i < 2 || !invertContactMidDet )
                        {
                            outStream << "enCal p " << i*2 << " " << resStripsCalMap[detKey][i][0] << " " << resStripsCalMap[detKey][i][2] << "\n";
                            outStream << "enCal p " << i*2 + 1 << " " << resStripsCalMap[detKey][i][1] << " " << 1 << "\n";
                            outStream << "enCal resStrip " << i << " 0 " << resStripsCalMap[detKey][i][3] << "\n";
                        }
                        else
                        {
                            outStream << "enCal p " << i*2 << " " << resStripsCalMap[detKey][i][1] << " " << 1 << "\n";
                            outStream << "enCal p " << i*2 + 1 << " " << resStripsCalMap[detKey][i][0] << " " << resStripsCalMap[detKey][i][2] << "\n";
                            outStream << "enCal resStrip " << i << " 0 " << resStripsCalMap[detKey][i][3] << "\n";
                        }
                    }
                    else
                    {
                        for ( int j =0; j < 3; j++ )
                        {
                            std::getline ( readFile, dump );
                            outStream << dump << "\n";
                        }
                    }
                }
            }
            else
            {
                for ( int i = 0; i < 12; i++ )
                {
                    std::getline ( readFile, dump );
                    outStream << dump << "\n";
                }
            }


            for ( int i = 0; i < 4; i++ )
            {
                std::getline ( readFile, dump );
                outStream << dump << "\n";
            }
        }
    }

    readFile.close();
    outStream.close();

    if ( mode == "overwrite" )
    {
        remove ( configFile.c_str() );
        rename ( newName.c_str(), configFile.c_str() );
    }

    return true;
}

TGraph* GoddessCalib::PlotSX3ResStripCalGraph ( TTree* tree, std::string varToPlot, unsigned short sector, unsigned short strip, std::string conditions )
{
    std::size_t upStreamCondPos = conditions.find ( "si.isUpstream" );

    std::string upStreamCond = "U";

    if ( upStreamCondPos == std::string::npos )
    {
        std::cerr << "isUpstream condition missing..." << std::endl;
        return 0;
    }

    if ( conditions[upStreamCondPos-1] == '!' ) upStreamCond = "D";

    std::cout << "Plotting sector #" << sector << " strip #" << strip << "..." << std::endl;

    tree->Draw ( Form ( "%s", varToPlot.c_str() ), Form ( "%s && si.sector == %d && si.E1.strip.p@.size() > 0 && si.E1.strip.p == %d", conditions.c_str(), sector, strip ) );

//     std::cout << "Retrieving TGraph*..." << std::endl;

    TGraph* newGraph = ( TGraph* ) gPad->GetPrimitive ( "Graph" );

    if ( newGraph == NULL ) return 0;

    std::string gName = Form ( "sector%s%d_strip%d", upStreamCond.c_str(), sector, strip );

    newGraph->SetName ( gName.c_str() );
    newGraph->SetTitle ( gName.c_str() );

    std::string currPath = ( std::string ) gDirectory->GetPath();

    std::string rootFileName = "Resistive_Strips_Calib_Graphs_";

    std::string treeFName = tree->GetCurrentFile()->GetName();

    std::size_t begRunName = treeFName.find ( "run" );
    std::size_t endRunName = treeFName.find ( "_", begRunName );

    if ( begRunName != std::string::npos && endRunName != std::string::npos ) rootFileName += treeFName.substr ( begRunName, endRunName ) + ".root";
    else rootFileName += treeFName;

    TFile* f = new TFile ( rootFileName.c_str(), "update" );

    if ( f->FindKey ( gName.c_str() ) != NULL || f->FindObject ( gName.c_str() ) != NULL )
    {
//         std::cout << "Deleting existing TGraph..." << std::endl;
        std::string objToDelete = gName + ";*";
        f->Delete ( objToDelete.c_str() );
    }

    f->cd();

//     std::cout << "Writing new TGraph..." << std::endl;

    newGraph->Write();

    f->Close();

    gDirectory->cd ( currPath.c_str() );

//     std::cout << "Storing the new TGraph in the TGraph map..." << std::endl;

    resStripsEnCalGraphsMap[Form ( "sector %s%d strip %d", upStreamCond.c_str(), sector, strip )] = newGraph;

    return newGraph;
}

TGraph* GoddessCalib::DrawPosCalGraph ( TTree* tree, bool isUpstream_, int nentries, unsigned short sector_, unsigned short strip_ )
{
    TGraph* newGraph = new TGraph();

    DrawPosCal ( tree, isUpstream_, sector_,strip_, nentries, newGraph );

    newGraph->Draw ( "AP" );

    return newGraph;
}

TH2F* GoddessCalib::DrawPosCalHist ( TTree* tree, bool isUpstream_, int nentries, int nbinsX, int binMinX, int binMaxX, int nbinsY, int binMinY, int binMaxY, std::string drawOpts, unsigned short sector_, unsigned short strip_ )
{
    std::string hname = Form ( "hPosCal_sector%s%d_strip%d", isUpstream_ ? "U" : "D", sector_, strip_ );

    std::string isUpstreamID = isUpstream_ ? "U" : "D";
    std::string hKey = Form ( "%s%d_%d", isUpstreamID.c_str(), sector_, strip_ );
    resStripsPosCalGraphsMap[hKey.c_str()] = new TH2F ( hname.c_str(), hname.c_str(), nbinsX, binMinX, binMaxX, nbinsY, binMinY, binMaxY );

    DrawPosCal ( tree, isUpstream_, sector_,strip_, nentries, resStripsPosCalGraphsMap[hKey.c_str()] );

    resStripsPosCalGraphsMap[hKey.c_str()]->Draw ( drawOpts.c_str() );

    return resStripsPosCalGraphsMap[hKey.c_str()];
}

std::map<std::string, TH2F*> GoddessCalib::DrawPosCalHistBatch ( TTree* tree, bool isUpstream_, int nentries, int nbinsX, int binMinX, int binMaxX, int nbinsY, int binMinY, int binMaxY, std::string drawOpts, unsigned short sector_ )
{
    return DrawPosCalHistBatch ( tree, isUpstream_, nentries,nbinsX,binMinX, binMaxX, nbinsY, binMinY, binMaxY, drawOpts, sector_ );
}

void GoddessCalib::DrawSX3EnCalGraph ( bool isUpstream, short unsigned int sector, short unsigned int strip )
{
    std::string isUpstreamID = isUpstream ? "U" : "D";
    std::string hKey = Form ( "sector %s%d strip %d", isUpstreamID.c_str(), sector, strip );

    auto itr = resStripsEnCalGraphsMap.find ( hKey );

    if ( itr != resStripsEnCalGraphsMap.end() )
    {
        itr->second->Draw ( "AP" );
    }
    else
        std::cerr << "This graph has not been generated yet!" << std::endl;
}

void GoddessCalib::DrawSX3PosCalHist ( bool isUpstream, short unsigned int sector, short unsigned int strip, std::string drawOpts )
{
    std::string isUpstreamID = isUpstream ? "U" : "D";
    std::string hKey = Form ( "%s%d_%d", isUpstreamID.c_str(), sector, strip );

    auto itr = resStripsPosCalGraphsMap.find ( hKey );

    if ( itr != resStripsPosCalGraphsMap.end() )
    {
        itr->second->Draw ( drawOpts.c_str() );
    }
    else
        std::cerr << "This graph has not been generated yet!" << std::endl;
}

TGraph* GoddessCalib::GetSX3EnCalGraph ( bool isUpstream, short unsigned int sector, short unsigned int strip )
{
    std::string isUpstreamID = isUpstream ? "U" : "D";
    std::string hKey = Form ( "sector %s%d strip %d", isUpstreamID.c_str(), sector, strip );

    auto itr = resStripsEnCalGraphsMap.find ( hKey );

    if ( itr != resStripsEnCalGraphsMap.end() )
    {
        return itr->second;
    }
    else
    {
        std::cerr << "This graph has not been generated yet!" << std::endl;
        return 0;
    }
}

TH2F* GoddessCalib::GetSX3PosCalHist ( bool isUpstream, short unsigned int sector, short unsigned int strip )
{
    std::string isUpstreamID = isUpstream ? "U" : "D";
    std::string hKey = Form ( "%s%d_%d", isUpstreamID.c_str(), sector, strip );

    auto itr = resStripsPosCalGraphsMap.find ( hKey );

    if ( itr != resStripsPosCalGraphsMap.end() )
    {
        return itr->second;
    }
    else
    {
        std::cerr << "This graph has not been generated yet!" << std::endl;
        return 0;
    }
}

int GoddessCalib::GetPosCalEnBinMax ( TH2F* input )
{
    int fstBin = input->GetXaxis()->GetFirst();
    int lstBin = input->GetXaxis()->GetLast();

    int binMax = 0;

    TH1D* proj = input->ProjectionY ( ( ( std::string ) "projY_" + input->GetName() ).c_str(), fstBin, lstBin );

    binMax = proj->GetMaximumBin();

    return binMax;
}

TH1D* GoddessCalib::GetPosCalProjX ( TH2F* input, int projCenter, int projWidth )
{
    TH1D* proj = input->ProjectionX ( ( ( std::string ) "projX_" + input->GetName() ).c_str(), projCenter - projWidth/2, projCenter + projWidth/2 );

    return proj;
}

TF1* GoddessCalib::FitLeftEdge ( TH2F* input, int projWidth )
{
    int binMaxY = GetPosCalEnBinMax ( input );

    TH1D* projX = GetPosCalProjX ( input, binMaxY, projWidth );

    int binMaxX = projX->GetMaximumBin();

    double maxContent = projX->GetBinContent ( binMaxX );

    int startBin = binMaxX;
    double startBinContent = maxContent;

    bool fellBelow20Perc = false;
    int binShoulder = -1;

    int counterMax = projX->GetNbinsX() / 25;

    while ( startBin > projX->GetXaxis()->GetFirst() && binShoulder == -1 )
    {
        int prevBin = startBin;
        double prevBinContent = startBinContent;

        int nextBin = startBin - 1;
        double nextBinContent = projX->GetBinContent ( nextBin );
        bool foundIncrease = false;

        int counter = 0;

        while ( nextBin > projX->GetXaxis()->GetFirst() && counter < counterMax )
        {
            if ( nextBinContent < startBinContent*0.2 )
            {
                fellBelow20Perc = true;
                binShoulder = startBin;
                break;
            }

            if ( nextBinContent > prevBinContent )
            {
                if ( !foundIncrease ) foundIncrease = true;
                else break;
            }

            prevBin = nextBin;
            prevBinContent = nextBinContent;

            nextBin--;
            nextBinContent = projX->GetBinContent ( nextBin );

            counter++;
        }

        startBin--;
        startBinContent = projX->GetBinContent ( startBin );
    }

    std::cout << "Found the left shoulder at bin #" << binShoulder << " (value = " << projX->GetBinCenter ( binShoulder ) << ")" << std::endl;

    TF1* fitfunc = new TF1 ( Form ( "myfit_%s",input->GetName() ), "[0]*exp(-0.5*((x-[1])/[2])**2)", projX->GetBinCenter ( binShoulder - counterMax ), projX->GetBinCenter ( binShoulder ) );

    fitfunc->FixParameter ( 0, projX->GetBinContent ( binShoulder ) );
    fitfunc->SetParameter ( 1, projX->GetBinCenter ( binShoulder ) );
    fitfunc->SetParameter ( 2, projX->GetBinCenter ( binShoulder ) - projX->GetBinCenter ( binShoulder - 1 ) );

    projX->Fit ( fitfunc, "QRMN" );

    float leftEdge = fitfunc->GetParameter ( 1 ) - TMath::Sqrt ( -2*pow ( fitfunc->GetParameter ( 2 ),2 ) * TMath::Log ( 0.7 ) );

    std::cout << "Found the left strip edge at " << leftEdge << std::endl;

    return fitfunc;
}

TF1* GoddessCalib::FitRightEdge ( TH2F* input, int projWidth )
{
    int binMaxY = GetPosCalEnBinMax ( input );

    TH1D* projX = GetPosCalProjX ( input, binMaxY, projWidth );

    int binMaxX = projX->GetMaximumBin();

    double maxContent = projX->GetBinContent ( binMaxX );

    int startBin = binMaxX;
    double startBinContent = maxContent;

    bool fellBelow20Perc = false;
    int binShoulder = -1;

    int counterMax = projX->GetNbinsX() / 30;

    while ( startBin < projX->GetXaxis()->GetLast() && binShoulder == -1 )
    {
        int prevBin = startBin;
        double prevBinContent = startBinContent;

        int nextBin = startBin + 1;
        double nextBinContent = projX->GetBinContent ( nextBin );
        bool foundIncrease = false;

        int counter = 0;

        while ( nextBin < projX->GetXaxis()->GetLast() && counter < counterMax )
        {
            if ( nextBinContent < startBinContent*0.2 )
            {
                fellBelow20Perc = true;
                binShoulder = startBin;
                break;
            }

            if ( nextBinContent > prevBinContent )
            {
                if ( !foundIncrease ) foundIncrease = true;
                else break;
            }

            prevBin = nextBin;
            prevBinContent = nextBinContent;

            nextBin++;
            nextBinContent = projX->GetBinContent ( nextBin );

            counter++;
        }

        startBin++;
        startBinContent = projX->GetBinContent ( startBin );
    }

    std::cout << "Found the right shoulder at bin #" << binShoulder << " (value = " << projX->GetBinCenter ( binShoulder ) << ")" << std::endl;

    TF1* fitfunc = new TF1 ( Form ( "myfit_%s",input->GetName() ), "[0]*exp(-0.5*((x-[1])/[2])**2)", projX->GetBinCenter ( binShoulder ), projX->GetBinCenter ( binShoulder + counterMax ) );

    fitfunc->FixParameter ( 0, projX->GetBinContent ( binShoulder ) );

    fitfunc->FixParameter ( 0, projX->GetBinContent ( binShoulder ) );
    fitfunc->SetParameter ( 1, projX->GetBinCenter ( binShoulder ) );
    fitfunc->SetParameter ( 2, projX->GetBinCenter ( binShoulder ) - projX->GetBinCenter ( binShoulder - 1 ) );

    projX->Fit ( fitfunc, "QRMN" );

    float rightEdge = fitfunc->GetParameter ( 1 ) + TMath::Sqrt ( -2*pow ( fitfunc->GetParameter ( 2 ),2 ) * TMath::Log ( 0.7 ) );

    std::cout << "Found the right strip edge at " << rightEdge << std::endl;

    return fitfunc;
}

void GoddessCalib::FitStripsEdges ( int projWidth, bool drawResults )
{
    for ( auto itr = resStripsPosCalGraphsMap.begin(); itr != resStripsPosCalGraphsMap.end(); itr++ )
    {
        std::cout << "Retreiving the edges of sector " << itr->first.substr ( 0, itr->first.find ( "_" ) ) << " strip #" << itr->first.substr ( itr->first.find ( "_" ) ) << " ..." << std::endl;

        TF1* lfit = FitLeftEdge ( itr->second, projWidth );
        TF1* rfit = FitRightEdge ( itr->second, projWidth );

        if ( drawResults )
        {
            TCanvas* newCanvas = new TCanvas ( Form ( "c_%s", itr->first.c_str() ) );

            newCanvas->cd();

            GetPosCalProjX ( itr->second, GetPosCalEnBinMax ( itr->second ), projWidth )->Draw();
            lfit->Draw ( "same" );
            rfit->Draw ( "same" );
        }
    }

    return;
}

TGraph* GoddessCalib::PlotSX3ResStripCalGraph ( TTree* tree, bool isUpstream, unsigned short sector, unsigned short strip )
{
    std::string upstreamCond = isUpstream ? "si.isUpstream" : "!si.isUpstream" ;
    std::string cond = "si.isBarrel && " + upstreamCond;

    return PlotSX3ResStripCalGraph ( tree, "si.E1.en.n:si.E1.en.p", sector, strip, cond );
}

void GoddessCalib::PlotSX3ResStripsCalGraphsFromTree()
{
    std::cout << "To generate the graphs for several sectors without drawing them (MUCH faster), call" << std::endl;
    std::cout << "PlotSX3ResStripsCalGraphsFromTree(TTree* tree, bool isUpstream, int nentries, int sector1, int sector2, int sector3, int ....)" << std::endl;
    std::cout << "where \"nenteries\" controls the number of entries to treat (0 == all the entries)" << std::endl;
}

void GoddessCalib::PlotSX3ResStripsCalGraphs()
{
    std::cout << "To plot several sectors in a row, call" << std::endl;
    std::cout << "PlotSX3ResStripsCalGraphs(TTree* tree, bool isUpstream, int sector1, int sector2, int sector3, int ....)" << std::endl;
    std::cout << std::endl;
    std::cout << "You can also change what to plot and specify the conditions by hand by calling" << std::endl;
    std::cout << "PlotSX3ResStripsCalGraphs(TTree* tree, string \"what to plot\", string conditions, sector1, sector2, sector3, ....)" << std::endl;
}

void GoddessCalib::CalibHelp()
{
    std::cout << "To plot the strip X of sector Y, call" << std::endl;
    std::cout << "PlotSX3ResStripCalGraph(TTree* tree, bool isUpstream, int sector1, int strip)" << std::endl;
    std::cout << std::endl;
    PlotSX3ResStripsCalGraphs();
    std::cout << std::endl;
    PlotSX3ResStripsCalGraphsFromTree();
    std::cout << std::endl;
    std::cout << "To write the results of the calibrations made during the last session, call" << std::endl;
    std::cout << "WriteResCalResults(string \"result file name\", string option = \"recreate\")" << std::endl;
    std::cout << std::endl;
    std::cout << "To update a config file with the results of the calibrations made during the last session, call" << std::endl;
    std::cout << "UpdateResParamsInConf(string \"config file name\", bool invertContactMidDet = true, string mode = \"protected\")" << std::endl;
    std::cout << "* invertContactMidDet should be set to \"true\" for SuperX3 because of the way the contacts numbers are incremented" << std::endl;
    std::cout << "* the \"protected\" mode will prevent you to overwrite your config file and generate a new config file from the input..." << std::endl;
    std::cout << "  switch it to \"overwrite\" if you really know what you're doing" << std::endl;
    std::cout << std::endl;
    std::cout << "To read a file containing previous calibration and update it, call" << std::endl;
    std::cout << "DumpFileToResCalMap(string \"previous calib file name\")" << std::endl;
    std::cout << std::endl;

}

ClassImp ( GoddessCalib )