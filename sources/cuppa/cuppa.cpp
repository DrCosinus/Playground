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

using namespace cuppa;

static constexpr char windowClassName[] = "cuppaWindowClass";

template<typename GRAPHICS_DRIVER_TYPE>
struct MSWindowsDriver
{
    void Setup(app& _app)
    {
        app_ = &_app;
        instance = this;
        auto hInstance = GetModuleHandle(NULL);
        RegisterWindowClass(hInstance);
        CreateAppWindow(hInstance);
    }

    void SetWindowSize(Meter _width, Meter _height)
    {
        // for now, we only handle initial width and height
        // TODO: handle SetWindowSize after the creation of the windows
        width = _width;
        height = _height;
    }
    Meter GetWindowWidth() const { return width; }
    Meter GetWindowHeight() const { return height; }

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
            CW_USEDEFAULT, CW_USEDEFAULT, width.getAs<int>(), height.getAs<int>(),
            NULL, NULL, hInstance, NULL);
        if (hWnd_ == NULL)
        {
            MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
            return;
        }
        ShowWindow(hWnd_, SW_NORMAL);
        UpdateWindow(hWnd_);
    }

    app* app_;

    inline static MSWindowsDriver* instance = nullptr;
    GRAPHICS_DRIVER_TYPE GraphicsDriver;
    HWND hWnd_ { 0 };
    Meter width = 240_m, height = 120_m;
};

namespace cuppa
{
    struct GdiplusDriver
    {
        void Init()
        {
            Gdiplus::GdiplusStartup(&token, &startupInput, NULL);
            fillBrush_.reset( new SolidBrush{ fillColor_ } );
            stroke_.reset( new Pen{ strokeColor_, strokeWeight_.getAs<float>() } );

            font_ = new Font(L"Verdana", 10, Gdiplus::FontStyle::FontStyleRegular, Gdiplus::Unit::UnitPoint);
        }

        void Draw(app& _app, HWND _hWnd, HDC _hdc)
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

        auto toNative(Point2D pt) { return PointF{pt.x.getAs<float>(), pt.y.getAs<float>()}; }
        auto toNative(Move2D sz) { return SizeF{sz.width.getAs<float>(), sz.height.getAs<float>()}; }
        auto toNative(Color col) { return GdiColor{ col.GetAlpha(), col.GetRed(), col.GetGreen(), col.GetBlue() }; }

        void background(Color color)
        {
            auto clearColor_ = toNative(color);
            graphics_->Clear(clearColor_);
        }

        void noStroke()
        {
            strokeEnabled_ = false;
        }

        void stroke(Color color)
        {
            strokeEnabled_ = true;
            strokeColor_ = toNative(color);
            if (strokeEnabled_)
                stroke_.reset( new Pen{ strokeColor_ });
        }

        void stroke(Meter _thickness)
        {
            strokeEnabled_ = true;
            strokeWeight_ = _thickness;
            stroke_->SetWidth(_thickness.getAs<float>());
        }

        void noFill()
        {
            fillEnabled_ = false;
        }

        void fill(Color color)
        {
            fillEnabled_ = true;
            fillColor_ = toNative(color);
            fillBrush_.reset( new SolidBrush( fillColor_ ) );
        }

        void point(Point2D pt)
        {
            RectF rc{ pt.x.getAs<float>()-0.5f, pt.y.getAs<float>()-0.5f, 1, 1 };
            SolidBrush b{ strokeColor_ };
            graphics_->FillRectangle( &b, rc );
        }

        void line(Point2D pt1, Point2D pt2)
        {
            if (strokeEnabled_)
                graphics_->DrawLine( stroke_.get(), toNative(pt1), toNative(pt2) );
        }

        void rect(Point2D center, Move2D size)
        {
            RectF rc{ toNative(center - size * 0.5f), toNative(size) };
            if (fillEnabled_)
                graphics_->FillRectangle( fillBrush_.get(), rc );
            if (strokeEnabled_)
                graphics_->DrawRectangle( stroke_.get(), rc );
        }

        void ellipse(Point2D center, Move2D size)
        {
            RectF rc{ toNative(center - size * 0.5f), toNative(size) };
            if (fillEnabled_)
                graphics_->FillEllipse( fillBrush_.get(), rc );
            if (strokeEnabled_)
                graphics_->DrawEllipse( stroke_.get(), rc );
        }

