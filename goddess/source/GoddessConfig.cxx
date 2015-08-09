#include "GoddessConfig.h"
#include <iostream>
#include <fstream>
#include <sstream>

#include "TClass.h"
#include "TMath.h"

#include "superX3.h"
#include "BB10.h"
#include "QQQ5.h"

/**
 *
 * \param[in] positionFile The configuration file specfying the central position 
 *  of each sector.
 * \param[in] configFile The configuration file specifying which detectors are where
 *  and their calibration parameters.
 *
 */
GoddessConfig::GoddessConfig(std::string positionFile, std::string configFile) {

	superX3s = new TClonesArray("superX3",0);
	bb10s = new TClonesArray("BB10",0);
	qqq5s = new TClonesArray("QQQ5",0);

	ReadPosition(positionFile);
	ReadConfig(configFile);
}	

GoddessConfig::~GoddessConfig() {
	for(auto iterator = chMap.begin(); iterator != chMap.end(); iterator++) {	
		delete (*iterator).second.first;
	}
	chMap.clear();
}

/**The GODDESS configuration file specifies the silicon and ion/scint detector 
 * configuration. 
 *
 * For the silicon detectors the supported types are: superX3, BB10, and QQQ5. A 
 * typical configuration would read as follows:
 * superX3 1234-56 U0dE 13 0 19 8
 *  enCal p 0 1 2 3 
 *  enCal p 1 1 2 3 
 *  posCal resStrip 0 1 2 3 
 *
 * This configuration example defines a superX3 detector with serial number 1234-56
 * in the upstream position of sector 0 as an energy-loss detector. The detctor p-type sides are
 * connected to DAQ type 13, channel 0, and the n-type  channels are connected to DAQ type 19, 
 * channel 8. Following this are a list of energy or position calibrations. Each calibration 
 * specifies the subtype of the channel, either p, n or resStrip for a superX3. This is followed 
 * by the subtype channel number and then the polynomial parameters. An arbitrary 
 * number of parameters can be listed in increasing order. 
 * \note The position calibration is only available for resStrip of superX3 
 *  detectors.
 *
 * Also supported, is an ion detector with a built-in scintillator. An example of a
 * typical configuration follows:
 * ion 5 4 2 3 13 0 19 16 
 *  enCal anode 0 0 1 2
 *  posCal scint 0 4 5 6
 *  timeCal scint 0 2 3 4
 *
 * This configuration defines an ion chamber with 5 anodes and 4 scintillator PMTs.
 * The energy loss signal is composed of the first two anodes and the residual 
 * energy is composed of the following 3 channels. There may be more anodes read out
 * then used in computing these two values. We then have the DAQ type 13, channel 0, for the anodes
 * then DAQ type 19, channel 16, for the scintillators. Following this is a list of calibration
 * parameters for energy, position and time. This calibration lines follow the same 
 * format as the silicons with subtypes or anode and scint.
 * \note The position and time calibration are only supported for the scint subtype.
 *
 *
 * \param[in] filename The configuration file to be read.
 */
