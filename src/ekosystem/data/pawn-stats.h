#pragma once

#include <suprengine/utils/usings.h>

namespace eks
{
	enum class PawnStatType : uint8
	{
		MoveSpeed,
		AttackSpeed,
		MAX,
	};

	struct PawnStatModifier
	{
		PawnStatType type;
		float value = 0.0f;
	};

	class PawnStatManager
	{
	private:
		float _stats[static_cast<size_t>( PawnStatType::MAX )] {};
	};
};