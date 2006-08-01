
//OpenSCADA system file: tbds.cpp
/***************************************************************************
 *   Copyright (C) 2003-2006 by Roman Savochenko                           *
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

#include <unistd.h>
#include <getopt.h>

#include "tsys.h"
#include "tmess.h"
#include "tmodule.h"
#include "tbds.h"

//================================================================
//=========== TBDS ===============================================
//================================================================
TBDS::TBDS( ) : TSubSYS("BD","Data Bases",true)
{
    //Generic system DB
    fldAdd( new TFld("user","User",TFld::String,FLD_KEY,"20") );
    fldAdd( new TFld("id","Value ID",TFld::String,FLD_KEY,"100") );
    fldAdd( new TFld("val","Value"  ,TFld::String,FLD_NOFLG,"1000") );
    
    //Open data bases DB structure
    el_db.fldAdd( new TFld("ID",Mess->I18N("ID"),TFld::String,FLD_KEY,"20") );
    el_db.fldAdd( new TFld("TYPE",Mess->I18N("DB type (module)"),TFld::String,FLD_KEY,"20") );
    el_db.fldAdd( new TFld("NAME",Mess->I18N("Name"),TFld::String,FLD_NOFLG,"50") );
    el_db.fldAdd( new TFld("DESCR",Mess->I18N("Description"),TFld::String,FLD_NOFLG,"200") );
    el_db.fldAdd( new TFld("ADDR",Mess->I18N("Address"),TFld::String,FLD_NOFLG,"100") );
    el_db.fldAdd( new TFld("EN",Mess->I18N("To enable"),TFld::Bool,FLD_NOFLG,"1","true") );
}

TBDS::~TBDS(  )
{

}

AutoHD<TTable> TBDS::open( const string &bdn, bool create )
{
    bool bd_op = false;
    AutoHD<TTable> tbl;    
    
    try
    {
	string bd_t = TSYS::strSepParse(bdn,0,'.');
	string bd_n = TSYS::strSepParse(bdn,1,'.');
	string bd_tbl = TSYS::strSepParse(bdn,2,'.');
	if(bd_t == "*") bd_t = TSYS::strSepParse(SYS->workDB(),0,'.');
	if(bd_n == "*") bd_n = TSYS::strSepParse(SYS->workDB(),1,'.');
	if( at(bd_t).at().at(bd_n).at().enableStat() )
	{    
	    if( !at(bd_t).at().at(bd_n).at().openStat(bd_tbl) )
		at(bd_t).at().at(bd_n).at().open(bd_tbl,create);
	    tbl = at(bd_t).at().at(bd_n).at().at(bd_tbl);
	}
    }
    catch(TError err)
    { 
	/*Mess->put(err.cat.c_str(),TMess::Warning,"%s",err.mess.c_str());*/ 
    }

    return tbl;
}

void TBDS::close( const string &bdn, bool del )
{
    try
    {
	string bd_t = TSYS::strSepParse(bdn,0,'.');
        string bd_n = TSYS::strSepParse(bdn,1,'.');
        string bd_tbl = TSYS::strSepParse(bdn,2,'.');
	if(bd_t == "*") bd_t = TSYS::strSepParse(SYS->workDB(),0,'.');
        if(bd_n == "*") bd_n = TSYS::strSepParse(SYS->workDB(),1,'.');
	if( at(bd_t).at().at(bd_n).at().enableStat()&& 
		at(bd_t).at().at(bd_n).at().openStat(bd_tbl) && 
		at(bd_t).at().at(bd_n).at().at(bd_tbl).at().nodeUse() == 1 )
	    at(bd_t).at().at(bd_n).at().close(bd_tbl,del);
    }catch(TError err) 
    { 
	Mess->put(err.cat.c_str(),TMess::Warning,"%s",err.mess.c_str());
	Mess->put(nodePath().c_str(),TMess::Warning,Mess->I18N("Close DB <%s> error!"),bdn.c_str()); 
    }
}

string TBDS::SysBD()
{
    return SYS->workDB()+".SYS";
}

string TBDS::openBD()
{
    return SYS->workDB()+".DB";
}

