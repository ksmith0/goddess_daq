#include "GoddessData.h"

//ROOT Headers
#include "TDirectory.h"
#include "TFile.h"
#include "TMath.h"

//GEBSort Headers
#include "gdecomp.h"

//GODDESS Headers
#include "Detector.h"
#include "QQQ5.h"
#include "superX3.h"
#include "BB10.h"
#include "LiquidScint.h"

#include <iostream>

GoddessData::GoddessData(std::string configFilename)
{
	config = new GoddessConfig("goddess.position",configFilename);

	orruba = new ORRUBA();
	gammaAnalogTimeDiffs = new std::vector<float>;
	gammaDigitalTimeDiffs = new std::vector<float>;
	gammaEnergies = new std::vector<float>;
	siDetEn = new std::vector<float>;
	siDetID = new std::vector<std::string>;
	siSector = new std::vector<int>;
	siUpstream = new std::vector<bool>;
	siStripEn = new std::vector<float>;
	siStripNum = new std::vector<short>;
	x = new std::vector<float>;
	lr=new std::vector<float>;
	dTG_god = new std::vector<float>;
	ppacde = new std::vector<float>;
	icde1 = new std::vector<float>;
	icde2 = new std::vector<float>;
	icde3 = new std::vector<float>;
	NeutID = new std::vector<float>;
	NeutPSD =  new std::vector<float>;
	NeutEnergy =  new std::vector<float>;
	NeutTAC =  new std::vector<float>;


	gDirectory->pwd();
	TFile *f = gDirectory->GetFile();
	if (!f) {
		std::cerr << "ERROR: Not in a ROOT File?\n";
		return;
	}
	f->cd("/trees");
	tree=new TTree("god","GODDESS Tree");
	tree->Branch("orruba",&orruba);
	tree->Branch("siDetMult",&siDetMult);
	tree->Branch("sectorMult",&sectorMult);
	tree->Branch("gamEn",&gammaEnergies);
	tree->Branch("gamDigitalDt",&gammaDigitalTimeDiffs);
	tree->Branch("gamAnalogDt",&gammaAnalogTimeDiffs);
	tree->Branch("digital",&digital);
	tree->Branch("analog",&analog);
	tree->Branch("siDetEn",&siDetEn);
	tree->Branch("siDetID",&siDetID);
	tree->Branch("siUpstream",&siUpstream);
	tree->Branch("siSector",&siSector);
	tree->Branch("siStripEn",&siStripEn);
	tree->Branch("siStripNum",&siStripNum);
	tree->Branch("siUpstreamMult",&siUpstreamMult);
	tree->Branch("siDownstreamMult",&siDownstreamMult);
	tree->Branch("siAnalogMult",&siAnalogMult);
	tree->Branch("siDigitalMult",&siDigitalMult);
	tree->Branch("siStripContactMult",&siDetContactMult);
	tree->Branch("capUpDetMult",endCapUpstreamDetMult);
	tree->Branch("barlUpDetMult",barrelUpstreamDetMult);
	tree->Branch("capDownDetMult",endCapDownstreamDetMult);
	tree->Branch("barlDownDetMult",barrelDownstreamDetMult);
	tree->Branch("capUpConMult",endCapUpstreamContactMult);
	tree->Branch("capDownConMult",endCapDownstreamContactMult);
	tree->Branch("barlUpConMult",barrelUpstreamContactMult);
	tree->Branch("barlDownConMult",barrelDownstreamContactMult);
	tree->Branch("DAQchannel",&DAQchannel);
	tree->Branch("DAQCh_Energy",DAQCh_Energy,"DAQCh_Energy[400]/F");
	tree->Branch("NeutEnergy",&NeutEnergy);
	tree->Branch("NeutPSD",&NeutPSD);
	tree->Branch("NeutTAC",&NeutTAC);
	tree->Branch("NeutID",&NeutID);
	tree->Branch("Neutron",&Neutron);
	tree->Branch("X",&x);
	tree->Branch("LR",&lr);
	tree->Branch("ppacde",&ppacde);
	tree->Branch("icde1",&icde1);
	tree->Branch("icde2",&icde2);
	tree->Branch("icde3",&icde3);
	tree->Branch("dTG_god",&dTG_god);	
	tree->Branch("DTFlag",&timeFlag);

	corr=new TTree("corr","Correlated Particle Gamma");

	// ORRUBA histograms
	f->cd("/hists");
	TDirectory *dirOrruba = gDirectory->mkdir("orruba");
	dirOrruba->cd();
	TDirectory *dirRaw = gDirectory->mkdir("raw");
	dirRaw->cd();
	enRawA = new TH2F("enRawA","Raw Analog Energies;Energy [Ch];Channel",4196,0,4096,400,0,400);
	enRawD = new TH2F("enRawD","Raw Digital Energies;Energy [Ch];Channel",4196,0,1E6,400,0,400);
	enCalA = new TH2F("enCalA","Calibrated Analog Energies;Energy [MeV];Channel",4196,0,4096,400,0,400);
	enCalD = new TH2F("enCalD","Calibrated Digital Energies;Energy [MeV];Channel",4196,0,4096,400,0,400);
	analogMult = new TH1F("analogMult", "Number of triggers in the analog system;Multiplicty;Counts / Multiplicty", 5,0,5);
	analogADCMult = new TH1F("analogADCMult", "Number of ADCs readout per trigger;Multiplicty;Counts / Multiplicty", 400,0,400);
	digitalMult = new TH1F("digitalMult", "Number of triggers in the digital system;Multiplicty;Counts / Multiplicty", 400,0,400);
	hDetPosMult = new TH1F("detPosMult", "Number of sectors hit in an event;Sector Multiplicity;Counts / Multiplicity", 32,0,32);
	detMult = new TH1F("detMult", "Number of detectors hit in an event; Detector Multiplicity;Counts / Multiplicty",48,0,48);

	dirOrruba->cd();
	endcapHitPatternUpstream = new TH2F("hitEndcapUp","Upstream Endcap Hit Pattern",16,0,TMath::TwoPi(),32,0,32);
	endcapHitPatternDownstream = new TH2F("hitEndcapDown","Downstream Endcap Hit Pattern",16,0,TMath::TwoPi(),32,0,32);
	sX3HitPattern = new TH2F("sX3HitPattern","Cumulative SuperX3 Hit Pattern;Azimuthal Angle [deg];Z Position [mm]",48,0,360,8,-80,80);


	dirOrruba->cd();
	InitSuperX3Hists();

	dirOrruba->cd();
	InitQQQ5Hists();

	dirOrruba->cd();
	InitBB10Hists();

	dirOrruba->cd();
	InitGammaHists();

	gDirectory->cd("/hists");
	TDirectory *dirLiquidScint = gDirectory->mkdir("LiquidScint");
	dirLiquidScint->cd();
	InitLiquidScintHists();

}
void GoddessData::InitBB10Hists() {
	TClonesArray* bb10s =config->GetBB10s();
	int nbb10s = bb10s->GetEntries();
	TDirectory *dirBB10 = gDirectory->mkdir("bb10");
	
	for (int i = 0; i < nbb10s; i++) {
	  BB10* bb10 = (BB10*) bb10s->At(i);
	        std::string name = bb10->GetPosID();
		int daqType = bb10->GetDAQType();
		dirBB10->cd();
		TDirectory *dirDet = gDirectory->mkdir(name.c_str());
		dirDet->cd();
		gDirectory->mkdir("en")->cd();

		int maxRawEn = 4096;
		int BB10_Enbins = 4096;
		if (daqType == GEB_TYPE_DFMA) maxRawEn = 5e5;
		BB10RawEn.emplace(name,new TH2F(Form("BB10RawEn_%s",name.c_str()),Form("Raw BB10 %s energy per strip;Energy [Ch];Channel",name.c_str()), BB10_Enbins,0,maxRawEn,8,0,8));
		BB10CalEn.emplace(name,new TH2F(Form("BB10CalEn_%s",name.c_str()),Form("Cal BB10 %s energy per strip;Energy [Ch];Channel",name.c_str()), BB10_Enbins,0,15,8,0,8));
		
	}
}

