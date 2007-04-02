
//OpenSCADA system module BD.MySQL file: my_sql.cpp
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

#include <getopt.h>
#include <string>
#include <mysql/mysql.h>
#include <mysql/errmsg.h>

#include <resalloc.h>
#include <tsys.h>
#include <tmess.h>
#include "my_sql.h"


//============ Modul info! =====================================================
#define MOD_ID      "MySQL"
#define MOD_NAME    "DB MySQL"
#define MOD_TYPE    "BD"
#define VER_TYPE    VER_BD
#define VERSION     "1.2.0"
#define AUTORS      "Roman Savochenko"
#define DESCRIPTION "BD modul. Allow support of the BD MySQL."
#define MOD_LICENSE "GPL"
//==============================================================================

BDMySQL::BDMod *BDMySQL::mod;

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
	BDMySQL::BDMod *self_addr = NULL;

	if( AtMod.id == MOD_ID && AtMod.type == MOD_TYPE && AtMod.t_ver == VER_TYPE )
	    self_addr = BDMySQL::mod = new BDMySQL::BDMod( source );       

	return ( self_addr );
    }
}

using namespace BDMySQL;

//==============================================================================
//====================== BDMySQL::BDMod ========================================
//==============================================================================

BDMod::BDMod(string name)
{
    mId 	= MOD_ID;
    mName	= MOD_NAME;
    mType  	= MOD_TYPE;
    mVers      	= VERSION;
    mAutor    	= AUTORS;
    mDescr  	= DESCRIPTION;
    mLicense   	= MOD_LICENSE;
    mSource    	= name;
}

BDMod::~BDMod()
{

}

TBD *BDMod::openBD( const string &name )
{    
    return new MBD(name,&owner().openDB_E());
}
	    
string BDMod::optDescr( )
{
    char buf[STR_BUF_LEN];

    snprintf(buf,sizeof(buf),_(
	"======================= The module <%s:%s> options =======================\n"
	"---------- Parameters of the module section <%s> in config file ----------\n\n"),
	MOD_TYPE,MOD_ID,nodePath().c_str());

    return(buf);
}			

void BDMod::modLoad( )
{
    //========== Load parameters from command line ============
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
}

//=============================================================
//====================== BDMySQL::MBD =========================
//=============================================================
MBD::MBD( string iid, TElem *cf_el ) : TBD(iid,cf_el)
{    
    conn_res = ResAlloc::resCreate( );
}

MBD::~MBD( )
{   
    ResAlloc::resDelete(conn_res);
}

void MBD::postDisable(int flag)
{
    TBD::postDisable(flag);
    
    if( flag && owner().fullDeleteDB() )
    {
	MYSQL connect;
    
	if(!mysql_init(&connect)) throw TError(nodePath().c_str(),_("Error initializing client."));
	connect.reconnect = 1;
	if(!mysql_real_connect(&connect,host.c_str(),user.c_str(),pass.c_str(),"",port,(u_sock.size())?u_sock.c_str():NULL,0))
	    throw TError(nodePath().c_str(),_("Connect to DB error: %s"),mysql_error(&connect));
    
	string req = "DROP DATABASE `"+bd+"`";
	if(mysql_real_query(&connect,req.c_str(),req.size()))
	    throw TError(nodePath().c_str(),_("Query to DB error: %s"),mysql_error(&connect));
	
	mysql_close(&connect);	
    }
}

void MBD::enable( )
{
    if( enableStat() )	return;

    host = TSYS::strSepParse(addr(),0,';');
    user = TSYS::strSepParse(addr(),1,';');
    pass = TSYS::strSepParse(addr(),2,';');
    bd   = TSYS::strSepParse(addr(),3,';');
    port = atoi(TSYS::strSepParse(addr(),4,';').c_str());
    u_sock = TSYS::strSepParse(addr(),5,';');
    cd_pg  = codePage().size()?codePage():Mess->charset();

    if(!mysql_init(&connect)) 
	throw TError(nodePath().c_str(),_("Error initializing client."));
    connect.reconnect = 1;
    if(!mysql_real_connect(&connect,host.c_str(),user.c_str(),pass.c_str(),"",port,(u_sock.size())?u_sock.c_str():NULL,0))
	throw TError(nodePath().c_str(),_("Connect to DB error: %s"),mysql_error(&connect));

    TBD::enable( );

    if( create() )
    {
        string req = "CREATE DATABASE IF NOT EXISTS `"+TSYS::strEncode(bd,TSYS::SQL)+"`";
	sqlReq(req);	
    }
    
    sqlReq("USE `"+TSYS::strEncode(bd,TSYS::SQL)+"`");
}

