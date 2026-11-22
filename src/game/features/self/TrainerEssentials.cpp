#include "core/commands/BoolCommand.hpp"
#include "core/commands/Commands.hpp"
#include "util/Joaat.hpp"

namespace YimMenu::Features
{
	// One switch for the core survival cheats, so after loading a save you
	// don't have to flip five separate toggles. Resolves the member
	// commands through the registry and toggles them with SetState, which
	// runs their OnEnable/OnDisable through the fiber pool exactly like a
	// manual click. Lookups are null-checked: if a command ever gets
	// renamed upstream, it is skipped instead of crashing.
	class TrainerEssentials : public BoolCommand
	{
		using BoolCommand::BoolCommand;

		virtual void OnEnable() override
		{
			for (joaat_t name : {"godmode"_J, "keepbarsfilled"_J,
			          "keepcoresfilled"_J, "infiniteammo"_J,
			          "infiniteclip"_J})
			{
				if (auto cmd = Commands::GetCommand<BoolCommand>(name))
					cmd->SetState(true);
			}
		}

		virtual void OnDisable() override
		{
			for (joaat_t name : {"godmode"_J, "keepbarsfilled"_J,
			          "keepcoresfilled"_J, "infiniteammo"_J,
			          "infiniteclip"_J})
			{
				if (auto cmd = Commands::GetCommand<BoolCommand>(name))
					cmd->SetState(false);
			}
		}
	};

	static TrainerEssentials _TrainerEssentials{"traineressentials", "Trainer Essentials", "One toggle: God Mode, bars & cores filled, infinite ammo and clip"};
}
