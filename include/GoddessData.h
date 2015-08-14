#ifndef GODDESSDATA_H
#define GODDESSDATA_H

#include "GoddessConfig.h"
#include "ORRUBA.h"
#include "orrubaDet.h"
#include "IonChamber.h"
#include "LiquidScint.h"


#include "TH1F.h"
#include "TH2F.h"
#include "TTree.h"

#include "GTMerge.h"

#include <vector>
#include <array>
#include <map>
#include <utility>

class GoddessData {
	public:
		GoddessData(std::string configFilename);
		~GoddessData();
		
		void Fill(std::vector<DGSEVENT> *dgsEvts, std::vector<DFMAEVENT> *dgodEvts, std::vector<AGODEVENT> *agodEvt);
	
	private:
		struct GammaData {
			float energy;
			unsigned long long timestamp;
		};

		GoddessConfig *config;

		ORRUBA *orruba;

		void InitLiquidScintHists();
		void InitSuperX3Hists();
		void InitQQQ5Hists();
		void InitBB10Hists();
		void InitGammaHists();

		void FillTrees(std::vector<DGSEVENT> *dgsEvts, std::vector<DFMAEVENT> *dgodEvts, std::vector<AGODEVENT> *agodEvt);
		void FillHists(std::vector<DGSEVENT> *dgsEvts);

		std::map<std::string,Detector*> firedDets;
		std::map<std::string,orrubaDet*> siDets;
		IonChamber *ionChamber;
		std::map<std::string,LiquidScint*> liquidScints;

		TH2F* enRawA;
		TH2F* enRawD;
		TH2F* enCalA;
		TH2F* enCalD;

		// QQQ5s
		std::map<std::string,TH2F*> QQQenRawFront;
		std::map<std::string,TH2F*> QQQenCalFront;
		std::map<std::string,TH2F*> QQQenRawBack;
		std::map<std::string,TH2F*> QQQenCalBack;

		std::map<std::string,TH2F*> QQQHitPat;
		std::map<std::string,TH1F*> QQQFrontMult;
		std::map<std::string,TH1F*> QQQBackMult;

		// superX3s
		std::map<std::string,TH2F*> sX3stripEnCal;
		std::map<std::string,TH2F*> sX3stripEnRaw;
		std::map<std::string,TH2F*> sX3backEnCal;
		std::map<std::string,TH2F*> sX3backEnRaw;
		std::map<std::string,TH2F*> sX3near_far;
		std::map<std::string,TH2F*> sX3enCal_posRaw;
		std::map<std::string,TH2F*> sX3enCal_posCal;
		std::map<std::string,TH2F*> sX3HitPat;
		std::map<std::string,TH1F*> sX3frontMult;
		std::map<std::string,TH1F*> sX3backMult;
		std::map<std::string,TH2F**> sX3nearFar;
		std::map<std::string,TH2F**> sX3posRaw_enRaw;
		std::map<std::string,TH2F**> sX3posRaw_enCal;
		std::map<std::string,TH2F**> sX3posCal_enCal;

		// IC
		std::map<std::string,TH1F*> icE1;
		std::map<std::string,TH1F*> icE2;
		std::map<std::string,TH1F*> icE;
		std::map<std::string,TH1F**> scint_T;
		std::map<std::string,TH1F**> scint_E;

		// gammas
		TH1F* upstreamGam;
		TH1F* downstreamGam;

		// liquid scint
		std::map<std::string,TH2F*> LiquidScint_PSD_E;
		std::map<std::string,TH1F*> LiquidScint_tof;



		TTree* tree;
		std::vector<float> *gammaEnergies;
		std::vector<float> *gammaAnalogTimeDiffs;
		std::vector<float> *gammaDigitalTimeDiffs;
		std::vector<float> *siStripEn;
		std::vector<short> *siStripNum;
		unsigned int siDetMult;
		unsigned int siDownstreamMult;
		unsigned int siUpstreamMult;
		unsigned int siDetContactMult;
		unsigned int sectorMult;
		unsigned int siAnalogMult;
		unsigned int siDigitalMult;
		std::vector<float> *siDetEn;
		std::vector<std::string> *siDetID;
		std::vector<int> *siSector;
		std::vector<bool> *siUpstream;

		bool analog;
		bool digital;

};

#endif