void GoddessData::InitQQQ5Hists() {

	TClonesArray *qqq5s=config->GetQQQ5s();
	int nqqq5s = qqq5s->GetEntries();
	TDirectory *dirQQQ5 = gDirectory->mkdir("qqq5");

	for (int i = 0; i < nqqq5s; i++) {
	  QQQ5* qqq5 = (QQQ5*)qqq5s->At(i);
		std::string name = qqq5->GetPosID();
		int daqType = qqq5->GetDAQType();
		dirQQQ5->cd();
		TDirectory *dirDet = gDirectory->mkdir(name.c_str());
		dirDet->cd();
		gDirectory->mkdir("en")->cd();

		int maxRawEn = 4096;
		int qqq5_Enbins = 4096;
		if (daqType == GEB_TYPE_DFMA) maxRawEn = 5e5;
		
		QQQenRawFront.emplace(name,new TH2F(Form("QQQenRawFront_%s",name.c_str()),Form("Raw QQQ5 %s energy per front strip;Energy [Ch];Channel",name.c_str()), qqq5_Enbins,0,maxRawEn,32,0,32));

		QQQenCalFront.emplace(name,new TH2F(Form("QQQenCalFront_%s",name.c_str()),Form("Cal QQQ5 %s energy per front strip;Energy [MeV];Channel",name.c_str()), qqq5_Enbins,0,15,32,0,32));

		QQQenRawBack.emplace(name,new TH2F(Form("QQQenRawBack_%s",name.c_str()),Form("Raw QQQ5 %s energy per back strip;Energy [Ch];Channel",name.c_str()), qqq5_Enbins,0,maxRawEn,4,0,4));

		QQQenCalBack.emplace(name,new TH2F(Form("QQQenCalBack_%s",name.c_str()),Form("Cal QQQ5 %s energy per back strip;Energy [MeV];Channel",name.c_str()), qqq5_Enbins,0,15,4,0,4));

		dirDet->cd();
		gDirectory->mkdir("mult")->cd();

		QQQHitPat.emplace(name,new TH2F(Form("QQQHitPat_%s",name.c_str()),Form("QQQ5 Hit Pattern %s;Front [strip];Back [strip]",name.c_str()),32,0,32,4,0,4));
		QQQFrontMult.emplace(name,new TH1F(Form("QQQFrontMult_%s",name.c_str()),Form("QQQ5 %s multiplicity;multiplicity",name.c_str()),20,0,20));
		QQQBackMult.emplace(name,new TH1F(Form("QQQBackMult_%s",name.c_str()),Form("QQQ5 %s multiplicitymultiplicity",name.c_str()),20,0,20));
	}

}
void GoddessData::InitSuperX3Hists() {
	TDirectory *dirSX3 = gDirectory->mkdir("sx5");

	TClonesArray *sx3s = config->GetSuperX3s();
	int nsx3s = sx3s->GetEntries();

	for (int i=0;i< nsx3s;i++) {
		std::string name = ((superX3*)sx3s->At(i))->GetPosID();
		dirSX3->cd();

		superX3* sx3 = (superX3*)sx3s->At(i);
		int daqType = sx3->GetDAQType();
		
		TDirectory *dirDet = gDirectory->mkdir(name.c_str());
		dirDet->cd();
		gDirectory->mkdir("en")->cd();

		int maxRawEn = 4096;
		int sx3_Enbins = 4096;
		if (daqType == GEB_TYPE_DFMA) maxRawEn = 5e5;

		sX3stripEnRaw[name] = new TH2F(Form("sX3stripEnRaw%s",name.c_str()),
			Form("SuperX3 strip raw energy vs strip %s;energy [ch];strip", name.c_str()),sx3_Enbins,0,maxRawEn, 8, 0,8);
		sX3stripEnCal[name] = new TH2F(Form("sX3stripEnCal%s",name.c_str()),
			Form("SuperX3 strip cal energy vs strip %s;energy [keV];strip", name.c_str()),sx3_Enbins,0,maxRawEn, 8, 0,8);
		sX3backEnRaw[name] = new TH2F(Form("sX3backEnRaw%s",name.c_str()),
			Form("SuperX3 back raw energy vs strip %s;energy [ch];strip", name.c_str()),sx3_Enbins,0,maxRawEn, 4, 0,4);
		sX3backEnCal[name] = new TH2F(Form("sX3backEnCal%s",name.c_str()),
			Form("SuperX3 back cal energy vs strip %s;energy [keV];strip", name.c_str()),sx3_Enbins,0,15, 4, 0,4);
		sx3StripTotEn[name] = new TH2F(Form("sX3StripTotEn%s",name.c_str()),Form("SuperX3 total energy vs strip %s;energy [ch];strip", name.c_str()),1024,0,maxRawEn, 4, 0,4);
		
		dirDet->cd();
		gDirectory->mkdir("pos")->cd();

		sX3nearFar[name] = new TH2F*[4];
		sX3nearFarCal[name] = new TH2F*[4];
		sX3posRaw_enRaw[name] = new TH2F*[4];
		sX3posRaw_enCal[name] = new TH2F*[4];
		sX3posCal_enCal[name] = new TH2F*[4];
		
		for (int strip=0; strip < 4; strip++) {
			sX3posRaw_enCal[name][strip] = new TH2F(Form("sX3posRaw_enCal_%s_%i",name.c_str(),strip),
				Form("super X3 raw position vs cal energy %s_%i",name.c_str(),strip),512, -1,1,1024,0,maxRawEn);

			sX3posCal_enCal[name][strip] = new TH2F(Form("sX3posCal_enCal_%s_%i",name.c_str(),strip),
				Form("super X3 cal position vs cal energy %s_%i",name.c_str(),strip),512, -1,1,1024,0,maxRawEn);

			sX3posRaw_enRaw[name][strip] = new TH2F(Form("sX3posRaw_enRaw_%s_%i",name.c_str(),strip),
				Form("super X3 raw position vs raw energy %s_%i",name.c_str(),strip),512, -1,1,1024,0,maxRawEn);

			sX3nearFar[name][strip] = new TH2F(Form("sX3nearFar_%s_%i",name.c_str(),strip),
				Form("sX3 near strip vs far strip %s %i", name.c_str(),strip), 1024,0,maxRawEn,1024,0,maxRawEn);
			sX3nearFarCal[name][strip] = new TH2F(Form("sX3nearFarCal_%s_%i",name.c_str(),strip),
				Form("sX3 near strip vs far calibrated strip %s %i", name.c_str(),strip), 1024,0,maxRawEn,1024,0,maxRawEn);
		}

		dirDet->cd();
		gDirectory->mkdir("mult")->cd();

		sX3HitPat[name] = new TH2F(Form("sX3HitPat_%s",name.c_str()),
			Form("SuperX3 Hit Pattern %s;Front [strip];Back [strip]",name.c_str()),8,0,8,4,0,4);

		sX3frontMult[name] = new TH1F(Form("sX3FrontMult_%s",name.c_str()),
			Form("super X3 %s multiplicity;multiplicity",name.c_str()),20,0,20);

		sX3backMult[name] = new TH1F(Form("sX3BackMult_%s",name.c_str()),
			Form("super X3 %s multiplicity;multiplicity",name.c_str()),20,0,20);

	}

}
void GoddessData::InitLiquidScintHists() {
	std::vector<LiquidScint*> liquids = config->GetLiquidScints();
	int nliquids = liquids.size();

	for (int iLiq=0;iLiq<nliquids;iLiq++) {
		std::string name=liquids[iLiq]->GetDescription();
		LiquidScint_PSD_E[name] = new TH2F(Form("LiquidScint_PSD_E_%s",name.c_str()),Form("Liquid Scintillator PSD vs E %s",name.c_str()),512,0,4096,512,0,4096);
		LiquidScint_psdRaw[name] = new TH1F(Form("LiquidScint_PSD_%s",name.c_str()),Form("Liquid Scintillator PSD %s",name.c_str()),4096,0,4096);
		LiquidScint_enRaw[name] = new TH1F(Form("LiquidScint_E_%s",name.c_str()),Form("Liquid Scintillator E %s",name.c_str()),4096,0,4096);
		LiquidScint_tacRaw[name] = new TH1F(Form("LiquidScint_TAC_%s",name.c_str()),Form("Liquid Scintillator TAC %s",name.c_str()),4096,0,4096);
		hLiqRawEnNeuGate[name] = new TH1F(Form("hLiqRawEnNeuGate_%s",name.c_str()),Form("Liquid Scint Raw E, Neutron Gated %s;Energy [ch]",name.c_str()),4096,0,4096);
		hLiqPSDNeuGate[name] = new TH1F(Form("hLiqPSDNeuGate_%s",name.c_str()),Form("Liquid Scint PSD, Neutron Gated %s;PSD [ch]",name.c_str()),4096,0,4096);
		hLiqTACNeuGate[name] = new TH1F(Form("hLiqTACNeuGate_%s",name.c_str()),Form("Liquid Scint TAC, Neutron Gated %s;TAC [ch]",name.c_str()),4096,0,4096);
		hGamNeuGate[name] = new TH1F(Form("hGamNeuGate_%s",name.c_str()),Form("Gammas, Neutron Gated %s;Energy [ch]",name.c_str()),1000,0,4000);
		hGamVsLiqNeuGate[name] = new TH2F(Form("hGamVsLiqNeuGate_%s",name.c_str()),Form("Gammas vs Liquid E, Neutron Gated %s;Energy [ch]",name.c_str()),512,0,4096,500,0,4000);

	}
}

