#include "pch.h"
#include "RadAudioMeterWnd.h"
#include "GdiUtils.h"
#include "VisualStyle.h"

#include <string>
#include <algorithm>

#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <Functiondiscoverykeys_devpkey.h>

extern HINSTANCE g_hInstance;

namespace
{
    ATL::CComPtr<IAudioMeterInformation> GetAudioMeter(std::wstring& device, IMMNotificationClient* pClient)
    {
        // Get enumerator for audio endpoint devices.
        printf("Discovering audio devices...\n");
        ATL::CComPtr<IMMDeviceEnumerator> pEnumerator;
        ATLENSURE_RETURN_VAL(SUCCEEDED(pEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER)), nullptr);

        if (pClient != nullptr)
            pEnumerator->RegisterEndpointNotificationCallback(pClient);

        // Get default audio-rendering device.
        ATL::CComPtr<IMMDevice> pDevice;
        ATLENSURE_RETURN_VAL(SUCCEEDED(pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice)), nullptr);

        // Open device properties
        ATL::CComPtr<IPropertyStore> pProps;
        ATLENSURE_RETURN_VAL(SUCCEEDED(pDevice->OpenPropertyStore(STGM_READ, &pProps)), nullptr);

        // Get the device's friendly name
        PROPVARIANT varName;
        PropVariantInit(&varName);
        ATLENSURE_RETURN_VAL(SUCCEEDED(pProps->GetValue(PKEY_Device_FriendlyName, &varName)), nullptr);

        device = varName.pwszVal;
        PropVariantClear(&varName);

        ATL::CComPtr<IAudioMeterInformation> pMeterInfo;
        ATLENSURE_RETURN_VAL(SUCCEEDED(pDevice->Activate(
            __uuidof(IAudioMeterInformation),
            CLSCTX_ALL,
            NULL,
            (void**)&pMeterInfo)), nullptr);

        return pMeterInfo;
    }
};

CRadAudioMeterWnd::CRadAudioMeterWnd()
    : m_pDeskBand(NULL)
    , m_fHasFocus(FALSE)
    , m_ptrVisualStyle(CVisualStyle::Create())
{
}

CRadAudioMeterWnd::~CRadAudioMeterWnd()
{
}

BOOL CRadAudioMeterWnd::Create(
    HWND hwndParent,
    LPUNKNOWN pDeskBand,
    LPUNKNOWN pInputObjectSite)
{
    if (!__super::Create(hwndParent))
        return FALSE;

    ATLASSERT(pDeskBand);
    m_pDeskBand = pDeskBand;

    ATLASSERT(pInputObjectSite);
    m_spInputObjectSite = pInputObjectSite;

    ATL::CComObject<CNotificationClient>* pCom;
    ATLVERIFY(SUCCEEDED(ATL::CComObject<CNotificationClient>::CreateInstance(&pCom)));
    m_pCom = pCom;
    m_pCom->Init(m_hWnd);

    std::wstring device;
    m_pAudioMeter = GetAudioMeter(device, m_pCom);
    UINT count = 0;
    ATLVERIFY(SUCCEEDED(m_pAudioMeter->GetMeteringChannelCount(&count)));
    m_peaks.resize(count);
    SetTimer(1, 100, nullptr);

    return TRUE;
}

POINTL CRadAudioMeterWnd::CalcMinimalSize() const
{
    const POINTL pt = {
        ::GetSystemMetrics(SM_CXMIN),
        ::GetSystemMetrics(SM_CYMIN)
    };

    return pt;
}

POINTL CRadAudioMeterWnd::CalcIdealSize() const
{
    if (!IsWindow()) return CalcMinimalSize();

    const POINTL pt = {
        15,
        ::GetSystemMetrics(SM_CYMIN)
    };

    return pt;
}

BOOL CRadAudioMeterWnd::HasFocus() const
{
    return m_fHasFocus;
}

