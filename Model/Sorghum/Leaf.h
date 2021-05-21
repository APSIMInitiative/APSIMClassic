//------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef LeafH
#define LeafH

#include <vector>
#include <string>
#include "Utilities.h"
using namespace std;

namespace Sorghum {
	//------------------------------------------------------------------------------------------------
	class Leaf : public PlantPart
		{
		protected:

			// Parameters ----------------------------------------------------------
			double noSeed;
			double noEmergence;
			double initRate;
			double initialTPLA;
			double tplaInflectionRatio,tplaProductionCoef;
			// leaf appearance
			double appearanceRate1;
			double appearanceRate2;
			double noRateChange;
			double minLeafNo,maxLeafNo;

			// leaf dry mass
			double initialDM;
			double translocFrac;
			double leafPartitionRate;
			double dmTotal;

			// leaf area
			double tillerCoef;
			double mainStemCoef;

			double slaMin;
			double slaMax;

			// sencescence
			double senRadnCrit;        // radiation level for onset of light senescence
			double senLightTimeConst;  // delay factor for light senescence
			double senWaterTimeConst;  // delay factor for water senescence
			double senThreshold;       // supply:demand ratio for onset of water senescence
			double frostKill;          // temperature threshold for leaf death
			double frostKillSevere;    // temperature threshold for plant death

			// nitrogen
			double initialSLN;
			double targetSLN;
			double newLeafSLN;
			double senescedLeafSLN;
			double dilnNSlope;
			double dilnNInt;

			// plant
			double density;

			//  Variables  -----------------------------------------------------
			// Leaf Number
			vector<double> leafNo;              // leaf number per stage
			vector<double> nodeNo;              // node number per stage

			double finalLeafNo;
			double dltLeafNo;
			double nLeaves;

			// LAI
			double lai;
			double dltPotentialLAI;
			double dltStressedLAI;
			double dltLAI;
			double maxLai;

			double extinctionCoef;
			double coverGreen;
			double coverSen;
			double coverTot;
			// TPLA
			double tplaMax;                     // maximum possible total plant leaf area
			double tplaPot;
			double tpla;
			double spla;
			double gpla;

			// senescence
			double sLai;
			double dltSlai;
			double maxLaiPossible;
			double waterStressLaiLoss;
			double dltSlaiLight;
			double dltSlaiWater;
			double dltSlaiFrost;
			//   double dltDmSenesced;
			vector<double> laiEquilibLight;
			vector<double> laiEquilibWater;
			vector<double> avSD;
			double dltSlaiN;

			// SLN
			double SLN;
			double SLN0;

			// SLA
			double SLA;
			double maxSLA;
			// tillers
			double tillersAdded;
			double tillers;


			// Private Methods -------------------------------------------------------
			virtual void  doRegistrations(void);
			virtual void  initialize(void);
			virtual void  initLeafNo(void);
			void  calcFinalLeafNo(void);
			void  calcLeafAppearance(void);
			void  calcTplaMax(void);
			double calcDltPotentialTPLA(void);
			double calcStressedLeafArea(void);
			double calcMaxLaiPossible(void);
			double calcLaiSenescenceLight(void);
			double calcLaiSenescenceWater(void);
			double calcLaiSenescenceFrost(void);
			double calcSLN(void);
			double calcLAI(void);
			void  calcCover(void);


			// public Methods -------------------------------------------------------
		public:
			Leaf(ScienceAPI2 &, Plant *p);
			~Leaf();

			// plant
			double dltNSenesced;
			virtual void  areaActual(void);
			void  senesceArea(void);

			virtual void  readParams (void);
			virtual void  updateVars(void);

			void  process(void);
			virtual void  calcLeafNo(void);
			virtual void  calcPotentialArea(void);

			// phenology
			double calcEmergFlagTT(void);

			// nitrogen
			double calcNDemand(void);
			double calcNewLeafNDemand(void);
			double provideN(double requiredN, bool fromLeaf);
			double getSLN(void)const{return SLN;}
			double laiToday(void)const;

			double getDltLAI(void)const{return dltLAI;}
			double getLAI(void)const{return lai;}
			double getDltSlai(void)const{return dltSlai;}
			void  addDltSlai(double add);
            bool   killedByFrostToday;


			double getCoverGreen(void)const{return coverGreen;}
			double getCoverTot(void)const{return coverTot;}

			// phosphorus
			double calcPDemand(void);

			// biomass
			void  calcSenescence(void);
			double partitionDM(double dltDM);
			double dmRetransAvailable(void);
			void  dmRetrans(double dltDm){dmRetranslocate = dltDm;}
			double getLeafNo(void){return nLeaves;}
			void  laiDetachment(vector<double> senDetachFrac);


			void  Summary(void);

			// phenology
			void  phenologyEvent(int);

			// tillers
			virtual void reduceTillers(double reduceLAI){};

		};  // Leaf
	}
#endif
