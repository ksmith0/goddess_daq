#ifndef QQQ5_H
#define QQQ5_H

#include "siDet.h"
#include "TVector3.h"

///Class designed to compute hit information on a Micron QQQ5 detector.
/**The QQQ5 is a annular detector.
 */
class QQQ5 : public siDet {
	private:
		///The detector position.
		TVector3 detPos; //!
		///The detector rotation around the z-axis.
		float detRotation;

		///Vector pointing to mid point of p type strip edge in mm.
		TVector3 pStripEdgePos[33]; //!
		///Vector pointing to mid point of n type strip edge in mm.
		TVector3 nStripEdgePos[5]; //!

		///The bin edges along the p-type strips in mm.	
		Float_t binsP[33]; //!
		///The bin edges along the n-type strips in mm.	
		Float_t binsN[5]; //!
		///The bin edges along the cylindrical radial directions rho in degrees.	
		Float_t binsRho[33]; //!
		///The bin edges along the phi axis in degrees.	
		Float_t binsAzimuthal[5]; //!
		///The bin edges along the phi axis in degrees.	
		Float_t binsPolar[33]; //!

		///Calibrated energy of the p type side.
		float enPtype;
		///Calibrated energy of the n type side.
		float enNtype;

		///Computed raw position of event.
		float stripPosRaw[4];
		///Computed calibrated position of event.
		float stripPosCal[4];

		///The constructed position of the event.
		TVector3 eventPos; 

		///Construct array of bin edges.
		void ConstructBins();

	public:
		///Default constructor.
		QQQ5(std::string name, TVector3 detPos, Float_t detRotation);
		///Default destructor.
		virtual ~QQQ5();

		///Clear the stored values in this detector.
		void Clear();

		///Return the number of bins.
		static int GetNumNtypeBins() {return 4;}
		///Return the number of bins.
		static int GetNumPtypeBins() {return 32;}
		///Return the bins boundaries of the dimension along the p type strips in mm.
		float* GetPtypeBins() {return binsP;};
		///Return the bins boundaries of the dimension along the n type strips in mm.
		float* GetNtypeBins() {return binsN;};
		///Return the  in mm.
		float* GetRhoBins() {return binsRho;};
		///Return the bins along the azimuthal direction in degrees.
		float* GetAzimuthalBins() {return binsAzimuthal;};
		///Return the bins along the polar direction in degrees.
		float* GetPolarBins() {return binsPolar;};
		
		///Returns true if strip number is valid.
		bool ValidStrip(int strip);

		///Return the computed event position.
		TVector3 GetEventPosition() {return eventPos;};
		///Return the energy from the p type side.
		float GetPtypeEnergy() {return enPtype;};
		///Return the energy from the n type side.
		float GetNtypeEnergy() {return enNtype;};

		///Set the raw energy of the contact and compute the calibrated value.
		virtual void SetEnergy(unsigned int contact, int rawValue, bool nType = false);

		/// \cond This is just for ROOT and doesn't need to be documented
		ClassDef(QQQ5,1);
		/// \endcond
};


#endif