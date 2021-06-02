//------------------------------------------------------------------------------------------------
#include <stdio.h>
#include "Plant.h"
#include "Leaf.h"

using namespace Maize;
//------------------------------------------------------------------------------------------------
//------ Leaf Constructor
//------------------------------------------------------------------------------------------------
Leaf::Leaf(ScienceAPI2 &api, Plant *p) : PlantPart(api)
   {
   plant = p;
   name = "Leaf";
   partNo = 1;
   frostKillSevere = -3.5;

   doRegistrations();
   initialize();
   }
//------------------------------------------------------------------------------------------------
//------ Destructor
//------------------------------------------------------------------------------------------------
Leaf::~Leaf()
   {
   }
//--------------------------------------------------------------------------------------------------
// Register variables for other modules
//--------------------------------------------------------------------------------------------------
void Leaf::doRegistrations(void)
   {
   scienceAPI.expose("LAI",            "m2/m2", "Live plant green LAI",            false, lai);
   scienceAPI.expose("DeltaLAI",       "m2/m2", "Leaf area growth rate",           false, dltLAI);
   scienceAPI.expose("SLAI",           "m2/m2", "Senesced plant LAI",              false, sLai);
   scienceAPI.expose("DeltaSLAI",      "m2/m2", "Leaf area senescence rate",       false, dltSlai);
   scienceAPI.expose("TPLA",           "m2",    "Total plant leaf area",           false, tpla);
   scienceAPI.expose("SPLA",           "m2",    "Senesced plant leaf area",        false, spla);
   scienceAPI.expose("GPLA",           "m2",    "Green plant leaf area",           false, gpla);
   scienceAPI.expose("LeafNo",         "leaves","Number of fully expanded leaves", false, nLeaves);
   scienceAPI.expose("FinalLeafNo", "leaves", "Final Leaf Number",                 false, finalLeafNo);
   scienceAPI.expose("LeafGreenWt",    "g/m^2", "Live leaf dry weight",            false, dmGreen);
   scienceAPI.expose("LeafSenescedWt", "g/m^2", "Senesced leaf dry weight",        false, dmSenesced);
   scienceAPI.expose("MaxLAI",         "m2/m2", "Maximum LAI reached",             false, maxLai);
   scienceAPI.expose("LeafGreenN",     "g/m^2", "N in green leaf",                 false, nGreen);
   scienceAPI.expose("LeafSenescedN",  "g/m^2", "N in senesced leaf",              false, nSenesced);
   scienceAPI.expose("SLN",            "g(N)/m2","Specific Leaf Nitrogen",         false, SLN);
   scienceAPI.expose("LeafGreenNConc", "%",     "Live leaf N concentration",       false, nConc);
   scienceAPI.expose("LeafNDemand",    "g/m^2", "Today's N demand from leaves",    false, nDemand);
   scienceAPI.expose("DeltaLeafGreenN","g/m^2", "Daily N increase in leaves",      false, dltNGreen);
   scienceAPI.expose("DeltaLeafNo",    "lvs/d", "Fraction of oldest leaf expanded",false, dltLeafNo);
   scienceAPI.expose("DeltaSlaiFrost", "m2/m2", "Senesced leaf area from frost",   false, dltSlaiFrost);
   scienceAPI.expose("DeltaSlaiLight", "m2/m2", "Senesced leaf area from light",   false, dltSlaiLight);
   scienceAPI.expose("DeltaSlaiAge",   "m2/m2", "Senesced leaf area from age",     false, dltSlaiAge);
   scienceAPI.expose("DeltaSlaiWater", "m2/m2", "Senesced leaf area from age",     false, dltSlaiWater);
   scienceAPI.expose("ExtinctionCoef", "()",    "Light Extinction coefficient",    false, extinctionCoef);
   scienceAPI.expose("LeafGreenP",     "g/m^2" ,"P in live leaf",                  false, pGreen);
	// leaf size
   scienceAPI.expose("LeafSize",        "",   "Size of individual leaves",         false, leafSize);

   }
