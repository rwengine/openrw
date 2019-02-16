#ifndef OPENALEXTENSIONS_HPP
#define OPENALEXTENSIONS_HPP

#include <efx.h>

/**
 * Functions to access OpenAL EFX extension
 */

extern LPALGENEFFECTS alGenEffects;
extern LPALDELETEEFFECTS alDeleteEffects;
extern LPALEFFECTI alEffecti;
extern LPALEFFECTF alEffectf;

// Aux slot
extern LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
extern LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
extern LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;
extern LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf;

/**
 * Initialize function pointers
 */
void initEfxFunctionPointers();

#endif // OPENALEXTENSIONS_HPP
