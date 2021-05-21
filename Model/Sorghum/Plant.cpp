
#include <stdio.h>
#include <math.h>

#include <vector>
#include <General/date_class.h>
#include <General/string_functions.h> //dcaps
#include <ComponentInterface2/ScienceAPI2.h>

#include "Plant.h"
#include "PlantInterface.h"
#include "LeafCulms.h"

using namespace std;
using namespace Sorghum;

//------------------------------------------------------------------------------------------------
//------------- Plant Constructor
//------------------------------------------------------------------------------------------------
Plant::Plant(ScienceAPI2 &api) : scienceAPI(api)
   {
   leafAreaCalcType = "tpla";
   initialize();
   }
//------------------------------------------------------------------------------------------------
void Plant::initialize(void)
   {
   eo =    0.0;
   das =   0;
   stage = 0.0;
   tempStress = 0.0;
   dltPlants =  0.0;
   frIntcRadn = 0.0;
   dltDeadPlants = 0.0;

   vpd = 0.0;
   radnIntercepted  = 0.0;
   ftn = 0.0;
}
//------------------------------------------------------------------------------------------------
//------------ read the crop and cultivar parameters
//------------------------------------------------------------------------------------------------
void Plant::readParams(void)
   {
	scienceAPI.write(string(" - reading constants for " +
							cropClass + "(" + cropType +") - " + cultivar + "\n"));

	tempStressTable.read(scienceAPI, "x_ave_temp","y_stress_photo");

	scienceAPI.read("rue", "", 0, rue, 0.0f, 10.0f);
	scienceAPI.read("transp_eff_cf", "", 0, transpEffCf, 0.0f, 0.1f);
	scienceAPI.read("svp_fract", "", 0, svpFract, 0.0f, 1.0f);
	scienceAPI.read("tt_emerg_limit", "", 0, ttEmergeLimit);

	//Read arrays for detachment
	if (!scienceAPI.read("sen_detach_frac", "", true, senDetachFrac))
		senDetachFrac.assign(5, 0.0);

	// CO2 stuff
	co2_te_modifier.read(scienceAPI, "x_co2_te_modifier", "y_co2_te_modifier");

	if (!scienceAPI.read("vp_source", "", true, vpSource))
		vpSource = "internal";
	To_lower(vpSource);

}

//------------------------------------------------------------------------------------------------
//------------- Plant Destructor
//------------------------------------------------------------------------------------------------
Plant::~Plant()
   {
   delete roots;
   delete leaf;
   delete stem;
   delete rachis;
   delete grain;
   
   delete phenology;
   delete nitrogen;
   delete phosphorus;
   delete water;
   delete biomass;
   }
//------------------------------------------------------------------------------------------------
//------------- Initialise plant
//------------------------------------------------------------------------------------------------
void Plant::plantInit1(void)
  {
   // parameters
   // initialise any variable that is needed before the crop is planted
   vector<string> sections;
   sections.push_back("constants");
   scienceAPI.setSearchOrder(sections);

   scienceAPI.read("crop_type",           "", false, cropType);
   scienceAPI.read("default_crop_class",  "", false, defaultCropClass);
   scienceAPI.read("row_spacing_default", "", false, rowSpacingDefault);
   scienceAPI.read("leaf_area_calc_type", "", true, leafAreaCalcType);

   roots = new Roots(scienceAPI, this);   PlantComponents.push_back(roots); PlantParts.push_back(roots);
	if (leafAreaCalcType == "bellcurve")
	{
		leaf = new LeafCulms(scienceAPI, this); 
	}
	else 
	{
		leaf = new LeafCulms_Fixed(scienceAPI, this);    
	}
   
   PlantComponents.push_back(leaf);  
   PlantParts.push_back(leaf);

   stem      = new Stem(scienceAPI, this);    PlantComponents.push_back(stem);  PlantParts.push_back(stem);
   rachis    = new Rachis(scienceAPI, this);  PlantComponents.push_back(rachis);PlantParts.push_back(rachis);
   grain     = new Grain(scienceAPI, this);   PlantComponents.push_back(grain); PlantParts.push_back(grain);

   phenology = new Phenology(scienceAPI, this); PlantComponents.push_back(phenology);
                                      PlantProcesses.push_back(phenology);
   nitrogen  = new Nitrogen(scienceAPI, this);  PlantComponents.push_back(nitrogen);
                                      PlantProcesses.push_back(nitrogen);
   phosphorus  = new Phosphorus(scienceAPI, this);  PlantComponents.push_back(phosphorus);
                                      PlantProcesses.push_back(phosphorus);
   water     = new Water(scienceAPI, this);     PlantComponents.push_back(water);
                                      PlantProcesses.push_back(water);
   biomass   = new Biomass(scienceAPI, this);   PlantComponents.push_back(biomass);
                                      PlantProcesses.push_back(biomass);

   doRegistrations();

   setStatus(out);

   //  sowing/harvesting events to UI
   scienceAPI.notifyFutureEvent("sowing");
   scienceAPI.notifyFutureEvent("harvesting");
   

   transpEff = 0.0;
   co2 = 0.0;
   coverGreen = 0.0;
  }
