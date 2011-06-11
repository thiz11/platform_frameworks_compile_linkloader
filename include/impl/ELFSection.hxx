#ifndef ELF_SECTION_HXX
#define ELF_SECTION_HXX

#include "utils/raw_ostream.h"

#include <llvm/Support/raw_ostream.h>

#include "ELFSectionHeader.h"
#include "ELFSectionStrTab.h"
#include "ELFSectionSymTab.h"
#include "ELFSectionProgBits.h"
#include "ELFSectionNoBits.h"
#include "ELFSectionRelTable.h"

template <unsigned Bitwidth>
template <typename Archiver>
inline ELFSection<Bitwidth> *
ELFSection<Bitwidth>::read(Archiver &AR,
                           ELFObject<Bitwidth> *owner,
                           ELFSectionHeader<Bitwidth> const *sh) {
  using namespace std;

  switch ((uint32_t)sh->getType()) {
    default:
      // Uknown type of ELF section.  Return NULL.
      llvm::errs() << "WARNING: Unknown section type.\n";
      return 0;

    case SHT_STRTAB:
      return ELFSectionStrTab<Bitwidth>::read(AR, sh);

    case SHT_SYMTAB:
      return ELFSectionSymTab<Bitwidth>::read(AR, owner, sh);

    case SHT_PROGBITS:
    {
#ifdef __arm__
      owner->getStubLayout();
#endif
      return ELFSectionProgBits<Bitwidth>::read(AR, sh);
    }

    case SHT_NOBITS:
      return ELFSectionNoBits<Bitwidth>::read(AR, sh);

    case SHT_REL:
    case SHT_RELA:
      return ELFSectionRelTable<Bitwidth>::read(AR, sh);

    case SHT_NULL:
      // TODO: Not Yet Implemented
      return 0;
  };
}

#endif // ELF_SECTION_HXX
