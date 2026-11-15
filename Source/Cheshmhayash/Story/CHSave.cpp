// Cheshm Hayash - checkpoint save and settings.

#include "Story/CHSave.h"
#include "AudioDevice.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ConfigCacheIni.h"

namespace
{
	const TCHAR* const SlotName = TEXT("CH_Checkpoint");
	const TCHAR* const Section = TEXT("/Script/Cheshmhayash.CHSettings");

	bool bLanguageCached = false;
	ECHLanguage CachedLanguage = ECHLanguage::English;

	TMap<FString, FString>& GetGermanMap()
	{
		static TMap<FString, FString> Map;
		static bool bInit = false;
		if (!bInit)
		{
			bInit = true;
			auto Add = [](const TCHAR* En, const TCHAR* De)
			{
				Map.Add(FString(En).TrimStartAndEnd().ToLower(), FString(De));
			};

			// Menus & Settings
			Add(TEXT("Continue"), TEXT("Fortsetzen"));
			Add(TEXT("New Game"), TEXT("Neues Spiel"));
			Add(TEXT("Chapters"), TEXT("Kapitel"));
			Add(TEXT("Settings"), TEXT("Einstellungen"));
			Add(TEXT("Quit"), TEXT("Beenden"));
			Add(TEXT("Resume"), TEXT("Weiter"));
			Add(TEXT("Main Menu"), TEXT("Hauptmenü"));
			Add(TEXT("Quit to Desktop"), TEXT("Zum Desktop beenden"));
			Add(TEXT("Back"), TEXT("Zurück"));
			Add(TEXT("PAUSED"), TEXT("PAUSIERT"));
			Add(TEXT("Press Any Key"), TEXT("Beliebige Taste drücken"));
			Add(TEXT("Enter  /  Click to continue"), TEXT("Enter  /  Klicken zum Fortfahren"));
			Add(TEXT("Saving..."), TEXT("Speichert..."));
			Add(TEXT("Loading..."), TEXT("Lädt..."));
			Add(TEXT("WASD Move  ·  Shift Run  ·  Ctrl Crouch  ·  E Interact  ·  Esc Pause"),
				TEXT("WASD Bewegen  ·  Shift Rennen  ·  Ctrl Ducken  ·  E Interagieren  ·  Esc Pause"));
			Add(TEXT("WASD Move  ·  Shift Run  ·  Ctrl Crouch  ·  RMB Aim / Film  ·  LMB Throw  ·  E Interact  ·  Esc Pause"),
				TEXT("WASD Bewegen  ·  Shift Rennen  ·  Ctrl Ducken  ·  RMT Zielen / Filmen  ·  LMT Werfen  ·  E Interagieren  ·  Esc Pause"));

			// Chapters
			Add(TEXT("Chapter One  —  Everything Is Fine"), TEXT("Kapitel Eins  —  Alles ist gut"));
			Add(TEXT("Chapter Two  —  Outside"), TEXT("Kapitel Zwei  —  Draußen"));
			Add(TEXT("Chapter Three  —  After"), TEXT("Kapitel Drei  —  Danach"));
			Add(TEXT("Everything Is Fine"), TEXT("Alles ist gut"));
			Add(TEXT("Outside"), TEXT("Draußen"));
			Add(TEXT("After"), TEXT("Danach"));
			Add(TEXT("CHAPTER ONE"), TEXT("KAPITEL EINS"));
			Add(TEXT("CHAPTER TWO"), TEXT("KAPITEL ZWEI"));
			Add(TEXT("CHAPTER THREE"), TEXT("KAPITEL DREI"));
			Add(TEXT("Her Eyes"), TEXT("Ihre Augen"));
			Add(TEXT("HER EYES"), TEXT("IHRE AUGEN"));

			// Speakers
			Add(TEXT("He"), TEXT("Er"));
			Add(TEXT("Sister"), TEXT("Schwester"));
			Add(TEXT("Mother"), TEXT("Mutter"));
			Add(TEXT("Father"), TEXT("Vater"));
			Add(TEXT("Brother"), TEXT("Bruder"));
			Add(TEXT("Kian"), TEXT("Kian"));
			Add(TEXT("Mina"), TEXT("Mina"));
			Add(TEXT("Guard"), TEXT("Wache"));
			Add(TEXT("Soldier"), TEXT("Soldat"));
			Add(TEXT("Old Woman"), TEXT("Alte Frau"));
			Add(TEXT("Old Man"), TEXT("Alter Mann"));
			Add(TEXT("Neighbour"), TEXT("Nachbar"));
			Add(TEXT("Driver"), TEXT("Fahrer"));
			Add(TEXT("Lawyer"), TEXT("Anwalt"));
			Add(TEXT("Professor"), TEXT("Professor"));
			Add(TEXT("Girl"), TEXT("Mädchen"));
			Add(TEXT("Boy"), TEXT("Junge"));
			Add(TEXT("Student"), TEXT("Student"));
			Add(TEXT("Woman"), TEXT("Frau"));
			Add(TEXT("Man"), TEXT("Mann"));
			Add(TEXT("Crowd"), TEXT("Menge"));
			Add(TEXT("Officer"), TEXT("Beamtin"));
			Add(TEXT("Morality Police"), TEXT("Sittenpolizei"));
			Add(TEXT("Morality police"), TEXT("Sittenpolizei"));
			Add(TEXT("Maman"), TEXT("Mama"));
			Add(TEXT("Baba"), TEXT("Papa"));
			Add(TEXT("Taxi driver"), TEXT("Taxifahrer"));
			Add(TEXT("Her friend"), TEXT("Ihre Freundin"));
			Add(TEXT("Stranger"), TEXT("Fremder"));
			Add(TEXT("Old woman"), TEXT("Alte Frau"));
			Add(TEXT("Old man"), TEXT("Alter Mann"));
			Add(TEXT("Official"), TEXT("Beamter"));
			Add(TEXT("Relative"), TEXT("Verwandte"));
			Add(TEXT("Other Roof"), TEXT("Anderes Dach"));
			Add(TEXT("News"), TEXT("Nachrichten"));
			Add(TEXT("Host"), TEXT("Gastgeber"));
			Add(TEXT("She hated this photo."), TEXT("Sie hasste dieses Foto."));
			Add(TEXT("She never wore it."), TEXT("Sie hat ihn nie getragen."));
			Add(TEXT("Everything they own. One page."), TEXT("Alles, was sie besitzen. Ein Blatt Papier."));
			Add(TEXT("Go. Along the wall. Don't stop."), TEXT("Geh. An der Wand entlang. Bleib nicht stehen."));
			Add(TEXT("I didn't walk her. I'm not leaving this one."), TEXT("Ich habe sie damals nicht begleitet. Diesen hier lasse ich nicht allein."));
			Add(TEXT("The last one. Go."), TEXT("Der Letzte. Geh."));
			Add(TEXT("They're all out. Now you. Up the ramp."), TEXT("Alle sind draußen. Jetzt du. Die Rampe hoch."));

			// HUD & Gameplay
			Add(TEXT("SEEN"), TEXT("ENTDECKT"));
			Add(TEXT("THEY NOTICE YOU"), TEXT("SIE BEMERKEN DICH"));
			Add(TEXT("HIDDEN"), TEXT("VERBORGEN"));
			Add(TEXT("Hold Right Mouse to film"), TEXT("Rechte Maustaste halten zum Filmen"));
			Add(TEXT("Hold Right Mouse to aim Molotov  ·  Left Click to throw"), TEXT("RMT halten zum Zielen  ·  LMT zum Werfen"));
			Add(TEXT("Press E to speak"), TEXT("E drücken zum Sprechen"));
			Add(TEXT("Press E to read"), TEXT("E drücken zum Lesen"));
			Add(TEXT("Press E to pick up"), TEXT("E drücken zum Aufheben"));
			Add(TEXT("Press E to knock"), TEXT("E drücken zum Klopfen"));
			Add(TEXT("Press E to open"), TEXT("E drücken zum Öffnen"));
			Add(TEXT("Press E"), TEXT("E drücken"));
			Add(TEXT("Close"), TEXT("Schließen"));
			Add(TEXT("Incoming call"), TEXT("Eingehender Anruf"));
			Add(TEXT("Video call"), TEXT("Videoanruf"));
			Add(TEXT("Voice message"), TEXT("Sprachnachricht"));
			Add(TEXT("News Alert"), TEXT("Eilmeldung"));
			Add(TEXT("online"), TEXT("online"));
			Add(TEXT("Send"), TEXT("Senden"));
			Add(TEXT("REC"), TEXT("AUFN"));
			Add(TEXT("Clip saved"), TEXT("Aufnahme gespeichert"));
			Add(TEXT("Caught."), TEXT("Erwischt."));
			Add(TEXT("Hold [Space] to chant"), TEXT("[Leertaste] halten zum Rufen"));
			Add(TEXT("Walk to your sister's door"), TEXT("Geh zur Tür deiner Schwester"));
			Add(TEXT("Look around your room, then knock on her door"), TEXT("Sieh dich im Zimmer um und klopfe an ihre Tür"));
			Add(TEXT("Approach the Vozara gate"), TEXT("Geh zum Tor von Vozara"));
			Add(TEXT("Find her ID card, a dark manteau, and the house deed"), TEXT("Finde ihren Ausweis, einen dunklen Mantel und die Hausurkunde"));
			Add(TEXT("Speak with the students on campus"), TEXT("Sprich mit den Studierenden auf dem Campus"));
			Add(TEXT("Slip through the shadows to the exit"), TEXT("Schleiche durch den Schatten zum Ausgang"));
			Add(TEXT("Film the confrontation and stay out of reach"), TEXT("Filme den Zusammenstoß und bleib außer Reichweite"));
			Add(TEXT("Hold back the guards with Molotovs (Hold RMB, Click LMB)"), TEXT("Halte die Wachen mit Molotows zurück (RMT halten, LMT werfen)"));
			Add(TEXT("Run down the alley!"), TEXT("Renn die Gasse hinunter!"));
			Add(TEXT("Walk into his room and read the letter on the desk"), TEXT("Geh in sein Zimmer und lies den Brief auf dem Schreibtisch"));
			Add(TEXT("Walk among the graves to find his stone"), TEXT("Geh zwischen den Gräbern zu seinem Grabstein"));

			// World Props & Interactions
			Add(TEXT("Sister's Door"), TEXT("Tür der Schwester"));
			Add(TEXT("Desk"), TEXT("Schreibtisch"));
			Add(TEXT("Window"), TEXT("Fenster"));
			Add(TEXT("Bookshelf"), TEXT("Bücherregal"));
			Add(TEXT("Calendar — Mehr 1401"), TEXT("Kalender — Mehr 1401"));
			Add(TEXT("Sister's ID Card"), TEXT("Ausweis der Schwester"));
			Add(TEXT("Dark Manteau"), TEXT("Dunkler Mantel"));
			Add(TEXT("House Deed"), TEXT("Hausurkunde"));
			Add(TEXT("The Letter"), TEXT("Der Brief"));
			Add(TEXT("Framed Photo"), TEXT("Gerahmtes Foto"));
			Add(TEXT("Tea Samovar"), TEXT("Teesamowar"));
			Add(TEXT("Grave Stone"), TEXT("Grabstein"));
			Add(TEXT("Vozara Gate Guard"), TEXT("Wache am Vozara-Tor"));
			Add(TEXT("Waiting Mother"), TEXT("Wartende Mutter"));
		}
		return Map;
	}

