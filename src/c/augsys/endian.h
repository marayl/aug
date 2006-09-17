/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_ENDIAN_H
#define AUGSYS_ENDIAN_H

#include "augsys/config.h"
#include "augsys/types.h"

AUGSYS_API uint16_t
aug_swap16(uint16_t i);

AUGSYS_API uint32_t
aug_swap32(uint32_t i);

AUGSYS_API uint64_t
aug_swap64(uint64_t i);

AUGSYS_API uint16_t
aug_ntoh16(uint16_t i);

AUGSYS_API uint32_t
aug_ntoh32(uint32_t i);

AUGSYS_API uint64_t
aug_ntoh64(uint64_t i);

AUGSYS_API uint16_t
aug_hton16(uint16_t i);

AUGSYS_API uint32_t
aug_hton32(uint32_t i);

AUGSYS_API uint64_t
aug_hton64(uint64_t i);

AUGSYS_API uint16_t
aug_decode16(const char* ptr);

AUGSYS_API uint32_t
aug_decode32(const char* ptr);

AUGSYS_API uint64_t
aug_decode64(const char* ptr);

AUGSYS_API void
aug_encode16(char* ptr, uint16_t i);

AUGSYS_API void
aug_encode32(char* ptr, uint32_t i);

AUGSYS_API void
aug_encode64(char* ptr, uint64_t i);

#endif /* AUGSYS_ENDIAN_H */