//------------------------------------------------------------------------------------------------
//------- Initialize variables
//------------------------------------------------------------------------------------------------
void Leaf::initialize(void)
   {
   // crop
   crop = plant->getCropType();
   // leaf area
   lai = 0.0;
   dltPotentialLAI = 0.0;
   maxLaiPossible = 0.0;
   waterStressLaiLoss = 0.0;
   sLai = 0.0;
   dltSlai = 0.0;
   tpla = 0.0;
   spla= 0.0;
   gpla= 0.0;
   tplaMax = 0.0;
   tplaPot = 0.0;
   dltSlaiN = 0.0;
   maxLai = 0.0;

   // leaf number
   finalLeafNo = 0.0;
   dltLeafNo = 0.0;
   nLeaves = 0.0;
   leafNo.clear();
   nodeNo.clear();

   nDeadLeaves = 0.0;
   dltDeadLeaves = 0.0;

   for(int i=0;i < nStages;i++)
      {
      leafNo.push_back(0.0);
      nodeNo.push_back(0.0);
      }

   // nitrogen
   SLN = 0.0;
   SLN0 = 0.0;
   senescedLeafSLN = 0.3;
   dltNSenesced = 0.0;

   partNo = 1;

   coverGreen = 0.0;
   coverSen = 0.0;
   coverTot = 0.0;

   tplaInflectionRatio = 0;
   tplaProductionCoef = 0;
   dmTotal = 0;
   tillerCoef = 0;
   mainStemCoef = 0;

   splaIntercept = 0;
   splaSlope = 0;
   splaProdCoef = 0;
   nTotal = 0;
   dltLAI = 0;
   dltSlaiAge = 0;
   dltSlaiLight = 0;
   dltSlaiWater = 0;
   dltSlaiFrost = 0;
   dltSlaiN = 0;
   deadLai = 0;
   SLN = 0;
   SLN0 = 0;
   dltNSenesced = 0;

   //canPhoto = NULL;

   PlantPart::initialize();
   }
//------------------------------------------------------------------------------------------------
//-----------  read leaf parameters
//------------------------------------------------------------------------------------------------
void Leaf::readParams (void)
   {
   scienceAPI.read("leaf_no_seed"    , "", false, noSeed);
   scienceAPI.read("leaf_init_rate"  , "", false, initRate);
   scienceAPI.read("leaf_no_at_emerg", "", false, noEmergence);
   scienceAPI.read("leaf_no_min"     , "", false, minLeafNo);
   scienceAPI.read("leaf_no_max"     , "", false, maxLeafNo);

   // leaf appearance rates
   scienceAPI.read("leaf_app_rate1"     ,"", false, appearanceRate1);
   scienceAPI.read("leaf_app_rate2"     ,"", false, appearanceRate2);
   scienceAPI.read("leaf_app_rate3"     ,"", false, appearanceRate3);
   scienceAPI.read("leaf_no_rate_change1","", false, noRateChange1);
   scienceAPI.read("leaf_no_rate_change2","", false, noRateChange2);

   // leaf area TPLA
   scienceAPI.read("initial_tpla"         ,"", false, initialTPLA);

   // leaf area individual leaf
   //Birch, Hammer bell shaped curve parameters
   scienceAPI.read("aX0"       ,"", false, aX0);    // Eqn 14
   scienceAPI.read("largestLeafParams"       ,"", false, largestLeafParams); // Eqn 13
   scienceAPI.read("bellCurveParams","", false,bellCurveParams); // Eqn 18,19
   scienceAPI.read("leaf_no_correction"       ,"", false, leafNoCorrection); //


   // dry matter
   scienceAPI.read("dm_leaf_init"       , "", false, initialDM);
   scienceAPI.read("Leaf_trans_frac"    , "", false, translocFrac);
   scienceAPI.read("partition_rate_leaf", "", false, leafPartitionRate);

   // sla
   slaMin.read(scienceAPI, "x_lai","lai_sla_min");
   slaMax.read(scienceAPI, "x_lai","y_lai_sla_max");


   // slai
   scienceAPI.read("leaf_no_dead_const"       , "", false, deadLeafConst);
   scienceAPI.read("leaf_no_dead_slope"       , "", false, deadLeafSlope);

   // nitrogen
   scienceAPI.read("initialLeafSLN", "", false, initialSLN);
   scienceAPI.read("targetLeafSLN" , "", false, targetSLN);
   scienceAPI.read("newLeafSLN"    , "", false, newLeafSLN);
   scienceAPI.read("leafDilnNSlope", "", false, dilnNSlope);
   scienceAPI.read("leafdilnNInt"  , "", false, dilnNInt);
   scienceAPI.read("senescedLeafSLN","", false, senescedLeafSLN);

   // senescence
   scienceAPI.read("sen_radn_crit"       , "", false, senRadnCrit);
   scienceAPI.read("sen_light_time_const", "", false, senLightTimeConst);
   scienceAPI.read("sen_threshold"       , "", false, senThreshold);
   scienceAPI.read("sen_water_time_const", "", false, senWaterTimeConst);
   scienceAPI.read("frost_kill"          , "", false, frostKill);
   scienceAPI.read("frost_kill_severe"   , "", true, frostKillSevere);

   // phosphorus
   pMaxTable.read(scienceAPI, "x_p_stage_code","y_p_conc_max_leaf");
   pMinTable.read(scienceAPI, "x_p_stage_code","y_p_conc_min_leaf");
   pSenTable.read(scienceAPI, "x_p_stage_code","y_p_conc_sen_leaf");
   scienceAPI.read("p_conc_init_leaf", "", false, initialPConc);

   density = plant->getPlantDensity();

   // calculate extinction coef
   TableFn extinction;
   extinction.read(scienceAPI,"x_row_spacing","y_extinct_coef");
   extinctionCoef = extinction.value(plant->getRowSpacing() / 1000.0);

   }
