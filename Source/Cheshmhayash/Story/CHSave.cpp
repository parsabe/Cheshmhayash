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
