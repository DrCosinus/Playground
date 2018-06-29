#include "cuppa.hpp"

#include <cstdlib>
#include <malloc.h>
#include <windows.h>

#include <memory>
#include <stack>

#pragma warning(push)
#pragma warning(disable:4458)
#include <gdiplus.h>
#pragma warning(pop)
// sample GDI+: https://codes-sources.commentcamarche.net/source/view/29875/966776#browser

template<auto N>
void Assert(bool _condition, const char (&_message)[N])
{
    if(!_condition)
    {
        MessageBox(NULL, _message, "Assertion error!", MB_ICONEXCLAMATION | MB_OK);
        exit(1);
        return;
    }
}

void Assert(bool _condition)
{
    Assert(_condition, "No description");
}

static constexpr char windowClassName[] = "cuppaWindowClass";

template<typename GRAPHICS_DRIVER_TYPE>
struct MSWindowsDriver
{
    void Setup(cuppa::app& _app)
    {
        app_ = &_app;
        instance = this;
        auto hInstance = GetModuleHandle(NULL);
        RegisterWindowClass(hInstance);
        CreateAppWindow(hInstance);
    }

    void SetWindowSize(unsigned int _width, unsigned int _height)
    {
        // for now, we only handle initial width and height
        // TODO: handle SetWindowSize after the creation of the windows
        width = _width;
        height = _height;
    }

    auto& GetGraphicsDriver() { return GraphicsDriver; }

private:
    void Draw()
    {
        PAINTSTRUCT ps;
        auto hdc = BeginPaint(hWnd_, &ps);
        GraphicsDriver.Draw(*app_, hWnd_, hdc);
        EndPaint(hWnd_, &ps);
    }

    void RefreshWindow(HWND _hWnd)
    {
        InvalidateRect(_hWnd, nullptr, FALSE);
        UpdateWindow(_hWnd);
    }

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch(msg)
        {
            case WM_CREATE:
                if (instance)
                {
                    instance->GraphicsDriver.Init();
                }
                SetTimer(hwnd, 1234, 16, (TIMERPROC)0);
                break;
            case WM_TIMER:
            // TODO: check the TIMER_ID
                instance->RefreshWindow(hwnd);
                break;
            case WM_PAINT:
                instance->Draw();
                break;
            case WM_CLOSE:
                DestroyWindow(hwnd);
                break;
            case WM_DESTROY:
                PostQuitMessage(0);
                break;
            default:
                return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        return 0;
    }

    void RegisterWindowClass(HINSTANCE hInstance)
    {
        WNDCLASSEX wc;
        wc.cbSize        = sizeof(WNDCLASSEX);
        wc.style         = 0;
        wc.lpfnWndProc   = WndProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = hInstance;
        wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = windowClassName;
        wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

        if(!RegisterClassEx(&wc))
        {
            MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
            return;
        }
    }

    void CreateAppWindow(HINSTANCE hInstance)
    {
        hWnd_ = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            windowClassName,
            "Cuppa",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, width, height,
            NULL, NULL, hInstance, NULL);
        if (hWnd_ == NULL)
        {
            MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
            return;
        }
        ShowWindow(hWnd_, SW_NORMAL);
        UpdateWindow(hWnd_);
    }

    cuppa::app* app_;

    inline static MSWindowsDriver* instance = nullptr;
    GRAPHICS_DRIVER_TYPE GraphicsDriver;
    HWND hWnd_ { 0 };
    unsigned int width = 240, height = 120;
};

struct GdiplusDriver
{
    void Init()
    {
        Gdiplus::GdiplusStartup(&token, &startupInput, NULL);
        fillBrush_.reset( new SolidBrush{ fillColor_ } );
        stroke_.reset( new Pen{ strokeColor_, strokeWeight_ } );

        font_ = new Font(L"Verdana", 10, Gdiplus::FontStyle::FontStyleRegular, Gdiplus::Unit::UnitPoint);
    }

