#include "core/commands/LoopedCommand.hpp"
#include "game/backend/Self.hpp"
#include "game/rdr/Natives.hpp"

namespace YimMenu::Features
{
	// Bonding levels run 0-4 in RDR2; 4 is the max. This header has no
	// getter native for the current level, so (like Godmode does with
	// invincibility) we simply re-assert level 4 every tick while enabled.
	// That also covers horse swaps: a newly mounted horse gets bonded
	// automatically on the next tick.
	class KeepMaxBonding : public LoopedCommand
	{
		using LoopedCommand::LoopedCommand;

		virtual void OnTick() override
		{
			auto mount = Self::GetMount();

			if (!mount || mount.IsDead())
				return;

			PED::_SET_MOUNT_BONDING_LEVEL(mount.GetHandle(), 4);
		}
	};

	static KeepMaxBonding _KeepMaxBonding{"keepmaxbonding", "Keep Max Bonding", "Keeps your horse's bonding maxed, including newly mounted horses"};
}
