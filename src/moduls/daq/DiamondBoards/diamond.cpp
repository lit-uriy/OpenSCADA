
//OpenSCADA system module DAQ.DiamondBoards file: diamond.cpp
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

#include <sys/time.h>
#include <getopt.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#include <resalloc.h>
#include <tsys.h>
#include <ttiparam.h>

#include "diamond.h"

//============ Modul info! =====================================================
#define MOD_ID      "DiamondBoards"
#define MOD_NAME    "Diamond DA boards"
#define MOD_TYPE    "DAQ"
#define VER_TYPE    VER_CNTR
#define VERSION     "0.9.0"
#define AUTORS      "Roman Savochenko"
#define DESCRIPTION "Allow access to Diamond systems DA boards. Include support of Athena board."
#define LICENSE     "GPL"
//==============================================================================

Diamond::TTpContr *Diamond::mod;

extern "C"
{
    TModule::SAt module( int n_mod )
    {
	TModule::SAt AtMod;

	if(n_mod==0)
	{
	    AtMod.id	= MOD_ID;
	    AtMod.type  = MOD_TYPE;
	    AtMod.t_ver = VER_TYPE;
	}
	else
    	    AtMod.id	= "";

	return( AtMod );
    }

    TModule *attach( const TModule::SAt &AtMod, const string &source )
    {
	Diamond::TTpContr *self_addr = NULL;

    	if( AtMod.id == MOD_ID && AtMod.type == MOD_TYPE && AtMod.t_ver == VER_TYPE )
	    self_addr = Diamond::mod = new Diamond::TTpContr( source );

	return ( self_addr );
    }
}

using namespace Diamond;

//======================================================================
//==== TTpContr ======================================================== 
//======================================================================
TTpContr::TTpContr( string name ) : 
    m_init(false), elem_ai("AI"), elem_ao("AO"), elem_di("DI"), elem_do("DO")
{
    mId 	= MOD_ID;
    mName       = MOD_NAME;
    mType  	= MOD_TYPE;
    mVers      	= VERSION;
    mAutor    	= AUTORS;
    mDescr  	= DESCRIPTION;
    mLicense   	= LICENSE;
    mSource    	= name;    
}

TTpContr::~TTpContr()
{    

}