void GoddessConfig::ReadConfig(std::string filename) {
	std::ifstream mapFile(filename);	

	if (!mapFile.good()) {
		std::cerr << "ERROR: Unable to read configuration file: " << filename << "\n";
		return;
	}

	std::string line;
	//Loop over every line in the config file
	while (std::getline(mapFile, line).good()) {
		//Convert string to stream
		std::istringstream lineStream(line);

		//Read the first line for a detector block
		// This line specifies the following in order:
		// 	type serialNum positionID daqType daqCh
		std::string detType, serialNum = "", id;
		orrubaDet *det = 0;

		if (!(lineStream >> detType)) {
			break;
		}
		//Report the one we are registering.
		std::cout << "Registering " << detType << " detector: ";

		if (detType == "ion") {
			int numAnode, numScint, numDE, numEres;
			int anodeDaqType, anodeDaqCh;
			int scintDaqType, scintDaqCh;
			if (!(lineStream >> numAnode >> numScint >> numDE >> numEres >> anodeDaqType >> anodeDaqCh >> scintDaqType >> scintDaqCh)) {
				break;
			}
			std::cout << " Anodes: " << numAnode << ", Scint PMTs: " << numScint;
			std::cout << " dE: " << numDE << " anodes, Eres " << numEres << "\n";

			ionChamber = new IonChamber(numAnode, numScint, numDE, numEres);
			if (IsInsertable(anodeDaqType, anodeDaqCh, detType, false) && IsInsertable(scintDaqType, scintDaqCh, detType, true)) {
				chMap[std::make_pair(anodeDaqType,anodeDaqCh)] = std::make_pair(ionChamber,false);
				chMap[std::make_pair(scintDaqType,scintDaqCh)] = std::make_pair(ionChamber,true);
			}
			else 
				std::cerr << "ERROR: Detector " << serialNum << " will not be unpacked!\n";
		}
		else {
			int pTypeDaqType, nTypeDaqType;
			int pTypeDaqCh, nTypeDaqCh;
			if (!(lineStream >> serialNum >> id >> pTypeDaqType >> pTypeDaqCh >> nTypeDaqType >> nTypeDaqCh)) {
				break;
			}

			std::cout << serialNum;
			std::cout << ", LocID: " << id;
			std::cout << ", pDAQ: " << pTypeDaqType << "-" << pTypeDaqCh;
			std::cout << ", nDAQ: " << nTypeDaqType << "-" << nTypeDaqCh << "\n";

			short sector, depth;
			bool upStream;
			if (!ParseID(id, sector, depth, upStream)) {
				std::cerr << "ERROR: Unable to parse position id: " << id << " of detector " << serialNum << "!\n";
				break;
			}
			std::cout << " Upstream: ";
			upStream ? std::cout << "U" : std::cout << "D";
			std::cout << ", Sector: " << sector << ", Depth: " << depth << "\n";

			SolidVector pos = GetPosVector(detType, sector, depth, upStream);

			std::cout << " Position: " << pos.x() << ", " << pos.y() << ", " << pos.z();
			std::cout << ", RotZ: " << pos.RotZ() * TMath::RadToDeg();
			std::cout << ", RotPhi: " << pos.RotPhi() * TMath::RadToDeg() << "\n";

			//Construct object for the specified type.
			if (detType == "superX3") {	
				det = (orrubaDet*) superX3s->ConstructedAt(superX3s->GetEntries());
			}	
			else if (detType == "BB10") {	
				det = (orrubaDet*) bb10s->ConstructedAt(bb10s->GetEntries());
			}	
			else if (detType == "QQQ5") {	
				det = (orrubaDet*) qqq5s->ConstructedAt(qqq5s->GetEntries());
			}
			else {
				std::cerr << "ERROR: Unknown detector type: " << detType << "!\n";
				std::cerr << " Valid options: ion, superX3, BB10, QQQ5.\n";
				break;
			}
			det->SetDetector(serialNum,sector,depth,upStream, pos);
			if (IsInsertable(pTypeDaqType, pTypeDaqCh,detType,false) && IsInsertable(nTypeDaqType, nTypeDaqCh, detType,true)) {
				chMap[std::make_pair(pTypeDaqType,pTypeDaqCh)] = std::make_pair(det,false);
				chMap[std::make_pair(nTypeDaqType,nTypeDaqCh)] = std::make_pair(det,true);
			}
			else 
				std::cerr << "ERROR: Detector " << serialNum << " will not be unpacked!\n";
		}

		//Read calibration information.
		std::istream::streampos prevPos = mapFile.tellg();
		while (std::getline(mapFile, line).good()) {
			std::istringstream lineStream(line);
			std::string calType, subType;
			int detChannel;
			//If we don't understand this line we rewind the stream and let the first parsing section have a chance.
			if (!(lineStream >> calType >> subType >> detChannel)) {
				mapFile.seekg(prevPos);
				break;
			}

			//Determine the calibration type.
			bool posCal = false;
			bool timeCal = false;
			if (calType == "posCal") {
				if (!(detType == "superX3" && subType == "resStrip") && !(detType == "ion" && subType == "scint")) {
					std::cout << " WARNING: Ignoring position calibration specified for " << subType << "-type part of " << detType << " detector " << serialNum << "\n";
					continue;
				}
				posCal = true;
			}
			else if (calType == "timeCal") {
				if (!(detType == "ion" && subType == "scint")) {
					std::cout << " WARNING: Ignoring time calibration specified for " << subType << "-type part of " << detType << " detector " << serialNum << "\n";
					continue;
				}
				timeCal = true;
			}
			else if (calType != "enCal") { 
				mapFile.seekg(-line.length()-1,std::ios_base::cur);
				break;
			}

			//Check that the subtype is valid
			bool secondaryType = false;
			if (detType == "ion") {
				if (subType != "scint" && subType != "anode") {
					std::cerr << " ERROR: Unknown subtype '" << subType << "'!\n";
					std::cerr << "  Valid options: anode, strip.\n";
					continue;
				}
			}
			else {
				if (subType == "n") secondaryType = true;
				else if (subType != "p" && subType != "resStrip") {
					std::cerr << " ERROR: Unknown subtype '" << subType << "'!\n";
					std::cerr << "  Valid options: p, n, resStrip.\n";
					continue;
				}
			}

			//Get the calibration parameters
			float param;
			std::vector< float > calParams;
			while (lineStream >> param) calParams.push_back(param);

			//Print a line about the calibration function.
			if (posCal) std::cout << " Pos. ";
			else std::cout << " En. ";
			std::cout << subType << "-type channel " << detChannel << ", ";
			std::cout << calParams.size()-1 << " order calib.: "; 

			for (auto itr=calParams.begin(); itr != calParams.end(); ++itr) {
				if (itr != calParams.begin()) std::cout << ", ";
				std::cout << *itr;
			}
			std::cout << "\n";

			if (calParams.empty()) std::cerr << " WARNING: No calibration parameters specified for " << subType << "-type channel " << detChannel << ".\n";

			///Assign the parameters to their respective container.
			if (detType == "ion") {	
				if (subType == "scint") {
					if (posCal) 
						std::cout << " WARNING: No support for scint position calibration!\n";	
					else if (timeCal) 
						ionChamber->SetScintTimeCalPars(detChannel, calParams);	
					else 
						ionChamber->SetScintEnCalPars(detChannel, calParams);	
				}
				else {
					ionChamber->SetAnodeEnCalPars(detChannel, calParams);	
				}
			}
			else {
				if (detType == "superX3" && subType == "resStrip") {
					if (posCal) 
						((superX3*) det)->SetStripPosCalibPars(detChannel, calParams);
					else ((superX3*) det)->SetStripEnCalibPars(detChannel, calParams);
				}
				else {
					if (!det->SetEnergyCalib(calParams, detChannel, secondaryType)) {
						std::cerr << "ERROR: Unable to set calibration for " << subType << "-type part of " << detType << " detector " << serialNum << "!\n";
					}
				}
			}

		}
	}

	//If we aborted for some reason and the file is good we must of had a parsing issue.
	if (mapFile.good()) {
		std::cerr << "ERROR: Unable to read configuration file! Failed on the following line:\n";
		std::cerr << line << "\n";
	}
}