//------------------------------------------------------------------------------------------------
//----------- update Leaf state variables at the end of the day
//------------------------------------------------------------------------------------------------
void Leaf::updateVars(void)
   {
   int iStage = (int)stage;

   // leaf number
   leafNo[iStage] += dltLeafNo;
   nLeaves = sumVector(leafNo);

   nDeadLeaves += dltDeadLeaves;
   dltDeadLeaves = 0.0;

   // Dry matter
   dmGreen += dltDmGreen;
   dmGreen += dmRetranslocate;
   dmSenesced += dltDmSenesced;
   dmGreen -= dltDmSenesced;

   nSenesced += dltNSenesced;

   // Nitrogen
   SLN0 = divide(nGreen,lai);
   nGreen += (dltNGreen +  dltNRetranslocate - dltNSenesced);
   dltNSenesced = 0.0;

   // leaf area
   lai += dltLAI;

   sLai += dltSlai;
   lai -= dltSlai;
   gpla = lai / density * smm2sm;
   spla = sLai / density * smm2sm;
   tpla = gpla + spla;

   dltSlaiN = 0.0;
   dltSlai = 0.0;

   SLN = divide(nGreen,lai);
   nConc = divide(nGreen,dmGreen,0) * 100;

   // phenology
   stage = plant->phenology->currentStage();

   nTotal = nGreen + nSenesced;
   dmTotal = dmGreen + dmSenesced;

   maxLai = Max(lai,maxLai);

   calcCover();
   }
//------------------------------------------------------------------------------------------------
void Leaf::potentialGrowth(void)
   {
	calcLeafNo();
   calcPotentialArea();
   }
//------------------------------------------------------------------------------------------------
void Leaf::actualGrowth(void)
	{
	// actual dltLai C limited
   areaActual();
   senesceArea();
	}
//------------------------------------------------------------------------------------------------
//------- react to a phenology event
//------------------------------------------------------------------------------------------------
void Leaf::phenologyEvent(int iStage)
   {
   ExternalMassFlowType EMF;
   switch (iStage)
      {
   case emergence :
      initLeafNo();
      lai = initialTPLA * smm2sm * density;
      tplaPot = initialTPLA;
      dmGreen = initialDM * density;
      SLN = initialSLN;
      nGreen = SLN * lai;
      pGreen = initialPConc * dmGreen;
      EMF.PoolClass = "crop";
      EMF.FlowType = "gain";
      EMF.DM = 0.0;
      EMF.N  = (float)(nGreen * gm2kg/sm2ha);
      EMF.P  = (float)(pGreen * gm2kg/sm2ha);
      EMF.C = 0.0; // ?????
      EMF.SW = 0.0;
      scienceAPI.publish("ExternalMassFlow", EMF);
      break;
   case flowering :
      //set the minimum weight of leaf; used for translocation to grain and stem
      double dmPlantLeaf = divide (dmGreen, density);
      dmPlantMin = dmPlantLeaf * (1.0 - translocFrac);
      break;

      }
   }
//------------------------------------------------------------------------------------------------
//-------  leaf number
//------------------------------------------------------------------------------------------------
void Leaf::calcLeafNo(void)
   {
   // calculate final leaf number up to initiation
   if(stage >= emergence && stage <= fi)
      {
      calcFinalLeafNo();
      // calculate potential individual leaf areas
      calcLeafSize();
      }
   if(stage >= emergence)
      {
      calcLeafAppearance();
      }
   }
//------------------------------------------------------------------------------------------------
//------- potential leaf area
//------------------------------------------------------------------------------------------------
void Leaf::calcPotentialArea(void)
   {
   dltPotentialLAI = 0.0;
   dltStressedLAI = 0.0;

   if(stage >= emergence && stage <= flag)
      {
      leafAreaPotBellShapeCurve();
      dltStressedLAI = calcStressedLeafArea();
      }
   }
