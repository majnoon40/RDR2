#include "core/commands/BoolCommand.hpp"
#include "core/frontend/Notifications.hpp"
#include "core/hooking/DetourHook.hpp"
#include "core/misc/RateLimiter.hpp"
#include "game/backend/Players.hpp"
#include "game/backend/Protections.hpp"
#include "game/backend/Self.hpp"
#include "game/backend/PlayerData.hpp"
#include "game/hooks/Hooks.hpp"
#include "game/rdr/data/StableEvents.hpp"
#include "game/rdr/data/TickerEvents.hpp"

#include <network/CNetGamePlayer.hpp>
#include <network/CScriptedGameEvent.hpp>

namespace YimMenu::Features
{
	BoolCommand _LogScriptEvents("logtses", "Log Script Events", "Log script events");
	BoolCommand _BlockHonorEvent("blockhonormanipulation", "Block Honor Event", "Blocks all honor manipulation events", true);
	BoolCommand _BlockDefensive("blockdefensive", "Block Force Defensive", "Blocks all force defensive events", true);
	BoolCommand _BlockOffensive("blockoffensive", "Block Force Offensive", "Blocks all force offensive events", true);
	BoolCommand _BlockPressCharges("blockpresscharges", "Block Press Charges", "Blocks all press charges events", true);
	BoolCommand _BlockStartParlay("blockstartparlay", "Block Start Parlay", "Blocks all start parlay events", true);
	BoolCommand _BlockEndParlay("blockendparlay", "Block End Parlay", "Blocks all end parlay events", true);
	BoolCommand _BlockTickerSpam("blocktickerspam", "Block Ticker Spam", "Blocks all ticker message spam events", true);
	BoolCommand _BlockStableEvents("blockstableevents", "Block Stable Events", "Blocks all stable events", true);
	BoolCommand _BlockKickFromMissionLobby("blockkickfrommissionlobby", "Block Kick From Mission Lobby", "Blocks players from kicking you from mission lobbies", true);
	BoolCommand _BlockScriptEventFlood("blockscripteventflood", "Block Script Event Flood", "Blocks players whose script events exceed the per-player rate limit (120 per 10s)", true);
	BoolCommand _BlockFetchManager("blockfetchmanager", "Block Fetch Manager", "Blocks all fetch manager script events", true);
	BoolCommand _BlockAceLauncher("blockacelauncher", "Block ACE Launcher", "Blocks all ACE launcher script events", true);
	BoolCommand _BlockNetStable("blocknetstable", "Block Stable Events (All)", "Blocks all stable script events, including variants not covered elsewhere", true);
	BoolCommand _BlockVsMissionIntro("blockvsmissionintro", "Block VS Mission Intro", "Blocks all versus mission intro script events", false);
	BoolCommand _BlockNetHud("blocknethud", "Block Net HUD", "Blocks all network HUD script events", false);
	BoolCommand _BlockPlayerCamp("blockplayercamp", "Block Player Camp", "Blocks all player camp script events", false);
}