bool TBDS::dataSeek( const string &bdn, const string &path, int lev, TConfig &cfg )
{
    int c_lev = 0;
    
    if( path.size() )
    {
	try
	{
	    XMLNode *nd = ctrId(&SYS->cfgRoot(),path);
	    for( int i_fld = 0; i_fld < nd->childSize(); i_fld++ )
	    {
		XMLNode *el = nd->childGet(i_fld);
		if( el->name() == "fld" )
		{
		    //Check keywords
		    vector<string> cf_el;
		    cfg.cfgList(cf_el);
		
		    //Check keywords		    
        	    int i_el;
        	    for( i_el = 0; i_el < cf_el.size(); i_el++ )
            		if( cfg.cfg(cf_el[i_el]).fld().flg()&FLD_KEY &&
	        	    cfg.cfg(cf_el[i_el]).getS().size() && 
			    cfg.cfg(cf_el[i_el]).getS() != el->attr(cf_el[i_el]) ) break;
		    if( i_el == cf_el.size() && lev <= c_lev++ )
		    {	
			for( int i_el = 0; i_el < cf_el.size(); i_el++ )
			    cfg.cfg(cf_el[i_el]).setS(Mess->codeConvIn("UTF8",el->attr(cf_el[i_el])));
			return true;
		    }
		}
	    }
	}
	catch(...){  }
    }
	
    if(bdn.size())
    {
        AutoHD<TTable> tbl = open(bdn);
        if( !tbl.freeStat() )
        {
	    bool rez = tbl.at().fieldSeek(lev-c_lev,cfg);
    	    tbl.free();
    	    close(bdn);
    	    return rez;
	}
    }    
    
    return false;
}

bool TBDS::dataGet( const string &bdn, const string &path, TConfig &cfg )
{
    AutoHD<TTable> tbl = open(bdn);

    //- Load from DB -
    if( !tbl.freeStat() )
    {
	bool db_true = true;
        try{ tbl.at().fieldGet(cfg); } catch(...){ db_true = false; }
	tbl.free();
        close(bdn);		
	if(db_true) return true;
    }
    //- Load from Config file if tbl no present -
    XMLNode *nd;
    try{ nd = ctrId(&SYS->cfgRoot(),path); }
    catch(...){ return false; }
    
    //-- Scan fields and fill Config --
    for( int i_fld = 0; i_fld < nd->childSize(); i_fld++ )
    {
        XMLNode *el = nd->childGet(i_fld);
        if( el->name() == "fld" )
    	{
	    //Check keywords
	    vector<string> cf_el;
    	    cfg.cfgList(cf_el);
		
	    int i_el;	
    	    for( i_el = 0; i_el < cf_el.size(); i_el++ )
    		if( cfg.cfg(cf_el[i_el]).fld().flg()&FLD_KEY && 
		    cfg.cfg(cf_el[i_el]).getS() != el->attr(cf_el[i_el]) ) break;
	    if( i_el == cf_el.size() )
	    {
	        for( int i_el = 0; i_el < cf_el.size(); i_el++ )
		    cfg.cfg(cf_el[i_el]).setS(Mess->codeConvIn("UTF8",el->attr(cf_el[i_el])));
		return true;    
	    }	    	    
	}
    }
    //throw TError(nodePath().c_str(),"Field <%s> no present.",path.c_str());
    return false;
}

void TBDS::dataSet( const string &bdn, const string &path, TConfig &cfg )
{
    AutoHD<TTable> tbl = open(bdn,true);
    
    if( !tbl.freeStat() )
    {
        tbl.at().fieldSet(cfg);
	tbl.free();
        close(bdn);		
	return;
    }
    //Load from Config file if tbl no present
    /*XMLNode *nd = ctrId(&SYS->cfgRoot(),path);

    //Scan fields and fill Config
    for( int i_fld = 0; i_fld < nd->childSize(); i_fld++ )
    {
        XMLNode *el = nd->childGet(i_fld);
        if( el->name() == "fld" )
        {
	    //Check keywords
	    vector<string> cf_el;
            cfg.cfgList(cf_el);
		
	    int i_el;	
            for( i_el = 0; i_el < cf_el.size(); i_el++ )
        	if( cfg.cfg(cf_el[i_el]).fld().flg()&FLD_KEY && 
		    cfg.cfg(cf_el[i_el]).getS() != el->attr(cf_el[i_el]) ) break;
	    if( i_el == cf_el.size() )
	    {
		for( int i_el = 0; i_el < cf_el.size(); i_el++ )
		    el->attr(cf_el[i_el],cfg.cfg(cf_el[i_el]).getS());
		return;    
	    }	    	    
	}
    }
    throw TError("BD","Field no present.");*/
}	