//------------------------------------------------------------------------------------------------
//-------  limit new leaf area by carbon
//------------------------------------------------------------------------------------------------
void Leaf::areaActual(void)
   {
   if(stage >= endJuv && stage < maturity)
      dltLAI = Min(dltStressedLAI,dltDmGreen * slaMax.value(lai) * smm2sm);
   else dltLAI = dltStressedLAI;
   }

//------------------------------------------------------------------------------------------------
//------- calc senesced leaf area
//------------------------------------------------------------------------------------------------
void Leaf::senesceArea(void)
   {

   maxLaiPossible = lai + sLai;
   if(stage >= fi && stage < flag)
      maxLaiPossible = calcMaxLaiPossible();

   // senesced leaf area due to age
   dltSlaiAge = 0.0;
   if(stage >= emergence && stage < harvest)
      dltSlaiAge = calcLaiSenescenceAge();

   dltSlai = Max(dltSlai,dltSlaiAge);

   // senesced leaf area due to light (crowding)
   dltSlaiLight = calcLaiSenescenceLight();

   dltSlai = Max(dltSlai,dltSlaiLight);

   // senesced leaf area due to water
   dltSlaiWater = calcLaiSenescenceWater();
   dltSlai = Max(dltSlai,dltSlaiWater);

   // senesced leaf area due to frost
   dltSlaiFrost = calcLaiSenescenceFrost();
   dltSlai = Max(dltSlai,dltSlaiFrost);

   // senesced leaf area due to N
   dltSlai = Max(dltSlai,dltSlaiN);

   }
//------------------------------------------------------------------------------------------------
double Leaf::calcLaiSenescenceFrost(void)
   {
   //frostKillSevere will kill the plant at any stage
   //frostKill will remove LAI but only kill the plant between FloralInit and Flowering
   
   //  calculate senescence due to frost
   if (plant->today.minT > frostKill) return 0.0;
   if (stage <= emergence) return 0.0;
   double senescedLAI = 0.0;

   char msg[120];
   if (plant->today.minT > frostKillSevere)
      {
      //Temperature is warmer than frostKillSevere, but cooler than frostKill
      if(stage < fi)
         {
         //the plant will survive but all of the leaf area is removed except a fraction
         sprintf(msg,"Frost Event: (Non Fatal) \n");
         scienceAPI.write(msg);
         sprintf(msg, "\t\tMin Temp      = %.2f \t\t Senesced LAI    = %.2f\n", plant->today.minT, lai - 0.01);
         scienceAPI.write(msg);

         senescedLAI =  Max(0.0,lai - 0.1);
         }
      else if(stage > flowering)
         {
            //after flowering it takes a servereFrost to kill the plant
            //not sure what the effect on LAI should be after this stage
            senescedLAI = 0.0;
         }
      else
         {
            //between floralInit and Flowering the plant will be killed
            senescedLAI = lai;
         }
      }
   else
      {
         //Temperature is colder or equal to frostKillSevere parameter
         sprintf(msg, "Frost Event: (Fatal) \n");
         scienceAPI.write(msg);
         sprintf(msg, "\t\tMin Temp      = %.2f \t\t Senesced LAI    = %.2f\n", plant->today.minT, lai - 0.01);
         scienceAPI.write(msg);
         //the plant will be killed as it's LAI will be 0
         senescedLAI = lai;
      }
      
      return senescedLAI;
   }
/* TODO : put in messages */
//------------------------------------------------------------------------------------------------
double Leaf::calcLaiSenescenceWater(void)
   {
   double dlt_dm_transp = plant->biomass->getDltDMPotTE();

   double effectiveRue = plant->biomass->getEffectiveRue();

   double radnCanopy = divide (plant->getRadnInt(), coverGreen, plant->today.radn);

   double sen_radn_crit = divide (dlt_dm_transp, effectiveRue, radnCanopy);
   double intc_crit = divide (sen_radn_crit, radnCanopy, 1.0);

   //            ! needs rework for row spacing
   double laiEquilibWaterToday;
   if (intc_crit < 1.0)
      laiEquilibWaterToday = -log (1.0 - intc_crit) / extinctionCoef;
   else
      laiEquilibWaterToday = lai;

   laiEquilibWater.push_back(laiEquilibWaterToday);

   double avLaiEquilibWater = 0.0;int nRecs = laiEquilibWater.size();
   // average of the last 10 days of laiEquilibWater
   int start = ( nRecs > 10 ? nRecs - 9 : 1);
   for(int i = start;i <= nRecs;i++)
      avLaiEquilibWater += laiEquilibWater[i-1];
   avLaiEquilibWater /= Min(10,nRecs);


   double dltSlaiWater = 0.0;
   if(plant->water->getSdRatio() < senThreshold)
      dltSlaiWater = Max(0.0,divide((lai - avLaiEquilibWater) , senWaterTimeConst,0.0));

   dltSlaiWater = Min(lai,dltSlaiWater);

   return dltSlaiWater;
   }
