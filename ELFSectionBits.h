#ifndef ELF_SECTION_BITS_H
#define ELF_SECTION_BITS_H

/*
 * This file is bits section CRTP class, only for progbits and nobits.
 */

#include "ELFSection.h"

#include "utils/helper.h"
#include "utils/serialize.h"

#include <sys/mman.h>
#include <llvm/ADT/OwningPtr.h>
#include <llvm/Support/raw_ostream.h>

template <size_t Bitwidth> class ELFSectionHeader;

template <size_t Bitwidth, typename ConcreteELFSectionBits>
class ELFSectionBits : public ELFSection<Bitwidth> {
protected:
  ELFSectionHeader<Bitwidth> const *section_header;
  unsigned char *buf;
  size_t buf_size;

protected:
  ELFSectionBits() : buf(NULL), buf_size(0) { }

  ~ELFSectionBits() {
    if (buf) {
      munmap(buf, buf_size);
    }
  }

public:
  template <typename Archiver>
  static ConcreteELFSectionBits *
  read(Archiver &AR,
       ELFSectionHeader<Bitwidth> const *sh);

  virtual void print() const {}

  unsigned char &operator[](size_t index) {
    return buf[index];
  }

  unsigned char const &operator[](size_t index) const {
    return buf[index];
  }

  size_t size() const {
    return buf_size;
  }

  unsigned char const *memory_protect() const;

};



//==================Inline Member Function Definition==========================


template <size_t Bitwidth, typename ConcreteELFSectionBits>
template <typename Archiver>
inline ConcreteELFSectionBits *
ELFSectionBits<Bitwidth, ConcreteELFSectionBits>::
read(Archiver &AR,
     ELFSectionHeader<Bitwidth> const *sh) {
  llvm::OwningPtr<ConcreteELFSectionBits> result(new ConcreteELFSectionBits());
  // TODO: Align.
  result->buf_size = sh->getSize();
  if (result->buf_size > 0) {
    result->buf = (unsigned char *)mmap(0, result->buf_size,
                                        PROT_READ | PROT_WRITE,
                                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  }

  // Check map success.
  if (result->buf == MAP_FAILED) {
    return 0;
  }

  // Save section_header
  result->section_header = sh;

  // Read the buffer from string tab
  if (!result->serialize(AR)) {
    // Unable to read the structure.  Return NULL.
    return 0;
  }

  if (!AR) {
    // Unable to read the progbits tab
    return 0;
  }

  return result.take();
}

template <size_t Bitwidth, typename ConcreteELFSectionBits>
inline unsigned char const *
ELFSectionBits<Bitwidth, ConcreteELFSectionBits>::memory_protect() const {
  int protect_type = PROT_READ;
  if (this->section_header->get_flags() & SHF_WRITE) {
    protect_type &= PROT_WRITE;
  }
  if (this->section_header->get_flags() &SHF_EXECINSTR) {
    protect_type &= PROT_EXEC;
  }
  if (buf_size > 0) {
    int ret = mprotect((void *)buf, buf_size, protect_type);
    if (ret == -1) {
      // FIXME: Throws excetion?
      llvm::errs()<<"Error: Can't mprotect.\n";
      return 0;
    }
  }
  return buf;
}

#endif // ELF_SECTION_BITS_H
