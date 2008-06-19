
//OpenSCADA system module UI.WebVision file: vca_sess.h
/***************************************************************************
 *   Copyright (C) 2007 by Roman Savochenko                                *
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

#ifndef VCA_SESS_H
#define VCA_SESS_H

#include <gd.h>
#include <gdfonts.h>
#include <gdfontt.h>


#include <string>
#include <vector>
#include <map>
#include <math.h>

#include <tcntrnode.h>


class Point
{
    public:
	Point( ) {};
	Point( double ix, double iy) : x(ix), y(iy)	{ };
	double x; 
	double y;
};

typedef map<int,Point> PntMap;

using std::string;
using std::vector;
using std::map;

namespace WebVision
{

class SSess;

//*************************************************
//* VCAObj                                        *
//*************************************************
class VCASess;

class VCAObj : public TCntrNode
{
    public:
	//Methods
	VCAObj( const string &iid );

	const string &id( )		{ return m_id; }

	virtual void getReq( SSess &ses ) = 0;
	virtual void postReq( SSess &ses ) = 0;
	virtual void setAttrs( XMLNode &node, const string &user ) = 0;

	VCASess &owner( );

    private:
	//Methods
	string nodeName( )		{ return m_id; }

	//Attributes
	string	m_id;
};

//*************************************************
//* ElFigure                                      *
//*************************************************
class ShapeItem 
{
    public:
	ShapeItem( )	{ }
	ShapeItem(const int num_1,const int num_2, const int num_3, const int num_4,const int num_5,
		  const Point &ctrlpos_4, const double iang, const int color, const int bcolor, const int iwidth,
		  const int bwidth, const int itype, const int istyle, const bool iflag_brd ) : 
	    ctrlPos4(ctrlpos_4), ang(iang), n1(num_1), n2(num_2), n3(num_3), n4(num_4), n5(num_5), lineColor(color),borderColor(bcolor),
	    width(iwidth), border_width(bwidth), type(itype), style(istyle), flag_brd(iflag_brd)
        { }

	Point		ctrlPos4;
	double		ang;
	int 		n1, n2, n3, n4, n5, 
			style,
			width,
			border_width,
			lineColor,
			borderColor,
			type;
	bool		flag_brd;
};

class InundationItem
{
    public:
	InundationItem( )	{ }
	InundationItem( vector<int> inumber_point, int color ) : number_point(inumber_point), P_color(color)
	{ }

	vector<int>	number_point;
	int		P_color;

};

class VCAElFigure : public VCAObj
{
    public:
	//Methods
	VCAElFigure( const string &iid );

	void getReq( SSess &ses );
	void postReq( SSess &ses );
	void setAttrs( XMLNode &node, const string &user );

	bool lineIntersect( double x1, double y1, double x2, double y2,
			    double x3, double y3, double x4, double y4 );
	//Point line_intersect_point( Point pt1, Point pt2, Point pt3, Point pt4 );
	double angle( const Point p1, const Point p2, const Point p3, const Point p4 );
	double length( const Point pt1, const Point pt2 );
	Point arc( double t, double a, double b );
	Point unrotate(const Point pnt, double alpha, double a, double b);
	Point rotate( const Point pnt, double alpha );
	Point bezier( double t, Point p1, Point p2, Point p3, Point p4 );
	double bezierDeltaT( Point p1, Point p2, Point p3, Point p4 );
	double ABS(double var);
	bool isPaintable( ShapeItem item, double xScale, double yScale );
	void paintFigure( gdImagePtr im, ShapeItem item, double xScale, double yScale, bool flag_allocate, bool flag_style, bool flag_clr_ln );
	void paintFigureBorders( gdImagePtr im, Point el_p1, Point el_p2, Point el_p3, Point el_p4, Point el_p5, Point el_p6, int  clr_el, int clr_el_line, double el_width, double el_border_width, int type );
	void dashDot( gdImagePtr im, Point el_p1, Point el_p2, Point el_p3, Point el_p4, Point el_p5, Point el_p6, int  clr_el, double el_width, int type, int style  );
	void dashDotFigureBorders( gdImagePtr im, Point el_p1, Point el_p2, Point el_p3, Point el_p4, Point el_p5, Point el_p6, int  clr_el, int clr_el_line, double el_width, double el_border_width, int type, double wdt, double wdt_1  );
	void paintFill( gdImagePtr im, Point pnt, InundationItem in_item, int color );
	Point unscaleUnrotate( Point point, double xScale, double yScale, bool flag_scale );
	Point scaleRotate( Point point, double xScale, double yScale, bool flag_scale );
	//Attributes
	double	width,		//Widget geometry
		height;
	int	geomMargin,	//Margin
		lineClr,
		lineDecor,
		bordWdth,
		bordClr,
		lineWdth,
		fillClr;
	string	elLst, lineStyle;
	double	orient;
	bool	active,		//Active diagram
		rel_list;

	PntMap pnts;
	vector<ShapeItem> shapeItems;
	vector<InundationItem> inundationItems;

	Res	mRes;
};


//*************************************************
//* VCADiagram                                    *
//*************************************************
class VCADiagram : public VCAObj
{
    public:
	//Methods
	VCADiagram( const string &iid );

	void getReq( SSess &ses );
	void postReq( SSess &ses );
	void setAttrs( XMLNode &node, const string &user );

	//Attributes
	int		width, height,		//Widget geometry
			geomMargin,		//Margin
			bordWidth;		//Border width
	bool		active,			//Active diagram
			tTimeCurent;		//Curent time
	long long	tTime, curTime;		//Trend time and trend cursor's time position
	int		trcPer;			//Tracing period
	float		tSize;			//Trend size (s)
	int		curColor, 		//Cursor line color
			sclColor,		//Scale grid color
			sclMarkColor;		//Scale markers color
	string		sclMarkFont,		//Scale markers font
			valArch;		//Value archivator
	int		sclHor, sclVer;		//Horisontal and Vertical scale mode 
	time_t		lstTrc;			//Last trace

    private:
	//Data
	//- Trend object's class -
	class TrendObj
	{
	    public:
		//Data
		class SHg
		{
		    public:
			SHg( long long itm, double ival ) : tm(itm), val(ival) { }
			long long tm;
			double val;
		};

		//Methods
		TrendObj( VCADiagram *owner );

		string addr( )		{ return m_addr; }
		double bordL( )		{ return m_bord_low; }
		double bordU( )		{ return m_bord_up; }
		int    color( )		{ return m_color; }
		double curVal( )	{ return m_curvl; }
		int    valTp( )		{ return val_tp; }
		long long valBeg( );
		long long valEnd( );
		int val( long long tm );
		deque<SHg> &val( )	{ return vals; }

		void setAddr( const string &vl );
		void setBordL( double vl )	{ m_bord_low = vl; }
		void setBordU( double vl )	{ m_bord_up  = vl; }
		void setColor( int vl )		{ m_color = vl; }
		void setCurVal( double vl )	{ m_curvl = vl; }

		void loadData( const string &user, bool full = false );

		VCADiagram &owner( );

	    private:
		//Attributes
		string		m_addr;		//A parameter or an archive item address
		double m_bord_low, m_bord_up;	//Borders
		double		m_curvl;	//Curent value
		int		m_color;	//Values line color
		//- Archive -
		int		arh_per;	//Archive period
		long long	arh_beg;	//Archive begin time
		long long	arh_end;	//Archive end time
		//- Values -
		int		val_tp;		//Values type
		deque<SHg>	vals;		//Values buffer

		VCADiagram	*m_owner;	//Owner object
	};

	//Methods
	void setCursor( long long tTime, const string& user );

	//Attributes
	vector<TrendObj> trnds;			//Trends container

	Res	mRes;
};

//*************************************************
//* VCASess                                       *
//*************************************************
class VCASess : public TCntrNode
{
    public:
	//Methods
	VCASess( const string &iid, bool isCreate );

	const string &id( )		{ return m_id; }
	time_t lstReq( )		{ return lst_ses_req; }

	void getReq( SSess &ses );
	void postReq( SSess &ses );

	//- Objects -
	void objList( vector<string> &list )		{ chldList(id_objs,list); }
	bool objPresent( const string &name )		{ return chldPresent(id_objs,name); }
	void objAdd( VCAObj *obj );
	void objDel( const string &name )		{ chldDel(id_objs,name); }
	AutoHD<VCAObj> objAt( const string &name )	{ return chldAt(id_objs,name); }

    private:
	//Methods
	string nodeName( )		{ return m_id; }
	void postDisable( int flag );

	//Attributes
	string	m_id;
	int	id_objs;		//Primitive object's container identifier
	time_t	lst_ses_req;
	bool	mIsCreate;
};

}
#endif //VCA_SESS_H