void MBD::disable( )
{
    if( !enableStat() )  return;
    
    TBD::disable( );
    
    ResAlloc resource(conn_res,true);
    mysql_close(&connect);
}

TTable *MBD::openTable( const string &inm, bool create )
{
    if( !enableStat() )
        throw TError(nodePath().c_str(),_("Error open table <%s>. DB disabled."),inm.c_str());

    return new MTable(inm,this,create);
}

void MBD::sqlReq( const string &ireq, vector< vector<string> > *tbl )
{
    MYSQL_RES *res = NULL;
    
    if( !enableStat() )	return;
    
    string req = Mess->codeConvOut(cd_pg.c_str(),ireq);

    ResAlloc resource(conn_res,true);
    
    int irez;
    if( irez = mysql_real_query(&connect,req.c_str(),req.size()) )
    {
	if(irez == CR_SERVER_GONE_ERROR || irez == CR_SERVER_LOST)
	{
	    resource.release();
	    disable();
	    enable();
	    resource.request(true);
	    irez = mysql_real_query(&connect,req.c_str(),req.size());
	}
	if(irez) throw TError(nodePath().c_str(),_("Query to DB error: %s"),mysql_error(&connect));
    }
    if( mysql_field_count(&connect) == 0 ) return;
    if( !(res = mysql_store_result(&connect)) )
    	throw TError(nodePath().c_str(),_("Store result error: %s"),mysql_error(&connect));
  	
    if( tbl )
    {
    	int num_fields = mysql_num_fields(res);
	MYSQL_ROW row;
	
        vector<string> fld;
        //Add head
        for(int i=0; i < num_fields; i++)
	    fld.push_back(mysql_fetch_field_direct(res,i)->name);
	tbl->push_back(fld);
        //Add data
        while( (row = mysql_fetch_row(res)) )
	{
	    fld.clear();
	    for(int i=0; i < num_fields; i++)
		fld.push_back(row[i]?Mess->codeConvIn(cd_pg.c_str(),row[i]):"");
	    tbl->push_back(fld);
	}
    }
    
    mysql_free_result(res);
}

//=============================================================
//=================== MBDMySQL::Table =========================
//=============================================================
MTable::MTable(string name, MBD *iown, bool create ) : TTable(name)
{
    string req;
    
    nodePrev(iown);
    
    if( create )
    {
        req = "CREATE TABLE IF NOT EXISTS `"+TSYS::strEncode(name,TSYS::SQL)+"` (`name` char(20) NOT NULL DEFAULT '' PRIMARY KEY)";
        owner().sqlReq( req );
    }
    //- Check for table present request -
    req ="DESCRIBE `"+TSYS::strEncode(name,TSYS::SQL)+"`";
    owner().sqlReq( req );    
    //req = "SELECT * FROM `"+TSYS::strEncode(name,TSYS::SQL)+"` LIMIT 0,1";
    //owner().sqlReq( req );
}

MTable::~MTable(  )
{

}

void MTable::postDisable(int flag)
{
    if( flag )
    {
	try{ owner().sqlReq("DROP TABLE `"+TSYS::strEncode(name(),TSYS::SQL)+"`"); }
	catch(TError err){ mess_warning(err.cat.c_str(),"%s",err.mess.c_str()); }    
    }
}