    void Draw(cuppa::app& _app, HWND _hWnd, HDC _hdc)
    {
        Assert(graphics_==nullptr);
        Graphics graphics{ _hdc };
        RECT rc;
        GetClientRect(_hWnd, &rc);
        auto rcWidth = static_cast<UINT>( rc.right-rc.left );
        auto rcHeight = static_cast<UINT>( rc.bottom-rc.top );
        if (drawBuffer_==nullptr || drawBuffer_->GetWidth() != rcWidth || drawBuffer_->GetHeight() != rcHeight)
        {
            drawBuffer_.reset(new Bitmap(rcWidth, rcHeight, PixelFormat32bppPARGB));
        }
        Graphics drawGraphics{ drawBuffer_.get() };
        graphics_ = &drawGraphics;
        drawGraphics.SetInterpolationMode(Gdiplus::InterpolationMode::InterpolationModeHighQuality);
        drawGraphics.SetCompositingMode(Gdiplus::CompositingMode::CompositingModeSourceOver);
        drawGraphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighQuality);

        _app.draw();
        graphics.SetInterpolationMode(Gdiplus::InterpolationMode::InterpolationModeLowQuality);
        graphics.SetCompositingMode(Gdiplus::CompositingMode::CompositingModeSourceCopy);
        graphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighSpeed);
        graphics.DrawImage( drawBuffer_.get(), 0, 0 );
        graphics_ = nullptr;
    }

    void resetMatrix()
    {
        Matrix identity;
        graphics_->SetTransform(&identity);
    }

    void pushMatrix()
    {
        matrixStack.emplace();
        graphics_->GetTransform(&matrixStack.top());
    }

    bool popMatrix()
    {
        if (!matrixStack.empty())
        {
            graphics_->SetTransform(&matrixStack.top());
            matrixStack.pop();
            return true;
        }
        else
        {
            return false;
        }
    }

    void translate(float xmove, float ymove, float)
    {
        graphics_->TranslateTransform(xmove, ymove, Gdiplus::MatrixOrder::MatrixOrderAppend);
    }

    void rotate(float angle_in_degree)
    {
        graphics_->RotateTransform(angle_in_degree, Gdiplus::MatrixOrder::MatrixOrderAppend);
    }

    void scale(float xscale, float yscale, float)
    {
        graphics_->ScaleTransform(xscale, yscale, Gdiplus::MatrixOrder::MatrixOrderAppend);
    }

    void shearX(float slope)
    {
        Matrix mxTransform{ 1, 0, slope, 1, 0, 0 };
        graphics_->MultiplyTransform( &mxTransform, Gdiplus::MatrixOrder::MatrixOrderAppend);
    }

    void shearY(float slope)
    {
        Matrix mxTransform{ 1, slope, 0, 1, 0, 0 };
        graphics_->MultiplyTransform(&mxTransform, Gdiplus::MatrixOrder::MatrixOrderAppend);
    }

    void background(unsigned int _red, unsigned int _green, unsigned int _blue, unsigned int _alpha)
    {
        auto clearColor_ = Color{ static_cast<BYTE>(_alpha), static_cast<BYTE>(_red), static_cast<BYTE>(_green), static_cast<BYTE>(_blue) };
        graphics_->Clear(clearColor_);
    }

    void noStroke()
    {
        strokeEnabled_ = false;
    }

    void stroke(unsigned int _red, unsigned int _green, unsigned int _blue, unsigned int _alpha)
    {
        strokeEnabled_ = true;
        strokeColor_ = Color{ static_cast<BYTE>(_alpha), static_cast<BYTE>(_red), static_cast<BYTE>(_green), static_cast<BYTE>(_blue) };
        if (strokeEnabled_)
            stroke_.reset( new Pen{ strokeColor_ });
    }

    void strokeWeight(float _thickness)
    {
        strokeEnabled_ = true;
        strokeWeight_ = _thickness;
        stroke_->SetWidth(_thickness);
    }

    void noFill()
    {
        fillEnabled_ = false;
    }

    void fill(unsigned int _red, unsigned int _green, unsigned int _blue, unsigned int _alpha)
    {
        fillEnabled_ = true;
        fillColor_ = Color{ static_cast<BYTE>(_alpha), static_cast<BYTE>(_red), static_cast<BYTE>(_green), static_cast<BYTE>(_blue) },
        fillBrush_.reset( new SolidBrush( fillColor_ ) );
    }

    void rectangle(int _centerX, int _centerY, int _width, int _height)
    {
        Rect rc{ _centerX - _width / 2, _centerY - _height / 2, _width, _height };
        if (fillEnabled_)
            graphics_->FillRectangle( fillBrush_.get(), rc );
        if (strokeEnabled_)
            graphics_->DrawRectangle( stroke_.get(), rc );
    }

    void ellipse(int _centerX, int _centerY, int _width, int _height)
    {
        Rect rc{ _centerX - _width / 2, _centerY - _height / 2, _width, _height };
        if (fillEnabled_)
            graphics_->FillEllipse( fillBrush_.get(), rc );
        if (strokeEnabled_)
            graphics_->DrawEllipse( stroke_.get(), rc );
    }

    void text(const char* c, int x, int y)
    {
        auto len = strlen(c);
        auto wcs = (WCHAR*)_malloca((len+1)*sizeof(WCHAR));
        decltype(len) ret;
        mbstowcs_s(&ret, wcs, len + 1, c, len+1);
        graphics_->DrawString(wcs, static_cast<INT>(len), font_, PointF{static_cast<REAL>(x), static_cast<REAL>(y)}, fillBrush_.get());
    }

    void point(cuppa::Point2D pt)
    {
        RectF rc{ pt.x.GetFloat()-0.5f, pt.y.GetFloat()-0.5f, 1, 1 };
        SolidBrush b{ strokeColor_ };
        graphics_->FillRectangle( &b, rc );
    }

    void line(int x1, int y1, int /*z1*/, int x2, int y2, int /*z2*/)
    {
        if (strokeEnabled_)
            graphics_->DrawLine( stroke_.get(), x1, y1, x2, y2 );
    }

    using ArcMode = cuppa::app::ArcMode;
    void arc(int x, int y, int width, int height, float start_angle, float end_angle, ArcMode mode)
    {
        Rect rc{ x - width / 2, y - height / 2, width, height };
        auto sweep_angle = end_angle - start_angle;
        if (mode==ArcMode::PIE)
        {
            if (fillEnabled_)
                graphics_->FillPie( fillBrush_.get(), rc, start_angle, sweep_angle );
            if (strokeEnabled_)
                graphics_->DrawPie( stroke_.get(), rc, start_angle, sweep_angle );
        }
        else
        {
            GraphicsPath path;

            path.AddArc(rc, start_angle, sweep_angle);
            if (mode==ArcMode::CHORD)
            {
                path.CloseFigure();
            }
            if (fillEnabled_)
                graphics_->FillPath( fillBrush_.get(), &path );
            if (strokeEnabled_)
                graphics_->DrawPath( stroke_.get(), &path);
        }
    }

    void quad(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
    {
        GraphicsPath path;

        path.AddLine( x1, y1, x2, y2 );
        path.AddLine( x3, y3, x4, y4 );
        path.CloseFigure();

        if (fillEnabled_)
            graphics_->FillPath( fillBrush_.get(), &path );
        if (strokeEnabled_)
            graphics_->DrawPath( stroke_.get(), &path);
    }

    void triangle(int x1, int y1, int x2, int y2, int x3, int y3)
    {
        GraphicsPath path;

        path.AddLine( x1, y1, x2, y2 );
        path.AddLine( x2, y2, x3, y3 );
        path.CloseFigure();

        if (fillEnabled_)
            graphics_->FillPath( fillBrush_.get(), &path );
        if (strokeEnabled_)
            graphics_->DrawPath( stroke_.get(), &path);
    }

private:
    using Graphics = Gdiplus::Graphics;
    using Bitmap = Gdiplus::Bitmap;
    using REAL = Gdiplus::REAL;
    using ARGB = Gdiplus::ARGB;
    using Matrix = Gdiplus::Matrix;
    using Color = Gdiplus::Color;
    using Pen = Gdiplus::Pen;
    using Brush = Gdiplus::Brush;
    using SolidBrush = Gdiplus::SolidBrush;
    using Point = Gdiplus::Point;
    using PointF = Gdiplus::PointF;
    using Rect = Gdiplus::Rect;
    using RectF = Gdiplus::RectF;
    using Font = Gdiplus::Font;
    using FontFamily = Gdiplus::FontFamily;
    using GraphicsPath = Gdiplus::GraphicsPath;

    Graphics*               graphics_ = nullptr;
    std::unique_ptr<Bitmap> drawBuffer_;

    std::unique_ptr<Brush>  fillBrush_;
    Color                   fillColor_ = (ARGB)Color::White;
    bool                    fillEnabled_ = true;

    std::unique_ptr<Pen>    stroke_;
    float                   strokeWeight_ = 1.0f;
    Color                   strokeColor_ = (ARGB)Color::Black;
    bool                    strokeEnabled_ = true;

    std::stack<Matrix>      matrixStack;

    Font*                   font_ = nullptr;

    ULONG_PTR                       token;
    Gdiplus::GdiplusStartupInput    startupInput;
};