	bool IsAllUpperASCII(const FString& Str)
	{
		bool bHasAlpha = false;
		for (TCHAR C : Str)
		{
			if (FChar::IsAlpha(C) && C < 128)
			{
				bHasAlpha = true;
				if (FChar::IsLower(C))
				{
					return false;
				}
			}
		}
		return bHasAlpha;
	}
}

bool CHSave::Exists()
{
	return UGameplayStatics::DoesSaveGameExist(SlotName, 0);
}

UCHSaveGame* CHSave::Load()
{
	if (!Exists())
	{
		return nullptr;
	}
	return Cast<UCHSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
}

bool CHSave::Write(UCHSaveGame* Data)
{
	return Data && UGameplayStatics::SaveGameToSlot(Data, SlotName, 0);
}

void CHSave::Delete()
{
	UGameplayStatics::DeleteGameInSlot(SlotName, 0);
}

bool CHSettings::HasLanguageSetting()
{
	int32 Val = -1;
	return GConfig && GConfig->GetInt(Section, TEXT("Language"), Val, GGameUserSettingsIni) && Val >= 0 && Val <= 2;
}

ECHLanguage CHSettings::GetLanguage()
{
	if (bLanguageCached)
	{
		return CachedLanguage;
	}
	int32 Val = 0;
	if (GConfig && GConfig->GetInt(Section, TEXT("Language"), Val, GGameUserSettingsIni))
	{
		Val = FMath::Clamp(Val, 0, 2);
		CachedLanguage = static_cast<ECHLanguage>(Val);
	}
	else
	{
		CachedLanguage = ECHLanguage::English;
	}
	bLanguageCached = true;
	return CachedLanguage;
}