//------------------------------------------------------------------------------------------------
void Plant::plantInit2(void)
  {
  }
//------------------------------------------------------------------------------------------------
void Plant::setStatus(Status status)
   {
   plantStatus = status;
   char statusStrings[3][6] = {"out", "dead", "alive"};
   statusString = statusStrings[status];
   }
//------------------------------------------------------------------------------------------------
//------------------- Field a Sow event
//------------------------------------------------------------------------------------------------
void Plant::onSowCrop(SowType &sow)
   {
   if(plantStatus != out)
      throw std::runtime_error("Crop is still in the ground -\n unable to sow until it is\n taken out by \"end_crop\" action.");

   scienceAPI.write("Sowing initiate\n");

   string temp;
   if (sow.crop_class == "")
      cropClass = defaultCropClass;
   else
      cropClass = sow.crop_class;

   if (sow.Cultivar == "")
      throw std::runtime_error("Cultivar not specified");
   else
      cultivar = sow.Cultivar;

   plantDensity = (float)sow.plants;
   if (plantDensity == 0)
      throw std::runtime_error("plant density ('plants') not specified");

   checkRange(scienceAPI, plantDensity, 0.0, 1000.0, "plants");

   sowingDepth = (float)sow.sowing_depth;
   if (sowingDepth == 0)
      throw std::runtime_error("sowing depth not specified");

   checkRange(scienceAPI,sowingDepth, 0.0, 100.0, "sowing_depth");

   rowSpacing = (float)sow.row_spacing;
   if (rowSpacing == 0)
      rowSpacing = (float)rowSpacingDefault;
   // row spacing was originally in metres
   // for compatibility, is now in mm
   // if < 10, assume metres and convert
   if(rowSpacing < 10.0)
      {
      rowSpacing *= 1000;
      scienceAPI.write("\n                 Row spacing converted from m to mm\n");
      }
   checkRange(scienceAPI, rowSpacing, 100.0, 10000.0, "row_spacing");

   skipRow = 1.0;
   if (sow.Skip != "")
      {
      if (sow.Skip == "single")skipRow = 1.5;
      else if (sow.Skip == "double")skipRow = 2.0;
      else if (sow.Skip == "solid")skipRow = 1.0;
      else
        throw std::runtime_error("Unknown skip row configuration '" + sow.Skip + "'");
      }             

   checkRange(scienceAPI,skipRow, 0.0, 2.0, "skiprow");
   
   if (sow.tiller_no_fertile == "")
      {
      scienceAPI.get( "latitude",  "", 0, latitude, -90.0f, 90.0f);
      // if no tiller number is entered, estimate it
      if(!estimateTillers(ftn))
         throw std::runtime_error("Cannot estimate Fertile tiller number at this location");
      }
   else
      ftn = atof(sow.tiller_no_fertile.c_str());


//   latitude = -29.0


   checkRange(scienceAPI,ftn, 0.0, 10.0, "tiller_no_fertile");


   phenology->setStage(sowing);
   setStatus(alive);

   char msg[120];
   scienceAPI.write("\n                 Crop Sowing Data\n");
   scienceAPI.write("    -------------------------------------------------------\n");
   scienceAPI.write("    Sowing   Depth  Plants Spacing Skiprow Cultivar    FTN\n");
   scienceAPI.write("    Day no     mm     m^2    mm     code     name       no\n");
   scienceAPI.write("    -------------------------------------------------------\n");

   sprintf(msg, "   %7d%8.1f%8.1f%6.0f%7.1f     %s%8.2f\n",
               today.doy, sowingDepth, plantDensity, rowSpacing,
               skipRow, cultivar.c_str(), ftn);   scienceAPI.write(msg);

   scienceAPI.write("    -------------------------------------------------------\n");
   scienceAPI.write("\n");

   // Set up which sections to look for parameters
   vector<string> sections;
   sections.push_back(cultivar);
   scienceAPI.setSearchOrder(sections);

   // now we have the cultivar, get all 'constants' and cultivar parameters.
   readParams(); 

   for(unsigned i=0;i < PlantComponents.size();i++) 
     PlantComponents[i]->readParams ();

   scienceAPI.publish("sowing");
   justSown = true;
   }
