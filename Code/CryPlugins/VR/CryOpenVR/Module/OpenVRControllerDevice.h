#pragma once

#include <CryInput/IInput.h>

namespace CryVR
{
namespace OpenVR
{
	class Controller;

	class CControllerDevice final : public IInputDevice
	{
	public:
		CControllerDevice(int index) : m_controllerIndex(index) {}
		virtual ~CControllerDevice() = default;

		// IInputDevice
		virtual const char*      GetDeviceName() const override { return "OpenVR_Controller"; }
		virtual EInputDeviceType GetDeviceType() const override { return eIDT_MotionController; }
		virtual TInputDeviceId   GetDeviceId() const override { return 0; }
		virtual int              GetDeviceIndex() const override { return m_controllerIndex; }

		//! Initialization.
		virtual bool Init() override { return true; }
		virtual void PostInit() override {}

		//! Update.
		virtual void Update(bool bFocus) override;

		//! Sets force feedback.
		//! \return true if successful.
		virtual bool SetForceFeedback(IFFParams params) override;

		//! Checks for key pressed and held.
		virtual bool InputState(const TKeyName& key, EInputState state) override;

		//! Sets/unsets DirectInput to exclusive mode.
		virtual bool SetExclusiveMode(bool value) override { return false; }

		//! Clears the key (pressed) state.
		virtual void ClearKeyState() override;

		//! Clears analog position state.
		virtual void ClearAnalogKeyState(TInputSymbols& clearedSymbols) override;

		virtual void                SetUniqueId(uint8 const uniqueId) override { m_uniqueId = uniqueId; }
		virtual const char*         GetKeyName(const SInputEvent& event) const override;
		virtual const char*         GetKeyName(const EKeyId keyId) const override;
		virtual uint32              GetInputCharUnicode(const SInputEvent& event) override { return 0; }
		virtual const char*         GetOSKeyName(const SInputEvent& event) override { return ""; }
		virtual SInputSymbol*       LookupSymbol(EKeyId id) const override;
		virtual const SInputSymbol* GetSymbolByName(const char* name) const override;
		virtual bool                IsOfDeviceType(EInputDeviceType type) const override { return type == eIDT_MotionController; }
		virtual void                Enable(bool enable) override { m_enabled = enable; }
		virtual bool                IsEnabled() const override { return m_enabled; }
		virtual void                OnLanguageChange() override {}

		//! Dead zone settings for input devices where this is relevant (i.e. controllers with analog sticks).
		//! \param fThreshold Between 0 and 1.
		virtual void SetDeadZone(float fThreshold) override {}
		virtual void RestoreDefaultDeadZone() override {}
		// ~IInputDevice

	protected:
		void PostInputEventIfChanged(const Controller& controller, EKeyId keyId);
		void PostInputEvent(SInputSymbol& symbol);
		SInputSymbol* LookupSymbol(const Controller& controller, EKeyId keyId) const;

	protected:
		int m_controllerIndex = 0;
		uint8 m_uniqueId = 0;
		bool m_enabled = true;

		using ButtonMask = uint64;

		ButtonMask m_buttonsPressed = 0;
		ButtonMask m_buttonsTouched = 0;
		float  m_triggerValue = 0.f;
		Vec2   m_touchPadValue = ZERO;
	};
}
}