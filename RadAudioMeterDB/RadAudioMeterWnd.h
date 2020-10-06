#pragma once

#include <vector>

#include <atlcom.h>
#include <mmdeviceapi.h>

class CVisualStyle;
struct IAudioMeterInformation;

#define WM_DEFAULTDEVICECHANGED (WM_USER+167)

class CNotificationClient :
    public ATL::CComObjectRootEx<ATL::CComSingleThreadModel>,
    public IMMNotificationClient
{
    BEGIN_COM_MAP(CNotificationClient)
        COM_INTERFACE_ENTRY(IMMNotificationClient)
    END_COM_MAP()

public:
    void Init(HWND hWnd)
    {
        m_hWnd = hWnd;
    }

    // IMMNotificationClient

    STDMETHOD(OnDeviceStateChanged)(
        _In_  LPCWSTR pwstrDeviceId, 
        _In_  DWORD dwNewState)
    {
        return S_OK;
    }

    STDMETHOD(OnDeviceAdded)(
        _In_  LPCWSTR pwstrDeviceId)
    {
        return S_OK;
    }

    STDMETHOD(OnDeviceRemoved)(
        _In_  LPCWSTR pwstrDeviceId)
    {
        return S_OK;
    }

    STDMETHOD(OnDefaultDeviceChanged)(
        _In_  EDataFlow flow,
        _In_  ERole role,
        _In_  LPCWSTR pwstrDefaultDeviceId)
    {
        SendMessage(m_hWnd, WM_DEFAULTDEVICECHANGED, (WPARAM) pwstrDefaultDeviceId, 0);
        return S_OK;
    }

    STDMETHOD(OnPropertyValueChanged)(
        _In_  LPCWSTR pwstrDeviceId,
        _In_  const PROPERTYKEY key)
    {
        return S_OK;
    }

private:
    HWND m_hWnd;
};

class CRadAudioMeterWnd :
    public ATL::CWindowImpl<CRadAudioMeterWnd>
{
public:
    CRadAudioMeterWnd();
    ~CRadAudioMeterWnd();

    BOOL Create(
        HWND hwndParent,
        LPUNKNOWN pDeskBand,
        LPUNKNOWN pInputObjectSite);

    POINTL CalcMinimalSize() const;
    POINTL CalcIdealSize() const;
    BOOL HasFocus() const;

BEGIN_MSG_MAP(CRadAudioMeterWnd)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_SETFOCUS, OnFocus)
    MESSAGE_HANDLER(WM_KILLFOCUS, OnFocus)
    MESSAGE_HANDLER(WM_THEMECHANGED, OnThemeChanged)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
    MESSAGE_HANDLER(WM_DEFAULTDEVICECHANGED, OnDefaultDeviceChanged)
END_MSG_MAP()

private:
    LRESULT OnFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnThemeChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDefaultDeviceChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    void Paint(HDC hdc, const RECT& rcPaint) const;

private:
    IUnknown* m_pDeskBand;
    BOOL m_fHasFocus;
    ATL::CComPtr<CNotificationClient> m_pCom;
    ATL::CComQIPtr<IInputObjectSite> m_spInputObjectSite;

    ATL::CAutoPtr<CVisualStyle> m_ptrVisualStyle;

    ATL::CComPtr<IAudioMeterInformation> m_pAudioMeter;
    std::vector<float> m_peaks;
};
