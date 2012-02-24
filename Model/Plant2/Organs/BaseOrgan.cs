using System;
using System.Collections.Generic;
using System.Text;


public class BaseOrgan : Organ
{
    public override double DMDemand { get { return 0; } }
    public override DMSupplyType DMSupply { get { return new DMSupplyType(); } }
    public override double DMSinkCapacity { get { return 0; } }
    public override double DMPotentialAllocation { set { } }
    public override DMAllocationType DMAllocation { set { } }

    public override double NDemand { get { return 0; } }
    public override NSupplyType NSupply { get { return new NSupplyType(); } }
    public override NAllocationType NAllocation { set {  } }
    public override double NFixationCost { get { return 0; } }

    public override double WaterDemand { get { return 0; } }
    public override double WaterSupply { get { return 0; } }
    public override double WaterUptake
    {
        get { return 0; }
        set { throw new Exception("Cannot set water uptake for " + Name); }
    }
    public override double WaterAllocation
    {
        get { return 0; }
        set { throw new Exception("Cannot set water allocation for " + Name); }
    }
    public override void DoWaterUptake(double Demand) { }
    public override void DoPotentialGrowth() { }
    public override void DoActualGrowth() { }

    public override double MaxNconc { get { return 0; } }
    public override double MinNconc { get { return 0; } }

    // Provide some variables for output until we get a better REPORT component that
    // can do structures e.g. NSupply.Fixation

    [Output]
    [Units("g/m^2")]
    public double DMSupplyPhotosynthesis { get { return DMSupply.Photosynthesis; } }

    [Output]
    [Units("g/m^2")]
    public double NSupplyUptake { get { return NSupply.Uptake; } }
}