void GoddessConfig::ReadPosition(std::string filename) {

}

/**We check that at the insertion point the previous entry has sufficient 
 * channels prior to the insertion point of this detector. As well as, check that 
 * the inserted detector will have sufficienct channel space before the following 
 * detector.
 *
 * \param[in] daqCh The DAQ channel at which the detector would be inserted into the
 *  map.
 * \param[in] type The type of detector to insert into the map.
 * \return True if there is no overlapping channels already loaded in the map.
 */
bool GoddessConfig::IsInsertable(short daqType, int daqCh, std::string detType, bool secondaryType) {
	bool insertable = true;
	std::pair<short, short> key = std::make_pair(daqType, daqCh);

	if (!chMap.empty()) {
		//Check the upper bound
		auto mapItr = chMap.upper_bound(key);
		if (mapItr != chMap.end()) {
			if (mapItr->first.first == daqType) {
				//Get the number of channels needed for each detector.
				int numCh = 1;
				if (detType == "superX3") numCh = 12;
				else if (detType == "BB10") numCh = 8;
				else if (detType == "QQQ5") numCh = 36;

				if (daqCh + numCh > mapItr->first.second) {
					orrubaDet* siDet = dynamic_cast<orrubaDet*>(mapItr->second.first);
					if (siDet) 
						std::cerr << "ERROR: Specified starting DAQ channel " << daqType << ":" << daqCh << " conflicts with detector " << siDet->GetSerialNum() << "\n";
					else
						std::cerr << "ERROR: Specified starting DAQ channel conflicts with ion chamber.\n";
					insertable = false;
				}
			}
		}

		//Check the lower bound
		if (mapItr != chMap.begin()) {
			--mapItr;
			if (mapItr->first.first == daqType)  {
				int numCh = 1;
				std::string checkType = mapItr->second.first->IsA()->GetName();
				if (checkType == "superX3") numCh = 12;
				else if (checkType == "BB10") numCh = 8;
				else if (checkType == "QQQ5") numCh = 36;

				if (mapItr->first.second + numCh > daqCh) {
					orrubaDet* siDet = dynamic_cast<orrubaDet*>(mapItr->second.first);
					if (siDet) 
						std::cerr << "ERROR: Specified starting DAQ channel " << daqType << ":" << daqCh << " conflicts with detector " << siDet->GetSerialNum() << "\n";
					else
						std::cerr << "ERROR: Specified starting DAQ channel conflict with ion chamber.\n";
					insertable = false;
				}
			}
		}
	}

	return insertable;
}