void TTpContr::postEnable()
{    
    TModule::postEnable();
    
    //==== Controler's bd structure ====
    fldAdd( new TFld("BOARD",I18N("Diamond system board"),TFld::Dec,FLD_SELECT,"2","25","0;25","DMM16;ATHENA") );
    fldAdd( new TFld("PRM_BD_A",I18N("Analog parameters' table"),TFld::String,0,"30","diamond_prm_a") );
    fldAdd( new TFld("PRM_BD_D",I18N("Digital parameters' table"),TFld::String,0,"30","diamond_prm_d") );    
    fldAdd( new TFld("ADDR",I18N("Base board address"),TFld::Hex,0,"3","640") );
    fldAdd( new TFld("INT",I18N("Interrupt vector"),TFld::Dec,0,"2","5") );    
    fldAdd( new TFld("DIO_CFG",I18N("Digit IO config byte"),TFld::Hex,0,"2","0") );
    fldAdd( new TFld("ADMODE",I18N("A/D interrupt mode"),TFld::Bool,FLD_PREV,"1","0") );
    fldAdd( new TFld("ADRANGE",I18N("A/D voltage range"),TFld::Dec,FLD_SELECT,"1",TSYS::int2str(RANGE_10).c_str(),
        (TSYS::int2str(RANGE_5)+";"+TSYS::int2str(RANGE_10)).c_str(),I18N("5v;10v")) );
    fldAdd( new TFld("ADPOLAR",I18N("A/D polarity"),TFld::Dec,FLD_SELECT,"1",TSYS::int2str(BIPOLAR).c_str(),
        (TSYS::int2str(BIPOLAR)+";"+TSYS::int2str(UNIPOLAR)).c_str(),I18N("Bipolar;Unipolar")) );
    fldAdd( new TFld("ADGAIN",I18N("A/D gain"),TFld::Dec,FLD_SELECT,"1",TSYS::int2str(GAIN_1).c_str(),
        (TSYS::int2str(GAIN_1)+";"+TSYS::int2str(GAIN_2)+";"+TSYS::int2str(GAIN_4)+";"+TSYS::int2str(GAIN_8)).c_str(),"x1;x2;x4;x8") );
    fldAdd( new TFld("ADCONVRATE",I18N("A/D convertion rate (Hz)"),TFld::Dec,0,"6","200","100;100000") );
    
    //==== Parameter type bd structure ====
    //---- Analog ----
    int t_prm = tpParmAdd("a_prm","PRM_BD_A",I18N("Analog parameter"));
    tpPrmAt(t_prm).fldAdd( new TFld("TYPE",I18N("Analog parameter type"),TFld::Dec,FLD_SELECT|FLD_NOVAL|FLD_PREV,"1","0","0;1",I18N("Input;Output")) );
    tpPrmAt(t_prm).fldAdd( new TFld("CNL",I18N("Channel"),TFld::Dec,FLD_NOVAL,"2","0") );
    tpPrmAt(t_prm).fldAdd( new TFld("GAIN",I18N("A/D converter gain"),TFld::Dec,FLD_SELECT|FLD_NOVAL|FLD_PREV,"1",TSYS::int2str(GAIN_1).c_str(),
	(TSYS::int2str(GAIN_1)+";"+TSYS::int2str(GAIN_2)+";"+TSYS::int2str(GAIN_4)+";"+TSYS::int2str(GAIN_8)).c_str(),"x1;x2;x4;x8") );
    //---- Digit ----
    t_prm = tpParmAdd("d_prm","PRM_BD_D",I18N("Digital parameter"));
    tpPrmAt(t_prm).fldAdd( new TFld("TYPE",I18N("Digital parameter type"),TFld::Dec,FLD_SELECT|FLD_NOVAL|FLD_PREV,"1","0","0;1",I18N("Input;Output")) );
    tpPrmAt(t_prm).fldAdd( new TFld("PORT",I18N("Port"),TFld::Dec,FLD_SELECT|FLD_NOVAL|FLD_PREV,"2","0","0;1;2","A;B;C") );
    tpPrmAt(t_prm).fldAdd( new TFld("CNL",I18N("Channel"),TFld::Dec,FLD_NOVAL|FLD_PREV,"1") );

    //==== Init value elements =====
    //---- Analog input ----
    elem_ai.fldAdd( new TFld("value",I18N("Value %"),TFld::Real,FLD_NWR|FLD_DRD,"",TSYS::real2str(EVAL_REAL).c_str(),"0;100","",1) );
    elem_ai.fldAdd( new TFld("voltage",I18N("Voltage V"),TFld::Real,FLD_NWR|FLD_DRD,"",TSYS::real2str(EVAL_REAL).c_str(),"-10;10","",2) );
    elem_ai.fldAdd( new TFld("code",I18N("A/D code"),TFld::Dec,FLD_NWR|FLD_DRD,"",TSYS::int2str(EVAL_INT).c_str(),"","",3) );
    //---- Analog output ----
    elem_ao.fldAdd( new TFld("value",I18N("Value %"),TFld::Real,FLD_DWR,"",TSYS::real2str(EVAL_REAL).c_str(),"0;100","",1) );
    elem_ao.fldAdd( new TFld("voltage",I18N("Voltage V"),TFld::Real,FLD_DWR,"",TSYS::real2str(EVAL_REAL).c_str(),"-10;10","",2) );
    //---- Digit input ----
    elem_di.fldAdd( new TFld("value",I18N("Value"),TFld::Bool,FLD_NWR|FLD_DRD,"",TSYS::int2str(EVAL_BOOL).c_str(),"","",1) );
    //---- Digit output ----
    elem_do.fldAdd( new TFld("value",I18N("Value"),TFld::Bool,FLD_DWR,"",TSYS::int2str(EVAL_BOOL).c_str(),"","",1) );

    m_init = true;
}

TController *TTpContr::ContrAttach( const string &name, const string &daq_db )
{
    return new TMdContr(name,daq_db,this);
}

