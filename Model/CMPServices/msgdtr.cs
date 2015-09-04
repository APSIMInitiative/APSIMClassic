using System;
using System.Collections.Generic;

namespace CMPServices
{
    //============================================================================
    /// <summary>
    /// Generic reference item that can map an itemID or msgID with the compID.
    /// </summary>
    //============================================================================
    public struct TCompItem
    {
        /// <summary>
        /// Component ID.
        /// </summary>
        public uint compID;
        /// <summary>
        /// Any item id such as a msgID.
        /// </summary>
        public uint itemID;
    }
    //============================================================================
    /// <summary>
    /// Stores details of a branched message
    /// </summary>
    //============================================================================
    public struct TTrunkMsg
    {
        /// <summary>
        /// The component that originated the msg
        /// </summary>
        public uint returnToCompID;
        /// <summary>
        /// incoming msgID
        /// </summary>
        public uint inMsgID;
        /// <summary>
        /// incoming msg type
        /// </summary>
        public int inMsgType;
        /// <summary>
        /// do acknowledgement. 0=false
        /// </summary>
        public int toAck;
        /// <summary>
        /// property ID. Usually the driving property ID
        /// </summary>
        public int propID;
        /// <summary>
        /// number of active branches in the branch list
        /// </summary>
        public uint branchCount;
        /// <summary>
        /// Array of <see cref="TCompItem">TCompItem</see> so that requests can be traced.
        /// </summary>
        public List<TCompItem> branchMsgList;
    }
    //============================================================================
    /// <summary>
    /// Used for maintaining records about the redirection and branching
    /// of messages that arrive in a system component.
    /// </summary>
    /// <example>
    /// <para>
    /// The branchedMsgList member of the <see cref="TTrunkMsg">TTrunkMsg</see> is 
    /// used to track messages branched in publishEvent, queryInfo, or queryValue 
    /// calls. All branches are new messages, not just
    /// copies of the original trunk. The branches are assumed to have unique
    /// message id's as they are generated by the same instance of a TMsgInterpreter.
    /// Since the branches all have unique id's relative to the sending system
    /// component, a trunk can be found using any branch msgID.
    /// </para>
    /// The terms message <b>trunk</b> and message <b>branch</b> are used to illustrate the
    /// way that an incoming message is branched off to other components.
    /// <code>
    /// trunk ----> || ------> branch
    ///             ||
    ///             || ------> branch
    ///             |
    /// </code></example>
    // N.Herrmann June 2001, converted to Pascal, Nov 2004. Converted to C# Aug 2006
    //============================================================================
    public class TMsgDirector
    {
        /// <summary>
        /// List of messages that are rebroadcast such as publishEvent(s).
        /// Items are <see cref="TTrunkMsg">TTrunkMsg</see>s
        /// </summary>
        private List<TTrunkMsg> trunkMsgList;