//------------------------------------------------------------------------------------------------
//------------------- Field a Prepare event
//------------------------------------------------------------------------------------------------
void Plant::prepare (void)
   {
   if (!scienceAPI.get("co2", "mg/kg", true, co2, 300.0f, 1000.0f))
       co2 = 350.0;
   else 
       if (fabs(co2 - 350.0) > 0.1 && co2_te_modifier.x.size() == 0) 
	       throw std::runtime_error("CO2 is not 350ppm and there is no CO2xTE parameterisation");
		   
   tempStress = tempStressTable.value(today.avgT);

   radnIntercepted = radnInt();

   float rueToday = (float)(rue * rue_co2_modifier());

   biomass->calcBiomassRUE(rueToday,radnIntercepted);
   transpEff = transpEfficiency();
   water->calcDemand();

      // at beginning of day, reset the daily dlt variables
   for(unsigned i=0;i < PlantParts.size();i++)
      {
      PlantParts[i]->resetDailyVars();
      }

   if(phosphorus->Active())
       phosphorus->prepare();


   }
//------------------------------------------------------------------------------------------------
//------------------- Field a process event
//------------------------------------------------------------------------------------------------
void Plant::process (void)                 // do crop preparation
   {
   stage = (float)phenology->currentStage();

   water->getOtherVariables();
   water->calcDailySupply();
	
	if (biomass->useDCAPS())
	{
		biomass->calcBiomassDCAPS();
		//DCAPS give sw demand
	}
	
	water->calcStresses();
	water->calcUptake();

   stem->calcCanopyHeight();

   leaf->calcLeafNo();

   phenology->development();
   stage = (float)phenology->currentStage();
   for (unsigned i = 0; i < PlantParts.size(); i++)
   {
	   PlantParts[i]->setStage(stage);
   }

   leaf->calcPotentialArea();

   biomass->calcBiomassTE();
   biomass->calcDltBiomass();

   grain->process();

   // biomass partitioning
   biomass->calcPartitioning();

   // biomass retranslocation
   if(stage >= startGrainFill && stage < endGrainFill)
      biomass->calcRetranslocation();

   // root length
   if(stage > germination)
      {
      roots->calcRootDistribution();
      }
   // actual dltLai C limited
   leaf->areaActual();


   // scenescence
   leaf->senesceArea();
   if(stage > germination)
      {
      roots->calcSenLength();
      }

   // nitrogen
   nitrogen->process();
   biomass->dmScenescence();         // moved because nitrogen now causes senescence

   if(phosphorus->Active())
       phosphorus->process();

   death();

   //Calculate detachment
   detachment();

   // at end of day, update class state variables
   for(unsigned i=0;i < PlantComponents.size();i++)
      {
      PlantComponents[i]->updateVars();
      }
//   phenology->development();
   updateVars();
   }
//------------------------------------------------------------------------------------------------
void Plant::updateVars(void)
   {
   das++;

   stage = (float)phenology->currentStage();
   // this is here for sysbal - needs to move!
   if(stage == emergence)
      {
      ExternalMassFlowType EMF;
      EMF.PoolClass = "crop";
      EMF.FlowType = "gain";
      EMF.DM = (float)(biomass->getTotalBiomass() * gm2kg/sm2ha);
      EMF.N  = 0.0;
      EMF.P  = 0.0;
      EMF.C = 0.0; // ?????
      EMF.SW = 0.0;

      scienceAPI.publish("ExternalMassFlow", EMF);
      }

   }
