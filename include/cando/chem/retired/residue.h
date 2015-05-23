      
       
//
// (C) 2004 Christian E. Schafmeister
//


/*
 *	residue.h
 *
 *	Maintain a Residue object, a collection of bonded atoms.
 *	Residues are contained within Molecule objects and can be
 *	bonded together.
 *
 *	To facilitate rapid lookup of Atoms within the Residue,
 *	use the getHandleForAtomNamed and atomFromHandle methods.
 *
 *		getHandleForAtomNamed returns a unique AtomHandle object that
 *			can later be used to quickly lookup the Atom
 *			that cooresponds to the handle using the
 *			atomFromHandle method.
 *
 *		atomFromHandle returns a pointer to the Atom that
 *			cooresponds to the AtomHandle object that
 *			is passed to it.
 *
 */

#ifndef	RESIDUE_H
#define	RESIDUE_H
#include <iostream>
#include <string>
#include <set>
#include <clasp/core/common.h>
#include <clasp/core/symbolSet.fwd.h>
#include <clasp/core/lispStream.fwd.h>
#include <clasp/core/stringSet.fwd.h>
#include <cando/chem/matter.h>
#include <cando/chem/residue.fwd.h>
#include <cando/chem/candoDatabaseReference.h>
#include <cando/chem/atomIdMap.fwd.h>
#include <cando/chem/atom.h>

#include <cando/chem/chemPackage.h>


namespace chem {


    class	Loop;

    SMART(CalculatePosition);
    SMART(Restraint);
    SMART(Alias);
    SMART(RestraintList);
    SMART(Constitution);
    SMART(Residue);

    SMART(Residue );
    class Residue_O : public Matter_O
    {
        friend class Constitution_O;
        friend class Molecule_O;
        LISP_BASE1(Matter_O);
        LISP_CLASS(chem,ChemPkg,Residue_O,"Residue");
//    DECLARE_SERIALIZE();
    public:

	void initialize();
    public:
	void	archiveBase(core::ArchiveP node);
	
	friend	class	Loop;
    private:
	bool		_Selected;
	int		_NetCharge;
	int		tempInt;
        MatterName		pdbName;
	uint		_FileSequenceNumber;
//	CandoDatabaseReference_sp	_Constitution;
        /*! Stores a unique label that is set when
         * a residue is created from a monomer.
         * This is used to match residues between different
         * Molecule objects that contain the exact same
         * molecule.
         * This allows Molecules that were built from
         * Oligomers to be saved to a file and then
         * loaded back and have their atoms matched
         * up with atoms of an identical Molecule that
         * was loaded from somewhere else.
         * This also allows Molecules that are sent between
         * processes to have their coordinates put into
         * ConformationExplorers and have Scorers applied to
         * them.
         * A MoleculeMatcher takes two Molecules and matches
         * their atoms 1 to 1 and allows coordinates to
         * be copied from either molecule to the other.
         */
        MatterName		_UniqueLabel;
	core::SymbolSet_sp	_MonomerAliases;
    public:
        static Residue_sp make(MatterName name);
    private:
	void	duplicateFrom(const Residue_O* r );
    public:

        virtual	char	getClass()	{return residueId; };
	char	getMatterType() { return RESIDUE_CLASS; };

        virtual string __repr__() const;
        virtual bool equal(core::T_sp obj) const;
        virtual void	transferCoordinates(Matter_sp other);


	contentIterator begin_atoms() { return this->_contents.begin(); };
	contentIterator end_atoms() {return this->_contents.end(); };

	virtual bool isResidue() { return true;};


	core::SymbolSet_sp getMonomerAliases();
	bool recognizesMonomerAlias(core::Symbol_sp s);
	void setMonomerAliases(core::SymbolSet_sp s);
	void addMonomerAlias(core::Symbol_sp s);

	/*! Return true if this residue contains the atom a
	 */
	bool	containsAtom(Atom_sp a);

	MatterName getPdbName() { return this->pdbName; };
	void	setPdbName(MatterName p) { this->pdbName = p;};

	void	setFileSequenceNumber(uint seq) { this->_FileSequenceNumber = seq;};
	uint	getFileSequenceNumber() { return this->_FileSequenceNumber;};