void CHSettings::SetLanguage(ECHLanguage Lang)
{
	CachedLanguage = Lang;
	bLanguageCached = true;
	if (GConfig)
	{
		GConfig->SetInt(Section, TEXT("Language"), static_cast<int32>(Lang), GGameUserSettingsIni);
		GConfig->Flush(false, GGameUserSettingsIni);
	}
}

bool CHSettings::IsPersian()
{
	return GetLanguage() == ECHLanguage::Persian;
}

bool CHSettings::IsGerman()
{
	return GetLanguage() == ECHLanguage::German;
}

bool CHSettings::IsEnglish()
{
	return GetLanguage() == ECHLanguage::English;
}

void CHSettings::RegisterGerman(const FString& En, const FString& De)
{
	const FString TrimmedEn = En.TrimStartAndEnd();
	const FString TrimmedDe = De.TrimStartAndEnd();
	if (!TrimmedEn.IsEmpty() && !TrimmedDe.IsEmpty())
	{
		GetGermanMap().Add(TrimmedEn.ToLower(), TrimmedDe);
	}
}

FString CHSettings::Latin(const FString& En, const FString& De)
{
	if (!IsGerman())
	{
		return En;
	}
	if (!De.IsEmpty())
	{
		return De;
	}
	if (En.IsEmpty())
	{
		return En;
	}

	const TMap<FString, FString>& Map = GetGermanMap();
	const FString Key = En.TrimStartAndEnd().ToLower();
	if (const FString* Found = Map.Find(Key))
	{
		if (IsAllUpperASCII(En) && En.Len() > 1)
		{
			return Found->ToUpper();
		}
		return *Found;
	}

	// Dynamic pattern matching for settings rows & HUD counters
	if (En.StartsWith(TEXT("Mouse Sensitivity: ")))
	{
		return FString(TEXT("Mausempfindlichkeit: ")) + En.Mid(19);
	}
	if (En.StartsWith(TEXT("Invert Y-Axis: ")))
	{
		FString Val = En.Mid(15);
		if (Val == TEXT("On")) Val = TEXT("Ein");
		else if (Val == TEXT("Off")) Val = TEXT("Aus");
		return FString(TEXT("Y-Achse umkehren: ")) + Val;
	}
	if (En.StartsWith(TEXT("Master Volume: ")))
	{
		return FString(TEXT("Gesamtlautstärke: ")) + En.Mid(15);
	}
	if (En.StartsWith(TEXT("Molotovs: ")))
	{
		return FString(TEXT("Molotows: ")) + En.Mid(10);
	}

	return En;
}

