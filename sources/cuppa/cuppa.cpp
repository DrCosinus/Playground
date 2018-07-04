#include "cuppa.hpp"

#include <cstdlib>
#include <malloc.h>
#include <windows.h>

#include <memory>
#include <stack>
#include <algorithm>

#include <iostream>
#include <random>

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

    void SetWindowSize(Pixel _width, Pixel _height)
    {
        // for now, we only handle initial width and height
        // TODO: handle SetWindowSize after the creation of the windows
        width = _width;
        height = _height;
    }
    Pixel GetWindowWidth() const { return width; }
    Pixel GetWindowHeight() const { return height; }

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
        RECT rc{0,0,width.getAs<int>(), height.getAs<int>()};
        AdjustWindowRectEx(&rc,WS_OVERLAPPEDWINDOW,FALSE, WS_EX_CLIENTEDGE);
        hWnd_ = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            windowClassName,
            "Cuppa",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, rc.right-rc.left, rc.bottom-rc.top,
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
    Pixel width = 240_px, height = 120_px;
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

        void stroke(Pixel _thickness)
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
            RectF rc{ pt.x.getAs<float>()-0.5f*strokeWeight_.getAs<float>(), pt.y.getAs<float>()-0.5f*strokeWeight_.getAs<float>(), strokeWeight_.getAs<float>(), strokeWeight_.getAs<float>() };
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

        void translate(Move2D translation)
        {
            graphics_->TranslateTransform( translation.width.getAs<float>(), translation.height.getAs<float>(), Gdiplus::MatrixOrder::MatrixOrderAppend);
        }

        void rotate(Angle angle)
        {
            graphics_->RotateTransform( angle.ToDegree(), Gdiplus::MatrixOrder::MatrixOrderAppend);
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

        Image loadImage(std::string_view filename)
        {
            auto len = filename.length();
            auto wcs = (WCHAR*)_malloca((len+1)*sizeof(WCHAR));
            std::transform(std::begin(filename), std::end(filename), wcs, [](char c){ WCHAR wc; mbtowc(&wc, &c, 1); return wc; });
            wcs[len] = L'\0';
            auto gdiImage = std::make_shared<GdiImage>( wcs );
            std::cout << gdiImage->GetWidth() << " x " << gdiImage->GetHeight() << " x " << gdiImage->GetPaletteSize() << std::endl;
            std::cout << gdiImage->GetHorizontalResolution() << " x " << gdiImage->GetVerticalResolution() << std::endl;
            SizeF sz;
            gdiImage->GetPhysicalDimension(&sz);
            std::cout << sz.Width << " x " << sz.Height << std::endl;
            return Image{ std::make_shared<GdiImage>( wcs ) };
        }

        void image(const Image& img, Point2D pt)
        {
            auto gdiImage = img.getNativeAs<std::shared_ptr<GdiImage>>().get();
            // static bool once = true;
            // if (once)
            // {
            //     std::cout << gdiImage->GetWidth() << " x " << gdiImage->GetHeight() << " x " << gdiImage->GetPaletteSize() << std::endl;
            //     std::cout << gdiImage->GetHorizontalResolution() << " x " << gdiImage->GetVerticalResolution() << std::endl;
            //     SizeF sz;
            //     gdiImage->GetPhysicalDimension(&sz);
            //     std::cout << sz.Width << " x " << sz.Height << std::endl;
            //     once = false;
            // }

            RectF rc{ toNative(pt), SizeF{static_cast<float>(gdiImage->GetWidth()),static_cast<float>(gdiImage->GetHeight())}};
            graphics_->DrawImage(img.getNativeAs<std::shared_ptr<GdiImage>>().get(), rc);
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
        using GdiImage = Gdiplus::Image;

        Graphics*               graphics_ = nullptr;
        std::unique_ptr<Bitmap> drawBuffer_;

        std::unique_ptr<Brush>  fillBrush_;
        GdiColor                fillColor_ = (ARGB)GdiColor::White;
        bool                    fillEnabled_ = true;

        std::unique_ptr<Pen>    stroke_;
        Pixel                   strokeWeight_ = 1_px;
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
        SystemDriver.GetGraphicsDriver().Init();
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

    void app::size(Pixel _width, Pixel _height)   {   SystemDriver.SetWindowSize(_width, _height);    }
    Pixel app::getWidth() const { return SystemDriver.GetWindowWidth(); }
    Pixel app::getHeight() const { return SystemDriver.GetWindowHeight(); }

    void app::noStroke()                {   SystemDriver.GetGraphicsDriver().noStroke();    }
    void app::stroke(Color color)       {   SystemDriver.GetGraphicsDriver().stroke(color); }
    void app::stroke(Pixel _thickness)  {   SystemDriver.GetGraphicsDriver().stroke(_thickness);  }

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

    void app::translate(Move2D translation)
    {
        SystemDriver.GetGraphicsDriver().translate(translation);
    }

    void app::rotate(Angle angle)
    {
        SystemDriver.GetGraphicsDriver().rotate(angle);
    }

    void app::scale(float xscale, float yscale, float zscale)
    {
        SystemDriver.GetGraphicsDriver().scale(xscale, yscale, zscale);
    }

    void app::shearX(float angle) { SystemDriver.GetGraphicsDriver().shearX(angle); }
    void app::shearY(float angle) { SystemDriver.GetGraphicsDriver().shearY(angle); }

    Image app::loadImage(std::string_view filename) { return SystemDriver.GetGraphicsDriver().loadImage(filename); }
    void app::image(const Image& img, Point2D pt) { return SystemDriver.GetGraphicsDriver().image(img, pt); }

    std::default_random_engine engine{ std::random_device()() };
    std::uniform_real_distribution<float> distribution{ 0.0f, 1.0f };

    float app::random() { return distribution(engine); }
} // namespace cuppa