void GoddessData::InitGammaHists() {
		TDirectory *dirDet = gDirectory->mkdir("gamma");
		dirDet->cd();
		gDirectory->mkdir("gamma")->cd();

		upstreamGam= new TH1F("upstreamGam","gammas gated on upstream particles", 4096, 0, 4096);
		downstreamGam= new TH1F("downstreamGam","gammas gated on downstream particles", 4096, 0, 4096);
}

void GoddessData::Fill(std::vector<DGSEVENT> *dgsEvts, std::vector<DFMAEVENT> *dgodEvts, std::vector<AGODEVENT> *agodEvts) {
	//Map of channels to suppress, This occurs if they were not found in the map.
	static std::map<std::pair<short, short>, bool> suppressCh;

	analogMult->Fill(agodEvts->size());
	// getting data from analog events
	for (size_t i=0;i<agodEvts->size();i++) {
		AGODEVENT agodEvt = agodEvts->at(i);

		analogADCMult->Fill(agodEvt.values.size());
		for (size_t j=0;j<agodEvt.values.size();j++) {
			unsigned long int value = agodEvt.values[j];
			unsigned short channel = agodEvt.channels[j];
			DAQchannel=channel;
			DAQCh_Energy[channel] = value;
			//unsigned long long timestamp = agodEvt.timestamp;

			enRawA->Fill(value,channel);

			std::pair<short, short> key = std::make_pair(GEB_TYPE_AGOD, channel);
			if (suppressCh.find(key) != suppressCh.end()) continue;

			Detector *det = config->SetRawValue(GEB_TYPE_AGOD, channel, value);
			if (!det) {
				suppressCh[key] = true;
				continue;
			}	
			//det->SetTimestamp(timestamp);

			std::string posID;
			orrubaDet* siDet = dynamic_cast<orrubaDet*>(det);
			IonChamber *ionChamber_ = dynamic_cast<IonChamber*>(det);
			LiquidScint *liquidScint_ = dynamic_cast<LiquidScint*>(det);
			if (siDet) {
				analog = true;
				posID = siDet->GetPosID();
				//We only push the detector back onto the silicon stack if we haven't already added it.
				if(siDets.find(posID)==siDets.end()) {
					siDets[posID] = siDet;
					siAnalogMult++;
				}
			}
			else if (liquidScint_) {
			        Neutron = true;
				posID = liquidScint_->GetDescription();
				liquidScints[posID] = liquidScint_;
			}
			else if (ionChamber) {
				posID = "ion";
				ionChamber = ionChamber_;
			}

			firedDets[posID] = det;
		}
	}

	digitalMult->Fill(dgodEvts->size());
	// getting data from digital events	
	for (size_t i=0;i<dgodEvts->size();i++) {
		DFMAEVENT dgodEvt = dgodEvts->at(i);
		unsigned int value=dgodEvt.ehi;
		unsigned short channel=dgodEvt.tid;
		//unsigned long long timestamp = dgodEvt.LEDts;

		DAQchannel=channel;
		//DAQCh_Energy[channel] = value; //filling this will overwrite the analog
		enRawD->Fill(value,channel);

		std::pair<short,short> key=std::make_pair(GEB_TYPE_DFMA,channel);
		if (suppressCh.find(key) != suppressCh.end()) {
			continue;
		}

		Detector *det=config->SetRawValue(GEB_TYPE_DFMA,channel,value);
		if (!det) {
			suppressCh[key]=true;
			continue;
		}
		siDetContactMult++;

		//Take whatever the timestamp is for this channel.
		//	This is not clear that it is the best method as one detecotr may have various 
		//	timestamps
		//det->SetTimestamp(timestamp);

		std::string posID;
		orrubaDet* siDet = dynamic_cast<orrubaDet*>(det);
		IonChamber *ionChamber_ = dynamic_cast<IonChamber*>(det);
		LiquidScint *liquidScint_ = dynamic_cast<LiquidScint*>(det);
		if (siDet) {
			digital = true;
			posID = siDet->GetPosID();
			//We only push the detector back onto the silicon stack if we haven't already added it.
			if(siDets.find(posID)==siDets.end()) {
				siDets[posID] = siDet;
				siDigitalMult++;
			}
		}
		else if (liquidScint_) {
			posID = liquidScint_->GetDescription();
			liquidScints[posID] = liquidScint_;
		}
		else if (ionChamber) {
			posID = "ion";
			ionChamber = ionChamber_;
		}

		firedDets[posID] = det;
	}

	FillHists(dgsEvts);
	FillTrees(dgsEvts,dgodEvts,agodEvts);

	//We clear everything here since we know what was actually fired.
	for (auto itr = firedDets.begin(); itr != firedDets.end(); ++itr) {
		itr->second->Clear();
	}
	firedDets.clear();
	siDets.clear();
	liquidScints.clear();
	
	for (int i = 0; i < 400; i++) {DAQCh_Energy[i] = 0.0;}
	
}

