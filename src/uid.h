#pragma once
#include <Arduino.h>

// Derives the 6-byte ELRS group address (UID) from the bind phrase.
// Formula (verified against ExpressLRS/Backpack python/binary_configurator.py):
//   md5( '-DMY_BINDING_PHRASE="' + phrase + '"' )[0:6]
// then clear the multicast bit of byte 0, matching SetSoftMACAddress().
void uidFromBindPhrase(const char *phrase, uint8_t out[6]);