//------------------------------------------------------------------------------------------------
//------- NITROGEN
//------------------------------------------------------------------------------------------------
double Leaf::calcNDemand(void)
   {
   // LEAF N demand (g/m2) to keep SLN = targetLeafSLN before flag, or sustain SLN after flag.

   //   double laiToday = Max(0.0, lai + dltLAI - dltSlai);
   double laiToday = calcLAI();
   double nRequired;
   stage = plant->phenology->currentStage();
   if(stage < flag)
      nRequired = laiToday * targetSLN;
   else
      nRequired = laiToday * Min(SLN0,targetSLN);

   double nToday = nGreen + dltNGreen;

   nDemand = Max(nRequired - nToday,0.0);
   return nDemand;
   }
//------------------------------------------------------------------------------------------------
double Leaf::calcNewLeafNDemand(void)
   {
   // New leaf demand is SLN = newLeafSLN (1.0)
   return dltLAI * newLeafSLN;
   }
//------------------------------------------------------------------------------------------------
double Leaf::calcLAI(void)
   {
   double laiToday = Max(0.0, lai + dltLAI - dltSlai);
   return laiToday;
   }
//------------------------------------------------------------------------------------------------
double Leaf::calcSLN(void)
   {
   double laiToday = calcLAI();
   double nGreenToday = nGreen + dltNGreen + dltNRetranslocate; //-ve
   double slnToday = divide(nGreenToday,laiToday);
   return slnToday;
   }
//------------------------------------------------------------------------------------------------
double Leaf::provideN(double requiredN, bool forLeaf)
   {
   // calculate the N available for translocation to other plant parts
   // N could be required for structural Stem/Rachis N, new leaf N or grain N
   // Canopy N is made available by dilution until SLN = 1.0 then by
   // dilution, reducing delta lai and senescence

   double laiToday = calcLAI();
   double slnToday = calcSLN();

   // total N available by dilution
   double dilutionN = plant->phenology->getDltTT()
      * (dilnNSlope * slnToday + dilnNInt) * laiToday;
   dilutionN = Max(dilutionN,0.0);

   // pre anthesis, get N from dilution, reduction in dltLai and senesence
   if(stage <= flowering)
      {
      double nProvided = Min(dilutionN,requiredN/2.0);
      dltNRetranslocate -= nProvided;
      requiredN -= nProvided;
      if(requiredN <= 0.0001)
         return nProvided;

      // not sufficient N from dilution - take from decreasing dltLai and senescence
      if(dltLAI > 0)
         {
          // Only half of the required N can be accounted for by reducing DltLAI
          // If the RequiredN is large enough, it will result in 0 new growth
          // Stem and Rachis can technically get to this point, but it doesn't
          // occur in any of the validation data sets.
         double n = dltLAI * newLeafSLN;
         double laiN = Min(n,requiredN/2.0);
         dltLAI = (n - laiN) / newLeafSLN;
         if (forLeaf)
         {
             requiredN -= laiN;
             //nProvided += laiN;
         }
         }
      // recalc the SLN after this N has been removed
      laiToday = calcLAI();
      slnToday = calcSLN();
      double maxN = plant->phenology->getDltTT()
         * (dilnNSlope * slnToday + dilnNInt) * laiToday;
	  maxN = Max(maxN, 0);
      requiredN = Min(requiredN,maxN);

      double senescenceLAI = Max(divide(requiredN,(slnToday-senescedLeafSLN)),0.0);

      double newN = Max(senescenceLAI * (slnToday-senescedLeafSLN),0.0);
      dltNRetranslocate -= newN;
      nProvided += newN;
      dltSlaiN += senescenceLAI;
      dltSlai = Max(dltSlai,dltSlaiN);

      dltNSenesced += senescenceLAI * senescedLeafSLN;

      return nProvided;
      }
   else        // post anthesis
      {
      // if sln > 1, dilution then senescence
      if(slnToday > 1.0)
         {
         double nProvided = Min(dilutionN,requiredN);
         dltNRetranslocate -= nProvided;
         requiredN -= nProvided;
         if(requiredN <= 0.0001)
            return nProvided;

         // rest from senescence
         // recalc the SLN after this N has been removed
         laiToday = calcLAI();
         slnToday = calcSLN();
         double maxN = plant->phenology->getDltTT()
            * (dilnNSlope * slnToday + dilnNInt) * laiToday;
		 maxN = Max(maxN, 0);
         requiredN = Min(requiredN,maxN);

         double senescenceLAI = Max(divide(requiredN,(slnToday-senescedLeafSLN)),0.0);

         double newN = Max(senescenceLAI * (slnToday-senescedLeafSLN),0.0);
         dltNRetranslocate -= newN;
         nProvided += newN;
         dltSlaiN += senescenceLAI;
         dltSlai = Max(dltSlai,dltSlaiN);

         dltNSenesced += senescenceLAI * senescedLeafSLN;

         return nProvided;
         }
      else     // sln < 1.0
         {
         // half from dilution and half from senescence
         // dilution
         double nProvided = Min(dilutionN,requiredN/2.0);
         dltNRetranslocate -= nProvided;
         requiredN -= nProvided;

         // senescence
         // recalc the SLN after this N has been removed
         laiToday = calcLAI();
         slnToday = calcSLN();
         double maxN = plant->phenology->getDltTT()
            * (dilnNSlope * slnToday + dilnNInt) * laiToday;
		 maxN = Max(maxN, 0);
         requiredN = Min(requiredN,maxN);

         double senescenceLAI = Max(divide(requiredN,(slnToday-senescedLeafSLN)),0.0);

         double newN = Max(senescenceLAI * (slnToday-senescedLeafSLN),0.0);
         dltNRetranslocate -= newN;
         nProvided += newN;
         dltSlaiN += senescenceLAI;
         dltSlai = Max(dltSlai,dltSlaiN);

         dltNSenesced += senescenceLAI * senescedLeafSLN;

         return nProvided;
         }
      }
   }