bool MTable::fieldSeek( int row, TConfig &cfg )
{
    vector< vector<string> > tbl;

    //- Get config fields list -
    vector<string> cf_el;
    cfg.cfgList(cf_el);

    //- Get table description -
    string req ="DESCRIBE `"+TSYS::strEncode(name(),TSYS::SQL)+"`";
    owner().sqlReq( req, &tbl );
    if( tbl.size() == 0 ) throw TError(nodePath().c_str(),_("Table is empty!"));

    //- Make SELECT and WHERE -
    req = "SELECT ";
    string req_where = "WHERE ";
    //Add use keys to list
    bool first_sel = true;
    bool next = false;
    for( int i_cf = 0; i_cf < cf_el.size(); i_cf++ )
	for( int i_fld = 1; i_fld < tbl.size(); i_fld++ )
    	    if( cf_el[i_cf] == tbl[i_fld][0] )
	    {
		if( cfg.cfg(cf_el[i_cf]).fld().flg()&TCfg::Key && cfg.cfg(cf_el[i_cf]).getS().size() )
    		{
        	    if( !next ) next = true;
        	    else req_where=req_where+"AND ";
        	    req_where=req_where+"`"+TSYS::strEncode(cf_el[i_cf],TSYS::SQL)+"`='"+
				TSYS::strEncode(cfg.cfg(cf_el[i_cf]).getS(),TSYS::SQL)+"' ";
		}
		else if( cfg.cfg(cf_el[i_cf]).fld().flg()&TCfg::Key || cfg.cfg(cf_el[i_cf]).view() )
		{
		    if( first_sel ) req=req+"`"+TSYS::strEncode(cf_el[i_cf],TSYS::SQL)+"`";
		    else req=req+",`"+TSYS::strEncode(cf_el[i_cf],TSYS::SQL)+"`";
		    first_sel = false;
		}
		break;
    	    }

    req = req + " FROM `"+TSYS::strEncode(name(),TSYS::SQL)+"` "+
		 ((next)?req_where:"") +" LIMIT "+TSYS::int2str(row)+",1";
    tbl.clear();
    owner().sqlReq( req, &tbl );
    if( tbl.size() < 2 ) return false;
    for( int i_cf = 0; i_cf < cf_el.size(); i_cf++ )
	for( int i_fld = 0; i_fld < tbl[0].size(); i_fld++ )
    	    if( cf_el[i_cf] == tbl[0][i_fld] )
	    {
		string val = tbl[1][i_fld];
		TCfg &u_cfg = cfg.cfg(cf_el[i_cf]);
		switch(u_cfg.fld().type())
		{
		    case TFld::String:	u_cfg.setS(val);break;
		    case TFld::Integer:	u_cfg.setI(atoi(val.c_str())); 	break;
		    case TFld::Real:	u_cfg.setR(atof(val.c_str()));	break;
		    case TFld::Boolean:	u_cfg.setB(atoi(val.c_str()));	break;
		}
		break;
	    }
    return true;
}

void MTable::fieldGet( TConfig &cfg )
{
    vector< vector<string> > tbl;
	
    //Get config fields list
    vector<string> cf_el;
    cfg.cfgList(cf_el);
    
    //Get present fields list
    string req ="DESCRIBE `"+TSYS::strEncode(name(),TSYS::SQL)+"`";
    owner().sqlReq( req, &tbl );
    if( tbl.size() == 0 ) throw TError(nodePath().c_str(),_("Table is empty!"));
    //Prepare request
    req = "SELECT ";
    string req_where;
    //Add fields list to queue
    bool first_sel = true;
    bool next_wr = false;
    for( int i_cf = 0; i_cf < cf_el.size(); i_cf++ )
        for( int i_fld = 1; i_fld < tbl.size(); i_fld++ )
            if( cf_el[i_cf] == tbl[i_fld][0] )
	    {
                if( cfg.cfg(cf_el[i_cf]).fld().flg()&TCfg::Key )
                {
                    if( !next_wr ) next_wr = true; else req_where=req_where+"AND ";
            	    req_where=req_where+"`"+TSYS::strEncode(tbl[i_fld][0],TSYS::SQL)+"`='"+
					TSYS::strEncode(cfg.cfg(cf_el[i_cf]).getS(),TSYS::SQL)+"' ";
                }
		else if( cfg.cfg(cf_el[i_cf]).view() )
		{
		    if( first_sel ) req=req+"`"+TSYS::strEncode(cf_el[i_cf],TSYS::SQL)+"`";
		    else req=req+",`"+TSYS::strEncode(cf_el[i_cf],TSYS::SQL)+"`";
		    first_sel = false;
		}		
		break;
	    }
    req = req+" FROM `"+TSYS::strEncode(name(),TSYS::SQL)+"` WHERE "+req_where;
    //Query
    //printf("TEST 01: query: <%s>\n",req.c_str());
    tbl.clear();
    owner().sqlReq( req, &tbl );
    if( tbl.size() < 2 ) throw TError(nodePath().c_str(),_("Row no present!"));
    //Processing of query
    for( int i_cf = 0; i_cf < cf_el.size(); i_cf++ )
	for( int i_fld = 0; i_fld < tbl[0].size(); i_fld++ )
	    if( cf_el[i_cf] == tbl[0][i_fld] )
	    {
		string val = tbl[1][i_fld];
                TCfg &u_cfg = cfg.cfg(cf_el[i_cf]);
		switch(u_cfg.fld().type())
		{
		    case TFld::String:	u_cfg.setS(val);break;
		    case TFld::Integer:	u_cfg.setI(atoi(val.c_str())); 	break;
		    case TFld::Real:	u_cfg.setR(atof(val.c_str()));	break;
		    case TFld::Boolean:	u_cfg.setB(atoi(val.c_str()));	break;
		}
		break;
            }
}

