// Cheshm Hayash - the phone. The phone is his voice: chat bubbles, "typing..." dots, drafts that
// are typed and then deleted, voice messages (waveform + transcript, never audio), a video call,
// a news alert, "No service".

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SScrollBox;
class SVerticalBox;

struct FCHPhoneMessage
{
	enum class EKind : uint8 { Text, Voice, Transcript, Note };

	EKind Kind = EKind::Text;
	bool bMine = false;
	FString Fa;
	FString En;
	bool bSent = false;
	float Age = 0.f;
};

class SCHPhoneOverlay : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCHPhoneOverlay) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void Advance(float DeltaTime);

	// ---- open / close
	void OpenChat(const FString& ContactFa, const FString& ContactEn, bool bClearMessages = true);
	void OpenCall(const FString& ContactFa, const FString& ContactEn);
	void Close();
	bool IsOpen() const { return bOpen; }

	/** True while an animation the story should wait for is running. */
	bool IsBusy() const;

	// ---- chat
	/** Incoming message: shows the typing dots for TypingSeconds first. */
	void Receive(const FString& Fa, const FString& En, float TypingSeconds);
	/** He types in the input field. bSend: it becomes a sent bubble. Otherwise it is deleted again. */
	void TypeDraft(const FString& Fa, const FString& En, bool bSend, float HoldSeconds = 1.2f);
	/** Small centred grey note inside the chat ("Today", "Unread"). */
	void AddNote(const FString& Fa, const FString& En);
	void SetStatus(const FString& Fa, const FString& En);

	// ---- voice message (no audio: waveform + transcript)
	void StartVoice(const FString& DurationText, int32 NumLines);
	/** Adds the next transcript line under the voice bubble. */
	void AddTranscriptLine(const FString& Fa, const FString& En);
	void EndVoice();

	// ---- news alert, service, call
	void ShowAlert(const FString& SourceFa, const FString& SourceEn, const FString& Fa, const FString& En);
	void HideAlert();
	void SetNoService(bool bInNoService) { bNoService = bInNoService; }
	void SetCallFrozen(bool bInFrozen) { bCallFrozen = bInFrozen; }
	void SetClock(const FString& InClock) { Clock = InClock; }

	/** True once per typed letter: the cue for the soft typing sound. */
	bool ConsumeTypeTick();

private:
	enum class EAction : uint8 { None, Receiving, DraftTyping, DraftHold, DraftDeleting, DraftSending };

	TSharedRef<SWidget> BuildChatView();
	TSharedRef<SWidget> BuildCallView();
	TSharedRef<SWidget> BuildAlert();
	TSharedRef<SWidget> MakeMessageWidget(int32 Index);
	void AddMessage(const FCHPhoneMessage& Message);
	void RebuildMessages();

	int32 GetViewIndex() const;
	FText GetContactFa() const;
	FText GetContactEn() const;
	FText GetStatusText() const;
	FText GetClockText() const;
	FText GetServiceText() const;
	FText GetDraftText() const;
	FText GetCallTimer() const;
	FText GetAlertSource() const;
	FText GetAlertFa() const;
	FText GetAlertEn() const;
	FSlateColor GetDotColor(int32 Index) const;
	EVisibility GetDotsVisibility() const;
	EVisibility GetFrozenVisibility() const;
	EVisibility GetAlertVisibility() const;
	FSlateColor GetDraftColor() const;
	FSlateColor GetServiceColor() const;
	float GetVoiceProgress() const;

	TSharedPtr<SWidget> PhoneBody;
	TSharedPtr<SWidget> AlertCard;
	TSharedPtr<SWidget> CallFigure;
	TSharedPtr<SScrollBox> Scroll;
	TSharedPtr<SVerticalBox> MessageBox;
	TArray<TSharedPtr<SWidget>> MessageWidgets;

	TArray<FCHPhoneMessage> Messages;

	FString ContactFa;
	FString ContactEn;
	FString StatusFa;
	FString StatusEn;
	FString Clock = TEXT("09:12");

	bool bOpen = false;
	bool bCall = false;
	bool bNoService = false;
	bool bCallFrozen = false;
	float OpenAlpha = 0.f;
	float Time = 0.f;
	float CallSeconds = 134.f;

	// current action
	EAction Action = EAction::None;
	float ActionTime = 0.f;
	float ActionDuration = 0.f;
	FString PendingFa;
	FString PendingEn;
	bool bPendingSend = false;
	float DraftChars = 0.f;
	int32 LastDraftCount = 0;
	bool bTypeTick = false;
	float SettleTime = 0.f;

	// voice
	int32 VoiceLines = 0;
	int32 VoiceShown = 0;
	float VoiceProgress = 0.f;
	FString VoiceDuration;
	bool bVoiceActive = false;

	// alert
	bool bAlert = false;
	float AlertAlpha = 0.f;
	FString AlertSourceFa;
	FString AlertSourceEn;
	FString AlertFa;
	FString AlertEn;
};