//======================================================================
//==== TMdContr 
//======================================================================
TMdContr::TMdContr( string name_c, const string &daq_db, ::TElem *cfgelem) : dsc_st(false),
	::TController(name_c,daq_db,cfgelem), m_addr(cfg("ADDR").getId()), 
	ad_int_mode(cfg("ADMODE").getBd())
{
    cfg("PRM_BD_A").setS("Diam_"+name_c+"_prm_a");
    cfg("PRM_BD_D").setS("Diam_"+name_c+"_prm_d");
    
    //Hide sevral config fields
    cfg("INT").view(false);
    cfg("DIO_CFG").view(false);
    cfg("ADCONVRATE").view(false);
    cfg("ADGAIN").view(false);
    
    //DSC resources
    DSC.gen_res = ResAlloc::resCreate();
    DSC.comm = 0;
    pthread_mutex_init(&DSC.th_mut,NULL);
    pthread_cond_init(&DSC.th_cv,NULL);
}

TMdContr::~TMdContr()
{
    ResAlloc::resDelete(DSC.gen_res);
    pthread_cond_destroy(&DSC.th_cv);
    pthread_mutex_destroy(&DSC.th_mut);
}

TParamContr *TMdContr::ParamAttach( const string &name, int type )
{    
    return new TMdPrm(name,&owner().tpPrmAt(type));
}

void TMdContr::load( )
{
    TController::load( );
}

void TMdContr::save( )
{
    TController::save();
}

void TMdContr::start( )
{
    TController::start();
    
    //- Check inited of Diamond API -
    if( !mod->initStat() )
        throw TError(mod->nodePath().c_str(),mod->I18N("Module no inited!"));
    
    if( !run_st )
    {    
	//-- Create DSC task --
	pthread_attr_t pthr_attr;
	pthread_attr_init(&pthr_attr);
	struct sched_param prior;
	pthread_attr_setschedpolicy(&pthr_attr,SCHED_OTHER);
    	pthread_create(&dsc_pthr,&pthr_attr,DSCTask,this);
    	pthread_attr_destroy(&pthr_attr);
	if( TSYS::eventWait(dsc_st, true, nodePath()+"dsc_task_start",5) )
    	    throw TError(nodePath().c_str(),mod->I18N("DSC driver task no started!"));
			    
	run_st = true;
	
	//Enable parameters
	vector<string> prm_list;
	list(prm_list);
	for( int i_prm = 0; i_prm < prm_list.size(); i_prm++ )
	    if( at(prm_list[i_prm]).at().toEnable() )
		at(prm_list[i_prm]).at().enable();	
    }
}

void TMdContr::stop( )
{  
    TController::stop();
    
    if( !run_st ) return;
    
    //Close AI DAQ task
    if( dsc_st )
    {
        endrun_req_dsc = true;
        pthread_kill( dsc_pthr, SIGALRM );
        if( TSYS::eventWait(dsc_st,false,nodePath()+"dsc_task_stop",5) )
            throw TError(nodePath().c_str(),mod->I18N("DSC task no stoped!"));
        pthread_join( dsc_pthr, NULL );
    }								
    
    //Disable parameters
    vector<string> prm_list;
    list(prm_list);
    for( int i_prm = 0; i_prm < prm_list.size(); i_prm++ )
        if( at(prm_list[i_prm]).at().enableStat() )
    	    at(prm_list[i_prm]).at().disable();							
    
    //close(port_hd);
    run_st = false;
} 

bool TMdContr::cfgChange( TCfg &icfg )
{
    if( icfg.fld().name() == "ADMODE" )
    {
	if( icfg.getB() ) 
	{
	    cfg("INT").view(true);
	    cfg("ADCONVRATE").view(true);
	    cfg("ADGAIN").view(true);
	}
	else 
	{
	    cfg("INT").view(false);
	    cfg("ADCONVRATE").view(false);
	    cfg("ADGAIN").view(false);
	}
	if( startStat() ) stop();
    }    
    return true;
}

