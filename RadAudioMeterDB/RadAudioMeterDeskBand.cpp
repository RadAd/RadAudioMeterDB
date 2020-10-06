#include "pch.h"
#include "RadAudioMeterDeskBand.h"

CRadAudioMeterDeskBand::CRadAudioMeterDeskBand()
    : m_nBandID(0)
    , m_dwViewMode(DBIF_VIEWMODE_NORMAL)
    , m_bRequiresSave(false)
    , m_bCompositionEnabled(TRUE)
{
}

HRESULT CRadAudioMeterDeskBand::UpdateDeskband()
{
    ATL::CComPtr<IInputObjectSite> spInputSite;
    HRESULT hr = GetSite(IID_IInputObjectSite, reinterpret_cast<void**>(&spInputSite));

    if (SUCCEEDED(hr))
    {
        ATL::CComQIPtr<IOleCommandTarget> spOleCmdTarget;
        spOleCmdTarget = spInputSite;

        if (spOleCmdTarget)
        {
            // m_nBandID must be `int' or bandID variant must be explicitly
            // set to VT_I4, otherwise IDeskBand::GetBandInfo won't
            // be called by the system.
            ATL::CComVariant bandID(m_nBandID);

            hr = spOleCmdTarget->Exec(&CGID_DeskBand, DBID_BANDINFOCHANGED, OLECMDEXECOPT_DODEFAULT, &bandID, NULL);
            ATLASSERT(SUCCEEDED(hr));

            if (SUCCEEDED(hr))
                m_wndRadAudioMeter.Invalidate();
        }
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE CRadAudioMeterDeskBand::SetSite(
    /* [in] */ IUnknown* pUnkSite)
{
    HRESULT hr = __super::SetSite(pUnkSite);

    if (SUCCEEDED(hr) && pUnkSite) // pUnkSite is NULL when band is being destroyed
    {
        ATL::CComQIPtr<IOleWindow> spOleWindow = pUnkSite;

        if (spOleWindow)
        {
            HWND hwndParent = NULL;
            hr = spOleWindow->GetWindow(&hwndParent);

            if (SUCCEEDED(hr))
            {
                m_wndRadAudioMeter.Create(hwndParent, static_cast<IDeskBand*>(this), pUnkSite);

                if (!m_wndRadAudioMeter.IsWindow())
                    hr = E_FAIL;
            }
        }
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE CRadAudioMeterDeskBand::UIActivateIO(
    /* [in] */ BOOL fActivate,
    /* [unique][in] */ PMSG /*pMsg*/)
{
    ATLTRACE(ATL::atlTraceCOM, 2, _T("IInputObject::UIActivateIO (%s)\n"), (fActivate ? _T("TRUE") : _T("FALSE")));

    if (fActivate)
        m_wndRadAudioMeter.SetFocus();

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CRadAudioMeterDeskBand::HasFocusIO()
{
    ATLTRACE(ATL::atlTraceCOM, 2, _T("IInputObject::HasFocusIO\n"));

    return (m_wndRadAudioMeter.HasFocus() ? S_OK : S_FALSE);
}

HRESULT STDMETHODCALLTYPE CRadAudioMeterDeskBand::TranslateAcceleratorIO(
    /* [in] */ PMSG /*pMsg*/)
{
    ATLTRACE(ATL::atlTraceCOM, 2, _T("IInputObject::TranslateAcceleratorIO\n"));

    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE CRadAudioMeterDeskBand::QueryContextMenu(
    /* [in] */ HMENU hMenu,
    /* [in] */ UINT indexMenu,
    /* [in] */ UINT idCmdFirst,
    /* [in] */ UINT /*idCmdLast*/,
    /* [in] */ UINT uFlags)
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CRadAudioMeterDeskBand::InvokeCommand(
    /* [in] */ LPCMINVOKECOMMANDINFO pici)
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CRadAudioMeterDeskBand::GetCommandString(
    /* [in] */ UINT_PTR /*idCmd*/,
    /* [in] */ UINT /*uType*/,
    /* [in] */ UINT* /*pReserved*/,
    /* [out] */ LPSTR /*pszName*/,
    /* [in] */ UINT /*cchMax*/)
{
    ATLTRACE(ATL::atlTraceCOM, 2, _T("IContextMenu::GetCommandString\n"));

    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// IDeskBand

HRESULT STDMETHODCALLTYPE CRadAudioMeterDeskBand::GetBandInfo(
    /* [in] */ DWORD dwBandID,
    /* [in] */ DWORD dwViewMode,
    /* [out][in] */ DESKBANDINFO* pdbi)
{
    ATLTRACE(ATL::atlTraceCOM, 2, _T("IDeskBand::GetBandInfo\n"));

    if (!pdbi) return E_INVALIDARG;

    m_nBandID = dwBandID;
    m_dwViewMode = dwViewMode;

    if (pdbi->dwMask & DBIM_MODEFLAGS)
    {
        pdbi->dwModeFlags = DBIMF_VARIABLEHEIGHT;
    }

    if (pdbi->dwMask & DBIM_MINSIZE)
    {
        pdbi->ptMinSize = m_wndRadAudioMeter.CalcIdealSize();
    }

    if (pdbi->dwMask & DBIM_MAXSIZE)
    {
        // the band object has no limit for its maximum height
        pdbi->ptMaxSize.x = -1;
        pdbi->ptMaxSize.y = -1;
    }

    if (pdbi->dwMask & DBIM_INTEGRAL)
    {
        pdbi->ptIntegral.x = 1;
        pdbi->ptIntegral.y = 1;
    }

    if (pdbi->dwMask & DBIM_ACTUAL)
    {
        pdbi->ptActual = m_wndRadAudioMeter.CalcIdealSize();
    }

    if (pdbi->dwMask & DBIM_TITLE)
    {
        if (dwViewMode == DBIF_VIEWMODE_FLOATING)
        {
            ATL::CString sDate;
            ATLVERIFY(sDate.LoadString(IDS_TITLE));
            lstrcpynW(pdbi->wszTitle, sDate, ARRAYSIZE(pdbi->wszTitle));
        }
        else
            pdbi->dwMask &= ~DBIM_TITLE; // do not show title
    }

    if (pdbi->dwMask & DBIM_BKCOLOR)
    {
        //Use the default background color by removing this flag.
        pdbi->dwMask &= ~DBIM_BKCOLOR;
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CRadAudioMeterDeskBand::CanRenderComposited(
    /* [out] */ BOOL* pfCanRenderComposited)
{
    if (pfCanRenderComposited)
        *pfCanRenderComposited = TRUE;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CRadAudioMeterDeskBand::SetCompositionState(
    /* [in] */ BOOL fCompositionEnabled)
{
    m_bCompositionEnabled = fCompositionEnabled;
    m_wndRadAudioMeter.Invalidate();

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CRadAudioMeterDeskBand::GetCompositionState(
    /* [out] */ BOOL* pfCompositionEnabled)
{
    if (pfCompositionEnabled)
        *pfCompositionEnabled = m_bCompositionEnabled;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CRadAudioMeterDeskBand::GetWindow(
    /* [out] */ HWND* phwnd)
{
    ATLTRACE(ATL::atlTraceCOM, 2, _T("IOleWindow::GetWindow\n"));

    if (phwnd) *phwnd = m_wndRadAudioMeter;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CRadAudioMeterDeskBand::ContextSensitiveHelp(
    /* [in] */ BOOL /*fEnterMode*/)
{
    ATLTRACE(ATL::atlTraceCOM, 2, _T("IOleWindow::ContextSensitiveHelp\n"));

    ATLTRACENOTIMPL("IOleWindow::ContextSensitiveHelp");
}

HRESULT STDMETHODCALLTYPE CRadAudioMeterDeskBand::ShowDW(
    /* [in] */ BOOL fShow)
{
    ATLTRACE(ATL::atlTraceCOM, 2, _T("IDockingWindow::ShowDW\n"));

    if (m_wndRadAudioMeter)
        m_wndRadAudioMeter.ShowWindow(fShow ? SW_SHOW : SW_HIDE);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CRadAudioMeterDeskBand::CloseDW(
    /* [in] */ DWORD /*dwReserved*/)
{
    ATLTRACE(ATL::atlTraceCOM, 2, _T("IDockingWindow::CloseDW\n"));

    if (m_wndRadAudioMeter)
    {
        m_wndRadAudioMeter.ShowWindow(SW_HIDE);
        m_wndRadAudioMeter.DestroyWindow();
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CRadAudioMeterDeskBand::ResizeBorderDW(
    /* [in] */ LPCRECT prcBorder,
    /* [in] */ IUnknown* punkToolbarSite,
    /* [in] */ BOOL /*fReserved*/)
{
    ATLTRACE(ATL::atlTraceCOM, 2, _T("IDockingWindow::ResizeBorderDW\n"));

    if (!m_wndRadAudioMeter) return S_OK;

    ATL::CComQIPtr<IDockingWindowSite> spDockingWindowSite = punkToolbarSite;

    if (spDockingWindowSite)
    {
        BORDERWIDTHS bw = { 0 };
        bw.top = bw.bottom = ::GetSystemMetrics(SM_CYBORDER);
        bw.left = bw.right = ::GetSystemMetrics(SM_CXBORDER);

        HRESULT hr = spDockingWindowSite->RequestBorderSpaceDW(
            static_cast<IDeskBand*>(this), &bw);

        if (SUCCEEDED(hr))
        {
            HRESULT hr = spDockingWindowSite->SetBorderSpaceDW(
                static_cast<IDeskBand*>(this), &bw);

            if (SUCCEEDED(hr))
            {
                m_wndRadAudioMeter.MoveWindow(prcBorder);
                return S_OK;
            }
        }
    }

    return E_FAIL;
}
