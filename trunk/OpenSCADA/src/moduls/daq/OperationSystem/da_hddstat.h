
//OpenSCADA system module DAQ.OperationSystem file: da_hddstat.h
/***************************************************************************
 *   Copyright (C) 2005-2006 by Roman Savochenko                           *
 *   rom_as@fromru.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
 
#ifndef DA_HDDSTAT_H
#define DA_HDDSTAT_H

#include "da.h"

namespace SystemCntr
{

class HddStat: public DA
{
    public:
        HddStat( );
	~HddStat( );
	
        string id( ) 	{ return "hddstat"; }
        string name( )	{ return "HDD statistic"; }
		    
        void init( TMdPrm *prm );
        void getVal( TMdPrm *prm );
	void setEVAL( TMdPrm *prm );
	
	void makeActiveDA( TMdContr *a_cntr );
	
    private:
        void dList( vector<string> &list, bool part = false );	
};

} //End namespace 

#endif //DA_HDDSTAT_H

