
//OpenSCADA system module Special.FLibMath file: stdmath.h
/***************************************************************************
 *   Copyright (C) 2005-2007 by Roman Savochenko                           *
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

#ifndef STDMATH_H
#define STDMATH_H

#include <math.h>

#include <string>
#include <vector>

#include <tfunction.h>

#include "statfunc.h"

using std::string;
using std::vector;

namespace FLibMath
{

//*************************************************
//* Standart mathematic functions:                *

//*************************************************
//* Arc cosine                                    *
//* Formula: y=acos(x)                            *
//*************************************************
class MathAcos : public TFunction
{
    public:
	MathAcos( ) : TFunction("acos")
	{
	    ioAdd( new IO("y",_("Y(rad.)"),IO::Real,IO::Return,"0") );
	    ioAdd( new IO("x",_("X"),IO::Real,IO::Default,"0") );
	}
	
	string name( )	{ return _("ArcCos"); }
	string descr( )	{ return _("Arc cosine math function."); }     

	void calc( TValFunc *val )
	{
	    val->setR(0,acos(val->getR(1)));
	}
};

//*************************************************
//* Arc sine                                      *
//* Formula: y=asin(x)                            *
//*************************************************
class MathAsin : public TFunction
{
    public:
	MathAsin( ) : TFunction("asin")
	{
	    ioAdd( new IO("y",_("Y(rad.)"),IO::Real,IO::Return,"0") );
	    ioAdd( new IO("x",_("X"),IO::Real,IO::Default,"0") );
	}
	
	string name( )	{ return _("ArcSin"); }
	string descr( )	{ return _("Arc sine math function."); }     

	void calc( TValFunc *val )
	{
	    val->setR(0,asin(val->getR(1)));
	}
};

//*************************************************
//* Arc tangent                                   *
//* Formula: y=atan(x)                            *
//*************************************************
class MathAtan : public TFunction
{
    public:
	MathAtan( ) : TFunction("atan")
	{
	    ioAdd( new IO("y",_("Y(rad.)"),IO::Real,IO::Return,"0") );
	    ioAdd( new IO("x",_("X"),IO::Real,IO::Default,"0") );
	}
	
	string name( )	{ return _("ArcTan"); }
	string descr( )	{ return _("Arc tangent math function."); }     

	void calc( TValFunc *val )
	{
	    val->setR(0,atan(val->getR(1)));
	}
};

//*************************************************
//* Cosine                                        *
//* Formula: y=cos(x)                             *
//*************************************************
class MathCos : public TFunction
{
    public:
	MathCos( ) : TFunction("cos")
	{
	    ioAdd( new IO("y",_("Y"),IO::Real,IO::Return,"0") );
	    ioAdd( new IO("x",_("X(rad.)"),IO::Real,IO::Default,"0") );
	}
	
	string name( )	{ return _("Cosine"); }
	string descr( )	{ return _("Cosine math function."); }     

	void calc( TValFunc *val )
	{
	    val->setR(0,cos(val->getR(1)));
	}
};

//*************************************************
//* Cosine hyperbolical                           *
//* Formula: y=cosh(x)                            *
//*************************************************
class MathCosh : public TFunction
{
    public:
	MathCosh( ) : TFunction("cosh")
	{
	    ioAdd( new IO("y",_("Y"),IO::Real,IO::Return,"0") );
	    ioAdd( new IO("x",_("X"),IO::Real,IO::Default,"0") );
	}
	
	string name( )	{ return _("Cosine hyperbolical"); }
	string descr( )	{ return _("Cosine hyperbolical math function."); }     

	void calc( TValFunc *val )
	{
	    val->setR(0,cosh(val->getR(1)));
	}
};

//*************************************************
//* Exponent                                      *
//* Formula: y=exp(x)                             *
//*************************************************
class MathExp : public TFunction
{
    public:
	MathExp( ) : TFunction("exp")
	{
	    ioAdd( new IO("y",_("Y"),IO::Real,IO::Return,"0") );
	    ioAdd( new IO("x",_("X"),IO::Real,IO::Default,"0") );
	}
	
	string name( )	{ return _("Exponent"); }
	string descr( )	{ return _("Exponent math function."); }     
    
	void calc( TValFunc *val )
	{
	    val->setR(0,exp(val->getR(1)));
	}
};

//*************************************************
//* Natural logarithm                             *
//* Formula: y=ln(x)                              *
//*************************************************
class MathLn : public TFunction
{
    public:
	MathLn( ) : TFunction("ln")
	{
	    ioAdd( new IO("y",_("Y"),IO::Real,IO::Return,"0") );
	    ioAdd( new IO("x",_("X"),IO::Real,IO::Default,"0") );
	}
	
	string name( )	{ return _("Natural logarithm"); }
	string descr( )	{ return _("Natural logarithm math function."); }     

	void calc( TValFunc *val )
	{
	    val->setR(0,log(val->getR(1)));
	}
};

//*************************************************
//* Decimal logarithm                             *
//* Formula: y=lg(x)                              *
//*************************************************
class MathLg : public TFunction
{
    public:
	MathLg( ) : TFunction("lg")
	{
	    ioAdd( new IO("y",_("Y"),IO::Real,IO::Return,"0") );
	    ioAdd( new IO("x",_("X"),IO::Real,IO::Default,"0") );
	}
	
	string name( )	{ return _("Decimal logarithm"); }
	string descr( )	{ return _("Decimal logarithm math function."); }     

	void calc( TValFunc *val )
	{
	    val->setR(0,log10(val->getR(1)));
	}
};

//*************************************************
//* Power                                         *
//* Formula: y=pow(x,p)                           *
//*************************************************
class MathPow : public TFunction
{
    public:
	MathPow( ) : TFunction("pow")
	{
	    ioAdd( new IO("y",_("Y"),IO::Real,IO::Return,"0") );
	    ioAdd( new IO("x",_("X"),IO::Real,IO::Default,"0") );
	    ioAdd( new IO("p",_("Power"),IO::Real,IO::Default,"0") );
	}
	
	string name( )	{ return _("Power"); }
	string descr( )	{ return _("Power math function."); }     

	void calc( TValFunc *val )
	{
	    val->setR(0,pow(val->getR(1),val->getR(2)));
	}
};

//*************************************************
//* Sine                                          *
//* Formula: y=sin(x)                             *
//*************************************************
class MathSin : public TFunction
{
    public:
	MathSin( ) : TFunction("sin")
	{
	    ioAdd( new IO("y",_("Y"),IO::Real,IO::Return,"0") );
	    ioAdd( new IO("x",_("X(rad.)"),IO::Real,IO::Default,"0") );
	}
	
	string name( )	{ return _("Sine"); }
	string descr( )	{ return _("Sine math function."); }     

	void calc( TValFunc *val )
	{
	    val->setR(0,sin(val->getR(1)));
	}
};

//*************************************************
//* Sine hyperbolical                             *
//* Formula: y=sinh(x)                            *
//*************************************************
class MathSinh : public TFunction
{
    public:
	MathSinh( ) : TFunction("sinh")
	{
	    ioAdd( new IO("y",_("Y"),IO::Real,IO::Return,"0") );
	    ioAdd( new IO("x",_("X"),IO::Real,IO::Default,"0") );
	}
	
	string name( )	{ return _("Sine hyperbolical"); }
	string descr( )	{ return _("Sine hyperbolical math function."); }     

	void calc( TValFunc *val )
	{
	    val->setR(0,sinh(val->getR(1)));
	}
};

//*************************************************
//* Square root                                   *
//* Formula: y=sqrt(x)                            *
//*************************************************
class MathSqrt : public TFunction
{
    public:
	MathSqrt( ) : TFunction("sqrt")
	{
	    ioAdd( new IO("y",_("Y"),IO::Real,IO::Return,"0") );
	    ioAdd( new IO("x",_("X"),IO::Real,IO::Default,"0") );
	}
	
	string name( )	{ return _("Square root"); }
	string descr( )	{ return _("Square root math function."); }     

	void calc( TValFunc *val )
	{
	    val->setR(0,sqrt(val->getR(1)));
	}
};

//*************************************************
//* Tangent                                       *
//* Formula: y=tan(x)                             *
//*************************************************
class MathTan : public TFunction
{
    public:
	MathTan( ) : TFunction("tan")
	{
	    ioAdd( new IO("y",_("Y"),IO::Real,IO::Return,"0") );
	    ioAdd( new IO("x",_("X(rad.)"),IO::Real,IO::Default,"0") );
	}
	
	string name( )	{ return _("Tangent"); }
	string descr( )	{ return _("Tangent math function."); }     

	void calc( TValFunc *val )
	{
	    val->setR(0,tan(val->getR(1)));
	}
};

//*************************************************
//* Tangent hyperbolical                          *
//* Formula: y=tanh(x)                            *
//*************************************************
class MathTanh : public TFunction
{
    public:
	MathTanh( ) : TFunction("tanh")
	{
	    ioAdd( new IO("y",_("Y"),IO::Real,IO::Return,"0") );
	    ioAdd( new IO("x",_("X"),IO::Real,IO::Default,"0") );
	}
	
	string name( )	{ return _("Tangent hyperbolical"); }
	string descr( )	{ return _("Tangent hyperbolical math function."); }     

	void calc( TValFunc *val )
	{
	    val->setR(0,tanh(val->getR(1)));
	}
};

//*************************************************
//* Ceil                                          *
//* Formula: y=ceil(x)                            *
//*************************************************
class MathCeil : public TFunction
{
    public:
	MathCeil( ) : TFunction("ceil")
	{
	    ioAdd( new IO("y",_("Y"),IO::Real,IO::Return,"0") );
	    ioAdd( new IO("x",_("X"),IO::Real,IO::Default,"0") );
	}
	
	string name( )	{ return _("Ceil"); }
	string descr( )	{ return _("Ceil math function."); }     

	void calc( TValFunc *val )
	{
	    val->setR(0,ceil(val->getR(1)));
	}
};

//*************************************************
//* Absolute                                      *
//* Formula: y=abs(x)                             *
//*************************************************
class MathAbs : public TFunction
{
    public:
	MathAbs( ) : TFunction("abs")
	{
	    ioAdd( new IO("y",_("Y"),IO::Real,IO::Return,"0") );
	    ioAdd( new IO("x",_("X"),IO::Real,IO::Default,"0") );
	}
	
	string name( )	{ return _("Absolute"); }
	string descr( )	{ return _("Absolute math function."); }     

	void calc( TValFunc *val )
	{
	    val->setR(0,fabs(val->getR(1)));
	}
};

//*************************************************
//* Floor                                         *
//* Formula: y=floor(x)                           *
//*************************************************
class MathFloor : public TFunction
{
    public:
	MathFloor( ) : TFunction("floor")
	{
	    ioAdd( new IO("y",_("Y"),IO::Real,IO::Return,"0") );
	    ioAdd( new IO("x",_("X"),IO::Real,IO::Default,"0") );
	}
	
	string name( )	{ return _("Floor"); }
	string descr( )	{ return _("Floor math function."); }     

	void calc( TValFunc *val )
	{
	    val->setR(0,floor(val->getR(1)));
	}
};

//*************************************************
//* Randomize (0..100)                            *
//* Formula: y=rand(x)                            *
//*************************************************
class MathRand : public TFunction
{
    public:
	MathRand( ) : TFunction("rand")
	{
	    ioAdd( new IO("y",_("Y"),IO::Real,IO::Return,"0") );
	}
	
	string name( )	{ return _("Rand"); }
	string descr( )	{ return _("Rand math function."); }     

	void calc( TValFunc *val )
	{
	    val->setR(0,100.*rand()/RAND_MAX);
	}
};

//*************************************************
//* Condition check                               *
//* Formula: y=if(cond,true,false)                *
//*************************************************
class MathIf : public TFunction
{
    public:
	MathIf( ) : TFunction("if")
	{
	    ioAdd( new IO("y",_("Y"),IO::Real,IO::Return,"0") );
	    ioAdd( new IO("cond",_("Condition"),IO::Real,IO::Default,"1") );
	    ioAdd( new IO("true",_("True value"),IO::Real,IO::Default,"0") );
	    ioAdd( new IO("false",_("False value"),IO::Real,IO::Default,"0") );
	}
	
	string name( )	{ return _("IF condition"); }
	string descr( )	{ return _("Function's type 'if'-condition."); }     

	void calc( TValFunc *val )
	{
	    val->setR(0,val->getR(1)?val->getR(2):val->getR(3));
	}
};

}

#endif //STDMATH_H

