#include "HelpMethods.h"

std::vector<std::string> splitString(
	std::string inputString,
	char charToSplit)
{
	std::vector<std::string> splittedString;
	std::string tempString;
	for (int i = 0; i < inputString.size(); i++)
	{
		if (inputString[i] == charToSplit)
		{
			if (tempString != "")
			{
				splittedString.push_back(tempString);
			}
			tempString.clear();
		}
		else
		{
			tempString.push_back(inputString[i]);
		}
	}
	splittedString.push_back(tempString);
	return splittedString;
}

bool isUpperCase(char c)
{
	return c >= 'A' && c <= 'Z';
}

bool isLowerCase(char c)
{
	return c >= 'a' && c <= 'z';
}

char charToLower(char c)
{
	return (c + ('a' - 'A'));
}

ChessColor getColorOfFenChar(char c, std::string errorMsgPrefix)
{
	return isUpperCase(c) ? White :
		isLowerCase(c) ? Black :
		throw errorMsgPrefix + " Could not convert Char to Color";
}

CastlingType getCastlingTypeOfFenChar(char c, std::string errorMsgPrefix)
{
	if (isUpperCase(c))
	{
		c = charToLower(c);
	}
	return c == 'q' ? CastleLong :
		c == 'k' ? CastleShort :
		throw errorMsgPrefix + " Could not convert Char to CastlingType";
}

Square getSquareFromString(
	std::string str,
	std::string errorMsgPrefix)
{
	if (str == "-")
	{
		return SQUARE_NONE;
	}
	else if (str.size() == 2)
	{
		int file = str[0] - 'a';
		int rank = str[1] - '1';

		int idx = (Square)(file + (rank * 8));

		if (idx >= 0 && idx < 64)
		{
			return (Square)idx;
		}
	}
	throw errorMsgPrefix + " Could not convert String to Square";
}

Direction getForwardForColor(ChessColor color)
{
	return color == White ? NORTH : SOUTH;
}

Direction getBackwardForColor(ChessColor color)
{
	return color == White ? SOUTH : NORTH;
}

ChessColor getOppositeColor(ChessColor color)
{
	return (ChessColor)((color + 1) % 2);
}

bool isRayType(PieceType type)
{
	return type == Rook || type == Bishop || type == Queen;
}