#pragma once
#include "../TmpEngine/position_surge.h"
#include "../TmpEngine/tables_surge.h"
#include "../TmpEngine/types_surge.h"

class chess_player
{
public:
	virtual Move get_move(Position& pos) = 0;
};