/**
 *
 */
Detector *GoddessConfig::SetRawValue(short daqType, short digitizerCh, int rawValue) {
	MapKey key = std::make_pair(daqType, digitizerCh);
	auto mapItr = chMap.upper_bound(key);
	if (mapItr == chMap.begin()) {
		std::cerr << "ERROR: Unable to find mapped channel for DAQ type: " << daqType << " ch: " << digitizerCh << "\n!";
		return NULL;
	}

	--mapItr;
	Detector *det = mapItr->second.first;
	bool secondaryType = mapItr->second.second;
	int detCh = digitizerCh - mapItr->first.second;

	det->SetRawValue(detCh, secondaryType, rawValue);

	return det;
	
}

/**Parse the id string describing the detector position int he barrel. The id is 
 * composed of the following:
 * 1. U/D: Indicating whether the detector is up/down stream of the target.
 * 2. A-D or 0-11: Indicating which sector of the barrel the detector occupies, 
 * 	letters and numbers denote end cap and barrel sectors respectively.
 * 3. A dash, '-'.
 * 4. dE, E1, or E2: Indicating the depthe of the detector in the stack.
 *
 *	\param[in] id The id string to be parsed.
 *	\param[out] sector The sector occupied by the detector.
 *	\param[out] depth The depth of the detector.
 *	\param[out] upStream Flag indicating if the detector is upstream of the target.
 *	\return True if successfully parses the id string.
 *
 */
