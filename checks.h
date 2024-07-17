#pragma once
#include "Script.h"

GEBool CanProcDisease(Entity& p_victim, gCScriptAdmin& scriptAdmin);
GEBool CanProcPoison(Entity& p_victim, gCScriptAdmin& scriptAdmin);
GEBool CanProcFreeze(Entity& p_victim);
GEBool CanProcBurn(Entity& p_victim);
GEBool CanProcSharp(Entity& p_victim);
GEBool CanProcBless(Entity& p_victim, gCScriptAdmin& scriptAdmin);
GEBool IsParade(gEAction action);