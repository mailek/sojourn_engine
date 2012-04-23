#pragma once
/********************************************************************
	created:	2012/04/23
	filename: 	VariantUtil.h
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
typedef enum _EVariantValueType
{
	eBoolean,
	eSInt32,
	ePointer
} EVariantValueType;

typedef struct _Variant
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