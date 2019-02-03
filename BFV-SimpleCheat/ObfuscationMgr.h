#pragma once
//https://github.com/Speedi13/BFV-Decryption

#include "FrostbiteSDK.h"

fb::ClientPlayer* GetPlayerById( int id );
fb::ClientPlayer* GetLocalPlayer( void );

void BypassObfuscationMgr();
