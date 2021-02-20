// RscAutoScreenshot.cpp : Defines the exported functions for the DLL application.
//

#include "RscAutoScreenshot.h"
#include <chrono>
#include <time.h>

BAKKESMOD_PLUGIN(RscAutoScreenshot, "RSC Auto Screenshot", "1.2", 0)

// make sure you're building the x64 release version
// right now, to enable the plugin, open up the BM console with f6 and type: plugin load rscautoscreenshot
void RscAutoScreenshot::onLoad()
{
	// settings
	cvarManager->registerCvar("autoscreenshot_enabled", "1", "Enable or Disable the Auto Screenshot Plugin", true, true, 0, true, 1, true);
	cvarManager->registerCvar("autoscreenshot_enabled_for_match_night", "1", "Enable or Disable the Auto Screenshot Plugin during match nights (Monday and Wednesday, 10-11PM EST)", true, true, 0, true, 1, true);
	cvarManager->registerCvar("screenshot_delay", "15", "How long to wait before taking screenshot", true, true, 0.0f, true, 60.0f, true);
	cvarManager->registerCvar("screenshot_button", "0", "Keyboard button to send for screenshot", true, true, 0, true, 1, true);

	// called when game ends
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet", bind(&RscAutoScreenshot::StatsScreen, this, std::placeholders::_1));
}

bool RscAutoScreenshot::isRSCMatchTime()
{
	const int TUESDAY = 2;
	const int THURSDAY = 4;

	// get system clock time
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	// get local time to check for daylight savings
	struct tm* local_tm = new tm;
	localtime_s(local_tm , &in_time_t);
	bool is_daylight_savings = local_tm->tm_isdst;

	// outside of dst: 3am-4am. in dst: 2am-3am
	int start_time = is_daylight_savings ? 2 : 3;
	int end_time = is_daylight_savings ? 3 : 4;

	// convert to UTC
	struct tm* gm_tm = new tm;
	gmtime_s(gm_tm, &in_time_t);

	// Match days in UTC/GM are Tuesdays and Thursdays from 3am-4am
	if (gm_tm->tm_wday == TUESDAY || gm_tm->tm_wday == THURSDAY) {
		if (gm_tm->tm_hour >= start_time && gm_tm->tm_hour <= end_time) {
			return true;
		}
	}

	return false;
}

void RscAutoScreenshot::StatsScreen(std::string eventName)
{
	isEnabled = cvarManager->getCvar("autoscreenshot_enabled").getBoolValue();
	isEnabledForMatchNight = cvarManager->getCvar("autoscreenshot_enabled_for_match_night").getBoolValue();
	// both off
	if (!isEnabled && !isEnabledForMatchNight) {
		return;
	}
	// only enabled for match night
	else if (!isEnabled && isEnabledForMatchNight) {
		if (!isRSCMatchTime()) {
			return;
		}
	}
	// else, enabled all the time

	screenshotDelay = cvarManager->getCvar("screenshot_delay").getFloatValue();

	// take a screenshot after 10 seconds
	gameWrapper->SetTimeout([this](GameWrapper* gw) {
		this->takeScreenshot();
		}, screenshotDelay);
}

void RscAutoScreenshot::takeScreenshot()
{
	screenshotButton = cvarManager->getCvar("screenshot_button").getFloatValue();
	if (screenshotButton == 3.00f)
	{
		// press and release print screen
		keybd_event(VK_SNAPSHOT, 0x2C, 0, 0);
		keybd_event(VK_SNAPSHOT, 0x2C, KEYEVENTF_KEYUP, 0);
	}
	else if (screenshotButton == 2.00f)
	{
		// press and release Win-G / Win-Alt-print Screen
		keybd_event(VK_LWIN, 0x5B, 0, 0);
		keybd_event(VkKeyScan('G'), 0, 0, 0);
		keybd_event(VkKeyScan('G'), 0, KEYEVENTF_KEYUP, 0);
		keybd_event(VK_LWIN, 0x5B, KEYEVENTF_KEYUP, 0);

		// win-alt-print screen to take screenshot
		keybd_event(VK_LWIN, 0x5B, 0, 0);
		keybd_event(VK_MENU, 0, 0, 0);
		keybd_event(VK_SNAPSHOT, 0, 0, 0);
		keybd_event(VK_SNAPSHOT, 0, KEYEVENTF_KEYUP, 0);
		keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
		keybd_event(VK_LWIN, 0x5B, KEYEVENTF_KEYUP, 0);
	}
	else if (screenshotButton == 1.00f)
	{
		// press and release f9, gyg's default screenshot button
		keybd_event(VK_F9, 0x78, 0, 0);
		keybd_event(VK_F9, 0x78, KEYEVENTF_KEYUP, 0);
	}
	else
	{
		// press and release f12, steam's default screenshot button
		keybd_event(VK_F12, 0x7B, 0, 0);
		keybd_event(VK_F12, 0x7B, KEYEVENTF_KEYUP, 0);
	}
}

void RscAutoScreenshot::onUnload()
{
	// do i need this? it's on RankViewer even though there's no event hooked to this
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded");
}
