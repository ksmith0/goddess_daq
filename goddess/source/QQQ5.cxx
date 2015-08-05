#include "QQQ5.h"
#include <cmath>
#include "TMath.h"

QQQ5::QQQ5() {
	siDet::SetNumContacts(32,4);
	Clear();
}
/**We assuming the position provided for the detector is at the radial center of the
 * detector and the clockwise is aligned at the rotation angle. 
 */
QQQ5::QQQ5(std::string serialNum, unsigned short sector, unsigned short depth,
	bool upStream, SolidVector position) :
	orrubaDet(serialNum, sector, depth, upStream, position)
{
	siDet::SetNumContacts(32,4);
	ConstructBins();
	Clear();
}

QQQ5::~QQQ5() {}

/**Construct arrays of bin edges for ROOT histograms based on the provided position 
 * of the detector. Currently constructs binning in the n, p and z direction in mm
 * and in the azimuthal and polar direction in degrees.
 *
 *
 * \note Assumes that the detector lies in the XY plane with the p strip normals 
 * along the radial dimension rho.
 */
void QQQ5::ConstructBins () {
	float pStripRad[33] = 
		{25.2, 27.75, 30.25, 32.7, 35.1, 37.45, 39.75, 42.0,
		 44.2, 46.35, 48.45, 50.5, 52.5, 54.45, 56.35, 58.2,
		 60.0, 61.75, 63.45, 65.1, 66.7, 68.25, 69.75, 71.2,
		 72.6, 73.95, 75.25, 76.5, 77.7, 78.85, 79.95, 81.0, 82.0}; //mm
	float nStripPitch = 90 / 4; //deg

	float nTypeRadius = pStripRad[32] / 2;
	for (unsigned int strip=0; strip<=4; strip++) {			
		//compute the binning along the p and n type strip directions in mm.
		binsN[strip] = strip * nStripPitch;

		nStripEdgePos[strip].SetXYZ(
			detPos.X() + nTypeRadius * cos(binsN[strip] + detPos.RotZ()),
			detPos.Y() + nTypeRadius * sin(binsN[strip] + detPos.RotZ()),
			detPos.Z());

		//Azimuthal angle 
		binsAzimuthal[strip] = TMath::RadToDeg() * nStripEdgePos[strip].Phi();
	}

	//Compute the fraction of the radius in the x and y plane.
	float xFrac = cos(binsN[2] + detPos.RotZ());
	float yFrac = sin(binsN[2] + detPos.RotZ());

	for (unsigned int strip=0; strip<=32; strip++) {			
		binsP[strip] = pStripRad[strip];

		//The strips have x and y computed from detector vector projected onto the 
		//XY vector from the detector origin to the strip edge.
		pStripEdgePos[strip].SetXYZ(
			detPos.X() + pStripRad[strip] * xFrac,
			detPos.Y() + pStripRad[strip] * yFrac,
			detPos.Z());

		binsRho[strip] = pStripEdgePos[strip].XYvector().Mod();
		int polarStrip = strip;
		if (detPos.Z() < 0) polarStrip = 32 - strip;
		binsPolar[polarStrip] = TMath::RadToDeg() * pStripEdgePos[strip].Theta();
	}

}

void QQQ5::Clear() {
	siDet::Clear();

	enPtype = 0;
	enNtype = 0;

	eventPos.SetXYZ(0,0,0);
}


/**This method is called when a contact energy is updated. We call the parent 
 * siDet::SetRawValue to handle storing the raw and calibrated value. If the update
 * was a p type contact we check if another p contact in the same strip has been set
 * and if so we make a call to compute the position the event occurred in the strip.
 *
 * \param[in] contact The number of the contact which was updated.
 *	\param[in] rawValue The raw contact value in channels.
 * \param[in] nType Whether the contact was n Type.
 */
void QQQ5::SetRawValue(unsigned int contact, bool nType, int rawValue) {
	if (!ValidContact(contact, nType)) return;

	//Call parent method to handle calibration.
	siDet::SetRawValue(contact, nType, rawValue);

	if (nType) {
		//Set the energy value only if the multiplicity is 1.
		if (GetContactMult(nType) == 1) 
			enNtype = GetCalEnergy(contact, nType);
		else enNtype = 0;
	}
	else {

		//Store the energy only if the multiplicity is one.
		if (GetContactMult(nType) == 1) 
			enNtype = GetCalEnergy(contact, nType);
		else
			enPtype = 0;
	}
}


ClassImp(QQQ5)