void *TMdContr::DSCTask( void *param )
{    
    long long vtm = SYS->curTime();
    long long chk_tm = vtm;
    struct timespec get_tm;    
    get_tm.tv_sec = get_tm.tv_nsec = 0;
    int prev_trans = 0;
    
    //AI interrupt dequisition
    vector<string> ai_prm;
    int p_beg = 15, p_end = 0;    
	
    TMdContr &cntr = *(TMdContr *)param;
    cntr.endrun_req_dsc = false;
    cntr.dsc_st = true;
    
    //DSC strucures init
    BYTE result;
    DSCB dscb;    
    ERRPARAMS errorParams;
    DSCADSETTINGS dscadsettings;
    memset(&dscadsettings, 0, sizeof(DSCADSETTINGS));
    DSCAIOINT dscaioint;
    memset(&dscaioint, 0, sizeof(DSCAIOINT));
    DSCS dscs;    
    
    //Main DSC code
    try
    {	
	//- Init DSCAD -
	if( dscInit( DSC_VERSION ) != DE_NONE )
	{
	    dscGetLastError(&errorParams);
	    throw TError(cntr.nodePath().c_str(),mod->I18N("dscInit error: %s %s"),dscGetErrorString(errorParams.ErrCode), errorParams.errstring );
	}
	//- Init Board -
	DSCCB dsccb;
	dsccb.base_address = cntr.m_addr;	    
	dsccb.int_level = cntr.cfg("INT").getI();		    
	if(dscInitBoard(cntr.cfg("BOARD").getI(), &dsccb, &dscb)!= DE_NONE)
	{
	    dscGetLastError(&errorParams);
	    throw TError(cntr.nodePath().c_str(),mod->I18N("dscInit error: %s %s"),dscGetErrorString(errorParams.ErrCode), errorParams.errstring );
	}
	
        //- Set DIO config -
	BYTE cfg_byte = cntr.cfg("DIO_CFG").getI()|0x80;
	if( (result = dscDIOSetConfig(dscb, &cfg_byte)) != DE_NONE)
     	{
	    dscGetLastError(&errorParams);
	    throw TError(cntr.nodePath().c_str(),mod->I18N("dscDIOSetConfig error: %s %s"),dscGetErrorString(errorParams.ErrCode), errorParams.errstring );
	}
	
	//- Init AD acquisition -
	dscadsettings.range = cntr.cfg("ADRANGE").getI();
	dscadsettings.polarity = cntr.cfg("ADPOLAR").getI();
	dscadsettings.gain = cntr.cfg("ADGAIN").getI();
	dscadsettings.load_cal = 0;           
	if( cntr.ad_int_mode )
	{
	    //Get AI param list and address border
	    cntr.list(ai_prm);
	    for( int i_p = 0; i_p < ai_prm.size(); i_p++ )
	    {
		AutoHD<TMdPrm> prm = cntr.at(ai_prm[i_p]);
		if(prm.at().type() != TMdPrm::AI)
		{ 
		    ai_prm.erase(ai_prm.begin()+i_p);
		    i_p--;
		}		    
		else
		{
		    p_beg = vmin(p_beg,prm.at().cnl());
		    p_end = vmax(p_end,prm.at().cnl());
		}
	    }
	    
	    if(ai_prm.size())
	    {		
		dscadsettings.current_channel = p_beg;		
		if( ( result = dscADSetSettings( dscb, &dscadsettings ) ) != DE_NONE )
		{
    		    dscGetLastError(&errorParams);
		    throw TError(cntr.nodePath().c_str(),mod->I18N("dscADSetSettings error: %s %s"), dscGetErrorString(errorParams.ErrCode), errorParams.errstring );
		}
	    
		//-- Init interrupt --
		dscaioint.conversion_rate = 2*(cntr.cfg("ADCONVRATE").getI()/2);
		dscaioint.num_conversions = 2*(int)dscaioint.conversion_rate*(p_end-p_beg+1);
		dscaioint.cycle = 1;
		dscaioint.internal_clock = 1;
		dscaioint.low_channel = p_beg;
		dscaioint.high_channel = p_end;
		dscaioint.external_gate_enable = 0;
		dscaioint.internal_clock_gate = 0;
		dscaioint.dump_threshold = dscaioint.num_conversions/2;
		dscaioint.fifo_enab = 1;
		dscaioint.fifo_depth = dscaioint.num_conversions;
		while(dscaioint.fifo_depth>46)
		{
		    if( dscaioint.fifo_depth > 2*46 )
			dscaioint.fifo_depth/=10;
		    else dscaioint.fifo_depth/=2;
		}
		dscaioint.sample_values = (DSCSAMPLE*)malloc( sizeof(DSCSAMPLE) * dscaioint.num_conversions );
		if( ( result = dscADScanInt( dscb, &dscaioint ) ) != DE_NONE )
		{
    		    dscGetLastError(&errorParams);
		    throw TError(cntr.nodePath().c_str(),mod->I18N("dscADScanInt error: %s %s"),dscGetErrorString(errorParams.ErrCode), errorParams.errstring );
		}		
		//- Init operation data -	
		dscs.transfers = 0;
		dscs.overflows = 0;
		dscs.op_type = OP_TYPE_INT;
	    }
	}
    
	int cond_rez;
	pthread_mutex_lock(&cntr.DSC.th_mut);
	while(!cntr.endrun_req_dsc)
	{
	    cond_rez = pthread_cond_timedwait(&cntr.DSC.th_cv,&cntr.DSC.th_mut,&get_tm);
	    if( !cond_rez )
	    {
		switch(cntr.DSC.comm)
		{
		    case 1:	//Get AI
	    	    {
			if(cntr.ad_int_mode)	break;
			dscadsettings.gain = cntr.DSC.prm2;
			dscadsettings.current_channel = cntr.DSC.prm1;
			if( (result = dscADSetSettings(dscb,&dscadsettings)) != DE_NONE )
			{
			    dscGetLastError(&errorParams);
			    Mess->put(cntr.nodePath().c_str(),TMess::Error,mod->I18N("dscADSetSettings error: %s %s"), dscGetErrorString(errorParams.ErrCode), errorParams.errstring );
			}
			DSCSAMPLE smpl;
			if( (result = dscADSample(dscb,&smpl)) != DE_NONE )
		       	{
			    dscGetLastError(&errorParams);
			    Mess->put(cntr.nodePath().c_str(),TMess::Error,mod->I18N("dscADSample error: %s %s"), dscGetErrorString(errorParams.ErrCode), errorParams.errstring );
			}
			cntr.DSC.prm2 = smpl;
			break;
		    }
		    case 2:	//Set AO
			if( (result = dscDAConvert(dscb,cntr.DSC.prm1,cntr.DSC.prm2)) != DE_NONE )
			{
                    	    dscGetLastError(&errorParams);
			    Mess->put(cntr.nodePath().c_str(),TMess::Error,mod->I18N("dscDAConvert error: %s %s"),dscGetErrorString(errorParams.ErrCode),errorParams.errstring );
	                }
			break;
		    case 3:	//Get DI
		    {
			BYTE i_bt;
			if( (result = dscDIOInputBit(dscb,cntr.DSC.prm1>>4, cntr.DSC.prm1&0x0f, &i_bt)) != DE_NONE )
			{
			    dscGetLastError(&errorParams);
			    Mess->put(cntr.nodePath().c_str(),TMess::Error,mod->I18N("dscDIOInputBit error: %s %s"), dscGetErrorString(errorParams.ErrCode),errorParams.errstring );
			}
			cntr.DSC.prm2 = i_bt;
			break;
		    }
		    case 4:	//Set DO
			if( (result = dscDIOOutputBit(dscb, cntr.DSC.prm1>>4, cntr.DSC.prm1&0x0f, (bool)cntr.DSC.prm2)) != DE_NONE )
			{
			    dscGetLastError(&errorParams);
			    Mess->put(cntr.nodePath().c_str(),TMess::Error,mod->I18N("dscDIOOutputBit error: %s %s"),dscGetErrorString(errorParams.ErrCode),errorParams.errstring );
	                }
			break;	
		}
		cntr.DSC.comm = 0;
	    }
	    else if( cond_rez == ETIMEDOUT || cond_rez == EINTR )
	    {
		if( cntr.endrun_req_dsc ) break;
		if( !cntr.ad_int_mode || !ai_prm.size() )
		{
		    chk_tm+=500000;
                    get_tm.tv_sec = chk_tm/1000000;
                    get_tm.tv_nsec = 1000*(chk_tm%1000000);						    
		    continue;
		}
		
		dscGetStatus(dscb, &dscs);
		
		if( dscs.transfers != prev_trans )
		{
		    int v_a_step;
		    int p_cnt = p_end-p_beg+1;
		    for(int i_p = 0; i_p < ai_prm.size(); i_p++ )
		    {
			if( !cntr.present(ai_prm[i_p]) )	continue;
			AutoHD<TMdPrm> prm = cntr.at(ai_prm[i_p]);
			int p_cnl = prm.at().cnl();
                	if(prm.at().type() != TMdPrm::AI || p_cnl < p_beg || p_cnl > p_end || !prm.at().enableStat() )
			    continue;
			int voff = (dscs.transfers+dscaioint.dump_threshold)%dscaioint.num_conversions;
			//Get code
			AutoHD<TVal> val = prm.at().vlAt("code");			
			if(!val.at().arch().freeStat() && val.at().arch().at().srcMode() == TVArchive::PassiveAttr)
			{
			    v_a_step = vmax(1,val.at().arch().at().period()*(int)dscaioint.conversion_rate/1000000);
			    for( int i_smpl = 0; i_smpl < dscaioint.conversion_rate; i_smpl+=v_a_step )
				val.at().arch().at().setI(dscaioint.sample_values[voff+p_cnl-p_beg+i_smpl*p_cnt],vtm+i_smpl*1000000/(int)dscaioint.conversion_rate);
			}
			val.at().setI(dscaioint.sample_values[voff+p_cnl-p_beg+((int)dscaioint.conversion_rate-1)*p_cnt],vtm+((int)dscaioint.conversion_rate-1)*1000000/(int)dscaioint.conversion_rate,true);
			//Get procent
			val = prm.at().vlAt("value");
			if(!val.at().arch().freeStat() && val.at().arch().at().srcMode() == TVArchive::PassiveAttr)
			{
			    v_a_step = vmax(1,val.at().arch().at().period()*(int)dscaioint.conversion_rate/1000000);
			    for( int i_smpl = 0; i_smpl < dscaioint.conversion_rate; i_smpl+=v_a_step )
				val.at().arch().at().setR( 100.*(double)dscaioint.sample_values[voff+p_cnl-p_beg+i_smpl*p_cnt]/32768.,vtm+i_smpl*1000000/(int)dscaioint.conversion_rate);
			}
			val.at().setR( 100.*(double)dscaioint.sample_values[voff+p_cnl-p_beg+((int)dscaioint.conversion_rate-1)*p_cnt]/32768.,vtm+((int)dscaioint.conversion_rate-1)*1000000/(int)dscaioint.conversion_rate,true);
			//Get voltage
			//for( int i_smpl = 0; i_smpl < dscaioint.conversion_rate; i_smpl++ )
			//	printf("Canal %d(%d): %xh\n",prm.at().cnl(),voff+prm.at().cnl()-p_beg+i_smpl*(p_end-p_beg+1),dscaioint.sample_values[voff+prm.at().cnl()-p_beg+i_smpl*(p_end-p_beg+1)]);
		    }
		    prev_trans = dscs.transfers;
		    vtm+=1000000;
		}		
		chk_tm+=200000;
		get_tm.tv_sec = chk_tm/1000000;		
		get_tm.tv_nsec = 1000*(chk_tm%1000000);
	    }
	}
    }catch( TError err )
    { 
	Mess->put(err.cat.c_str(),TMess::Error,"%s",err.mess.c_str());
	Mess->put(cntr.nodePath().c_str(),TMess::Error,mod->I18N("DSC task error."));
    }

    cntr.dsc_st = false;
    
    //- Deinit DSC -
    if(cntr.ad_int_mode)
    {
    	if( dscs.op_type != OP_TYPE_NONE )
	    dscCancelOp(dscb);
	free( dscaioint.sample_values );    
    }    
    pthread_mutex_unlock(&cntr.DSC.th_mut);
    dscFree();
    
    return NULL;
}