//------------------------------------------------------------------------------------------------
void Plant::death(void)
   {
   // Emergence
   float ttEmergeLimit;
   scienceAPI.read("tt_emerg_limit", "", 0, ttEmergeLimit);

   if(stage < emergence)
      {
      if(phenology->sumTTtotal(germination,harvest) > ttEmergeLimit)
         {
      scienceAPI.write(" ********** Crop failed emergence due to deep planting\n");
         dltDeadPlants = -plantDensity;
         }
      }


   //If leaves are killed from frost, leaf->dltSlai is set to leaf->lai
   //need to kill plant if lai = 0
   //gmc & rlv
   /* TODO : Check this to see what happens if LAI < 0.1 before flag - or if possible */
   //kill plant if lai falls to 0 - very severe stress or severe frost event
   char msg[120];

   if (stage > fi && stage < maturity)
      {
      if (leaf->killedByFrostToday || 
          leaf->getLAI() - leaf->getDltSlai() < 0.0001)
         {
         dltDeadPlants = -plantDensity;
         scienceAPI.write(" ********** Crop failed due to loss of leaf area ********\n");
         sprintf(msg, "\tLAI: %.3f \t\tDltSLAI: %.3f \t\tDltLAI: %.2f\n", leaf->getLAI(), leaf->getDltSlai(), leaf->getDltLAI());
         scienceAPI.write(msg);
         }
      }

   if (stage >= flag && stage < maturity)
      {
      if (leaf->killedByFrostToday || 
          leaf->getLAI() - leaf->getDltSlai() < 0.1)
         {
         dltDeadPlants = -plantDensity;
         scienceAPI.write(" ********** Crop failed due to loss of leaf area ********");
         sprintf(msg, "\tLAI: %.3f \t\tDltSLAI: %.3f \t\tDltLAI: %.2f\n", leaf->getLAI(), leaf->getDltSlai(), leaf->getDltLAI());
         scienceAPI.write(msg);
         }
      }

   //Check to see if plant death should terminate crop
   if(plantDensity + dltDeadPlants < 0.01)
      {
      killCrop();
      }
   }
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
//- Science --------------------------------------------------------------------------------------
//----------  Radiation intercepted by leaves (mj/m^2)
//------------------------------------------------------------------------------------------------
double Plant::radnInt(void)
   {
   if (isEqual(frIntcRadn,0.0))return leaf->getCoverGreen() * today.radn;
   else
      // interception has already been calculated for us
      return frIntcRadn * today.radn;
   }
////------------------------------------------------------------------------------------------------
////------------------------------------------------------------------------------------------------
//---   Calculate today's transpiration efficiency from the transpiration efficiency coefficient
//---   and vapour pressure deficit, which is calculated from min and max temperatures.
//------------------------------------------------------------------------------------------------
double Plant::transpEfficiency(void)
{
	// get vapour pressure deficit when net radiation is positive.
	if (vpSource == "apsim" && today.vp > 0.0)
		vpd = std::max(0.01, svpFract * svp(today.maxT) + (1.0 - svpFract) * svp(today.minT) - today.vp * mb2kpa);
	else
		vpd = std::max(svpFract * (svp(today.maxT) - svp(today.minT)), 0.01);

	return divide(transpEffCf, vpd, 0.0) / g2mm;
}
//------------------------------------------------------------------------------------------------
//-------- function to get saturation vapour pressure for a given temperature in oC (kpa)
//------------------------------------------------------------------------------------------------
double Plant::svp(double temp)
   {
   return 6.1078 * exp(17.269 * temp / (237.3 + temp)) * mb2kpa;
   }
//------------------------------------------------------------------------------------------------
//-------- function to calculate detachment
//------------------------------------------------------------------------------------------------
void Plant::detachment(void)
   {
   biomass->detachment(senDetachFrac);
   leaf->laiDetachment(senDetachFrac);
   nitrogen->detachment(senDetachFrac);
   }

//------------------------------------------------------------------------------------------------
//-------- Kill the crop
//------------------------------------------------------------------------------------------------
void Plant::killCrop(void)
   {
   if (plantStatus == alive)
      {
      setStatus(dead);
      }

   //Report
   char msg[120];
   sprintf(msg, "Plant Death. Standing above-ground dm = %.2f (kg/ha)\n", 
           biomass->getAboveGroundBiomass());
   scienceAPI.write(msg);
   }
//------------------------------------------------------------------------------------------------
void Plant::phenologyEvent(int iStage)
   {
   //  report

   char msg[120];   
   sprintf(msg,"\t\tBiomass       = %6.2f \t\t LAI            = %6.2f\n",
      biomass->getAboveGroundBiomass() / 10.0, leaf->getLAI());
   scienceAPI.write(msg);
   sprintf(msg,"\t\tStover N Conc = %6.2f \t\t Extractable SW = %6.2f\n",
      nitrogen->getNStover(),water->getESWAvail());
   scienceAPI.write(msg);

   // output the current stage
   string stage = phenology->returnStageName();
   scienceAPI.publish(stage);

   }
//------------------------------------------------------------------------------------------------
void Plant::get_cover_green(float &result)
      {
   result = (float)leaf->getCoverGreen();
      }
//------------------------------------------------------------------------------------------------
void Plant::get_cover_tot(float &result)
      {
   result =(float) leaf->getCoverTot();
      }
//------------------------------------------------------------------------------------------------
void Plant::get_height(float &result)
      {
   result = (float)stem->getCanopyHeight();
   }
