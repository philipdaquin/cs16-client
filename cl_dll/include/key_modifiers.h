#pragma once

#ifndef KEY_MODIFIERS_H
#define KEY_MODIFIERS_H

typedef enum
{
	KeyModifier_None = 0,
	KeyModifier_LeftShift = (1 << 0),
	KeyModifier_RightShift = (1 << 1),
	KeyModifier_LeftCtrl = (1 << 2),
	KeyModifier_RightCtrl = (1 << 3),
	KeyModifier_LeftAlt = (1 << 4),
	KeyModifier_RightAlt = (1 << 5),
	KeyModifier_LeftSuper = (1 << 6),
	KeyModifier_RightSuper = (1 << 7),
	KeyModifier_NumLock = (1 << 8),
	KeyModifier_CapsLock = (1 << 9)
} key_modifier_t;

#endif