        //============================================================================
        /// <summary>
        /// Default constructor
        /// </summary>
        //============================================================================
        public TMsgDirector()
        {
            trunkMsgList = new List<TTrunkMsg>();
        }
        //============================================================================
        /// <summary>
        /// Stores the details of an incoming msg (trunk) that can later contain
        /// references to branched msgs.
        /// </summary>
        /// <param name="returnToCompID"></param>
        /// <param name="trunkMsgID"></param>
        /// <param name="trunkMsgType"></param>
        /// <param name="propID"></param>
        /// <param name="toAck"></param>
        /// <returns></returns>
        // N.Herrmann May 2001
        //============================================================================
        public TTrunkMsg addMsgTrunk(uint returnToCompID, uint trunkMsgID,
                                      int trunkMsgType, int propID, int toAck)
        {
            TTrunkMsg trunk;

            int listPos = -1;
            int i = 0;
            while ((listPos < 0) && (i < trunkMsgList.Count))
            {
                if (trunkMsgList[i].inMsgID == 0)
                {  //if unused then
                    trunk = trunkMsgList[i];            //reuse it
                    trunk.branchMsgList.Clear();
                    trunk.branchCount = 0;
                    listPos = i;
                }
                i++;
            }

            if (listPos < 0)
            {
                trunk = new TTrunkMsg();
                trunkMsgList.Add(trunk);
                listPos = trunkMsgList.Count - 1;
            }

            trunk.returnToCompID = returnToCompID;
            trunk.inMsgID = trunkMsgID;         		//incoming msgID
            trunk.inMsgType = trunkMsgType;
            trunk.propID = propID;              		//incoming property ID
            trunk.toAck = toAck;
            trunk.branchCount = 0;
            trunk.branchMsgList = new List<TCompItem>();  	//0 items
            trunkMsgList[listPos] = trunk;  //store the changes

            return trunk;
        }
        //==============================================================================
        /// <summary>
        /// Adds a sub item (branch) to the incoming message item.
        /// </summary>
        /// <param name="returnToCompID"></param>
        /// <param name="inMsgID"></param>
        /// <param name="toCompID"></param>
        /// <param name="outgoingID"></param>
        //==============================================================================
        public void addMsgBranch(uint returnToCompID, uint inMsgID, uint toCompID, uint outgoingID)
        {
            bool found;
            TTrunkMsg bMsg;

            //find the incoming msgid in the list
            found = false;
            int i = trunkMsgList.Count - 1;
            while ((!found) && (i >= 0))
            {
                bMsg = trunkMsgList[i];
                //test for the correct source (trunk) message
                if ((bMsg.inMsgID == inMsgID) && (bMsg.returnToCompID == returnToCompID))
                {
                    found = true;

                    //add outgoingid to the list of branches
                    TCompItem msgItem;
                    msgItem.compID = toCompID;
                    msgItem.itemID = outgoingID;
                    bMsg.branchMsgList.Add(msgItem);
                    ++(bMsg.branchCount);
                    trunkMsgList[i] = bMsg;
                }
                i--;
            }
        }
        //==============================================================================
        /// <summary>
        /// Gets the branched (trunk/source) message information.
        /// Uses the toCompID and branchMsgID to find the branch msg. Having found the
        /// branch, I can then return the trunk.
        /// </summary>
        /// <param name="trunkMsgType"></param>
        /// <param name="toCompID"></param>
        /// <param name="branchMsgID"></param>
        /// <param name="msgFound">A ref to the trunk msg found.</param>
        /// <returns>True if the message is found.</returns>
        // N.Herrmann Nov 2004
        //==============================================================================
        public bool getBranch(uint trunkMsgType, uint toCompID, uint branchMsgID, ref TTrunkMsg msgFound)
        {
            TTrunkMsg bMsg;
            TCompItem msgItem;
            int i;
            int b;
            bool found = false;

            i = trunkMsgList.Count - 1;
            while (!found && (i >= 0))
            {  //while more message trunks
                bMsg = trunkMsgList[i];
                if (bMsg.inMsgType == trunkMsgType) //if the incoming request was this type
                {
                    //now look through the branches to find the correct trunk
                    b = 0;
                    while (!found && (b < bMsg.branchMsgList.Count))
                    {  //while more branches
                        msgItem = bMsg.branchMsgList[b];
                        if ((msgItem.itemID == branchMsgID) && (msgItem.compID == toCompID))
                        {  //if the branch is found
                            found = true;     //set found
                            msgFound = bMsg;  //store the trunk msg
                        }
                        b++;
                    }
                }
                i--;
            }

            return found;
        }