bool GoddessConfig::ParseID(std::string id, short& sector, short& depth, bool& upStream) {
	std::string subStr = id.substr(0,1);
	if (subStr == "U") upStream = true;
	else if (subStr == "D") upStream = false;
	else {
		std::cerr << "ERROR: Unexpected character '" << subStr << "' in up/down stream position of id!\n";
		return false;
	}

//	bool barrel = false;
	subStr = id.substr(1,id.length() - 4);
	if (subStr == "A") sector = 0;
	else if (subStr == "B") sector = 1;
	else if (subStr == "C") sector = 2;
	else if (subStr == "D") sector = 3;
	else if (subStr.find_first_of("0123456789") != std::string::npos) {
		sector = std::stoi(subStr);
		if (sector > 11) {
			std::cerr << "ERROR: Unexpected sector '" << subStr << "' in id!\n";
			return false;
		}
//		barrel = true;
	}
	else {
		std::cerr << "ERROR: Unexpected character '" << subStr << "' in sector position of id!\n";
		return false;
	}

	//The depth index position depends on whether we read out a barrel or end cap.
	subStr = id.substr(id.length() - 2, 2);
	if (subStr == "dE") depth = 0;
	else if (subStr == "E1") depth = 1;
	else if (subStr == "E2") depth = 2;
	else {
		std::cerr << "ERROR: Unexpected depth '" << subStr << "' in id!\n";
		return false;
	}

	return true;
}

/**Based on the detector type, sector, depth and whether the detector is up stream a
 * vector position is computed and the rotation angle of the detector is determined.
 * 
 * \param[in] type The detector type.
 * \param[in] sector The sector occupied by the detector.
 * \param[in] depth The depth of the detector in the stack.
 * \param[in] upStream Flag indicating if the detector is up stream of the target.
 * \param[out] angle The computed rotation angle.
 * \return A TVector3 pointing to the detector corner. 
 */
SolidVector GoddessConfig::GetPosVector(std::string type, short sector, short depth, bool upStream) {
	static float barrelRadius = 3.750 * 25.4; //mm
	static float halfBarrelLength = (4.375 - 0.7) * 25.4; //mm
	SolidVector pos(0,0,0);

	//Compute position for barrel detectors.
	if (type == "superX3" || type == "BB10") {
		//The barrel is divided into twelve sectors with the zero sector in the 
		// positive vertical (y-axis) direction. Therefore the 0 sector is at 90 
		// degrees and we subtract for each additional sector and then offset by
		// two pi if less than zero.
		float azimuthal = TMath::PiOver2() - sector / 12. * TMath::TwoPi();
		if (azimuthal < 0) azimuthal += TMath::TwoPi();
		std::cout << azimuthal;
		//The rotation angle is also dependent on the sector with a rotation of zero 
		// in the zero sector.
		float rotZ = azimuthal + TMath::PiOver2();
		if (rotZ > TMath::TwoPi()) rotZ -= TMath::TwoPi();
		//The z position is oriented along the beam line and depends only on if the 
		// detector is upstream or downstream of the target.
		float z;
		if (upStream) z = -halfBarrelLength / 2;
		else z = halfBarrelLength / 2;
		//Set the computed x,y,z positions.
		float rotphi;
		if (upStream) rotphi = 0;
 		else rotphi = TMath::Pi();

		pos.SetXYZ(barrelRadius * cos(azimuthal), barrelRadius * sin(azimuthal), z);
		pos.SetRotationZ(rotZ);
		pos.SetRotationPhi(rotphi);

	}
	else if (type == "QQQ5") {
		// Make the pos vector point to middle of QQQ5 with the 0 sector at rotZ=90deg then sector 1 moving clockwise at rotZ=0
		float rotZ = (TMath::PiOver2()) - sector / 4. * TMath::TwoPi();
		// need offset because detPos will not be exaclty pointing at origin
		// number based on edge of the detectors separated by 6.35mm
		float offset = 4.49;//mm 
		pos.SetXYZ(offset * cos( rotZ ) , offset * sin( rotZ ) , halfBarrelLength);
		pos.SetRotationZ(rotZ);
		// Still need to correct for depth in stack (i.e. dE, E1,E2)

	}

	return pos;
}
