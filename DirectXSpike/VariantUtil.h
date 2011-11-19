#pragma once

typedef enum
{
	eBoolean,
	eSInt32,
	ePointer
} EVariantValueType;

typedef struct
{
	union
	{
		bool	bool_val;
		void*	ptr_val;
		int		sint32_val;
	}
	value;
	EVariantValueType type;

} Variant;

inline void InitVariantBool(Variant* var, bool value)
{
	var->type = eBoolean;
	var->value.bool_val = value;
}