        //==============================================================================
        /// <summary>
        /// Find the trunk message for this branch and remove the branch. If there are no remaining branches then
        /// the trunk with be removed also.
        /// This function combines getBranch(), pruneBranch() and removeTrunk()
        /// </summary>
        /// <param name="trunkMsgType"></param>
        /// <param name="toCompID"></param>
        /// <param name="branchMsgID"></param>
        /// <param name="msgFound">A ref to the trunk msg found.</param>
        /// <param name="remainingBranches">The number of branches remaining after this one is removed</param>
        /// <returns>True if the branch is found</returns>
        //==============================================================================
        public bool pullBranch(uint trunkMsgType, uint toCompID, uint branchMsgID, ref TTrunkMsg msgFound, ref uint remainingBranches)
        {
            TTrunkMsg bMsg;
            TCompItem msgItem;
            int i;
            int b;
            bool found = false;

            i = trunkMsgList.Count - 1;
            while (!found && (i >= 0))
            {  //while more message trunks
                bMsg = trunkMsgList[i];
                if (bMsg.inMsgType == trunkMsgType) //if the incoming request was this type
                {
                    //now look through the branches to find the correct trunk
                    b = 0;
                    while (!found && (b < bMsg.branchMsgList.Count))
                    {  //while more branches
                        msgItem = bMsg.branchMsgList[b];
                        if ((msgItem.itemID == branchMsgID) && (msgItem.compID == toCompID))    //if the branch is found
                        {
                            found = true;     //set found
                            msgFound = bMsg;  //store the trunk msg

                            if (bMsg.branchCount <= 1)  // if this is the last branched msg
                            {
                                //clear branch list and deactivate the trunk
                                bMsg.branchMsgList.Clear();
                                bMsg.branchCount = 0;
                                bMsg.inMsgID = 0;        //flag unused
                                remainingBranches = 0;
                            }
                            else  
                            {
                                //there are more branches so just deactivate this branch
                                msgItem.compID = 0;     //flag unused
                                bMsg.branchMsgList[b] = msgItem;
                                bMsg.branchCount--;
                                remainingBranches = bMsg.branchCount;   //store the count of branches
                            }
                            trunkMsgList[i] = bMsg;	 //store
                        }
                        b++;
                    }
                }
                i--;
            }

            return found;
        }
        //==============================================================================
        /// <summary>
        /// Tests the outgoing message ID's to see if it is listed as a branch of
        /// any incoming message id's.
        /// </summary>
        /// <param name="trunkMsgType"></param>
        /// <param name="toCompID"></param>
        /// <param name="branchMsgID"></param>
        /// <returns></returns>
        // N.Herrmann Nov 2004
        //==============================================================================
        public bool isABranch(uint trunkMsgType, uint toCompID, uint branchMsgID)
        {
            TTrunkMsg bMsg = new TTrunkMsg();

            return getBranch(trunkMsgType, toCompID, branchMsgID, ref bMsg);
        }
        //==============================================================================
        /// <summary>
        /// Count of active messages in the trunk list.
        /// </summary>
        /// <returns>Count of active messages in the trunk list.</returns>
        // N.Herrmann Nov 2004
        //==============================================================================
        public int msgCount()
        {
            int count = 0;
            int i;

            for (i = 0; i <= trunkMsgList.Count - 1; i++)
            {
                if (trunkMsgList[i].inMsgID != 0)
                {
                    count++;
                }
            }
            return count;
        }

        //==============================================================================
        /// <summary>
        /// Removes the branch (only one) msg using the trunkMsgType and branchedMsgID to find it.
        /// </summary>
        /// <param name="trunkMsgType"></param>
        /// <param name="toCompID"></param>
        /// <param name="branchMsgID"></param>
        /// <returns>The count of branch msgs left in the sublist.</returns>
        // N.Herrmann Nov 2004
        //==============================================================================
        public uint pruneBranch(uint trunkMsgType, uint toCompID, uint branchMsgID)
        {
            int b, i;
            uint remainingBranches;
            bool found;
            TTrunkMsg bMsg;
            TCompItem msgItem;

            remainingBranches = 0;

            found = false;
            i = trunkMsgList.Count - 1;
            while (!found && (i >= 0))
            {  //while more message trunks
                bMsg = trunkMsgList[i];
                if (bMsg.inMsgType == trunkMsgType) //if the incoming request was this type
                {
                    //now look through the branches to find the correct trunk
                    b = 0;
                    while (!found && (b < bMsg.branchMsgList.Count))
                    {  //while more branches
                        msgItem = bMsg.branchMsgList[b];
                        if ((msgItem.itemID == branchMsgID) && (msgItem.compID == toCompID))
                        {  //if the branch is found
                            found = true;     //set found
                            msgItem.compID = 0;
                            bMsg.branchMsgList[b] = msgItem;
                            bMsg.branchCount--;
                            remainingBranches = bMsg.branchCount;   //store the count of branches
                            trunkMsgList[i] = bMsg;
                        }
                        b++;
                    }
                }
                i--;
            }
            return remainingBranches;
        }
        //==============================================================================
        /// <summary>
        /// Removes the main trunk (incoming) message and any branched items from the list.
        /// does not delete the item, just sets it to unused.
        /// </summary>
        /// <param name="returnToCompID"></param>
        /// <param name="trunkMsgID"></param>
        // N.Herrmann Nov 2004
        //==============================================================================
        public void removeTrunk(uint returnToCompID, uint trunkMsgID)
        {
            int t;
            bool found;
            TTrunkMsg bMsg;

            found = false;
            t = trunkMsgList.Count;                //look through the list in reverse order
            while ((!found) && (t > 0))          //while more message trunks
            {
                t--;
                bMsg = trunkMsgList[t];
                if ((bMsg.inMsgID == trunkMsgID) && (bMsg.returnToCompID == returnToCompID))
                {
                    found = true;
                    //remove any branched items in the outgoingMsgList
                    bMsg.branchMsgList.Clear();
                    bMsg.branchCount = 0;
                    bMsg.inMsgID = 0;        //flag unused
                    trunkMsgList[t] = bMsg;	//store
                }
            }
        }
    }
}