//------------------------------------------------------------------------------------------------
// Leaf Number / Leaf Size   / Appearance
//------------------------------------------------------------------------------------------------
void Leaf::initLeafNo(void)
   {
   nLeaves = noEmergence;
   leafNo[2] = noEmergence;
   }
//------------------------------------------------------------------------------------------------
// estimate the final leaf no from an approximated thermal time
//  emergence to floral initiation.
//------------------------------------------------------------------------------------------------
void Leaf::calcFinalLeafNo(void)
   {
	double ttFi = plant->phenology->sumTTtarget(emergence,fi);
	finalLeafNo = bound(divide(ttFi,initRate) + noSeed,minLeafNo,maxLeafNo);
   }
//------------------------------------------------------------------------------------------------
void Leaf::calcLeafAppearance(void)
   {
   dltLeafNo = 0.0;
   double remainingLeaves = finalLeafNo - nLeaves;
   if(remainingLeaves <= 0.0)
      {
      return;
      }
   // Peter's 2 stage version used here, modified to apply to last few leaves before flag
   // i.e. c_leaf_no_rate_change is leaf number from the top down (e.g. 4)
   double leafAppRate;
   if (remainingLeaves <= noRateChange2)
      {
      leafAppRate = appearanceRate3;
      }
   else if (remainingLeaves <= noRateChange1)
      {
      leafAppRate = appearanceRate2;
      }
   else leafAppRate = appearanceRate1;

   // if leaves are still growing, the cumulative number of phyllochrons or fully expanded
   // leaves is calculated from thermal time for the day.
   dltLeafNo = bound(divide(plant->phenology->getDltTT(),leafAppRate),0.0,remainingLeaves);
   }
//------------------------------------------------------------------------------------------------
void Leaf::calcLeafSize(void)
   {
   leafSize.clear();
   for(int i=0;i < finalLeafNo;i++)
      leafSize.push_back(calcIndividualLeafSize(i+1));

   }
//------------------------------------------------------------------------------------------------
double Leaf::calcIndividualLeafSize(double leafNo)
   {
   // use finalLeafNo to calculate the size of the individual leafs
   // Eqn 5 from Improved methods for predicting individual leaf area and leaf senescence in maize
   // (Zea mays) C.J. Birch, G.L. Hammer and K.G. Ricket. Aust. J Agric. Res., 1998, 49, 249-62
	// TODO	externalise these variables
	   
   double a0 = bellCurveParams[0];
   double a1 = bellCurveParams[1];
   double b0 = bellCurveParams[2];
   double b1 = bellCurveParams[3];

	double aMaxA = largestLeafParams[0];
   double aMaxB = largestLeafParams[1];
	double aMaxC = largestLeafParams[2];

   double a = a0 - exp(a1 * finalLeafNo);						// Eqn 18
   double b = b0 - exp(b1 * finalLeafNo);						// Eqn 19

   double aMax = aMaxA *exp(aMaxB + aMaxC * finalLeafNo);			// Eqn 13

	double x0 = aX0 * finalLeafNo;											// Eqn 14

	return aMax * exp(a * pow((leafNo - x0),2) + b * pow((leafNo - x0),3)) * 100;  // Eqn 5
   }