        using ArcMode = app::ArcMode;
        void arc(Point2D center, Move2D size, Angle start, Angle end, ArcMode mode)
        {
            RectF rc{ toNative(center - size * 0.5f), toNative(size) };
            auto sweep = end - start;
            if (mode==ArcMode::PIE)
            {
                if (fillEnabled_)
                    graphics_->FillPie( fillBrush_.get(), rc, start.ToDegree(), sweep.ToDegree() );
                if (strokeEnabled_)
                    graphics_->DrawPie( stroke_.get(), rc, start.ToDegree(), sweep.ToDegree() );
            }
            else
            {
                GraphicsPath path;

                path.AddArc(rc, start.ToDegree(), sweep.ToDegree());
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

        void quad(Point2D pt1, Point2D pt2, Point2D pt3, Point2D pt4)
        {
            GraphicsPath path;

            path.AddLine( toNative(pt1), toNative(pt2) );
            path.AddLine( toNative(pt3), toNative(pt4) );
            path.CloseFigure();

            if (fillEnabled_)
                graphics_->FillPath( fillBrush_.get(), &path );
            if (strokeEnabled_)
                graphics_->DrawPath( stroke_.get(), &path);
        }

        void triangle(Point2D pt1, Point2D pt2, Point2D pt3)
        {
            GraphicsPath path;

            path.AddLine( toNative(pt1), toNative(pt2) );
            path.AddLine( toNative(pt2), toNative(pt3) );
            path.CloseFigure();

            if (fillEnabled_)
                graphics_->FillPath( fillBrush_.get(), &path );
            if (strokeEnabled_)
                graphics_->DrawPath( stroke_.get(), &path);
        }

        void text(const char* c, int x, int y)
        {
            auto len = strlen(c);
            auto wcs = (WCHAR*)_malloca((len+1)*sizeof(WCHAR));
            decltype(len) ret;
            mbstowcs_s(&ret, wcs, len + 1, c, len+1);
            graphics_->DrawString(wcs, static_cast<INT>(len), font_, PointF{static_cast<REAL>(x), static_cast<REAL>(y)}, fillBrush_.get());
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

    private:
        using Graphics = Gdiplus::Graphics;
        using Bitmap = Gdiplus::Bitmap;
        using REAL = Gdiplus::REAL;
        using ARGB = Gdiplus::ARGB;
        using Matrix = Gdiplus::Matrix;
        using GdiColor = Gdiplus::Color;
        using Pen = Gdiplus::Pen;
        using Brush = Gdiplus::Brush;
        using SolidBrush = Gdiplus::SolidBrush;
        using Point = Gdiplus::Point;
        using PointF = Gdiplus::PointF;
        using SizeF = Gdiplus::SizeF;
        using Rect = Gdiplus::Rect;
        using RectF = Gdiplus::RectF;
        using Font = Gdiplus::Font;
        using FontFamily = Gdiplus::FontFamily;
        using GraphicsPath = Gdiplus::GraphicsPath;

        Graphics*               graphics_ = nullptr;
        std::unique_ptr<Bitmap> drawBuffer_;

        std::unique_ptr<Brush>  fillBrush_;
        GdiColor                fillColor_ = (ARGB)GdiColor::White;
        bool                    fillEnabled_ = true;

        std::unique_ptr<Pen>    stroke_;
        Meter                   strokeWeight_ = 1_m;
        GdiColor                strokeColor_ = (ARGB)GdiColor::Black;
        bool                    strokeEnabled_ = true;

        std::stack<Matrix>      matrixStack;

        Font*                   font_ = nullptr;

        ULONG_PTR                       token;
        Gdiplus::GdiplusStartupInput    startupInput;
    };
} // namespace cuppa

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

    void app::background(Color color)
    {
        SystemDriver.GetGraphicsDriver().background(color);
    }

    void app::size(Meter _width, Meter _height)   {   SystemDriver.SetWindowSize(_width, _height);    }
    Meter app::getWidth() const { return SystemDriver.GetWindowWidth(); }
    Meter app::getHeight() const { return SystemDriver.GetWindowHeight(); }

    void app::noStroke()                {   SystemDriver.GetGraphicsDriver().noStroke();    }
    void app::stroke(Color color)       {   SystemDriver.GetGraphicsDriver().stroke(color); }
    void app::stroke(Meter _thickness)  {   SystemDriver.GetGraphicsDriver().stroke(_thickness);  }

    void app::noFill()                          {   SystemDriver.GetGraphicsDriver().noFill();  }
    void app::fill(Color color)                 {   SystemDriver.GetGraphicsDriver().fill(color);   }

    void app::point(Point2D pt)                     {   SystemDriver.GetGraphicsDriver().point(pt);             }
    void app::line(Point2D pt1, Point2D pt2)        {   SystemDriver.GetGraphicsDriver().line(pt1, pt2);        }
    void app::rect(Point2D center, Move2D size)     {   SystemDriver.GetGraphicsDriver().rect(center, size);    }

    void app::ellipse(Point2D center, Move2D size) const
    {
        SystemDriver.GetGraphicsDriver().ellipse( center, size );
    }

    void app::text(const char* c, int x, int y)     {   SystemDriver.GetGraphicsDriver().text( c, x, y);    }


    void app::arc(Point2D center, Move2D size, Angle start, Angle end, ArcMode mode)
    {
        SystemDriver.GetGraphicsDriver().arc( center, size, start, end, mode);
    }

    void app::quad(Point2D pt1, Point2D pt2, Point2D pt3, Point2D pt4)
    {
        SystemDriver.GetGraphicsDriver().quad( pt1, pt2, pt3, pt4);
    }

    void app::triangle(Point2D pt1, Point2D pt2, Point2D pt3)
    {
        SystemDriver.GetGraphicsDriver().triangle( pt1, pt2, pt3);
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