void TBDS::dataDel( const string &bdn, const string &path, TConfig &cfg )
{
    AutoHD<TTable> tbl = open(bdn,true);
    
    if( !tbl.freeStat() )
    {
	tbl.at().fieldDel(cfg);
	tbl.free();
        close(bdn);
	return;
    }		
}

void TBDS::genDBSet(const string &path, const string &val, const string &user)
{
    string rez;
    
    if(SYS->present("BD"))
    {
	AutoHD<TTable> tbl = SYS->db().at().open(SYS->db().at().SysBD(),true);
	if( !tbl.freeStat() )
	{
    	    TConfig db_el(&SYS->db().at());
	    db_el.cfg("user").setS(user);
    	    db_el.cfg("id").setS(path);
	    db_el.cfg("val").setS(val);
	
    	    try{ tbl.at().fieldSet(db_el); }
    	    catch(TError err){ }
	
	    tbl.free();
    	    SYS->db().at().close(SYS->db().at().SysBD());
	}
    }
}

string TBDS::genDBGet(const string &path, const string &oval, const string &user, bool onlyCfg )
{
    bool bd_ok=false;
    string rez = oval;
    //Get from generic DB
    if(SYS->present("BD") && !onlyCfg)
    {
	AutoHD<TTable> tbl = SYS->db().at().open(SYS->db().at().SysBD());
	if( !tbl.freeStat() )
	{
	    TConfig db_el(&SYS->db().at());
	    db_el.cfg("user").setS(user);
	    db_el.cfg("id").setS(path);
	    try
	    { 
		tbl.at().fieldGet(db_el);
		rez = db_el.cfg("val").getS();
		bd_ok = true;
	    }
	    catch(TError err){  }
	
	    tbl.free();
	    SYS->db().at().close(SYS->db().at().SysBD());
	}      
    }
    //Get from config file
    if(!bd_ok)	
	try{ rez = Mess->codeConvIn("UTF8",SYS->ctrId(&SYS->cfgRoot(),path)->text()); }
	catch(...){ }
    
    return rez;
}

string TBDS::optDescr(  )
{
    char buf[STR_BUF_LEN];
    snprintf(buf,sizeof(buf),Mess->I18N(
    	"========================= Subsystem \"DB\" options ========================\n"
	"------------ Parameters of section <%s> in config file -----------\n\n"
	),nodePath().c_str());

    return(buf);
}

