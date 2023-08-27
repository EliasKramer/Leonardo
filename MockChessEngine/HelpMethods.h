#pragma once
#include <string>
#include <vector>
#include "DataAndTypes.h"
#include "MyBitBoard.h"

std::vector<std::string> splitString(
	std::string inputString,
	char charToSplit);

bool isUpperCase(char c);
bool isLowerCase(char c);

char charToLower(char c);


ChessColor getColorOfFenChar(
	char c,
	std::string errorMsgPrefix = "ERROR");

CastlingType getCastlingTypeOfFenChar(
	char c,
	std::string errorMsgPrefix = "ERROR");

Square getSquareFromString(
	std::string str,
	std::string errorMsgPrefix = "ERROR");

Direction getForwardForColor(ChessColor color);
Direction getBackwardForColor(ChessColor color);

ChessColor getOppositeColor(ChessColor color);

bool isRayType(PieceType type);