namespace YimMenu::Hooks
{
	bool Protections::HandleScriptedGameEvent(CScriptedGameEvent* event, CNetGamePlayer* src, CNetGamePlayer* dst)
	{
		if (Features::_LogScriptEvents.GetState())
		{
			std::string script_args = "{ ";
			for (std::size_t i = 0; i < event->m_DataSize / 8; i++)
			{
				if (i)
					script_args += ", ";

				script_args += std::to_string((int)event->m_Data[i]);
			}
			script_args += " };";

			LOG(VERBOSE) << "Script Event:\n"
			             << "\nPlayer: " << src->GetName() << "\n"
			             << "\n\tArgs: " << script_args << "\n\tScript: " << HEX(event->m_ScriptId.m_ScriptHash)
			             << "\n\tHas Metadata Index: " << (event->m_HasScriptMetadataIdx ? "YES" : "NO")
			             << "\n\tID Overriden: " << (event->m_ScriptIdOverridden ? "YES" : "NO");
		}

		// per-player script event flood gate: mod menus spam malformed script
		// events to crash clients; legit remote scripts never come close to
		// 120 events in 10 seconds from a single sender
		if (src && Features::_BlockScriptEventFlood.GetState()
		    && Player(src).IsValid() && Player(src).GetData().m_ScriptEventFloodLimit.Process())
		{
			if (Player(src).GetData().m_ScriptEventFloodLimit.ExceededLastProcess())
				LOGF(NET_EVENT, WARNING, "Blocked script event flood from {}", src->GetName());
			return true;
		}

		auto evnt_id = static_cast<ScriptEvent>((int)event->m_Data[0]);
		switch (evnt_id)
		{
		case ScriptEvent::SCRIPT_EVENT_PERSONA_HONOR:
		{
			if (event->m_Data[4] == 2 && Features::_BlockHonorEvent.GetState())
			{
				Notifications::Show("Protections", std::format("Blocked honor manipulation from {}", src->GetName()), NotificationType::Warning);
				return true;
			}
			break;
		}
		case ScriptEvent::SCRIPT_EVENT_NOTORIETY_FORCE_PASSIVE:
		{
			if (event->m_Data[8] == 2 && Features::_BlockDefensive.GetState())
			{
				Notifications::Show("Protections", std::format("Blocked force defensive mode from {}", src->GetName()), NotificationType::Warning);
				return true;
			}
			break;
		}
		case ScriptEvent::SCRIPT_EVENT_NOTORIETY_FORCE_NOT_PASSIVE_HORSE:
		{
			if (event->m_Data[1] == Self::GetPlayer().GetId() && Features::_BlockOffensive.GetState())
			{
				Notifications::Show("Protections", std::format("Blocked force offensive mode from {}", src->GetName()), NotificationType::Warning);
				return true;
			}
			break;
		}
		case ScriptEvent::SCRIPT_EVENT_NOTORIETY_PRESS_CHARGES:
		{
			if (event->m_Data[11] && event->m_Data[4] == 2 || event->m_Data[4] == 3 && Features::_BlockPressCharges.GetState())
			{
				Notifications::Show("Protections", std::format("Blocked press charges from {}", src->GetName()), NotificationType::Warning);
				return true;
			}
			break;
		}
		case ScriptEvent::SCRIPT_EVENT_PARLAY:
		{
			if (event->m_Data[4] == 3 && Features::_BlockStartParlay.GetState())
			{
				Notifications::Show("Protections", std::format("Blocked start parlay from {}", src->GetName()), NotificationType::Warning);
				return true;
			}
			if (event->m_Data[4] == 5 && Features::_BlockEndParlay.GetState())
			{
				Notifications::Show("Protections", std::format("Blocked end parlay from {}", src->GetName()), NotificationType::Warning);
				return true;
			}
			break;
		}
		case ScriptEvent::SCRIPT_EVENT_TICKER_MESSAGE:
		{
			if (Player(src).GetData().m_TickerMessageRateLimit.Process() && Features::_BlockTickerSpam.GetState())
			{
				if (Player(src).GetData().m_TickerMessageRateLimit.ExceededLastProcess())
				{
					LOGF(NET_EVENT, WARNING, "Blocked ticker spam ({}) from {}", event->m_Data[4], src->GetName());
				}
				return true;
			}
			break;
		}
		case ScriptEvent::SCRIPT_EVENT_NET_STABLE_MOUNT:
		{
			if (event->m_Data[1] == Self::GetPlayer().GetId() && Features::_BlockStableEvents.GetState())
			{
				Notifications::Show("Protections",
				    std::format("Blocked stable event from {} ({})",
				        src->GetName(),
				        Data::g_StableMountEvent[event->m_Data[4]].second),
				    NotificationType::Warning);
				return true;
			}
			break;
		}
		case ScriptEvent::SCRIPT_EVENT_FETCH_MANAGER:
		{
			if (Features::_BlockFetchManager.GetState())
			{
				Notifications::Show("Protections", std::format("Blocked fetch manager event from {}", src->GetName()), NotificationType::Warning);
				return true;
			}
			break;
		}
		case ScriptEvent::SCRIPT_EVENT_ACE_LAUNCHER:
		{
			if (Features::_BlockAceLauncher.GetState())
			{
				Notifications::Show("Protections", std::format("Blocked ACE launcher event from {}", src->GetName()), NotificationType::Warning);
				return true;
			}
			break;
		}
		case ScriptEvent::SCRIPT_EVENT_NET_STABLE:
		{
			if (Features::_BlockNetStable.GetState())
			{
				Notifications::Show("Protections", std::format("Blocked stable event from {}", src->GetName()), NotificationType::Warning);
				return true;
			}
			break;
		}
		case ScriptEvent::SCRIPT_EVENT_NET_VS_MISSION_INTRO:
		{
			if (Features::_BlockVsMissionIntro.GetState())
			{
				Notifications::Show("Protections", std::format("Blocked VS mission intro event from {}", src->GetName()), NotificationType::Warning);
				return true;
			}
			break;
		}
		case ScriptEvent::SCRIPT_EVENT_NET_HUD:
		{
			if (Features::_BlockNetHud.GetState())
			{
				Notifications::Show("Protections", std::format("Blocked network HUD event from {}", src->GetName()), NotificationType::Warning);
				return true;
			}
			break;
		}
		case ScriptEvent::SCRIPT_EVENT_PLAYER_CAMP:
		{
			if (Features::_BlockPlayerCamp.GetState())
			{
				Notifications::Show("Protections", std::format("Blocked player camp event from {}", src->GetName()), NotificationType::Warning);
				return true;
			}
			break;
		}
		}

		return BaseHook::Get<Protections::HandleScriptedGameEvent, DetourHook<decltype(&Protections::HandleScriptedGameEvent)>>()->Original()(event, src, dst);
	}
}
