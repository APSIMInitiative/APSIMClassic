using System;
using System.Collections.Generic;
using System.Text;
using CSGeneral;

public class Nodule : GenericOrgan, BelowGround
{
  #region Class Data Members
    public double RespiredWt = 0;
    public double PropFixationDemand = 0;
    public double _NFixed = 0;

    [Link]
    Function FixationMetabolicCost = null;

    [Link]
    Function SpecificNitrogenaseActivity = null;

    [Link]
    Function FT = null;

    [Link]
    Function FW = null;

    [Link]
    Function FWlog = null;

  #endregion
        
 #region Fixation methods

    public override double NFixationCost
    {
        get
        {
            return FixationMetabolicCost.Value;
        }
    }
    public override NAllocationType NAllocation
    {
        set
        {
            base.NAllocation = value;    // give N allocation to base first.
            _NFixed = value.Fixation;    // now get our fixation value.
        }
    }
    [Output]
    public double RespiredWtFixation
    {
        get
        {
            return RespiredWt;
        }
    }
    public override NSupplyType NSupply
    {
        get
        {
            NSupplyType Supply = base.NSupply;   // get our base GenericOrgan to fill a supply structure first.
            if (Live != null)
            {
                // Now add in our fixation
                Supply.Fixation = Live.StructuralWt * SpecificNitrogenaseActivity.Value * Math.Min(FT.Value, Math.Min(FW.Value, FWlog.Value));
            }
            return Supply;
        }
    }
    public override DMAllocationType DMAllocation
    {
        set
        //This is the DM that is consumed to fix N.  this is calculated by the arbitrator and passed to the nodule to report
        {
            base.DMAllocation = value;      // Give the allocation to our base GenericOrgan first
            RespiredWt = value.Respired;    // Now get the respired value for ourselves.
        }
    }
    [Output]
    public double NFixed
    {
        get
        {
            return _NFixed;
        }
    }
 #endregion
}