//------------------------------------------------------------------------------------------------

void Leaf::leafAreaPotBellShapeCurve(void)
   {
   //once leaf no is calculated leaf area of largest expanding leaf is determined
   double leafNoEffective = Min(sumVector(leafNo) + leafNoCorrection, finalLeafNo);

   dltPotentialLAI = dltLeafNo *
      calcIndividualLeafSize(leafNoEffective) * smm2sm * density;

   return;
   }

//------------------------------------------------------------------------------------------------

double Leaf::calcStressedLeafArea()
   {
    double expansionStress = Min(plant->water->getExpansionStress(), Min(plant->nitrogen->getExpansionStress(), plant->phosphorus->getExpansionStress()));
   return dltPotentialLAI * expansionStress;
   }
//------------------------------------------------------------------------------------------------
double Leaf::calcMaxLaiPossible(void)
   {
   waterStressLaiLoss += (dltPotentialLAI - dltStressedLAI);
   return lai + sLai - waterStressLaiLoss;
   }
//------------------------------------------------------------------------------------------------
//  Return the lai that would senesce on the current day from natural ageing
//------------------------------------------------------------------------------------------------
double Leaf::calcLaiSenescenceAge(void)
   {
   dltDeadLeaves = calcDltDeadLeaves();
   double deadLeaves = nDeadLeaves + dltDeadLeaves;
   double laiSenescenceAge = 0.0;
   if(deadLeaves)
      {
      int leafDying = (int) ceil(deadLeaves);
      double areaDying  = fmod(deadLeaves,(double)1.0) * leafSize[leafDying-1];
      laiSenescenceAge = (sumVector(leafSize,leafDying - 1) + areaDying) * smm2sm * density;
      }
   return  Max(laiSenescenceAge - sLai,0.0);
   }
//------------------------------------------------------------------------------------------------
double Leaf::calcDltDeadLeaves(void)
   {
   double nDeadYesterday = nDeadLeaves;
   double nDeadToday = finalLeafNo *
      (deadLeafConst + deadLeafSlope * plant->phenology->sumTTtotal(emergence,maturity));
   nDeadToday = bound(nDeadToday,nDeadYesterday,finalLeafNo);
   return nDeadToday - nDeadYesterday;
   }
//------------------------------------------------------------------------------------------------
//  Return the lai that would senesce on the current day from light (crowding)
double Leaf::calcLaiSenescenceLight(void)
   {
   //    senRadnCrit
   double critTransmission = divide(senRadnCrit,plant->today.radn);
   /* TODO : needs rework for row spacing */

   double laiEqlbLightToday;
   if (critTransmission > 0.0)laiEqlbLightToday = -log (critTransmission)/extinctionCoef;
   else laiEqlbLightToday = lai;


   // average of the last 10 days of laiEquilibLight
   laiEquilibLight.push_back(laiEqlbLightToday);
   double avLaiEquilibLight = movingAvgVector(laiEquilibLight,10);

   double radnTransmitted = plant->today.radn - plant->getRadnInt();
   double dltSlaiLight = 0.0;
   if (radnTransmitted < senRadnCrit)
      dltSlaiLight = Max(0.0,divide (lai - avLaiEquilibLight, senLightTimeConst , 0.0));
   dltSlaiLight = Min(dltSlaiLight,lai);
   return dltSlaiLight;
   }
//------------------------------------------------------------------------------------------------
void Leaf::calcCover()
   {
   double skipRow = plant->getSkipRow();
   coverGreen = divide(1.0 - exp(-extinctionCoef * lai * skipRow), skipRow,0.0);
   coverSen = divide(1.0 - exp(-extinctionCoef * sLai * skipRow), skipRow,0.0);
	coverTot = 1.0 - (1.0 - coverGreen) * (1 - coverSen);
   }
//------------------------------------------------------------------------------------------------
double Leaf::calcEmergFlagTT(void)
   {
   // estimate emergence to flag using leaf appearance rates
   double nLeavesAtChange1 = bound(finalLeafNo - noRateChange1,noEmergence,finalLeafNo);
   double nLeavesAtChange2 = bound(finalLeafNo - noRateChange2,noEmergence,finalLeafNo);
   return (nLeavesAtChange1 - noEmergence) * appearanceRate1 +
      (nLeavesAtChange2 - nLeavesAtChange1) * appearanceRate2 +
      (finalLeafNo - nLeavesAtChange2) * appearanceRate3; 
   }
