#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
using namespace std;
#include "main.h"
#include "moesi.h"


cache_line * MOESI::allocate_line_dir(ulong addr) {
	return NULL;
}
void MOESI::signalRd(ulong addr, int processor_number){
}
void MOESI::signalRdX(ulong addr, int processor_number){
}
void MOESI::signalUpgr(ulong addr, int processor_number){
}
void MOESI::Inv(ulong addr) {
}
void MOESI::Int(ulong addr) {
}
void MOESI::PrRdDir(ulong addr, int processor_number) {
}
void MOESI::PrWrDir(ulong addr, int processor_number) {
}




void MOESI::PrRd(ulong addr, int processor_number) 
{
    cache_state state;
    current_cycle++;
    reads++;
    cache_line * line = find_line(addr);
    if (line == NULL || line->get_state() == I)
    {
    	read_misses++;
    	cache_line *newline = allocate_line(addr);
        if(sharers_exclude(addr,processor_number) != 0) {
           newline->set_state(S);
           I2S++;
        }
        else {
           newline->set_state(E);
           I2E++;
        }
        bus_reads++;
        if(c2c_supplier(addr,processor_number) != 0)
        {
        	cache2cache++;
        }
        else
        {
        	memory_transactions++;
        }

        sendBusRd(addr, processor_number);
    }
    else if ((line->get_state() == M) || (line->get_state() == S) || (line->get_state() == O) || (line->get_state() == E))
    {
    	update_LRU(line);
    }
}

void MOESI::PrWr(ulong addr, int processor_number) 
{
	cache_state state;
    current_cycle++;
    writes++;
    cache_line * line = find_line(addr);
    if (line == NULL || line->get_state() == I)
	{
		write_misses++;
        cache_line *newline = allocate_line(addr);
        I2M++;
        newline->set_state(M);
        bus_readxs++;
        if(c2c_supplier(addr,processor_number) != 0)
        {
        	cache2cache++;
        }
        else
        {
        	memory_transactions++;
        }

        sendBusRdX(addr, processor_number);
    }
    else if (line->get_state() == S) 
    {
	S2M++;
        line->set_state(M);
        update_LRU(line);
        bus_upgrades++;
        sendBusUpgr(addr, processor_number);
	
    }
    else if (line->get_state() == O)
    {
    	O2M++;
        line->set_state(M);
        update_LRU(line);
        bus_upgrades++;
        sendBusUpgr(addr, processor_number);
    }
    else if (line->get_state() == M) 
    {
    	update_LRU(line);
    }   
    else if (line->get_state() == E) 
    {
	E2M++;
    	update_LRU(line);
        line->set_state(M);
    }   
}

void MOESI::BusRd(ulong addr) 
{
    cache_line * line=find_line(addr);
    if (line!=NULL)    
    {
    	cache_state state;
		state=line->get_state();
        if (state == M)
        {
        	flushes++;
		interventions++;
        	line->set_state(O);
        	M2O++;
        }
        if (state == O)
        {
        	flushes++;
        }
        if (state == E)
        {
		E2S++;
        	line->set_state(S);
        }
    }
}

void MOESI::BusRdX(ulong addr) 
{
    cache_line * line=find_line(addr);
    if (line!=NULL)  
    {
    	cache_state state;
		state=line->get_state();
		if ((state == M) || (state == O))
		{
			flushes++;
			invalidations++;
			line->set_state(I);
		}	
		if (state == S)
		{
			invalidations++;
			line->set_state(I);
		}	
		if (state == E)
		{
			invalidations++;
			line->set_state(I);
		}	
    }
}

void MOESI::BusUpgr(ulong addr)
{
	cache_line * line=find_line(addr);
    if (line!=NULL)  
    {
    	cache_state state;
		state=line->get_state();
		if ((state == S) || (state == O))
		{
			invalidations++;
			line->set_state(I);			
		}
    }
}

bool MOESI::writeback_needed(cache_state state) {
//edit this function to return the correct boolean value
	if((state == M) || (state == O))
		{return true;}
	else 
		{return false;}
}