void TMdContr::cntrCmd_( const string &a_path, XMLNode *opt, TCntrNode::Command cmd )
{
    if( cmd==TCntrNode::Info )
    {
        TController::cntrCmd_( a_path, opt, cmd );
	
        ctrMkNode("area",opt,-1,a_path.c_str(),"/board",owner().I18N("Board config"));
        ctrMkNode("area",opt,-1,a_path.c_str(),"/board/dio",owner().I18N("Digital IO ports. Select input!"));
	ctrMkNode("fld",opt,-1,a_path.c_str(),"/board/dio/a",owner().I18N("Port A"),0664,0,0,1,"tp","bool");
	ctrMkNode("fld",opt,-1,a_path.c_str(),"/board/dio/b",owner().I18N("Port B"),0664,0,0,1,"tp","bool");
	ctrMkNode("fld",opt,-1,a_path.c_str(),"/board/dio/c1",owner().I18N("Port C1"),0664,0,0,1,"tp","bool");
	ctrMkNode("fld",opt,-1,a_path.c_str(),"/board/dio/c2",owner().I18N("Port C2"),0664,0,0,1,"tp","bool");
    }
    else if( cmd==TCntrNode::Get )
    {
	if( a_path.substr(0,11) == "/board/dio/" )
	{
	    string port_n = TSYS::pathLev(a_path,2);
	    int	cfg_b = cfg("DIO_CFG").getI();
	    if( port_n == "a" )		ctrSetB( opt, cfg_b&0x10 );
	    else if( port_n == "b" ) 	ctrSetB( opt, cfg_b&0x02 );
	    else if( port_n == "c1" )	ctrSetB( opt, cfg_b&0x01 );
	    else if( port_n == "c2" )   ctrSetB( opt, cfg_b&0x08 );	
	}
	else TController::cntrCmd_( a_path, opt, cmd );
    }
    else if( cmd==TCntrNode::Set )
    {
	if( a_path.substr(0,11) == "/board/dio/" )
        {
            string port_n = TSYS::pathLev(a_path,2);
	    int cfg_b = cfg("DIO_CFG").getI();
	    if( port_n == "a" )         cfg("DIO_CFG").setI(ctrGetB(opt)?cfg_b|0x10:cfg_b&(~0x10));
    	    else if( port_n == "b" )    cfg("DIO_CFG").setI(ctrGetB(opt)?cfg_b|0x02:cfg_b&(~0x02));
	    else if( port_n == "c1" )   cfg("DIO_CFG").setI(ctrGetB(opt)?cfg_b|0x01:cfg_b&(~0x01));
	    else if( port_n == "c2" )   cfg("DIO_CFG").setI(ctrGetB(opt)?cfg_b|0x08:cfg_b&(~0x08));	    
	}
	else TController::cntrCmd_( a_path, opt, cmd );
    }
}			    