void MTable::fieldSet( TConfig &cfg )
{
    vector< vector<string> > tbl, tbl_str;
	
    //Get config fields list
    vector<string> cf_el;
    cfg.cfgList(cf_el);
		    
    //Fix BD structure
    fieldFix(cfg);
			    
    //Get present fields list
    string req ="DESCRIBE `"+TSYS::strEncode(name(),TSYS::SQL)+"`";
    owner().sqlReq( req, &tbl_str );
    if( tbl_str.size() == 0 ) throw TError(nodePath().c_str(),_("Table is empty!"));
    
    //Get present fields list
    string req_where = "WHERE ";
    //Add key list to queue
    bool next = false;
    for( int i_cf = 0; i_cf < cf_el.size(); i_cf++ )
	for( int i_fld = 1; i_fld < tbl_str.size(); i_fld++ )
    	    if( cf_el[i_cf] == tbl_str[i_fld][0] && cfg.cfg(cf_el[i_cf]).fld().flg()&TCfg::Key )
            {
                if( !next ) next = true; else req_where=req_where+"AND ";
                req_where=req_where+"`"+TSYS::strEncode(cf_el[i_cf],TSYS::SQL)+"`='"+TSYS::strEncode(cfg.cfg(cf_el[i_cf]).getS(),TSYS::SQL)+"' ";
		break;
            }
    //Query
    req = "SELECT 1 FROM `"+TSYS::strEncode(name(),TSYS::SQL)+"` "+req_where;
    owner().sqlReq( req, &tbl );
    if( tbl.size() < 2 )
    {
        //Add line
        req = "INSERT INTO `"+TSYS::strEncode(name(),TSYS::SQL)+"` ";
        string ins_name, ins_value;
        next = false;
        for( int i_cf = 0; i_cf < cf_el.size(); i_cf++ )
            for( int i_fld = 1; i_fld < tbl_str.size(); i_fld++ )
                if( cf_el[i_cf] == tbl_str[i_fld][0] )
		{
		    if( !(cfg.cfg(cf_el[i_cf]).fld().flg()&TCfg::Key) && !cfg.cfg(cf_el[i_cf]).view() )
			break;
                    
		    if( !next ) next = true;
                    else
            	    {
                        ins_name=ins_name+",";
                        ins_value=ins_value+",";
                    }
            	    ins_name=ins_name+"`"+TSYS::strEncode(cf_el[i_cf],TSYS::SQL)+"` ";
                    string val;
                    TCfg &u_cfg = cfg.cfg(cf_el[i_cf]);
		    switch(u_cfg.fld().type())
		    {
			case TFld::String:	val = u_cfg.getS();	break;
			case TFld::Integer:	val = SYS->int2str(u_cfg.getI());	break;
			case TFld::Real:	val = SYS->real2str(u_cfg.getR());	break;
			case TFld::Boolean:	val = SYS->int2str(u_cfg.getB());	break;
		    }
                    ins_value=ins_value+"'"+TSYS::strEncode(val,TSYS::SQL)+"' ";
		    break;
                }
        req = req + "("+ins_name+") VALUES ("+ins_value+")";
    }
    else
    {
        //Update line
        req = "UPDATE `"+TSYS::strEncode(name(),TSYS::SQL)+"` SET ";
        next = false;
        for( int i_cf = 0; i_cf < cf_el.size(); i_cf++ )
            for( int i_fld = 1; i_fld < tbl_str.size(); i_fld++ )
                if( cf_el[i_cf] == tbl_str[i_fld][0] )
		{
		    if( cfg.cfg(cf_el[i_cf]).fld().flg()&TCfg::Key || !cfg.cfg(cf_el[i_cf]).view() )
			break;
		    
                    if( !next ) next = true; else req=req+",";
                    string val;
                    TCfg &u_cfg = cfg.cfg(cf_el[i_cf]);
		    switch(u_cfg.fld().type())
		    {
			case TFld::String:	val = u_cfg.getS();	break;
			case TFld::Integer:	val = SYS->int2str(u_cfg.getI());	break;
			case TFld::Real:	val = SYS->real2str(u_cfg.getR());	break;
			case TFld::Boolean:	val = SYS->int2str(u_cfg.getB());	break;
		    }
            	    req=req+"`"+TSYS::strEncode(cf_el[i_cf],TSYS::SQL)+"`='"+TSYS::strEncode(val,TSYS::SQL)+"' ";
		    break;
                }
        req = req + req_where;
    }
    //Query
    //printf("TEST 02: query: <%s>\n",req.c_str());
    owner().sqlReq( req );
}

