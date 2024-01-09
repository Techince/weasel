#include "stdafx.h"
#include "Globals.h"

HINSTANCE g_hInst;

LONG g_cRefDll = -1;

CRITICAL_SECTION g_cs;

// {A3F4CDED-B1E9-41EE-9CA6-7B4D0DE6CB0A}
static const GUID c_clsidTextService = 
{ 0xa3f4cded, 0xb1e9, 0x41ee, { 0x9c, 0xa6, 0x7b, 0x4d, 0xd, 0xe6, 0xcb, 0xa } };

// {3D02CAB6-2B8E-4781-BA20-1C9267529467}
static const GUID c_guidProfile = 
{ 0x3d02cab6, 0x2b8e, 0x4781, { 0xba, 0x20, 0x1c, 0x92, 0x67, 0x52, 0x94, 0x67 } };

// {341F9E3A-B8AD-499D-936C-48701E329FB2}
static const GUID c_guidLangBarItemButton = 
{ 0x341f9e3a, 0xb8ad, 0x499d, { 0x93, 0x6c, 0x48, 0x70, 0x1e, 0x32, 0x9f, 0xb2 } };

// {2AC87E79-3260-4B32-9DEA-F8390976C20B}
static const GUID c_guidDisplayAttributeInput =
{ 0x2ac87e79, 0x3260, 0x4b32, { 0x9d, 0xea, 0xf8, 0x39, 0x09, 0x76, 0xc2, 0x0b } };

#ifdef WEASEL_USING_OLDER_TSF_SDK

/* For Windows 8 */
const GUID GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT =
{ 0x13A016DF, 0x560B, 0x46CD, { 0x94, 0x7A, 0x4C, 0x3A, 0xF1, 0xE0, 0xE3, 0x5D } };

const GUID GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT =
{ 0x25504FB4, 0x7BAB, 0x4BC1, { 0x9C, 0x69, 0xCF, 0x81, 0x89, 0x0F, 0x0E, 0xF5 } };

#endif

const GUID GUID_LBI_INPUTMODE =
{ 0x2C77A81E, 0x41CC, 0x4178, { 0xA3, 0xA7, 0x5F, 0x8A, 0x98, 0x75, 0x68, 0xE6 } };

const GUID GUID_IME_MODE_PRESERVED_KEY =
{ 0x0bd899fc, 0xa8f7, 0x4b42, { 0xa9, 0x6d, 0xce, 0xc7, 0xc5, 0x0e, 0x0e, 0xae } };

// {023B034E-06FE-4FC4-957E-C1EF9C54C966}
static const GUID WEASEL_COMPARTMENT_PUNCTUATION =
{ 0x23b034e, 0x6fe, 0x4fc4, { 0x95, 0x7e, 0xc1, 0xef, 0x9c, 0x54, 0xc9, 0x66 } };

// {0581C6CF-F159-4843-B7ED-F9196F5C933E}
static const GUID WEASEL_COMPARTMENT_GLOAL_DEAMON =
{ 0x581c6cf, 0xf159, 0x4843, { 0xb7, 0xed, 0xf9, 0x19, 0x6f, 0x5c, 0x93, 0x3e } };

// {E6F08988-01FA-4FC1-B845-731DBA51C0A9}
static const GUID WEASEL_UILESS_MODE_PRESERVED_KEY =
{ 0xe6f08988, 0x1fa, 0x4fc1, { 0xb8, 0x45, 0x73, 0x1d, 0xba, 0x51, 0xc0, 0xa9 } };

// {690E1C86-C919-45FE-A9DE-C148A61054AB}
static const GUID WEASEL_CARET_FOLLOWING_PRESERVED_KEY =
{ 0x690e1c86, 0xc919, 0x45fe, { 0xa9, 0xde, 0xc1, 0x48, 0xa6, 0x10, 0x54, 0xab } };

// {BDD3F624-2AF4-4E70-8C8F-63A8293FE3CB}
static const GUID WEASEL_DAEMON_PRESERVED_KEY =
{ 0xbdd3f624, 0x2af4, 0x4e70, { 0x8c, 0x8f, 0x63, 0xa8, 0x29, 0x3f, 0xe3, 0xcb } };

// This GUID is used in Windows Vista/7/8 by MS_IME to represents if the
// candidate window is visible or not.
// TODO: Make sure if it is safe to use this GUID.
// {745537CC-8130-4056-920B-018457A4EC06}
static const GUID WEASEL_CUAS_CANDIDATE_MESSAGE_COMPARTMENT =
{ 0x745537cc, 0x8130, 0x4056, { 0x92, 0xb, 0x1, 0x84, 0x57, 0xa4, 0xec, 0x6 } };
