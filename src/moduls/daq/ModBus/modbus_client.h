
//OpenSCADA system module DAQ.ModBus file: modbus_client.h
/***************************************************************************
 *   Copyright (C) 2007-2008 by Roman Savochenko                           *
 *   rom_as@fromru.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
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
 
#ifndef MODBUS_CLIENT_H
#define MODBUS_CLIENT_H

#include <tcontroller.h>
#include <ttipdaq.h>
#include <tparamcontr.h>

#include <string>
#include <vector>
#include <map>

#undef _
#define _(mess) mod->I18N(mess)

using std::string;
using std::vector;
using std::map;

namespace ModBusDAQ
{

//******************************************************
//* TMdPrm                                             *
//******************************************************
class TMdContr;
    
class TMdPrm : public TParamContr
{
    public:
	//Methods
    	TMdPrm( string name, TTipParam *tp_prm );
	~TMdPrm( );

	void enable( );
	void disable( );

	TElem &elem( )		{ return p_el; }
	TMdContr &owner( )	{ return (TMdContr&)TParamContr::owner(); }
	
        //Attributes
	int    isErr;		//Error cod

    private:
	//Methods
        void postEnable( int flag );
	void vlGet( TVal &val );
	void vlSet( TVal &val );
	void vlArchMake( TVal &val );
	
        //Attributes
	string  &m_attrLs;
        TElem   p_el;           //Work atribute elements
};

//******************************************************
//* TMdContr                                           *
//******************************************************
class TMdContr: public TController
{
    public:
	//Methods
    	TMdContr( string name_c, const string &daq_db, TElem *cfgelem);
	~TMdContr( );

	double period( )	{ return vmax(m_per,0.1); }
	int    prior( )		{ return m_prior; }

	AutoHD<TMdPrm> at( const string &nm )	{ return TController::at(nm); }

	void load( );
	void save( );
	void start_( );
	void stop_( );
	
	void prmEn( const string &id, bool val );
	string modBusReq( string &pdu );

    protected:
	//Methods
    	void cntrCmdProc( XMLNode *opt );       //Control interface command process
	bool cfgChange( TCfg &cfg );
	
    private:
	//Methods
	TParamContr *ParamAttach( const string &name, int type );
	static void *Task( void *icntr );
	
	//Attributes
	Res	en_res, req_res;		//Resource for enable params and request values
	double	&m_per;     			//Acquisition task (seconds)
	int	&m_prior,			//Process task priority
		&m_prt,				//Protocol
		&m_node;			//Node
	string	&m_addr;			//Transport device address
		
	bool    prc_st,				//Process task active
		endrun_req;			//Request to stop of the Process task
        vector< AutoHD<TMdPrm> >  p_hd;
	
	pthread_t procPthr;     		//Process task thread
	
	double 	tm_gath;			//Gathering time
};

//*************************************************
//* SSerial                                       *
//*************************************************
class TTpContr;

class SSerial : public TCntrNode, public TConfig
{
    public:
	//Data
	enum	Prot 	{ Free, ASCII, RTU };

	//Methods
	SSerial( const string &dev, TTpContr *iown );	

	string 	id( )		{ return m_id; }
	int 	speed( )	{ return m_speed; }
	int	len( )		{ return m_len; }
	bool	twostop( )	{ return m_twostop; }
	int	parity( )	{ return m_parity; }
	int	timeoutFrame( )	{ return frTm; }
	double	timeoutChar( )	{ return charTm; }
	int	timeoutReq( )	{ return reqTm; }
	bool	hasOpen( )	{ return (fd>0); }

	void 	setSpeed( int val, bool tmAdj = false );
	void	setLen( int val );
	void	setTwostop( bool val );
	void	setParity( int val );
	void 	setTimeoutFrame( int val )	{ frTm = val; }
	void	setTimeoutChar( double val )	{ charTm = val; }
	void	setTimeoutReq( int val )	{ reqTm = val; }
	void	setOpen( bool vl );

	//- DB commands -
	void load( );
	void save( );
	
	string req( const string &vl );

	TTpContr &owner( )	{ return *(TTpContr *)nodePrev(); }

    protected:
        //Methods
        string nodeName( )	{ return id(); }
	void postDisable( int flag );

    private:
	//Attributes
	string	&m_id;				//Serial port device
	int	&m_speed;			//Speed
	int	&m_len;				//Length
	bool	&m_twostop;			//Two stop bits
	int	&m_parity;			//Parity check
	int 	&frTm;				//Frame timeout in ms
	double	&charTm;			//Char timeout in ms
	int	&reqTm;				//Request timeout in ms
		
	Res	m_res;				//Serial port resource
	int	fd;				//Serial port
};

//*************************************************
//* TTpContr                                      *
//*************************************************
class TTpContr: public TTipDAQ
{
    public:
	//Methods
    	TTpContr( string name );
	~TTpContr( );

	void modLoad( );
        void modSave( );

	TElem &serDevE( )	{ return el_ser_dev; }

	//- Serial devices -
	string serDevDB( );
        void serDevList( vector<string> &list )		{ chldList(m_sdev,list); }
	void serDevAdd( const string &dev );
        void serDevDel( const string &dev, bool full = false )		{ chldDel(m_sdev,dev,-1,full); }
        bool serDevPresent( const string &dev )		{ return chldPresent(m_sdev,dev); }
	AutoHD<SSerial> serDevAt( const string &dev )	{ return chldAt(m_sdev,dev); }
	
	//- Special modbus protocol's -
	ui16 CRC16( const string &mbap );
	ui8  LRC( const string &mbap );
	string DataToASCII( const string &in );
	string ASCIIToData( const string &in );

    protected:
	//Methods
        void cntrCmdProc( XMLNode *opt );       //Control interface command process

    private:
	//Methods
	void postEnable( int flag );
	TController *ContrAttach( const string &name, const string &daq_db );
	string optDescr( );
	
	//Attributes
	TElem   		el_ser_dev;
	int             	m_sdev;	
	
	//- Special modbus protocol's -
	static ui8 CRCHi[];
	static ui8 CRCLo[];
};

extern TTpContr *mod;

} //End namespace 

#endif //MODBUS_CLIENT_H