//------------------------------------------------------------------------------------------------
double Leaf::laiToday(void)const
   {
   return Max(0.0, lai + dltLAI - dltSlai);
   }
//------------------------------------------------------------------------------------------------
void Leaf::addDltSlai(double add)
   {
   dltSlai += add;
   dltSlai = Min(sLai, lai + dltLAI);
   }
//------------------------------------------------------------------------------------------------
void Leaf::calcSenescence(void)
   {
   // Derives seneseced plant dry matter (g/m^2) for the day
   // calculate scenesced N
   double laiToday = lai + dltLAI;

   double dmGreenLeafToday = dmGreen + dltDmGreen + dmRetranslocate;               // -ve
   double slaToday = divide(laiToday,dmGreenLeafToday);
   double nGreenToday = nGreen + dltNGreen + dltNRetranslocate;

   // dh - dltSenescedLai can be greater than slaToday if we senesce most of the leaf.
   // In this scenario, dltDmSenesced could end up greater than dmGreen(!).
   // To fix this, we divide by start-of-day (pre-senescence) sla.

   dltDmSenesced = divide(dltSlai,slaToday);

   double slnToday = divide(nGreenToday,laiToday);
   dltNSenesced += dltSlai * Max(slnToday, 0.0);
   }
//------------------------------------------------------------------------------------------------
double Leaf::partitionDM(double dltDM)
   {
   double dltDmLeafMax = divide (dltStressedLAI,slaMin.value(lai) * smm2sm);

   double leafPartitionCoef = 1.0 / (1.0 + leafPartitionRate * pow(nLeaves,(double)2.0));
   // limit the delta leaf area to maximum  using sla
   dltDmGreen = Min(leafPartitionCoef * dltDM,dltDmLeafMax);
   return dltDmGreen;
   }
//------------------------------------------------------------------------------------------------
double Leaf::dmRetransAvailable(void)
   {
   // calculate dry matter available for translocation to grain
   double leafWt = dmGreen + dltDmGreen;
   double leafWtAvail = leafWt - dmPlantMin * density;
   return Max(leafWtAvail,0.0);
   }
//------------------------------------------------------------------------------------------------
//------- Calculate detachment of lai (based upon fractional decay rates)
//------------------------------------------------------------------------------------------------
void Leaf::laiDetachment(vector<double> senDetachFrac)
   {
   //These are change within the call but are not used anywhere
   double sLaiDetachedDelta;
   //double dLaiDetachedDelta;

   //Do Calculations
   calcPartFractionDelta (partNo, senDetachFrac, sLai, sLaiDetachedDelta);
   }
//------------------------------------------------------------------------------------------------
double Leaf::calcPDemand(void)
   {
   // Leaf P demand
   double rel_growth_rate = divide(plant->biomass->getDltDMPotRUE(),
      plant->biomass->getAboveGroundBiomass(),0.0);

   double deficit = pConcMax() * dmGreen * (1.0 + rel_growth_rate) - pGreen;

   pDemand = Max(deficit,0.0);
   return pDemand;
   }
//------------------------------------------------------------------------------------------------
void Leaf::Summary(void)
   {
   char msg[120];
   sprintf(msg,"Maximum LAI           = %.1f \t\t Number of leaves        = %.1f\n",
      maxLai,nLeaves);
   scienceAPI.write(msg);
   }
//------------------------------------------------------------------------------------------------
void Leaf::calcTplaMax(void)
   {
   tplaMax = (pow(plant->getFtn() + (double)1.0, tillerCoef) * pow(finalLeafNo,mainStemCoef)) * scm2smm;
   }
//------------------------------------------------------------------------------------------------
double Leaf::calcDltPotentialTPLA(void)
   {
   // need to ramp the dltPotTPLA for the first 60Cd because the function evaluates to approx 2000
   // when ttElapsed = 0
   double ttTPLAPhase = plant->phenology->sumTTtarget(emergence,flag);
   double ttElapsed   = plant->phenology->sumTTtotal (emergence,flag);
   double tplaInflection = ttTPLAPhase * tplaInflectionRatio;
   double tplaToday;

   if(ttElapsed < 80)
      {
      double exponent = tplaProductionCoef * (80 - tplaInflection);
      double tpla80 = divide(tplaMax,1.0 + exp(-1 * exponent));
      tplaToday = divide(ttElapsed,80) * tpla80;
      }
   else
      {
      double exponent = tplaProductionCoef * (ttElapsed - tplaInflection);
      tplaToday = divide(tplaMax,1.0 + exp(-1 * exponent));
      }
   tplaToday = Max(tplaToday,tplaPot);
   return tplaToday - tplaPot;
   }
//------------------------------------------------------------------------------------------------