//------------------------------------------------------------------------------------------------
double Plant::rue_co2_modifier(void)                 //!CO2 level (ppm)
   {
   //  Purpose : Calculation of the CO2 modification on rue
   const double scale = 1.0 / 350.0 * 0.05;
   return (scale * this->co2 + 0.95); //Mark Howden, personal communication
   }
double Plant::getTranspEff(void) const
   {
   if (co2_te_modifier.x.size() > 0)
     return (transpEff * co2_te_modifier.value(co2));
   return (transpEff);
   }
//------------------------------------------------------------------------------------------------
//------------------- Estimate tillers
//------------------------------------------------------------------------------------------------
bool Plant::estimateTillers(double &ftn)
   {
   // estimate tillering given latitude, density, time of planting and row configuration
   // this will be replaced with dynamic calculations in the near future
   // above latitude -25 is CQ, -25 to -29 is SQ, below is NNSW

   double intercept, slope;

   if(latitude > -12.5 || latitude < -38.0)
      return false;                                // unknown region

   if(latitude > -25.0)                            // CQ
      {
      if(today.doy < 319 && today.doy > 182)       //   1-Jul  - 15-Nov 
         {
         if(skipRow > 1.9)                         // double  (2.0)
            {
            intercept = 0.5786; slope = -0.0521;
            }
         else if(skipRow > 1.4)                    // single  (1.5)
            {
            intercept = 0.8786; slope = -0.0696;
            }
         else                                      // solid   (1.0)
            {
            intercept = 1.1786; slope = -0.0871;
            }
         }
      else                                         //  > 15-Nov
         {
         if(skipRow > 1.9)                         // double  (2.0)
            {
            intercept = 0.4786; slope = -0.0421;
            }
         else if(skipRow > 1.4)                    // single  (1.5)
            {
            intercept = 0.6393; slope = -0.0486;
            }
         else                                      // solid   (1.0)
            {
            intercept = 0.8000; slope = -0.0550;
            }
         }
      }
   else if(latitude > -29.0)                       // SQ
      {
      if(today.doy < 319 && today.doy > 182)       //   1-Jul  - 15-Nov 
         {
         if(skipRow > 1.9)                         // double  (2.0)
            {
            intercept = 1.1571; slope = -0.1043;
            }
         else if(skipRow > 1.4)                    // single  (1.5)
            {
            intercept = 1.7571; slope = -0.1393;
            }
         else                                      // solid   (1.0)
            {
            intercept = 2.3571; slope = -0.1743;
            }
         }
      else                                         //  > 15-Nov
         {
         if(skipRow > 1.9)                         // double  (2.0)
            {
            intercept = 0.6786; slope = -0.0621;
            }
         else if(skipRow > 1.4)                    // single  (1.5)
            {
            intercept = 1.1679; slope = -0.0957;
            }
         else                                      // solid   (1.0)
            {
            intercept = 1.6571; slope = -0.1293;
            }
         }
      }
   else                                            // NNSW
      {
      if(today.doy < 319 && today.doy > 182)       //  1-Jul  - 15-Nov 
         {
         if(skipRow > 1.9)                         // double  (2.0)
            {
            intercept = 1.3571; slope = -0.1243;
            }
         else if(skipRow > 1.4)                    // single  (1.5)
            {
            intercept = 2.2357; slope = -0.1814;
            }
         else                                      // solid   (1.0)
            {
            intercept = 3.1143; slope = -0.2386;
            }
         }
      else if (today.doy > 349 || today.doy < 182)   // > 15-Dec and < 1-Jul
         {
         if(skipRow > 1.9)                         // double  (2.0)
            {
            intercept = 0.4000; slope = -0.0400;
            }
         else if(skipRow > 1.4)                    // single  (1.5)
            {
            intercept = 1.0571; slope = -0.0943;
            }
         else                                      // solid   (1.0)
            {
            intercept = 1.7143; slope = -0.1486;
            }
         }
      else                                         // > 15-Nov < 15 -Dec
         {
         if(skipRow > 1.9)                         // double  (2.0)
            {
            intercept = 0.8786; slope = -0.0821;
            }
         else if(skipRow > 1.4)                    // single  (1.5)
            {
            intercept = 1.6464; slope = -0.1379;
            }
         else                                      // solid   (1.0)
            {
            intercept = 2.4143; slope = -0.1936;
            }
         }
      }

   ftn = Max(slope * plantDensity + intercept,0.0);
   return true;
   }
//------------------------------------------------------------------------------------------------


