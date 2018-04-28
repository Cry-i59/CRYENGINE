#include "StdAfx.h"

#include "OpenVRControllerDevice.h"

namespace CryVR
{
namespace OpenVR
{
	void CControllerDevice::Update(bool hasFocus)
	{
		if (Device* pDevice = Resources::GetAssociatedDevice())
		{
			const Controller& controller = static_cast<const Controller&>(*pDevice->GetController());

			// Send button events (if necessary)
			PostInputEventIfChanged(controller, eKI_Motion_OpenVR_System);
			PostInputEventIfChanged(controller, eKI_Motion_OpenVR_ApplicationMenu);
			PostInputEventIfChanged(controller, eKI_Motion_OpenVR_Grip);
			//PostInputEventIfChanged(controller, eKI_Motion_OpenVR_TriggerBtn);
			//PostInputEventIfChanged(controller, eKI_Motion_OpenVR_TouchPadBtn);

			// send trigger event (if necessary)
			if (controller.m_state[m_controllerIndex].trigger != m_triggerValue)
			{
				SInputSymbol& symbol = *LookupSymbol(controller, eKI_Motion_OpenVR_Trigger);
				symbol.ChangeEvent(controller.m_state[m_controllerIndex].trigger);
				PostInputEvent(symbol);

				m_triggerValue = controller.m_state[m_controllerIndex].trigger;
			}

			// send touch pad events (if necessary)
			for (uint8 i = 0; i < 2; ++i)
			{
				if (controller.m_state[m_controllerIndex].touchPad[i] != m_touchPadValue[i])
				{
					SInputSymbol& symbol = *LookupSymbol(controller, static_cast<EKeyId>(eKI_Motion_OpenVR_TouchPad_X + i));
					symbol.ChangeEvent(controller.m_state[m_controllerIndex].touchPad[i]);
					PostInputEvent(symbol);

					m_touchPadValue[i] = controller.m_state[m_controllerIndex].touchPad[i];
				}
			}
		}
	}

	void CControllerDevice::PostInputEventIfChanged(const Controller &controller, EKeyId keyId)
	{
		const uint32 keyIndex = keyId - eKI_Motion_OpenVR_First;
		const uint64 keyMask = 1ull << keyIndex;
		const uint64 vrButtonMask = vr::ButtonMaskFromId(static_cast<vr::EVRButtonId>(controller.m_symbols[keyIndex]->devSpecId & (~OPENVR_SPECIAL)));
		const bool isPressed = (controller.m_state[m_controllerIndex].buttonsPressed & vrButtonMask) > 0;
		const bool wasPressed = (m_buttonsPressed & vrButtonMask) > 0;

		if (isPressed != wasPressed)
		{
			SInputSymbol& symbol = *LookupSymbol(controller, keyId);
			symbol.PressEvent(isPressed);
			PostInputEvent(symbol);

			m_buttonsPressed ^= keyMask;
		}
	}

	void CControllerDevice::PostInputEvent(SInputSymbol& symbol)
	{
		SInputEvent event;
		symbol.AssignTo(event);
		event.deviceIndex = m_controllerIndex;
		event.deviceType = eIDT_MotionController;
		gEnv->pInput->PostInputEvent(event);
	}

	bool CControllerDevice::SetForceFeedback(IFFParams params)
	{
		if (Device* pDevice = Resources::GetAssociatedDevice())
		{
			const_cast<IHmdController*>(pDevice->GetController())->ApplyForceFeedback(static_cast<EHmdController>(m_controllerIndex), params.strengthA, 0);
			return true;
		}

		return false;
	}

	bool CControllerDevice::InputState(const TKeyName& key, EInputState state)
	{
		if (const SInputSymbol* pSymbol = GetSymbolByName(key))
		{
			return pSymbol->state == state;
		}

		return false;
	}

	void CControllerDevice::ClearKeyState()
	{
		if (Device* pDevice = Resources::GetAssociatedDevice())
		{
			const Controller& controller = static_cast<const Controller&>(*pDevice->GetController());

			m_buttonsPressed = controller.m_state->buttonsPressed;
			m_buttonsTouched = controller.m_state->buttonsTouched;
		}
		else
		{
			m_buttonsPressed = 0;
			m_buttonsTouched = 0;
		}
	}

	void CControllerDevice::ClearAnalogKeyState(TInputSymbols& clearedSymbols)
	{
		if (Device* pDevice = Resources::GetAssociatedDevice())
		{
			const Controller& controller = static_cast<const Controller&>(*pDevice->GetController());

			m_triggerValue = controller.m_state->trigger;
			m_touchPadValue = controller.m_state->touchPad;
		}
		else
		{
			m_triggerValue = 0.f;
			m_touchPadValue = ZERO;
		}
	}

	const char* CControllerDevice::GetKeyName(const SInputEvent& event) const
	{
		return GetKeyName(event.keyId);
	}

	const char* CControllerDevice::GetKeyName(const EKeyId keyId) const
	{
		if (SInputSymbol* pSymbol = LookupSymbol(keyId))
		{
			return pSymbol->name;
		}

		return "";
	}

	SInputSymbol* CControllerDevice::LookupSymbol(EKeyId keyId) const
	{
		if (Device* pDevice = Resources::GetAssociatedDevice())
		{
			const Controller& controller = static_cast<const Controller&>(*pDevice->GetController());
			return LookupSymbol(controller, keyId);
		}

		return nullptr;
	}

	SInputSymbol* CControllerDevice::LookupSymbol(const Controller& controller, EKeyId keyId) const
	{
		const uint32 keyIndex = keyId - eKI_Motion_OpenVR_First;
		return controller.m_symbols[keyIndex];
	}

	const SInputSymbol* CControllerDevice::GetSymbolByName(const char* name) const
	{
		if (Device* pDevice = Resources::GetAssociatedDevice())
		{
			const Controller& controller = static_cast<const Controller&>(*pDevice->GetController());
			for (const SInputSymbol* pSymbol : controller.m_symbols)
			{
				if (!strcmp(pSymbol->name, name))
				{
					return pSymbol;
				}
			}
		}

		return nullptr;
	}
}
}