void GoddessData::FillHists(std::vector<DGSEVENT> *dgsEvts) {
	std::map<std::string, int> numSectorHits;

	unsigned short numDetsOverThresh = 0;
	// loop over fired detectors
	for (auto detItr=siDets.begin();detItr!=siDets.end(); ++detItr) {
		//Get detector pointer from iteratot
		orrubaDet* det = detItr->second;
		//Get some information about the detector.
		std::string detPosID = det->GetPosID();
		std::string detType = det->IsA()->GetName();
		//unsigned short sector = det->GetSector();
		
		if (det->GetContactMult()) numDetsOverThresh++;

		//Get the maps of the raw and calibrated energies for front and back.
		// Each map has the channel that fired as the key.
		siDet::ValueMap frontRawEn = det->GetRawEn(siDet::pType);
		siDet::ValueMap frontCalEn = det->GetCalEn(siDet::pType);
		siDet::ValueMap backRawEn = det->GetRawEn(siDet::nType);
		siDet::ValueMap backCalEn = det->GetCalEn(siDet::nType);

		//Handle BB10 detectors.
		if(detType =="BB10"){
		  //---Raw Energy---
		  for (auto itr=frontRawEn.begin(); itr!=frontRawEn.end();++itr) {
		    BB10RawEn[detPosID]->Fill(itr->second, itr->first);
		  }
		  //---Cal Energy---
		  for (auto itr=frontCalEn.begin(); itr!=frontCalEn.end();++itr) {
		    BB10CalEn[detPosID]->Fill(itr->second, itr->first);
		  }
		}
		//Handle QQQ5 detectors.
		else if (detType == "QQQ5") {
			//---Raw Energy---
			for (auto itr=frontRawEn.begin(); itr!=frontRawEn.end();++itr) {
			        QQQenRawFront[detPosID]->Fill(itr->second, itr->first);
			}
			for (auto itr=backRawEn.begin(); itr!=backRawEn.end();++itr) {
				QQQenRawBack[detPosID]->Fill(itr->second, itr->first);
			}

			//Lets ignore the hits with all strips below threhsold.
			if (det->GetContactMult() == 0) {
				continue;
			}

			//---Multiplicty---
			QQQFrontMult[detPosID]->Fill(frontCalEn.size());
			QQQBackMult[detPosID]->Fill(backCalEn.size());

			//---Cal Energy---
			for (auto itr=frontCalEn.begin(); itr!=frontCalEn.end();++itr) {
				QQQenCalFront[detPosID]->Fill(itr->second, itr->first);
			}
			for (auto itr=backCalEn.begin(); itr!=backCalEn.end();++itr) {
				QQQenCalBack[detPosID]->Fill(itr->second, itr->first);
			}

			//---Hit Patterns---
			TH2F *endcapHitPattern;
			if (det->GetUpStream()) endcapHitPattern = endcapHitPatternUpstream;
			else endcapHitPattern = endcapHitPatternDownstream;
			for (auto itrFront=frontCalEn.begin();itrFront!=frontCalEn.end();++itrFront) {
				for (auto itrBack=backCalEn.begin();itrBack!=backCalEn.end();++itrBack) {
					QQQHitPat[detPosID]->Fill(itrFront->first,itrBack->first);
					float angle = ((QQQ5*)det)->GetAzimuthalBins()[itrBack->first] * TMath::DegToRad() + TMath::Pi()/16;
					endcapHitPattern->Fill(angle,itrFront->first);
				}
			}
		} //Done with QQQ5 detectors
		//Handle Super X3 detectors
		else if (detType == "superX3") {
			superX3 *sx3= (superX3*) det;

			//---Raw Energy---
			for (auto itr=frontRawEn.begin(); itr!=frontRawEn.end();++itr) {
				sX3stripEnRaw[detPosID]->Fill(itr->second, itr->first);
			}
			for (auto itr=backRawEn.begin(); itr!=backRawEn.end();++itr) {
				sX3backEnRaw[detPosID]->Fill(itr->second,itr->first);
			}
			
			//---Raw Positions---
			//for loop over 8 contacts/4 strips
			for (int i=0;i<4;i++) {
			  unsigned short near = sx3->GetNearContact(i);
			  unsigned short far = sx3->GetFarContact(i);
			  if ( (frontRawEn.find(near)!=frontRawEn.end()) && (frontRawEn.find(far)!=frontRawEn.end()) ) {
				  sX3posRaw_enRaw[detPosID][i]->Fill( ((frontRawEn[far]-frontRawEn[near])/(frontRawEn[far]+frontRawEn[near])) , (frontRawEn[far] + frontRawEn[near]) );
				  sX3nearFar[detPosID][i]->Fill(frontRawEn[far], frontRawEn[near]);
				  sx3StripTotEn[detPosID]->Fill(frontRawEn[far]+frontRawEn[near], i );
			  }
			}	

			//Lets ignore the detectors with all strips below threhsold.
			if (det->GetContactMult() == 0) {
				continue;
			}
			sX3frontMult[detPosID]->Fill(frontCalEn.size());
			sX3backMult[detPosID]->Fill(backCalEn.size());
			
			//---Cal Energy---
			for (auto itr=frontCalEn.begin(); itr!=frontCalEn.end();++itr) {
				sX3stripEnCal[detPosID]->Fill(itr->second, itr->first);
			}
			for (auto itr=backCalEn.begin(); itr!=backCalEn.end();++itr) {
				sX3backEnCal[detPosID]->Fill(itr->second,itr->first);
			}

			
			//for loop over 8 contacts/4 strips
			/// -- Trying to take the values that were further calibrated in superX3 class 
			/// -- and fill histograms here. All are returning zeros.
			for (int i=0;i<4;i++) {
			  unsigned short near = sx3->GetNearContact(i);
			  unsigned short far = sx3->GetFarContact(i);
			        if ((frontCalEn.find(near)!=frontCalEn.end()) && (frontCalEn.find(far)!=frontCalEn.end())) {
				  sX3posRaw_enCal[detPosID][i]->Fill(sx3->GetStripPosRaw()[i],sx3->GetStripEnergies()[i]);
				  sX3posCal_enCal[detPosID][i]->Fill(sx3->GetStripPosCal()[i],sx3->GetStripEnergies()[i]);
				  sX3nearFarCal[detPosID][i]->Fill(sx3->GetNearCalEnergy()[i],sx3->GetFarCalEnergy()[i]);
				}
			}
			
			numSectorHits[detPosID]++;
			// hit pattern
			for (auto itrFront=frontCalEn.begin();itrFront!=frontCalEn.end();++itrFront) {
				for (auto itrBack=backCalEn.begin();itrBack!=backCalEn.end();++itrBack) {
					sX3HitPat[detPosID]->Fill(itrFront->first,itrBack->first);
					float angle = ((superX3*)det)->GetAzimuthalCenterBins()[itrFront->first];
					float zPos = ((superX3*)det)->GetZCenterBins()[itrBack->first];
					sX3HitPattern->Fill(angle,zPos);
				}
			}
		} //Done with Super X3 detectors

		//Loop over the DGS events.
		for (size_t i=0;i<dgsEvts->size();i++) {
			//Fill a histogram based on upstream / downstream.
			if (det->GetUpStream()) {
				upstreamGam->Fill(dgsEvts->at(i).ehi);
			} else {
				downstreamGam->Fill(dgsEvts->at(i).ehi);
			}
		}
	} //Finished looping over fired silicon detectors.
	
	//Begin loop of liquid scintillators.
	for (auto lsItr=liquidScints.begin();lsItr!=liquidScints.end();++lsItr) {
	  std::string description = lsItr->first;
	  LiquidScint* liquidScint = lsItr->second;

		float rawEnergy = liquidScint->GetRawEnergy();
		float psd_ = liquidScint->GetRawPSD();
		float tac_ = liquidScint->GetRawTAC();
		
		NeutEnergy->push_back(rawEnergy);
		NeutPSD->push_back(psd_);
		NeutTAC->push_back(tac_);
		if(description =="90deg") NeutID->push_back(1);
		else NeutID->push_back(2);
				
		//Fill Raw properties.
		LiquidScint_PSD_E[description]->Fill(rawEnergy,psd_);
		LiquidScint_enRaw[description]->Fill(rawEnergy);
		LiquidScint_psdRaw[description]->Fill(psd_);
		if(tac_ != 0){
			LiquidScint_tacRaw[description]->Fill(tac_);
		}

		//Rough neutron gates based on PSD and Timing.
		if ((description == "90deg" && psd_ > 350 && tac_ < 660) || (description == "downstream" && psd_ > 725 && tac_ > 2141)) {
			hLiqRawEnNeuGate[description]->Fill(rawEnergy);
			hLiqPSDNeuGate[description]->Fill(psd_);
			hLiqTACNeuGate[description]->Fill(tac_);
			for (size_t i=0;i<dgsEvts->size();i++) {
				hGamNeuGate[description]->Fill(dgsEvts->at(i).ehi);
				hGamVsLiqNeuGate[description]->Fill(rawEnergy,dgsEvts->at(i).ehi);
			}
		}
	} //End loop over liquid scintillators.

	//Fill the detector and position multiplicty histograms.
	detMult->Fill(numDetsOverThresh);
	hDetPosMult->Fill(numSectorHits.size());	
}