void TBDS::subLoad( )
{
    //- Load parameters from command line -
    int next_opt;
    char *short_opt="h";
    struct option long_opt[] =
    {
	{"help"    ,0,NULL,'h'},
	{NULL      ,0,NULL,0  }
    };

    optind=opterr=0;	
    do
    {
	next_opt=getopt_long(SYS->argc,(char * const *)SYS->argv,short_opt,long_opt,NULL);
	switch(next_opt)
	{
	    case 'h': fprintf(stdout,optDescr().c_str()); break;
	    case -1 : break;
	}
    } while(next_opt != -1);

    //- Load parameters from config file -

    //- Open DB load -
    //Load and enable other DB store DB
    string db_tp = TSYS::strSepParse(openBD(),0,'.');
    string db_nm = TSYS::strSepParse(openBD(),1,'.');
    if( !at(db_tp).at().openStat(db_nm) )
    {
	at(db_tp).at().open(db_nm);
	at(db_tp).at().at(db_nm).at().load();
	at(db_tp).at().at(db_nm).at().enable();
    }
    
    string id,type;
    //- Search and open new DB -
    try
    {	
        TConfig c_el(&el_db);    
        int fld_cnt = 0;
        while( SYS->db().at().dataSeek(openBD(),nodePath()+"DB/",fld_cnt++,c_el) )
	{
            id = c_el.cfg("ID").getS();
    	    type = c_el.cfg("TYPE").getS();	    
	    if( (type+"."+id) != SYS->workDB() && !at(type).at().openStat(id) )
    		at(type).at().open(id);
	    c_el.cfg("ID").setS("");
            c_el.cfg("TYPE").setS("");
        }
    }catch( TError err )
    { 
	Mess->put(err.cat.c_str(),TMess::Error,"%s",err.mess.c_str());
	Mess->put(nodePath().c_str(),TMess::Error,Mess->I18N("Search and open new DB error."));
    }    
    
    //- Load and enable DB -
    vector<string> t_lst, o_lst;
    modList(t_lst);
    for( int i_t = 0; i_t < t_lst.size(); i_t++ )
    {
        AutoHD<TTipBD> mod = at(t_lst[i_t]);
        mod.at().list(o_lst);
        for( int i_o = 0; i_o < o_lst.size(); i_o++ )
	    try
	    {	
		mod.at().at(o_lst[i_o]).at().load();
		if( !mod.at().at(o_lst[i_o]).at().enableStat() && mod.at().at(o_lst[i_o]).at().toEnable() )
        	    mod.at().at(o_lst[i_o]).at().enable(); 
	    } catch(...){ }	
    }

    //- Load modules -
    TSubSYS::subLoad();
}

void TBDS::subSave( )
{
    vector<string> t_lst, o_lst;
    
    //Opened DB save to DB
    modList(t_lst);
    for( int i_t = 0; i_t < t_lst.size(); i_t++ )
    {
        AutoHD<TTipBD> mod = at(t_lst[i_t]);
        mod.at().list(o_lst);
        for( int i_o = 0; i_o < o_lst.size(); i_o++ )
            mod.at().at(o_lst[i_o]).at().save();
    }
}

void TBDS::cntrCmdProc( XMLNode *opt )
{
    //Get page info
    if( opt->name() == "info" )
    {
	TSubSYS::cntrCmdProc(opt);
	ctrMkNode("area",opt,0,"/bd",Mess->I18N("Subsystem"),0444,"root",subId().c_str());
	ctrMkNode("comm",opt,-1,"/bd/load_bd",Mess->I18N("Load"),0440,"root",subId().c_str());
        ctrMkNode("comm",opt,-1,"/bd/upd_bd",Mess->I18N("Save"),0440,"root",subId().c_str());
	ctrMkNode("fld",opt,-1,"/help/g_help",Mess->I18N("Options help"),0440,"root",subId().c_str(),3,"tp","str","cols","90","rows","5");
	return;
    }
    //Process command to page
    string a_path = opt->attr("path");
    if( a_path == "/help/g_help" && ctrChkNode(opt,"get",0440,"root",subId().c_str(),SEQ_RD) ) 		opt->text(optDescr());
    else if( a_path == "/bd/load_bd" && ctrChkNode(opt,"set",0440,"root",subId().c_str(),SEQ_RD) )	subLoad();
    else if( a_path == "/bd/upd_bd" && ctrChkNode(opt,"set",0440,"root",subId().c_str(),SEQ_RD) )	subSave();
    else TSubSYS::cntrCmdProc(opt);
}

//================================================================
//=========== TTipBD =============================================
//================================================================
TTipBD::TTipBD( ) : full_db_del(false)
{ 
    m_db = grpAdd("db_");
}

TTipBD::~TTipBD( )
{
    nodeDelAll();
}

void TTipBD::open( const string &iid )
{
    if( openStat(iid) ) return;
    chldAdd(m_db,openBD(iid));
}

