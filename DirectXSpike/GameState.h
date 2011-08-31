#pragma once

interface IGameState 
{
	virtual bool HandleEvent( UINT eventId, void* data, UINT data_sz )=0;
};