void GoddessData::FillTrees(std::vector<DGSEVENT> *dgsEvts, std::vector<DFMAEVENT> *dgodEvts, std::vector<AGODEVENT> *agodEvts) {

	for (auto detItr=siDets.begin();detItr!=siDets.end(); ++detItr) {
		orrubaDet* det = detItr->second;
		//Skip detectors with no contacts above threshold.
		if (det->GetContactMult() == 0) continue;
		std::string detPosID = det->GetPosID();
		std::string detType = det->IsA()->GetName();

		siDetID->push_back(detPosID);
		siSector->push_back(det->GetSector());
		siUpstream->push_back(det->GetUpStream());
		if(det->GetUpStream()) {
			siUpstreamMult++;
			if (detType == "QQQ5") {
				endCapUpstreamDetMult[det->GetSector()]++;
				endCapUpstreamContactMult[det->GetSector()] += det->GetContactMult(siDet::pType);
				endCapUpstreamContactMult[det->GetSector()] += det->GetContactMult(siDet::nType);
			}
			else {
				barrelUpstreamDetMult[det->GetSector()]++;
				barrelUpstreamContactMult[det->GetSector()] += det->GetContactMult(siDet::pType);
				barrelUpstreamContactMult[det->GetSector()] += det->GetContactMult(siDet::nType);
			}
		} else {
			if (detType == "QQQ5") {
				endCapDownstreamDetMult[det->GetSector()]++;
				endCapDownstreamContactMult[det->GetSector()] += det->GetContactMult(siDet::pType);
				endCapDownstreamContactMult[det->GetSector()] += det->GetContactMult(siDet::nType);
			}
			else {
				barrelDownstreamDetMult[det->GetSector()]++;
				barrelDownstreamContactMult[det->GetSector()] += det->GetContactMult(siDet::pType);
				barrelDownstreamContactMult[det->GetSector()] += det->GetContactMult(siDet::nType);
			}
			siDownstreamMult++;
		}
		siDetEn->push_back(det->GetEnergy());


	}

	
	for (unsigned int dgsEvtNum=0;dgsEvtNum<(dgsEvts->size());dgsEvtNum++) {
	  // type 1 = Germanium detectors, type 2 = BGO
	  if ((int)dgsEvts->at(dgsEvtNum).tpe ==GE) 
	    {
	      gammaEnergies->push_back(dgsEvts->at(dgsEvtNum).ehi);	
	    }

	}
 	
	for (unsigned int dgsEvtNum=0;dgsEvtNum<dgsEvts->size();dgsEvtNum++) {
		for (size_t i=0;i<agodEvts->size();i++) {
			int dT = double(dgsEvts->at(dgsEvtNum).event_timestamp) - double(agodEvts->at(i).timestamp);
			gammaAnalogTimeDiffs->push_back(dT);
		}
		for (size_t i=0;i<dgodEvts->size();i++) {
			int dT = double(dgsEvts->at(dgsEvtNum).event_timestamp) - double(dgodEvts->at(i).LEDts);
			gammaDigitalTimeDiffs->push_back(dT);
		}
	}
	

	//Build a total ORRUBA event
	float dE=0, E1=0, E2=0;
	TVector3 pos(0,0,0);
	std::string sector;
	bool valid = true;
	sectorMult = 0;
	for (auto itr=siDets.begin();itr!=siDets.end(); ++itr) {
		orrubaDet* det = itr->second;
		std::string detPosID = det->GetPosID();
		unsigned short depth = det->GetDepth();
		std::string detType = det->IsA()->GetName();
		std::string detPosSector = detPosID.substr(0, detPosID.length() - 3);

		siDet::ValueMap frontRawEn = det->GetRawEn(siDet::pType);
		siDet::ValueMap backRawEn = det->GetRawEn(siDet::nType);
		for (auto itrStrip=frontRawEn.begin();itrStrip!=frontRawEn.end();++itrStrip) {
			siStripEn->push_back(itrStrip->second);
			siStripNum->push_back(itrStrip->first);
		}
		for (auto itrStrip=backRawEn.begin();itrStrip!=backRawEn.end();++itrStrip) {
			siStripEn->push_back(itrStrip->second);
			siStripNum->push_back(itrStrip->first + det->GetNumChannels(siDet::pType));
		}


		if (sector.empty()) {
			sector = detPosSector;
			sectorMult++;
		}
		//If we had detectors from different quadrants we give up on the event.
		else if (sector != detPosSector) {
			sectorMult++;
			valid = false;
		}
		if (depth == 0) {
			if (!dE) dE = det->GetEnergy();
		}
		else if (depth == 1) {
			if (!E1) E1 = det->GetEnergy();
		}
		else if (depth == 2) {
			if (!E2) E2 = det->GetEnergy();
		}
		if (detType == "superX3" || detType == "QQQ5") {
			//Save the position if a position has not been saved or
			//	if this is a QQQ5 dE layer.
			if (!pos.Mag2() || (detType == "QQQ5" && depth == 0))
				pos = det->GetEventPosition();
		}
	} 
	if (valid) {
		orruba->SetEvent(sector,dE,E1,E2,pos);
	}

	unsigned int left =0;
	unsigned int right = 0;
	for (unsigned int i=0;i<dgodEvts->size();i++) {
	  if(dgodEvts->at(i).tpe == FP){
	    dgodEvts->at(i).ehi=dgodEvts->at(i).ehi/30;
	    if (dgodEvts->at(i).tid==1) { left=dgodEvts->at(i).ehi;}
	    if (dgodEvts->at(i).tid==2) { right=dgodEvts->at(i).ehi;}
	    if (dgodEvts->at(i).tid==3) { ppacde->push_back(dgodEvts->at(i).ehi); }
	    if (dgodEvts->at(i).tid==5) { icde1->push_back(dgodEvts->at(i).ehi); }
	    if (dgodEvts->at(i).tid==6) { icde2->push_back(dgodEvts->at(i).ehi);}
	    if (dgodEvts->at(i).tid==7) { icde3->push_back(dgodEvts->at(i).ehi); }
	    if(left>0 && right>0){
	      x->push_back(left-right+8000);
	      lr->push_back(left+right);
	    }
	  }
	  if( (dgodEvts->at(i).LEDts > 0) && (dgodEvts->at(i).tpe == FP) ){
	    for (unsigned int j=0;j<dgodEvts->size();j++) {
	      if(dgodEvts->at(j).tpe == DSSD && dgodEvts->at(j).tid>0 && dgodEvts->at(j).tid<107 ){
		  dTG_god->push_back( (double)(dgodEvts->at(j).LEDts) - (double)(dgodEvts->at(i).LEDts));
		  if( (double)(dgodEvts->at(j).LEDts) - (double)(dgodEvts->at(i).LEDts) > 250 && (double)(dgodEvts->at(j).LEDts) - (double)(dgodEvts->at(i).LEDts) < 280){
		    timeFlag = 1;
		  }
		  else timeFlag =0;
	      }
	    }

	  }
    
	}



	siDetMult = siDets.size();
	tree->Fill();
	
	NeutEnergy->clear();
	NeutPSD->clear();
	NeutID->clear();
	NeutTAC->clear();
	lr->clear();
	x->clear();
	dTG_god->clear();
	ppacde->clear();
	icde1->clear();
	icde2->clear();
	icde3->clear();
	gammaEnergies->clear();
	gammaAnalogTimeDiffs->clear();
	gammaDigitalTimeDiffs->clear();
	siDetID->clear();
	siDetEn->clear();
	siSector->clear();
	siStripNum->clear();
	siStripEn->clear();
	siUpstream->clear();
	siAnalogMult=0;
	siDigitalMult=0;
	siDetContactMult=0;
	siDownstreamMult=0;
	siUpstreamMult=0;
	for (int i=0;i<4;i++) {
		endCapUpstreamDetMult[i] = 0;
		endCapDownstreamDetMult[i] = 0;
		endCapUpstreamContactMult[i] = 0;
		endCapDownstreamContactMult[i] = 0;
	}
	for (int i=0;i<12;i++) {
		barrelUpstreamDetMult[i] = 0;
		barrelDownstreamDetMult[i] = 0;
		barrelUpstreamContactMult[i] = 0;
		barrelDownstreamContactMult[i] = 0;
	}
	analog = false;
	digital = false;
}

