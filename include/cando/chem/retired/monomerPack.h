       
       
#ifndef	MonomerPack_H
#define MonomerPack_H




#include <clasp/core/foundation.h>
#include <cando/chem/entityNameSet.h>
#include <cando/chem/atomIndexer.h>


#include <cando/chem/chemPackage.h>

#include "core/cons.fwd.h"// monomerPack.h wants Cons needs cons.fwd.h
#include "core/symbolList.fwd.h"// monomerPack.h wants SymbolList needs symbolList.fwd.h

namespace chem {

SMART(MapOfMonomerNamesToAtomIndexers);
SMART(Cons);
SMART(SymbolList);

SMART(MonomerPack);
class MonomerPack_O : public EntityNameSet_O
{
    LISP_BASE1(EntityNameSet_O);
    LISP_CLASS(chem,ChemPkg,MonomerPack_O,"MonomerPack");

public:
	void initialize();
public:
//	void	archive(core::ArchiveP node);

private:
		//! Store the unique IDs of the Interesting atoms for each monomer
	core::SymbolList_sp				_InterestingAtomAliases;
	MapOfMonomerNamesToAtomIndexers_sp	_AtomIndexers;

private:
	void _checkAtomIndexers();

public:

//	void	setInterestingAtomAliasesFromString(const string& nm);
	void	setInterestingAtomAliasesFromSymbolList(core::SymbolList_sp nm);
	string	getInterestingAtomAliasesAsString();

    virtual core::SymbolList_sp getInterestingAtomAliases() { return this->_InterestingAtomAliases; };

	void	setMonomerNameOrPdb(core::Symbol_sp nm);
	void	addMonomerName(core::Symbol_sp nm);
	void	removeMonomerName(core::Symbol_sp nm);

		/*! Build the contents from information in a Cons */
	void	defineContentsFromCons(core::Cons_sp atomAliases, core::Cons_sp parts);

	virtual string	getMonomerNameWithAtoms(core::Symbol_sp nm);

	void extendAliases( core::Cons_sp atomAliases, core::Cons_sp parts);

    virtual	bool	supportsInterestingAtomAliases() { return true;};
    virtual void setInterestingAtomNamesForMonomerName(core::Symbol_sp monomerName, const string& atomIndexerNames);
    virtual void setInterestingAtomNamesForMonomerNameStringList(core::Symbol_sp monomerName, core::StringList_sp atomIndexerNames);
    virtual void setInterestingAtomNamesForMonomerNameFromCons(core::Symbol_sp, core::Cons_sp atomIndexerNames);
    virtual string getInterestingAtomNamesForMonomerName(core::Symbol_sp name);
    virtual bool hasInterestingAtomAlias(Alias_sp alias);
    virtual int getInterestingAtomAliasIndex(Alias_sp alias);
    virtual AtomIndexer_sp getAtomIndexerForMonomerName(core::Symbol_sp monomerName);

	MonomerPack_O( const MonomerPack_O& ss ); //!< Copy constructor


	DEFAULT_CTOR_DTOR(MonomerPack_O);
};



};


TRANSLATE(chem::MonomerPack_O);
#endif