void TTipBD::cntrCmdProc( XMLNode *opt )
{
    string grp = owner().subId().c_str();
    //Get page info
    if( opt->name() == "info" )
    {
	TModule::cntrCmdProc(opt);
	ctrMkNode("grp",opt,-1,"/br/db_",Mess->I18N("Opened DB"),0444,"root","root",1,"list","/bd/obd");
	ctrMkNode("area",opt,0,"/bd",Mess->I18N("DB"),0444);
	ctrMkNode("fld",opt,-1,"/bd/ful_db_del",Mess->I18N("Full DB delete"),0660,"root",grp.c_str(),1,"tp","bool");
	ctrMkNode("list",opt,-1,"/bd/obd",Mess->I18N("Opened DB"),0664,"root",grp.c_str(),4,"tp","br","idm","1","s_com","add,del","br_pref","db_");
	return;
    }

    //Process command to page
    string a_path = opt->attr("path");
    if( a_path == "/bd/ful_db_del"  )
    {
	if( ctrChkNode(opt,"get",0660,"root",grp.c_str(),SEQ_RD) )	opt->text(full_db_del?"1":"0");
	if( ctrChkNode(opt,"set",0660,"root",grp.c_str(),SEQ_WR) )	full_db_del = atoi(opt->text().c_str());
    }	
    else if( a_path == "/bd/obd" )
    {
	if( ctrChkNode(opt,"get",0664,"root",grp.c_str(),SEQ_RD) )
	{
	    vector<string> lst;
	    list(lst);
	    for( int i_l=0; i_l < lst.size(); i_l++)
		opt->childAdd("el")->attr("id",lst[i_l])->text(at(lst[i_l]).at().name());
	}
	if( ctrChkNode(opt,"add",0664,"root",grp.c_str(),SEQ_WR) )
        {
	    open(opt->attr("id"));
	    at(opt->attr("id")).at().name(opt->text());
        }
        if( ctrChkNode(opt,"del",0664,"root",grp.c_str(),SEQ_WR) )	close(opt->attr("id"),true);
    }
    else TModule::cntrCmdProc(opt);
}    

//================================================================
//=========== TBD ================================================
//================================================================
TBD::TBD( const string &iid, TElem *cf_el ) : TConfig( cf_el ),
    m_id(cfg("ID").getSd()), m_name(cfg("NAME").getSd()), m_dscr(cfg("DESCR").getSd()), 
    m_addr(cfg("ADDR").getSd()), m_toen(cfg("EN").getBd()), m_creat(false), m_en(false)
{
    m_id = iid;
    m_tbl = grpAdd("tbl_");
}

void TBD::postEnable( )
{
    cfg("TYPE").setS(owner().modId());
}

TBD::~TBD()
{

}

void TBD::preDisable(int flag)
{
    disable();
}

void TBD::postDisable(int flag)
{
    try
    {
        if( flag )
            SYS->db().at().dataDel(owner().owner().openBD(),SYS->db().at().nodePath()+"DB/",*this);
    }catch(TError err)
    { Mess->put(err.cat.c_str(),TMess::Warning,"%s",err.mess.c_str()); }
}

string TBD::name()
{
    return (m_name.size())?m_name:id();
}

void TBD::enable( )
{
    if( enableStat() ) return;
    
    m_en = true;
}

void TBD::disable( )
{
    if( !enableStat() ) return;
    //Close all tables
    vector<string> t_list;
    list(t_list);
    for(int i_l = 0; i_l < t_list.size(); i_l++)
        close(t_list[i_l]);
	    
    m_en = false;
}

void TBD::open( const string &table, bool create )
{
    if( chldPresent(m_tbl,table) ) return;
    chldAdd(m_tbl,openTable(table, create)); 
}

void TBD::load( )
{
    SYS->db().at().dataGet(owner().owner().openBD(),SYS->db().at().nodePath()+"DB/",*this);
}

void TBD::save( )
{
    SYS->db().at().dataSet(owner().owner().openBD(),SYS->db().at().nodePath()+"DB/",*this);
}