static MSWindowsDriver<GdiplusDriver> SystemDriver;

namespace cuppa
{
    void app::run()
    {
        // __argc, __argv
        setup();
        SystemDriver.Setup(*this);
        MSG Msg;
        while(GetMessage(&Msg, NULL, 0, 0) > 0)
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
            update();
        }
    }

    void app::background(unsigned int _red, unsigned int _green, unsigned int _blue, unsigned int _alpha)
    {
        SystemDriver.GetGraphicsDriver().background(_red, _green, _blue, _alpha);
    }

    void app::size(unsigned int _width, unsigned int _height)   {   SystemDriver.SetWindowSize(_width, _height);    }

    void app::noStroke()                                        {   SystemDriver.GetGraphicsDriver().noStroke();    }

    void app::stroke(unsigned int _red, unsigned int _green, unsigned int _blue, unsigned int _alpha)
    {
        SystemDriver.GetGraphicsDriver().stroke(_red, _green, _blue, _alpha);
    }

    void app::strokeWeight(float _thickness)    {   SystemDriver.GetGraphicsDriver().strokeWeight(_thickness);  }

    void app::noFill()                          {   SystemDriver.GetGraphicsDriver().noFill();  }

    void app::fill(unsigned int _red, unsigned int _green, unsigned int _blue, unsigned int _alpha)
    {
        SystemDriver.GetGraphicsDriver().fill(_red, _green, _blue, _alpha);
    }

    void app::rect(int _centerX, int _centerY, unsigned int _width, unsigned int _height)
    {
        SystemDriver.GetGraphicsDriver().rectangle( _centerX, _centerY, _width, _height );
    }

    void app::ellipse(int _centerX, int _centerY, unsigned int _width, unsigned int _height) const
    {
        SystemDriver.GetGraphicsDriver().ellipse( _centerX, _centerY, _width, _height );
    }

    void app::text(const char* c, int x, int y)     {   SystemDriver.GetGraphicsDriver().text( c, x, y);    }

    void app::point(Point2D pt)                     {   SystemDriver.GetGraphicsDriver().point(pt);    }

    void app::line(int x1, int y1, int x2, int y2)  {   SystemDriver.GetGraphicsDriver().line( x1, y1, 0, x2, y2, 0);   }

    void app::arc(int x, int y, int width, int height, float start_angle, float end_angle, ArcMode mode)
    {
        SystemDriver.GetGraphicsDriver().arc( x, y, width, height, start_angle / PI * 180, end_angle / PI * 180, mode);
    }

    void app::quad(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
    {
        SystemDriver.GetGraphicsDriver().quad( x1, y1, x2, y2, x3, y3, x4, y4);
    }

    void app::triangle(int x1, int y1, int x2, int y2, int x3, int y3)
    {
        SystemDriver.GetGraphicsDriver().triangle( x1, y1, x2, y2, x3, y3);
    }

    void app::pushMatrix()  { SystemDriver.GetGraphicsDriver().pushMatrix(); }
    void app::popMatrix()   { SystemDriver.GetGraphicsDriver().popMatrix(); }
    void app::resetMatrix() { SystemDriver.GetGraphicsDriver().resetMatrix(); }

    void app::translate(float xmove, float ymove, float zmove)
    {
        SystemDriver.GetGraphicsDriver().translate(xmove, ymove, zmove);
    }

    void app::rotate(float angle)
    {
        SystemDriver.GetGraphicsDriver().rotate(angle / PI * 180);
    }

    void app::scale(float xscale, float yscale, float zscale)
    {
        SystemDriver.GetGraphicsDriver().scale(xscale, yscale, zscale);
    }

    void app::shearX(float angle) { SystemDriver.GetGraphicsDriver().shearX(angle); }
    void app::shearY(float angle) { SystemDriver.GetGraphicsDriver().shearY(angle);}
} // namespace cuppa