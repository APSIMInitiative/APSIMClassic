//------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef LeafH
#define LeafH

#include <vector>
#include <string>
#include "Utilities.h"
//#include "CanopyPhotosynthesis.h"
using namespace std;
namespace Maize {

   class CanopyPhotosynthesis;

   //------------------------------------------------------------------------------------------------
   class Leaf : public PlantPart
      {
      private:

         // Parameters ----------------------------------------------------------
         string crop;

         double noSeed;
         double noEmergence;
         double initRate;
         double initialTPLA;
         double tplaInflectionRatio,tplaProductionCoef;
         // leaf appearance
         double appearanceRate1;
         double appearanceRate2;
         double appearanceRate3;
         double noRateChange1;
         double noRateChange2;
         double minLeafNo,maxLeafNo;

         // leaf dry mass
         double initialDM;
         double translocFrac;
         double leafPartitionRate;
         double dmTotal;

         // leaf area   TPLA
         double tillerCoef;
         double mainStemCoef;

         double leafNoCorrection;       //corrects for other growing leaves
         //Birch, Hammer bell shaped curve parameters
         double aX0;                       // Eqn 14 calc x0 - position of largest leaf
         vector<double> largestLeafParams; // Eqn 13 calc aMax - area of largest leaf
		   vector<double> bellCurveParams;   // Eqn 18,19
         // sla
         TableFn slaMin;
         TableFn slaMax;

         // sencescence
         double splaIntercept;
         double splaSlope;
         double splaProdCoef;
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
         double nTotal;
         double dilnNSlope;
         double dilnNInt;

         // plant
         double density;


         //  Variables  -----------------------------------------------------
         // Leaf Number
         vector<double> leafNo;              // leaf number per stage
         vector<double> nodeNo;              // node number per stage
         vector<double> leafSize;            // size of individual laeves

         double finalLeafNo;
         double dltLeafNo;
         double nLeaves;
         double nDeadLeaves;
         double dltDeadLeaves;

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
         double dltSlaiAge;
         double dltSlaiLight;
         double dltSlaiWater;
         double dltSlaiFrost;
         //   double dltDmSenesced;
         vector<double> laiEquilibLight;
         vector<double> laiEquilibWater;
         vector<double> avSD;
         double dltSlaiN;

         // dead
         double deadLai;
         // sLai
         double deadLeafConst;
         double deadLeafSlope;

         // SLN
         double SLN;
         double SLN0;

         // Private Methods -------------------------------------------------------
         void   doRegistrations(void);
         void   initialize(void);
         void   initLeafNo(void);
         void   calcFinalLeafNo(void);
         void   calcLeafAppearance(void);
         void   calcTplaMax(void);
         double calcDltPotentialTPLA(void);
         double calcStressedLeafArea(void);
         double calcMaxLaiPossible(void);
         double calcLaiSenescenceAge(void);
         double calcDltDeadLeaves(void);

         double calcLaiSenescenceLight(void);
         double calcLaiSenescenceWater(void);
         double calcLaiSenescenceFrost(void);
         double calcSLN(void);
         double calcLAI(void);
         void  calcCover(void);

         // leaf area individual leaf
         void leafAreaPotBellShapeCurve(void);
         void calcLeafSize(void);
         double calcIndividualLeafSize(double leafNo);

			// potential leaf growth
			void calcLeafNo(void);           // plant
         void calcPotentialArea(void);    // plant

			// actual growth
			void areaActual(void);          
         void senesceArea(void);          

         // public Methods -------------------------------------------------------
      public:
         Leaf(ScienceAPI2 &, Plant *p);
         ~Leaf();

         ///////////////////////////
         double dltNSenesced;

         //////////////////////////
         void readParams (void);
         void updateVars(void);

         void potentialGrowth(void);      // plant
			void actualGrowth(void);			// plant

         double calcEmergFlagTT(void);                     // phenology

         // nitrogen
         double calcNDemand(void);                         // nitrogen
         double calcNewLeafNDemand(void);                  // nitrogen
         double provideN(double requiredN, bool forLeaf);
         double getSLN(void)const{return SLN;}             // nitrogen
         double laiToday(void)const;                       // nitrogen

         double getDltLAI(void)const{return dltLAI;}       // nitrogen
         double getLAI(void)const{return lai;}             // nitrogen
         double getDltSlai(void)const{return dltSlai;}     // nitrogen
         void  addDltSlai(double add);                     // nitrogen


         double getCoverGreen(void)const{return coverGreen;}
         double getCoverTot(void)const{return coverTot;}

         // phosphorus
         double calcPDemand(void);
         void  calcSenescence(void);                              // biomass
         double partitionDM(double dltDM);                        // biomass
         double dmRetransAvailable(void);                         // biomass
         void  dmRetrans(double dltDm){dmRetranslocate = dltDm;}  // biomass
         double getLeafNo(void){return nLeaves;}
         void  laiDetachment(vector<double> senDetachFrac);

         void Summary(void);

         // phenology
         void phenologyEvent(int);

      };  // Leaf
   }
#endif