void TBD::cntrCmdProc( XMLNode *opt )
{
    string grp = owner().owner().subId();
    //Get page info
    if( opt->name() == "info" )
    {
        ctrMkNode("oscada_cntr",opt,-1,"/",Mess->I18N("Data base: ")+name());
        ctrMkNode("area",opt,-1,"/prm",Mess->I18N("Data base"));
        ctrMkNode("area",opt,-1,"/prm/st",Mess->I18N("State"));
        ctrMkNode("fld",opt,-1,"/prm/st/st",Mess->I18N("Enable"),0664,"root",grp.c_str(),1,"tp","bool");
	ctrMkNode("fld",opt,-1,"/prm/st/mk",Mess->I18N("Create new DB"),0664,"root",grp.c_str(),1,"tp","bool");
        ctrMkNode("area",opt,-1,"/prm/cfg",Mess->I18N("Config"));
        ctrMkNode("fld",opt,-1,"/prm/cfg/id",cfg("ID").fld().descr(),0444,"root",grp.c_str(),1,"tp","str");
        ctrMkNode("fld",opt,-1,"/prm/cfg/nm",cfg("NAME").fld().descr(),0664,"root",grp.c_str(),1,"tp","str");
        ctrMkNode("fld",opt,-1,"/prm/cfg/dscr",cfg("DESCR").fld().descr(),0664,"root",grp.c_str(),3,"tp","str","cols","50","rows","3");
        ctrMkNode("fld",opt,-1,"/prm/cfg/addr",cfg("ADDR").fld().descr(),0664,"root",grp.c_str(),1,"tp","str");
	ctrMkNode("fld",opt,-1,"/prm/cfg/toen",cfg("EN").fld().descr(),0664,"root",grp.c_str(),1,"tp","bool");
        ctrMkNode("comm",opt,-1,"/prm/cfg/load",Mess->I18N("Load"),0440,"root",grp.c_str());
        ctrMkNode("comm",opt,-1,"/prm/cfg/save",Mess->I18N("Save"),0440,"root",grp.c_str());    
	opt->attr("rez","0");
        return;
    }

    //Process command to page
    string a_path = opt->attr("path");
    if( a_path == "/prm/st/st" )
    {
    	if( ctrChkNode(opt,"get",0664,"root",grp.c_str(),SEQ_RD) )	opt->text(enableStat()?"1":"0");
	if( ctrChkNode(opt,"set",0664,"root",grp.c_str(),SEQ_WR) )	atoi(opt->text().c_str())?enable():disable();
    }
    else if( a_path == "/prm/st/mk" )
    {
	if( ctrChkNode(opt,"get",0664,"root",grp.c_str(),SEQ_RD) )	opt->text(create()?"1":"0");
	if( ctrChkNode(opt,"set",0664,"root",grp.c_str(),SEQ_WR) )	create(atoi(opt->text().c_str()));
    }
    else if( a_path == "/prm/cfg/id" && ctrChkNode(opt) )	opt->text(id());
    else if( a_path == "/prm/cfg/nm" )
    {
	if( ctrChkNode(opt,"get",0664,"root",grp.c_str(),SEQ_RD) )	opt->text(name());
	if( ctrChkNode(opt,"set",0664,"root",grp.c_str(),SEQ_WR) )	m_name = opt->text();
    }
    else if( a_path == "/prm/cfg/dscr" )
    {
	if( ctrChkNode(opt,"get",0664,"root",grp.c_str(),SEQ_RD) )	opt->text(dscr());
	if( ctrChkNode(opt,"set",0664,"root",grp.c_str(),SEQ_WR) )	m_dscr = opt->text();
    }	
    else if( a_path == "/prm/cfg/addr" )
    {
	if( ctrChkNode(opt,"get",0664,"root",grp.c_str(),SEQ_RD) )	opt->text(addr());
	if( ctrChkNode(opt,"set",0664,"root",grp.c_str(),SEQ_WR) )	m_addr = opt->text();
    }	
    else if( a_path == "/prm/cfg/toen" )
    {
	if( ctrChkNode(opt,"get",0664,"root",grp.c_str(),SEQ_RD) )	opt->text(toEnable()?"1":"0");
	if( ctrChkNode(opt,"set",0664,"root",grp.c_str(),SEQ_WR) )	toEnable( atoi(opt->text().c_str()) );
    }
    else if( a_path == "/prm/cfg/load" && ctrChkNode(opt,"set",0440,"root",grp.c_str(),SEQ_RD) )	load();
    else if( a_path == "/prm/cfg/save" && ctrChkNode(opt,"set",0440,"root",grp.c_str(),SEQ_RD) )    	save();
}

//================================================================
//=========== TTable =============================================
//================================================================
TTable::TTable( const string &name ) :  m_name(name)
{

}

TTable::~TTable()
{ 

}
    
