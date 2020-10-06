#pragma once

#include <memory>

inline LONG Height(const RECT& rc)
{
    return rc.bottom - rc.top;
}

inline LONG Width(const RECT& rc)
{
    return rc.right - rc.left;
}

inline BOOL WINAPI Rectangle(_In_ HDC hdc, _In_ const RECT& rc)
{
    return Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
}

template <class T>
class GDIDeleter
{
public:
    typedef T pointer;
    void operator()(T hObject)
    {
        DeleteObject(hObject);
    }
};

template <class T>
class GDIPtr : public std::unique_ptr<T, GDIDeleter<T>>
{
public:
    using std::unique_ptr<T, GDIDeleter<T>>::unique_ptr;

    operator T() const { return std::unique_ptr<T, GDIDeleter<T>>::get(); }
};

class SelectGdiObject
{
public:
    SelectGdiObject(HDC hDC, HGDIOBJ hGdiObj) :
        m_hDC(hDC),
        m_hGdiObj(hGdiObj),
        m_hSaveGdiObj(::SelectObject(hDC, hGdiObj))
    {
        ATLASSERT(m_hSaveGdiObj && m_hSaveGdiObj != HGDI_ERROR);
    }

    ~SelectGdiObject()
    {
        if (m_hSaveGdiObj && m_hSaveGdiObj != HGDI_ERROR)
            ::SelectObject(m_hDC, m_hSaveGdiObj);
    }

    operator HGDIOBJ() const { return m_hGdiObj; }

private:
    HDC m_hDC;
    HGDIOBJ m_hGdiObj;
    HGDIOBJ m_hSaveGdiObj;

private:
    // not copyable
    SelectGdiObject(const SelectGdiObject&);
    SelectGdiObject& operator=(const SelectGdiObject&);
};

class DoubleBufferPaint
{
public:
    DoubleBufferPaint(HWND hWnd, DWORD dwRop = SRCCOPY) : m_hWnd(hWnd), m_dwRop(dwRop)
    {
        m_hdc = ::BeginPaint(m_hWnd, &m_ps);

        int x, y, cx, cy;
        InitDims(&x, &y, &cx, &cy);

        m_hdcMem = ::CreateCompatibleDC(m_hdc);
        m_hbmMem = ::CreateCompatibleBitmap(m_hdc, cx, cy);
        m_hbmSave = ::SelectObject(m_hdcMem, m_hbmMem);
        ::SetWindowOrgEx(m_hdcMem, x, y, NULL);
    }

    ~DoubleBufferPaint()
    {
        int x, y, cx, cy;
        InitDims(&x, &y, &cx, &cy);

        ::BitBlt(m_hdc, x, y, cx, cy, m_hdcMem, x, y, m_dwRop);

        ::SelectObject(m_hdcMem, m_hbmSave);
        ::DeleteObject(m_hbmMem);
        ::DeleteDC(m_hdcMem);

        ::EndPaint(m_hWnd, &m_ps);
    }

    HDC GetDC() const { return m_hdcMem; }
    const RECT& GetPaintRect() const { return m_ps.rcPaint; }

private:
    void InitDims(int* px, int* py, int* pcx, int* pcy) const
    {
        *px = m_ps.rcPaint.left;
        *py = m_ps.rcPaint.top;
        *pcx = m_ps.rcPaint.right - m_ps.rcPaint.left;
        *pcy = m_ps.rcPaint.bottom - m_ps.rcPaint.top;
    }

private:
    HWND m_hWnd;
    DWORD m_dwRop;

    HDC m_hdc;
    HDC m_hdcMem;
    HGDIOBJ m_hbmMem;
    HGDIOBJ m_hbmSave;
    PAINTSTRUCT m_ps;

private:
    // not copyable
    DoubleBufferPaint(const DoubleBufferPaint&);
    DoubleBufferPaint& operator=(const DoubleBufferPaint&);
};