void MTable::fieldDel( TConfig &cfg )
{
    vector< vector<string> > tbl;
	
    //Get config fields list
    vector<string> cf_el;
    cfg.cfgList(cf_el);
    
    //Get present fields list
    string req ="DESCRIBE `"+TSYS::strEncode(name(),TSYS::SQL)+"`";
    owner().sqlReq( req, &tbl );
    if( tbl.size() == 0 ) throw TError(nodePath().c_str(),_("Table is empty!"));
				    
    //Prepare request
    req = "DELETE FROM `"+TSYS::strEncode(name(),TSYS::SQL)+"` WHERE ";
    //Add key list to queue
    bool next = false;
    for( int i_cf = 0; i_cf < cf_el.size(); i_cf++ )
        for( int i_fld = 1; i_fld < tbl.size(); i_fld++ )
            if( cf_el[i_cf] == tbl[i_fld][0] )
	    {
		if( cfg.cfg(cf_el[i_cf]).fld().flg()&TCfg::Key && cfg.cfg(cf_el[i_cf]).getS().size() )
        	{
        	    if( !next ) next = true; else req=req+"AND ";
            	    req=req+"`"+TSYS::strEncode(tbl[i_fld][0],TSYS::SQL)+"`='"+TSYS::strEncode(cfg.cfg(cf_el[i_cf]).getS(),TSYS::SQL)+"' ";
        	}
		break;
	    }
    owner().sqlReq( req );
}