//======================================================================
//==== TMdPrm 
//======================================================================
TMdPrm::TMdPrm( string name, TTipParam *tp_prm ) :
    TParamContr(name,tp_prm), m_cnl(cfg("CNL").getId()), m_tp(NONE)
{

}

TMdPrm::~TMdPrm( )
{   
    nodeDelAll();
}

void TMdPrm::postEnable()
{
    TParamContr::postEnable();

    if( TParamContr::type().name() == "a_prm" )     	type(AI);
    else if( TParamContr::type().name() == "d_prm" )	type(DI);
}

/*void TMdPrm::preDisable( int flag )
{   
    TParamContr::preDisable(flag); 
    //type(NONE);
}*/

void TMdPrm::enable( )
{
    TParamContr::enable();
    
    if( type() == AI )
    {
	if( owner().ADIIntMode() )	cfg("GAIN").view(false);
	else cfg("GAIN").view(true);    
    }
}

void TMdPrm::type( TMdPrm::Type vtp )
{
    //Free previos type
    switch(m_tp)
    {
	case AI: vlElemDet( &mod->elemAI() ); break;
	case AO: vlElemDet( &mod->elemAO() ); break;
	case DI: vlElemDet( &mod->elemDI() ); break;
	case DO: vlElemDet( &mod->elemDO() ); break;
    }
    
    //Init new type
    switch(vtp)
    {
	case AI:    
	    cfg("GAIN").view(true);
	    m_gain = cfg("GAIN").getI();
	    vlElemAtt( &mod->elemAI() );
	    break;
	case AO:    
	    cfg("GAIN").view(false);    
	    vlElemAtt( &mod->elemAO() );
	    break;
	case DI:
	    m_dio_port = (cfg("PORT").getI()<<4)+cfg("CNL").getI();
	    vlElemAtt( &mod->elemDI() );
	    break;
	case DO:    
    	    vlElemAtt( &mod->elemDO() );
	    break;
    }
    m_tp = vtp;
}