	void	setUniqueLabel(MatterName str) { this->_UniqueLabel = str;};
	MatterName getUniqueLabel() { return this->_UniqueLabel; };

	int	getNetCharge() { return this->_NetCharge; };
	void	setNetCharge(int nc) { this->_NetCharge = nc; };

//	void	resetConstitution();
	bool	hasConstitution();
	Constitution_sp getConstitution();
	void	setConstitution(Constitution_sp mm);

//	friend	Residue_sp	Residue();

	int		getTempInt() {return this->tempInt;};
	void		setTempInt(int j) {this->tempInt = j;};
        core::SymbolSet_sp	getAtomNamesAsSymbolSet();

	void		addAtom(Atom_sp a );
	void		addAtomRetainId(Atom_sp a ) {this->addMatterRetainId(Matter_sp(a));};
	void		removeAtomsWithNames( core::Cons_sp atomNames );
	void		removeAtomDeleteBonds( Atom_sp a );
	void		removeAtomDontDeleteBonds( Atom_sp a );


	int	totalNetResidueCharge() { return this->_NetCharge; };

        /*! Add a virtual atom and calculate its position using
         * the provided procedure
         */
	void	addVirtualAtom(MatterName virtualAtomName, 
                               CalculatePosition_sp proc);

        VectorAtom	getAtoms();
//	gctools::Vec0<Bond_sp>	g_etUniqueIntraResidueBonds();
        BondList_sp	getOutGoingBonds();

        virtual string	subMatter() { return Atom_O::static_className(); };
        virtual string 	description() const { stringstream ss; ss << "residue("<<this->getName()<<")@"<<std::hex<<this<<std::dec; return ss.str();};

	Atom_sp		atomWithName(MatterName sName ) { return this->contentWithName(sName).as<Atom_O>(); };
	Atom_sp		atomWithAlias(AtomAliasName sName );
	Atom_sp		atomWithAliasOrNil(AtomAliasName sName );


	Atom_sp	aliasAtomOrNil(Alias_sp alias) {IMPLEMENT_ME();};
	Residue_sp aliasResidueOrNil(Alias_sp alias) {IMPLEMENT_ME();};

	Vector3		positionOfAtomWithName(MatterName sName );
	bool		hasAtomWithName(MatterName sName ) { return (this->hasContentWithName(sName)); };

	void setAliasesForAtoms(core::Cons_sp aliasAtoms, core::Cons_sp atomAliases);

//	bool		hasAtomWithAlias(const string& anAlias);
//	Atom_sp		atomWithAlias(const string& anAlias);

        Atom_sp		atomWithId( int lid ) { return this->contentWithId(lid).as<Atom_O>(); };
	bool		hasAtomWithId( int lid ) { return this->hasContentWithId(lid); };
        core::SymbolSet_sp 	getAllUniqueAtomNames();

        /*! Return true if all the atom names are unique
         * If not describe the atoms with overlapping names in (problems).
         */
	bool		testIfAllAtomNamesAreUnique(core::T_sp problemStream);

	void fillInImplicitHydrogensOnCarbon();

	Atom_sp		firstAtom() { return downcast<Atom_O>(this->contentAt(0)); };

	void	makeAllAtomNamesInEachResidueUnique();

        void		writeToStream(string prefix, std::ostream& out);
	bool		testResidueConsistancy()	{return this->testConsistancy(Matter_sp());};

	//! For every atom, turn on the FIXED flag and copy the atom coordinates into the AnchorCoordinates
	void		useAtomCoordinatesToDefineAnchors();


	bool		isSelected() {return this->_Selected;};
	void		setSelected(bool s) {this->_Selected = s;};

	bool		invalid();

	void		failIfInvalid();

	/*! Build a map of AtomIds to Atoms */
	virtual AtomIdToAtomMap_sp buildAtomIdMap() const;

        virtual Atom_sp atomWithAtomId(AtomId_sp atomId) const;


	virtual uint	numberOfAtoms( );

	Residue_O( const Residue_O& res );


    public:
	virtual Matter_sp	copy();
    protected:
	virtual Matter_sp copyDontRedirectAtoms();
	virtual void redirectAtoms();


        DEFAULT_CTOR_DTOR(Residue_O);
    };




};
TRANSLATE(chem::Residue_O);
#endif