FString CHSettings::Pick(const FString& Fa, const FString& En, const FString& De)
{
	if (IsPersian())
	{
		return !Fa.IsEmpty() ? Fa : En;
	}
	return Latin(!En.IsEmpty() ? En : Fa, De);
}

float CHSettings::GetSensitivity()
{
	float Value = 1.f;
	GConfig->GetFloat(Section, TEXT("Sensitivity"), Value, GGameUserSettingsIni);
	return FMath::Clamp(Value, 0.2f, 4.f);
}

bool CHSettings::GetInvertY()
{
	bool bValue = false;
	GConfig->GetBool(Section, TEXT("InvertY"), bValue, GGameUserSettingsIni);
	return bValue;
}

float CHSettings::GetVolume()
{
	float Value = 0.8f;
	GConfig->GetFloat(Section, TEXT("Volume"), Value, GGameUserSettingsIni);
	return FMath::Clamp(Value, 0.f, 1.f);
}

void CHSettings::SetSensitivity(float Value)
{
	GConfig->SetFloat(Section, TEXT("Sensitivity"), Value, GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
}

void CHSettings::SetInvertY(bool bValue)
{
	GConfig->SetBool(Section, TEXT("InvertY"), bValue, GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
}

void CHSettings::SetVolume(float Value)
{
	GConfig->SetFloat(Section, TEXT("Volume"), Value, GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
	ApplyVolume();
}

void CHSettings::ApplyVolume()
{
	if (GEngine)
	{
		if (FAudioDevice* Device = GEngine->GetMainAudioDevice().GetAudioDevice())
		{
			Device->SetTransientPrimaryVolume(GetVolume());
		}
	}
}