bool TMdPrm::cfgChange( TCfg &i_cfg )
{
    //Change TYPE parameter
    if( i_cfg.name() == "TYPE" )
    {
	if( i_cfg.getI() == 0 && type() == AO )		type(AI);
	else if( i_cfg.getI() == 0 && type() == DO ) 	type(DI);
	else if( i_cfg.getI() == 1 && type() == AI ) 	type(AO);
        else if( i_cfg.getI() == 1 && type() == DI ) 	type(DO);
	else return false;
	return true;
    }
    switch(type())    
    {
	case AI:
	    if( i_cfg.name() == "GAIN" ) m_gain = i_cfg.getI();
	    else return false;
	case DI: case DO:
	    if( i_cfg.name() == "PORT" ) 	m_dio_port = (i_cfg.getI()<<4)+cfg("CNL").getI();
	    else if( i_cfg.name() == "CNL" )	m_dio_port = (cfg("PORT").getI()<<4)+i_cfg.getI();
	    else return false;	        
    }
    
    return true;
}									

void TMdPrm::vlSet( TVal &val )
{    
    if( !enableStat() )	return;
    switch(type())
    {
	case AO:
	{
	    int code;
	    if(val.fld().workId()==1)		code = (int)(4095.*val.getR(0,true)/100.);
	    else if(val.fld().workId()==2)	code = (int)(4095.*val.getR(0,true)/10.);
	    
	    ResAlloc::resRequestW(owner().DSC.gen_res);	//Request access to DSC
	    pthread_mutex_lock(&owner().DSC.th_mut);	//Request DSC ready
	    owner().DSC.comm = 2;
	    owner().DSC.prm1 = m_cnl;
	    owner().DSC.prm2 = code;
	    pthread_cond_signal(&owner().DSC.th_cv);
	    pthread_mutex_unlock(&owner().DSC.th_mut);
	    ResAlloc::resReleaseW(owner().DSC.gen_res);
	    break;
	}
	case DO:
	{
	    //Get prev port stat
	    ResAlloc::resRequestW(owner().DSC.gen_res);	//Request access to DSC
    	    pthread_mutex_lock(&owner().DSC.th_mut);	//Request DSC ready
	    owner().DSC.comm = 4;
	    owner().DSC.prm1 = m_dio_port;
	    owner().DSC.prm2 = val.getB(0,true);
	    pthread_cond_signal(&owner().DSC.th_cv);
	    pthread_mutex_unlock(&owner().DSC.th_mut);
	    ResAlloc::resReleaseW(owner().DSC.gen_res);
	}
    }
}

