/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "include/global.h"
#include "include/unifiedtime.h"

#include "../Util/include/runtime.h"

namespace Core
{
	
	/* Flag Declarations */
	bool fTimeUnified = false;

	
	/* The Average Offset of all Nodes. */
	int UNIFIED_AVERAGE_OFFSET = 0;
	
	
	/* The Current Record Iterating for Moving Average. */
	int UNIFIED_MOVING_ITERATOR = 0;

	
	/* Unified Time Declarations */
	std::vector<int> UNIFIED_TIME_DATA;
    
	
	/** Gets the UNIX Timestamp from the Nexus Network **/
	uint64 UnifiedTimestamp(bool fMilliseconds){ return Timestamp(fMilliseconds) + (UNIFIED_AVERAGE_OFFSET / (fMilliseconds ? 1 : 1000)); }


	/* Calculate the Average Unified Time. Called after Time Data is Added */
	int GetUnifiedAverage()
	{
		if(UNIFIED_TIME_DATA.empty())
			return UNIFIED_AVERAGE_OFFSET;
			
		int nAverage = 0;
		for(int index = 0; index < std::min(MAX_UNIFIED_SAMPLES, (int)UNIFIED_TIME_DATA.size()); index++)
			nAverage += UNIFIED_TIME_DATA[index];
			
		return std::round(nAverage / (double) std::min(MAX_UNIFIED_SAMPLES, (int)UNIFIED_TIME_DATA.size()));
	}
	

    /** Regulator of the Unified Clock **/
    void ThreadUnifiedSamples(void* parg)
    {
        /** Compile the Seed Nodes into a set of Vectors. **/
        SEED_NODES    = DNS_Lookup(fTestNet ? DNS_SeedNodes_Testnet : DNS_SeedNodes);
        
        /* If the node happens to be offline, wait and recursively attempt to get the DNS seeds. */
        if(SEED_NODES.empty()) {
            Sleep(10000);
            
            fTimeUnified = true;
            ThreadUnifiedSamples(parg);
        }
        
        /** Iterator to be used to ensure every time seed is giving an equal weight towards the Global Seeds. **/
        int nIterator = -1;
        
        for(int nIndex = 0; nIndex < SEED_NODES.size(); nIndex++)
            SEEDS.push_back(SEED_NODES[nIndex].ToStringIP());
        
        /** The Entry Client Loop for Core LLP. **/
        string ADDRESS = "";
        LLP::CoreOutbound SERVER("", strprintf("%u", (fTestNet ? TESTNET_CORE_LLP_PORT : NEXUS_CORE_LLP_PORT)));
        loop
        {
            try
            {
            
                /** Increment the Time Seed Connection Iterator. **/
                nIterator++;
                
                
                /** Reset the ITerator if out of Seeds. **/
                if(nIterator == SEEDS.size())
                nIterator = 0;
                    
                    
                /** Connect to the Next Seed in the Iterator. **/
                SERVER.IP = SEEDS[nIterator];
                SERVER.Connect();
                
                
                /** If the Core LLP isn't connected, Retry in 10 Seconds. **/
                if(!SERVER.Connected())
                {
                    printf("***** Core LLP: Failed To Connect To %s:%s\n", SERVER.IP.c_str(), SERVER.PORT.c_str());
                    
                    continue;
                }

                
                /** Use a CMajority to Find the Sample with the Most Weight. **/
                CMajority<int> nSamples;
                
                
                /** Get 10 Samples From Server. **/
                SERVER.GetOffset((unsigned int)GetLocalTimestamp());
                    
                    
                /** Read the Samples from the Server. **/
                while(SERVER.Connected() && !SERVER.Errors() && !SERVER.Timeout(5))
                {
                    Sleep(100);
                
                    SERVER.ReadPacket();
                    if(SERVER.PacketComplete())
                    {
                        LLP::Packet PACKET = SERVER.NewPacket();
                        
                        /** Add a New Sample each Time Packet Arrives. **/
                        if(PACKET.HEADER == SERVER.TIME_OFFSET)
                        {
                            int nOffset = bytes2int(PACKET.DATA);
                            nSamples.Add(nOffset);
                            
                            SERVER.GetOffset((unsigned int)GetLocalTimestamp());
                            
                            if(GetArg("-verbose", 0) >= 2)
                                printf("***** Core LLP: Added Sample %i | Seed %s\n", nOffset, SERVER.IP.c_str());
                        }
                        
                        SERVER.ResetPacket();
                    }
                    
                    /** Close the Connection Gracefully if Received all Packets. **/
                    if(nSamples.Samples() == 9)
                    {
                        SERVER.Close();
                        break;
                    }
                }
                
                
                /** If there are no Successful Samples, Try another Connection. **/
                if(nSamples.Samples() == 0)
                {
                    printf("***** Core LLP: Failed To Get Time Samples.\n");
                    SERVER.Close();
                    
                    continue;
                }
                
                /* These checks are for after the first time seed has been established. 
                    TODO: Look at the possible attack vector of the first time seed being manipulated.
                            This could be easily done by allowing the time seed to be created by X nodes and then check the drift. */
                if(fTimeUnified)
                {
                
                    /* Check that the samples don't violate time changes too drastically. */
                    if(nSamples.Majority() > GetUnifiedAverage() + MAX_UNIFIED_DRIFT ||
                        nSamples.Majority() < GetUnifiedAverage() - MAX_UNIFIED_DRIFT ) {
                    
                        printf("***** Core LLP: Unified Samples Out of Drift Scope Current (%u) Samples (%u)\n", GetUnifiedAverage(), nSamples.Majority());
                        SERVER.Close();
                    
                        continue;
                    }
                }
                
                /** If the Moving Average is filled with samples, continue iterating to keep it moving. **/
                if(UNIFIED_TIME_DATA.size() >= MAX_UNIFIED_SAMPLES)
                {
                    if(UNIFIED_MOVING_ITERATOR >= MAX_UNIFIED_SAMPLES)
                        UNIFIED_MOVING_ITERATOR = 0;
                                        
                    UNIFIED_TIME_DATA[UNIFIED_MOVING_ITERATOR] = nSamples.Majority();
                    UNIFIED_MOVING_ITERATOR ++;
                }
                    
                    
                /** If The Moving Average is filling, move the iterator along with the Time Data Size. **/
                else
                {
                    UNIFIED_MOVING_ITERATOR = UNIFIED_TIME_DATA.size();
                    UNIFIED_TIME_DATA.push_back(nSamples.Majority());
                }
                

                /** Update Iterators and Flags. **/
                if((UNIFIED_TIME_DATA.size() > 0))
                {
                    fTimeUnified = true;
                    UNIFIED_AVERAGE_OFFSET = GetUnifiedAverage();
                    
                    if(GetArg("-verbose", 0) >= 1)
                        printf("***** %i Iterator | %i Offset | %i Current | %"PRId64"\n", UNIFIED_MOVING_ITERATOR, nSamples.Majority(), UNIFIED_AVERAGE_OFFSET, GetUnifiedTimestamp());
                }
                
                /* Sleep for 5 Minutes Between Sample. */
                Sleep(60000);
            }
            catch(std::exception& e){ printf("UTM ERROR: %s\n", e.what()); }
        }
    }
}


