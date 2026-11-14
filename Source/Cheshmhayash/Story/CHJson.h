// Cheshm Hayash - small helpers to read story beats (FJsonObject) without boilerplate.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"

namespace CHJson
{
	inline FString Str(const FJsonObject& Object, const TCHAR* Key, const FString& Default = FString())
	{
		FString Out;
		return Object.TryGetStringField(Key, Out) ? Out : Default;
	}

	inline double Num(const FJsonObject& Object, const TCHAR* Key, double Default)
	{
		double Out = 0.0;
		return Object.TryGetNumberField(Key, Out) ? Out : Default;
	}

	inline float Float(const FJsonObject& Object, const TCHAR* Key, float Default)
	{
		return static_cast<float>(Num(Object, Key, Default));
	}

	inline bool Bool(const FJsonObject& Object, const TCHAR* Key, bool Default)
	{
		bool Out = false;
		return Object.TryGetBoolField(Key, Out) ? Out : Default;
	}

	/** Accepts either a single string or an array of strings. */
	inline TArray<FString> Lines(const FJsonObject& Object, const TCHAR* Key)
	{
		TArray<FString> Out;
		const TArray<TSharedPtr<FJsonValue>>* Array = nullptr;
		if (Object.HasTypedField<EJson::Array>(Key) && Object.TryGetArrayField(Key, Array) && Array)
		{
			for (const TSharedPtr<FJsonValue>& Value : *Array)
			{
				FString Line;
				if (Value.IsValid() && Value->TryGetString(Line))
				{
					Out.Add(Line);
				}
			}
		}
		else if (Object.HasTypedField<EJson::String>(Key))
		{
			Out.Add(Object.GetStringField(Key));
		}
		return Out;
	}

	/** Array of objects under Key (empty if missing). */
	inline TArray<TSharedPtr<FJsonObject>> Objects(const FJsonObject& Object, const TCHAR* Key)
	{
		TArray<TSharedPtr<FJsonObject>> Out;
		const TArray<TSharedPtr<FJsonValue>>* Array = nullptr;
		if (Object.HasTypedField<EJson::Array>(Key) && Object.TryGetArrayField(Key, Array) && Array)
		{
			for (const TSharedPtr<FJsonValue>& Value : *Array)
			{
				const TSharedPtr<FJsonObject>* Item = nullptr;
				if (Value.IsValid() && Value->TryGetObject(Item) && Item && Item->IsValid())
				{
					Out.Add(*Item);
				}
			}
		}
		return Out;
	}
}