LRESULT CRadAudioMeterWnd::OnFocus(
    UINT uMsg,
    WPARAM /*wParam*/,
    LPARAM /*lParam*/,
    BOOL& /*bHandled*/)
{
    m_fHasFocus = (uMsg == WM_SETFOCUS);

    if (m_spInputObjectSite)
        m_spInputObjectSite->OnFocusChangeIS(m_pDeskBand, m_fHasFocus);

    return 0L;
}

LRESULT CRadAudioMeterWnd::OnDestroy(
    UINT /*uMsg*/,
    WPARAM /*wParam*/,
    LPARAM /*lParam*/,
    BOOL& /*bHandled*/)
{
    m_pAudioMeter.Release();

    return 0;
}

LRESULT CRadAudioMeterWnd::OnEraseBackground(
    UINT /*uMsg*/,
    WPARAM /*wParam*/,
    LPARAM /*lParam*/,
    BOOL& /*bHandled*/)
{
    return 1;
}

LRESULT CRadAudioMeterWnd::OnPaint(
    UINT /*uMsg*/,
    WPARAM /*wParam*/,
    LPARAM /*lParam*/,
    BOOL& /*bHandled*/)
{
    DoubleBufferPaint dbuffPaint(m_hWnd);

    Paint(dbuffPaint.GetDC(), dbuffPaint.GetPaintRect());

    return 0;
}

LRESULT CRadAudioMeterWnd::OnThemeChanged(
    UINT /*uMsg*/,
    WPARAM /*wParam*/,
    LPARAM /*lParam*/,
    BOOL& /*bHandled*/)
{
    // re-create theme style
    m_ptrVisualStyle.Free();
    m_ptrVisualStyle.Attach(CVisualStyle::Create());

    return 0;
}

LRESULT CRadAudioMeterWnd::OnTimer(
    UINT /*uMsg*/,
    WPARAM /*wParam*/,
    LPARAM /*lParam*/,
    BOOL& /*bHandled*/)
{
    if (!m_peaks.empty())
    {
        ATLVERIFY(SUCCEEDED(m_pAudioMeter->GetChannelsPeakValues((UINT32)m_peaks.size(), m_peaks.data())));
        Invalidate();
    }

    return 0;
}

LRESULT CRadAudioMeterWnd::OnDefaultDeviceChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    std::wstring device;
    m_pAudioMeter = GetAudioMeter(device, nullptr);
    UINT count = 0;
    ATLVERIFY(SUCCEEDED(m_pAudioMeter->GetMeteringChannelCount(&count)));
    m_peaks.resize(count);

    return 0;
}

void CRadAudioMeterWnd::Paint(HDC hdc, const RECT& rcPaint) const
{
    m_ptrVisualStyle->DrawBackground(m_hWnd, hdc, rcPaint);

    if (!m_peaks.empty())
    {
        RECT rcClient = { 0 };
        GetClientRect(&rcClient);
        //InflateRect(&rcClient, 0, -5);

        GDIPtr<HPEN> hBorderPen(GetStockPen(WHITE_PEN));
        GDIPtr<HBRUSH> hEnabledBrush(CreateSolidBrush(RGB(255, 255, 255)));
        GDIPtr<HBRUSH> hDisabledBrush(CreateSolidBrush(RGB(128, 128, 128)));

        const LONG spacing = 1;
        const LONG w = (Width(rcClient) + spacing) / (LONG)m_peaks.size();

        SelectObject(hdc, hBorderPen);

        RECT rcBox(rcClient);
        rcBox.right = rcBox.left + w - spacing;
        for (float f : m_peaks)
        {
            SelectObject(hdc, hEnabledBrush);
            RECT rcBoxF(rcBox);
            rcBoxF.top = rcBoxF.bottom - (LONG)(f * Height(rcBoxF) + 0.5);
            Rectangle(hdc, rcBoxF);

            OffsetRect(&rcBox, w, 0);
        }

        SelectObject(hdc, GetStockObject(BLACK_PEN));
        SelectObject(hdc, GetSysColorBrush(COLOR_WINDOW));
    }
}