void TMdPrm::vlGet( TVal &val )
{
    if( val.fld().workId() == 0 )
    {
        if( !owner().startStat() )
            val.setS(mod->I18N("2:Controller stoped"),0,true);
        else if( !enableStat() )
            val.setS(mod->I18N("1:Parameter disabled"),0,true);
	else val.setS("0",0,true);
	return;
    }						

    switch(type())
    {
	case AI:
	{
	    if(owner().ADIIntMode()) return;
	    short gval;
	    if( enableStat() )
	    {
    		ResAlloc::resRequestW(owner().DSC.gen_res);	//Request access to DSC
		pthread_mutex_lock(&owner().DSC.th_mut);	//Request DSC ready
		owner().DSC.comm = 1;
		owner().DSC.prm1 = m_cnl;
		owner().DSC.prm2 = m_gain;
		pthread_cond_signal(&owner().DSC.th_cv);
		pthread_mutex_unlock(&owner().DSC.th_mut);
		while(owner().DSC.comm);
		gval = owner().DSC.prm2;
		ResAlloc::resReleaseW(owner().DSC.gen_res);
	    }
	    switch(val.fld().workId())
	    {
		case 1: val.setR(enableStat()?(100.*((double)gval/32768.)):EVAL_REAL,0,true); break;
		case 2: val.setR(enableStat()?(10.*((double)gval/32768.)):EVAL_REAL,0,true);  break;
		case 3: val.setI(enableStat()?gval:EVAL_INT,0,true);  break;
	    }
	    break;
	}
	case DI:
	{
	    char gval = EVAL_BOOL;
	    if( enableStat() )
            {
                ResAlloc::resRequestW(owner().DSC.gen_res);	//Request access to DSC
		pthread_mutex_lock(&owner().DSC.th_mut);	//Request DSC ready
		owner().DSC.comm = 3;
		owner().DSC.prm1 = m_dio_port;
		pthread_cond_signal(&owner().DSC.th_cv);
		pthread_mutex_unlock(&owner().DSC.th_mut);
		while(owner().DSC.comm);
		gval = (bool)owner().DSC.prm2;
		ResAlloc::resReleaseW(owner().DSC.gen_res);
	    }
	    val.setB(gval,0,true);
	    break;
	}
    }
}	
