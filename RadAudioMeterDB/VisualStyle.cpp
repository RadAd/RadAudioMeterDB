#include "pch.h"
#include "VisualStyle.h"

class CClassicVisualStyle : public CVisualStyle
{
public:
    CClassicVisualStyle()
    {
        CreateFont();
        CreateTextColor();
    }

    virtual void DrawBackground(HWND /*hWnd*/, HDC hdc, const RECT& rcPaint) const
    {
        ::FillRect(hdc, &rcPaint, ::GetSysColorBrush(CTLCOLOR_DLG));
    }

private:
    bool CreateFont()
    {
        NONCLIENTMETRICS ncm = { 0 };
        ncm.cbSize = sizeof(NONCLIENTMETRICS);

        if(::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0))
            m_hFont = ::CreateFontIndirect(&ncm.lfMessageFont);

        ATLASSERT(m_hFont);

        return (m_hFont != NULL);
    }

    bool CreateTextColor()
    {
        m_clrText = ::GetSysColor(COLOR_MENUTEXT);

        return true;
    }
};

class CThemedVisualStyle : public CVisualStyle
{
public:
    CThemedVisualStyle()
    {
        CreateFont();
        CreateTextColor();
    }

    virtual void DrawBackground(HWND hWnd, HDC hdc, const RECT& rcPaint) const
    {
        ::DrawThemeParentBackground(hWnd, hdc, &rcPaint);
    }

private:
    bool CreateFont()
    {
        HTHEME hTheme = ::OpenThemeData(NULL, VSCLASS_REBAR);
        ATLASSERT(hTheme);

        if(hTheme)
        {
            LOGFONT lf = { 0 };
            const HRESULT hr = ::GetThemeFont(hTheme,
                NULL, RP_BAND, 0, TMT_FONT, &lf);
            ATLASSERT(SUCCEEDED(hr));

            if(SUCCEEDED(hr))
            {
                m_hFont = ::CreateFontIndirect(&lf);
                ATLASSERT(m_hFont);
            }

            ::CloseThemeData(hTheme);
        }

        return (m_hFont != NULL);
    }

    bool CreateTextColor()
    {
        bool bRet = false;
        HTHEME hTheme = ::OpenThemeData(NULL, VSCLASS_TASKBAND);
        ATLASSERT(hTheme);

        if(hTheme)
        {
            const HRESULT hr = ::GetThemeColor(hTheme,
                TDP_GROUPCOUNT, 0, TMT_TEXTCOLOR, &m_clrText);
            ATLASSERT(SUCCEEDED(hr));

            ::CloseThemeData(hTheme);
            bRet = SUCCEEDED(hr);
        }

        return bRet;
    }
};


CVisualStyle::~CVisualStyle()
{
    if(m_hFont) ::DeleteObject(m_hFont);
}

CVisualStyle* CVisualStyle::Create(VisualStyle vs)
{
    CVisualStyle* pVS = NULL;

    if(vs == VisualStyle::Auto) vs = (::IsAppThemed() ? VisualStyle::Themed : VisualStyle::Classic);

    switch(vs)
    {
    case VisualStyle::Classic:
        pVS = new CClassicVisualStyle;
        break;
    case VisualStyle::Themed:
        pVS = new CThemedVisualStyle;
        break;
    default:
        ATLASSERT(FALSE);
    }

    return pVS;
}