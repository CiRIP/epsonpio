#ifndef _CRC_H
#define _CRC_H

#ifdef __cplusplus
extern "C" {
#endif

unsigned int crc32(const void *data, unsigned int length, unsigned int crc);

#ifdef __cplusplus
}
#endif
#endif /* _CRC_H */