void MTable::fieldFix( TConfig &cfg )
{
    bool next, next_key;
    vector< vector<string> > tbl;
	
    //Get config fields list
    vector<string> cf_el;
    cfg.cfgList(cf_el);
	    
    //Get present fields list
    string req ="DESCRIBE `"+TSYS::strEncode(name(),TSYS::SQL)+"`";
    owner().sqlReq( req, &tbl );
    if( tbl.size() == 0 ) throw TError(nodePath().c_str(),_("Table is empty!"));

    //Prepare request for fix structure
    req = "ALTER TABLE `"+TSYS::strEncode(name(),TSYS::SQL)+"` DROP PRIMARY KEY, ";
	    
    next = false;
    next_key = false;
    string pr_keys;
    for( int i_cf = 0; i_cf < cf_el.size(); i_cf++ )
    {
	TCfg &u_cfg = cfg.cfg(cf_el[i_cf]);
	//Check primary key
	if( u_cfg.fld().flg()&TCfg::Key )
	{
	    if( !next_key ) next_key = true;
	    else pr_keys=pr_keys+",";
	    pr_keys=pr_keys+"`"+TSYS::strEncode(u_cfg.name(),TSYS::SQL)+"`";
	}
	
	int i_fld;
        for( i_fld = 1; i_fld < tbl.size(); i_fld++ )
            if( cf_el[i_cf] == tbl[i_fld][0] )	break;
        if( i_fld < tbl.size() )
	{
	    //if( i_cf == i_fld-1 )	//Check order 
	    //{	
		string f_tp;
		switch(u_cfg.fld().type())
	    	{
    		    case TFld::String:
			if( u_cfg.fld().len() < 200 )
			    f_tp = "char("+TSYS::int2str(u_cfg.fld().len())+")";
			else f_tp = "text";
			break;
		    case TFld::Integer:
		   	f_tp = "int("+TSYS::int2str(u_cfg.fld().len())+")";     
			break;
		    case TFld::Real:
			f_tp = "double("+TSYS::int2str(u_cfg.fld().len())+","+TSYS::int2str(u_cfg.fld().dec())+")";
			break;
		    case TFld::Boolean:
 			f_tp = "tinyint(1)";
			break;
		}
		if( tbl[i_fld][1] == f_tp )	continue;
	    //}
	    //Change field
	    if( !next ) next = true; else req=req+",";
	    req=req+"CHANGE `"+TSYS::strEncode(cf_el[i_cf],TSYS::SQL)+"` `"+TSYS::strEncode(cf_el[i_cf],TSYS::SQL)+"` ";
	    fieldPrmSet(u_cfg,(i_cf>0)?cf_el[i_cf-1]:"",req);	    
	}
	else
	{
	    if( !next ) next = true; else req=req+",";
	    //Add field	    
	    req=req+"ADD `"+TSYS::strEncode(cf_el[i_cf],TSYS::SQL)+"` ";
	    fieldPrmSet(u_cfg,(i_cf>0)?cf_el[i_cf-1]:"",req);
	}
    }
    //DROP fields
    for( int i_fld = 1; i_fld < tbl.size(); i_fld++ )
    {
	int i_cf;
	for( i_cf = 0; i_cf < cf_el.size(); i_cf++ )
	    if( cf_el[i_cf] == tbl[i_fld][0] )	break;
	if( i_cf >= cf_el.size() )
	{
	    if( !next ) next = true; else req=req+",";
	    req=req+"DROP `"+TSYS::strEncode(tbl[i_fld][0],TSYS::SQL)+"` ";
	}	
    }
    req=req+",ADD PRIMARY KEY ("+pr_keys+") ";
    
    //if( next ) printf("TEST 02a: query: <%s>\n",req.c_str());
    if( next ) owner().sqlReq( req );
}
    
void MTable::fieldPrmSet( TCfg &cfg, const string &last, string &req )
{
    //Type param
    switch(cfg.fld().type())
    {
	case TFld::String:
	    if( cfg.fld().len() < 200 )
		req=req+"char("+SYS->int2str(cfg.fld().len())+") NOT NULL DEFAULT '"+cfg.fld().def()+"' ";
	    else req=req+"text NOT NULL DEFAULT '"+cfg.fld().def()+"' ";
	    break;
	case TFld::Integer:
	    req=req+"int("+SYS->int2str(cfg.fld().len())+") NOT NULL DEFAULT '"+cfg.fld().def()+"' ";
	    break;
	case TFld::Real:
	    req=req+"double("+SYS->int2str(cfg.fld().len())+","+SYS->int2str(cfg.fld().dec())+") NOT NULL DEFAULT '"+cfg.fld().def()+"' ";
	    break;
	case TFld::Boolean:
	    req=req+"tinyint(1) NOT NULL DEFAULT '"+cfg.fld().def()+"' ";
	    break;
    }
    //Position param
    //if( last.size() )	req=req+"AFTER `"+last+"` ";
}

