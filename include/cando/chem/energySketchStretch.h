/*
    File: energySketchStretch.h
*/
/*
Open Source License
Copyright (c) 2016, Christian E. Schafmeister
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 
This is an open source license for the CANDO software from Temple University, but it is not the only one. Contact Temple University at mailto:techtransfer@temple.edu if you would like a different license.
*/
/* -^- */
       
       

//
// (C) 2004 Christian E. Schafmeister
//


/*
 *	energySketchStretch.h
 *
 *	Maintains a database of stretch types
 */

#ifndef EnergySketchStretch1_H  //[
#define	EnergySketchStretch1_H
#include <stdio.h>
#include <string>
#include <vector>
#include <set>
#include <clasp/core/common.h>
#include <cando/geom/vector3.h>
#include <cando/chem/energyComponent.h>
#include <cando/chem/energySketchStretch.fwd.h>

namespace       chem {

struct TermSketchStretch {
  REAL	kb;	//!< SketchStretch force constant, this must match Mathematica code!
  REAL	r0;	//!< SketchStretch equilibrium distance, this must match Mathematica code!
  INT	I1;	//!< i*3 index into coordinate vector for atom1, this must match Mathematica code!
  INT	I2;	//!< i*3 index into coordinate vector for atom2, this must match Mathematica code!
  TermSketchStretch(REAL k,REAL r, INT i1, INT i2) : kb(k), r0(r),I1(i1),I2(i2) {};
  TermSketchStretch() {};
};


/*! Store a SketchStretch energy term
 */
class EnergySketchStretch : public EnergyTerm {
public:
  string	className()	{ return "EnergySketchStretch"; };
public:
  TermSketchStretch	term;
  double	getR0()	{return this->term.r0;};
  double	getR();

public:
  EnergySketchStretch(size_t i1, size_t i2, double kb, double r0) : term(kb,r0,i1,i2) {};
  EnergySketchStretch() {};
  
public:
		//! Initialize self from a FFSketchStretch parameter and two EnergyAtom(s)
  core::List_sp encode() const;
  void decode(core::List_sp alist);
};
};


namespace translate {

template <>
struct	to_object<chem::EnergySketchStretch >
{
  typedef	core::Cons_sp ExpectedType;
  typedef	core::Cons_sp DeclareType;
  static core::T_sp convert(const chem::EnergySketchStretch& stretch)
  {
    return stretch.encode();
  }
};

template <>
struct	from_object<chem::EnergySketchStretch>
{
  typedef	chem::EnergySketchStretch	ExpectedType;
  typedef	ExpectedType 		DeclareType;
	DeclareType _v;
	from_object(core::T_sp o)
	{
          SIMPLE_ERROR(BF("Implement me"));
        }
};
};



namespace chem {

double	_evaluateEnergyOnly_SketchStretch (
		double x1,
		double y1,
		double z1,
		double x2,
		double y2,
		double z2,
		double r0,
		double kb );

class EnergySketchStretch_O : public EnergyComponent_O
{
    LISP_CLASS(chem,ChemPkg,EnergySketchStretch_O,"EnergySketchStretch",EnergyComponent_O);
public:
    bool fieldsp() const { return true; };
    void fields(core::Record_sp node);
public: // virtual functions inherited from Object
    void	initialize();
public:
    typedef EnergySketchStretch	TermType;
public: // instance variables
    gctools::Vec0<TermType>	_Terms;
    gctools::Vec0<TermType>	_BeyondThresholdTerms;
public:	
    typedef gctools::Vec0<TermType>::iterator iterator;
    iterator begin() { return this->_Terms.begin(); };
    iterator end() { return this->_Terms.end(); };
	
public:
    virtual size_t numberOfTerms() { return this->_Terms.size();};
    void addTerm(const TermType& term);

    CL_DEFMETHOD core::T_mv safe_amber_energy_stretch_term(size_t index) {
      if (index >= this->numberOfTerms() ) {
        SIMPLE_ERROR(BF("Illegal term index %zu must be less than %zu") % index % this->_Terms.size() );
      }
      return Values(core::DoubleFloat_O::create(this->_Terms[index].term.kb),
                    core::DoubleFloat_O::create(this->_Terms[index].term.r0),
                    core::make_fixnum(this->_Terms[index].term.I1),
                    core::make_fixnum(this->_Terms[index].term.I2));
    }

    /*! Extract the components of the energy term into a collection of parallel vectors.
        The vectors are returned as an alist with keyword keys.
        The keys are :kb, :r0, :i1, :i2, :atom1 :atom2 */
    virtual core::List_sp extract_vectors_as_alist() const;
    virtual void fill_from_vectors_in_alist(core::List_sp vectors);
    
    virtual void setupHessianPreconditioner(NVector_sp nvPosition,
					    AbstractLargeSquareMatrix_sp m );
    virtual double evaluateAll( NVector_sp 	pos,
                                bool 		calcForce,
                                gc::Nilable<NVector_sp> 	force,
                                bool		calcDiagonalHessian,
                                bool		calcOffDiagonalHessian,
                                gc::Nilable<AbstractLargeSquareMatrix_sp>	hessian,
                                gc::Nilable<NVector_sp>	hdvec,
                                gc::Nilable<NVector_sp> dvec);

  void addSketchStretchTerm(size_t i1, size_t i2, double kb, double r0);
  
  void walkSketchStretchTerms(core::T_sp callback);
  void modifySketchStretchTermKb(size_t index, float kb);
  void modifySketchStretchTermR0(size_t index, float r0);
    
  void reset();
public:
    EnergySketchStretch_O( const EnergySketchStretch_O& ss ); //!< Copy constructor

    DEFAULT_CTOR_DTOR(EnergySketchStretch_O);
};

};

#endif //]
