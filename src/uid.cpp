#include "uid.h"
#include <MD5Builder.h>

void uidFromBindPhrase(const char *phrase, uint8_t out[6])
{
    String input = String("-DMY_BINDING_PHRASE=\"") + phrase + "\"";

    MD5Builder md5;
    md5.begin();
    md5.add(input);
    md5.calculate();

    uint8_t digest[16];
    md5.getBytes(digest);
    memcpy(out, digest, 6);

    // Clear multicast bit so it is a valid unicast MAC (matches ELRS Backpack)
    out[0] &= ~0x01;
}
