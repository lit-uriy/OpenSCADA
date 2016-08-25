//OpenSCADA system module DAQ.FT3 file: ODOR.cpp
/***************************************************************************
 *   Copyright (C) 2011-2016 by Maxim Kochetkov                            *
 *   fido_max@inbox.ru                                                     *
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

#include <sys/times.h>

#include <tsys.h>

#include "mod_FT3.h"
#include "ODOR.h"

using namespace FT3;

B_ODOR::B_ODOR(TMdPrm& prm, uint16_t id, uint16_t n, bool has_params) :
	DA(prm), ID(id), count_n(n), with_params(has_params)
{
    mTypeFT3 = GRS;
    blkID = 0;
    TFld * fld;
    mPrm.p_el.fldAdd(fld = new TFld("state", _("State"), TFld::Integer, TFld::NoWrite));
    fld->setReserve("0:0");
    for(int i = 0; i < count_n; i++) {
	AddChannel(i);
    }
    loadIO(true);
}

B_ODOR::~B_ODOR()
{

}

void B_ODOR::AddChannel(uint8_t iid)
{
    data.push_back(SPchannel(iid, this));
    AddAttr(data.back().State.lnk, TFld::Integer, TFld::NoWrite, TSYS::strMess("%d:0", iid + 1));
    AddAttr(data.back().Value.lnk, TFld::Real, TFld::NoWrite, TSYS::strMess("%d:1", iid + 1));
    if(with_params) {
	AddAttr(data.back().Period.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().Sens.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:3", iid + 1));
	AddAttr(data.back().Count.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:4", iid + 1));
	AddAttr(data.back().Vdrop.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:5", iid + 1));
    }
}

string B_ODOR::getStatus(void)
{
    string rez;
    if(NeedInit) {
	rez = "20: Опрос";
    } else {
	rez = "0: Норма";
    }
    return rez;

}

void B_ODOR::loadIO(bool force)
{
    if(mPrm.owner().startStat() && !force) {
	mPrm.modif(true);
	return;
    }	//Load/reload IO context only allow for stopped controllers for prevent throws

    for(int i = 0; i < count_n; i++) {
	loadLnk(data[i].State.lnk);
	loadLnk(data[i].Value.lnk);
	loadLnk(data[i].Period.lnk);
	loadLnk(data[i].Sens.lnk);
	loadLnk(data[i].Count.lnk);
	loadLnk(data[i].Vdrop.lnk);
    }
}

void B_ODOR::saveIO()
{
    for(int i = 0; i < count_n; i++) {
	saveLnk(data[i].State.lnk);
	saveLnk(data[i].Value.lnk);
	saveLnk(data[i].Period.lnk);
	saveLnk(data[i].Sens.lnk);
	saveLnk(data[i].Count.lnk);
	saveLnk(data[i].Vdrop.lnk);
    }
}

void B_ODOR::tmHandler(void)
{
    for(int i = 0; i < count_n; i++) {
	if(with_params) {
	    UpdateParam8(data[i].Period, PackID(ID, (i + 1), 2), 1);
	    UpdateParamFl(data[i].Sens, PackID(ID, (i + 1), 3), 1);
	    UpdateParam32(data[i].Count, PackID(ID, (i + 1), 4), 0);
	    UpdateParamFl(data[i].Vdrop, PackID(ID, (i + 1), 5), 1);
	}
	UpdateParamFlState(data[i].Value, data[i].State, data[i].Sens, PackID(ID, (i + 1), 1), 2);
    }
    NeedInit = false;
}

uint16_t B_ODOR::Task(uint16_t uc)
{
    tagMsg Msg;
    uint16_t rc = 0;
    switch(uc) {
    case TaskRefresh:
	Msg.L = 5;
	Msg.C = AddrReq;
	*((uint16_t *) Msg.D) = PackID(ID, 0, 0); //state
	if(mPrm.owner().DoCmd(&Msg)) {
	    if(Msg.C == GOOD3) {
		if(with_params) {
		    for(int i = 1; i <= count_n; i++) {
			Msg.L = 13;
			Msg.C = AddrReq;
			*((uint16_t *) Msg.D) = PackID(ID, i, 1);
			*((uint16_t *) (Msg.D + 2)) = PackID(ID, i, 2);
			*((uint16_t *) (Msg.D + 4)) = PackID(ID, i, 3);
			*((uint16_t *) (Msg.D + 6)) = PackID(ID, i, 4);
			*((uint16_t *) (Msg.D + 8)) = PackID(ID, i, 5);
			if(mPrm.owner().DoCmd(&Msg)) {
			    if(Msg.C == GOOD3) {
				rc = 1;
			    } else {
				rc = 0;
				break;
			    }
			} else {
			    rc = 0;
			    break;
			}

		    }
		} else {
		    rc = 1;
		}
	    }
	}
	if(rc) NeedInit = false;
	break;
    }
    return rc;
}

uint16_t B_ODOR::HandleEvent(int64_t tm, uint8_t * D)
{
    FT3ID ft3ID = UnpackID(TSYS::getUnalign16(D));
    if(ft3ID.g != ID) return 0;
    uint16_t l = 0;
    switch(ft3ID.k) {
    case 0:
	switch(ft3ID.n) {
	case 0:
	    mPrm.vlAt("state").at().setI(D[2], tm, true);
	    l = 3;
	    break;
	case 1:
	    mPrm.vlAt("state").at().setI(D[2], tm, true);
	    l = 3 + count_n * 5;
	    for(int j = 1; j <= count_n; j++) {
		mPrm.vlAt(TSYS::strMess("state_%d", j)).at().setI(D[(j - 1) * 5 + 3], tm, true);
		mPrm.vlAt(TSYS::strMess("value_%d", j)).at().setR(TSYS::getUnalignFloat(D + (j - 1) * 5 + 4), tm, true);
	    }
	    break;

	}
	break;
    default:
	if(ft3ID.k && (ft3ID.k <= count_n)) {
	    switch(ft3ID.n) {
	    case 0:
		mPrm.vlAt(TSYS::strMess("state_%d", ft3ID.k)).at().setI(D[2], tm, true);
		l = 3;
		break;
	    case 1:
		mPrm.vlAt(TSYS::strMess("state_%d", ft3ID.k)).at().setI(D[2], tm, true);
		mPrm.vlAt(TSYS::strMess("value_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 3), tm, true);
		l = 7;
		break;
	    case 2:
		mPrm.vlAt(TSYS::strMess("period_%d", ft3ID.k)).at().setI(D[3], tm, true);
		l = 4;
		break;
	    case 3:
		mPrm.vlAt(TSYS::strMess("sens_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 3), tm, true);
		l = 7;
		break;
	    case 4:
		mPrm.vlAt(TSYS::strMess("countP_%d", ft3ID.k)).at().setI(TSYS::getUnalign32(D + 3), tm, true);
		l = 7;
		break;
	    case 5:
		mPrm.vlAt(TSYS::strMess("vdrop_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 3), tm, true);
		l = 7;
		break;
	    }
	}
	break;
    }
    return l;
}

uint8_t B_ODOR::cmdGet(uint16_t prmID, uint8_t * out)
{
    FT3ID ft3ID = UnpackID(prmID);
    if(ft3ID.g != ID) return 0;
    uint l = 0;
    if(ft3ID.k == 0) {
	switch(ft3ID.n) {
	case 0:
	    //state
	    out[0] = 0 | blkID;
	    l = 1;
	    break;
	case 1:

	    out[0] = 0 | blkID;
	    l = 1;
	    //value
	    for(uint8_t i = 0; i < count_n; i++) {
		out[i * 5 + 1] = data[i].State.vl;
		for(uint8_t j = 0; j < 4; j++)
		    out[i * 5 + 2 + j] = data[i].Value.b_vl[j];
		l += 5;
	    }
	    break;
	}
    } else {
	if(ft3ID.k <= count_n) {
	    switch(ft3ID.n) {
	    case 0:
		out[0] = data[ft3ID.k - 1].State.vl;
		l = 1;
		break;
	    case 1:
		out[0] = data[ft3ID.k - 1].State.vl;
		for(uint8_t j = 0; j < 4; j++)
		    out[1 + j] = data[ft3ID.k - 1].Value.b_vl[j];
		l = 5;
		break;
	    case 2:
		out[0] = data[ft3ID.k - 1].Period.s;
		out[1] = data[ft3ID.k - 1].Period.vl;
		l = 2;
		break;
	    case 3:
		out[0] = data[ft3ID.k - 1].Sens.s;
		for(uint8_t j = 0; j < 4; j++)
		    out[1 + j] = data[ft3ID.k - 1].Sens.b_vl[j];
		l = 5;
		break;
	    case 4:
		out[0] = data[ft3ID.k - 1].Count.s;
		for(uint8_t j = 0; j < 4; j++)
		    out[1 + j] = data[ft3ID.k - 1].Count.b_vl[j];
		l = 5;
		break;
	    case 5:
		out[0] = data[ft3ID.k - 1].Vdrop.s;
		for(uint8_t j = 0; j < 4; j++) {
		    out[1 + j] = data[ft3ID.k - 1].Vdrop.b_vl[j];
		}
		l = 5;
		break;

	    }

	}
    }
    return l;
}

uint8_t B_ODOR::cmdSet(uint8_t * req, uint8_t addr)
{
    uint16_t prmID = TSYS::getUnalign16(req);
    FT3ID ft3ID = UnpackID(prmID);
    if(ft3ID.g != ID) return 0;
    uint l = 0;
    if((ft3ID.k > 0) && (ft3ID.k <= count_n)) {
	switch(ft3ID.n) {
	case 2:
	    l = SetNew8Val(data[ft3ID.k - 1].Period, addr, prmID, req[2]);
	    break;
	case 3:
	    l = SetNewflVal(data[ft3ID.k - 1].Sens, addr, prmID, TSYS::getUnalignFloat(req + 2));
	    break;
	case 4:
	    l = SetNew32Val(data[ft3ID.k - 1].Count, addr, prmID, TSYS::getUnalign32(req + 2));
	    break;
	case 5:
	    l = SetNewflVal(data[ft3ID.k - 1].Vdrop, addr, prmID, TSYS::getUnalignFloat(req + 2));
	    break;
	}
    }
    return l;
}

uint16_t B_ODOR::setVal(TVal &val)
{
    int off = 0;
    FT3ID ft3ID;
    ft3ID.k = s2i(TSYS::strParse(val.fld().reserve(), 0, ":", &off));
    ft3ID.n = s2i(TSYS::strParse(val.fld().reserve(), 0, ":", &off));
    ft3ID.g = ID;

    tagMsg Msg;
    Msg.L = 0;
    Msg.C = SetData;
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ft3ID));
    switch(ft3ID.n) {
    case 2:
	Msg.L += SerializeB(Msg.D + Msg.L, val.getI(0, true));
	break;
    case 3:
    case 5:
	Msg.L += SerializeF(Msg.D + Msg.L, val.getR(0, true));
	break;
    case 4:
	Msg.L += SerializeUi32(Msg.D + Msg.L, val.getI(0, true));
	break;
    }
    if(Msg.L > 2) {
	Msg.L += 3;
	mPrm.owner().DoCmd(&Msg);
    }
    return 0;
}

//---------------------------------------------------------------------------
