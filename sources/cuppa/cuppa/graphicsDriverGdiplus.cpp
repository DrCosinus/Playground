#include "graphicsDriverInterface.hpp"

#include "cuppa.hpp" // for cuppa::app

#include <windows.h>

#pragma warning(push)
#pragma warning(disable:4458)
#include <gdiplus.h>
#pragma warning(pop)
// sample GDI+: https://codes-sources.commentcamarche.net/source/view/29875/966776#browser

#include <algorithm>
#include <stack>
#include <iostream> // todo: need a logger!!
#include <malloc.h>
#include <memory>
#include <vector>

using namespace Gdiplus;
using GdiColor = Gdiplus::Color;
using Color = cuppa::Color;
using GdiImage = Gdiplus::Image;
using GdiBitmap = Gdiplus::Bitmap;
using GdiPoint = Gdiplus::Point;
using Image = cuppa::Image;
using GdiFont = Gdiplus::Font;
using Font = cuppa::Image;

namespace cuppa
{
    struct graphicsDriverGdiplus : graphicsDriverInterface
    {
        void setup(app& _app) override
        {
            app_ = &_app;
            Gdiplus::GdiplusStartup(&token, &startupInput, NULL);
            fillBrush_.reset( new SolidBrush{ fillColor_ } );
            stroke_.reset( new Pen{ strokeColor_, strokeWeight_.getAs<float>() } );

            auto defaultFont = loadFont("Verdana", 10);
            textFont(defaultFont);
        }

        void draw(DeviceContext _dc) override
        {
            Assert(graphics_==nullptr);
            auto hdc = _dc.getNativeAs<HDC>();
            Graphics graphics{ hdc };
            // RECT rc;
            // GetClientRect(_hWnd, &rc);
            // auto rcWidth = static_cast<UINT>( rc.right-rc.left );
            // auto rcHeight = static_cast<UINT>( rc.bottom-rc.top );
            auto rcWidth = app_->getWidth().getAs<UINT>();
            auto rcHeight = app_->getHeight().getAs<UINT>();

            if (drawBuffer_==nullptr || drawBuffer_->GetWidth() != rcWidth || drawBuffer_->GetHeight() != rcHeight)
            {
                drawBuffer_.reset(new Bitmap(rcWidth, rcHeight, PixelFormat32bppPARGB));
            }
            Graphics drawGraphics{ drawBuffer_.get() };
            graphics_ = &drawGraphics;
            drawGraphics.SetInterpolationMode(Gdiplus::InterpolationMode::InterpolationModeLowQuality);
            drawGraphics.SetCompositingMode(Gdiplus::CompositingMode::CompositingModeSourceOver);
            drawGraphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighSpeed);

            app_->draw();
            graphics.SetInterpolationMode(Gdiplus::InterpolationMode::InterpolationModeLowQuality);
            graphics.SetCompositingMode(Gdiplus::CompositingMode::CompositingModeSourceCopy);
            graphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighSpeed);
            graphics.DrawImage( drawBuffer_.get(), 0, 0 );
            graphics_ = nullptr;
        }

        auto toNative(Point pt) { return PointF{pt.x.getAs<float>(), pt.y.getAs<float>()}; }
        auto toNative(Direction sz) { return SizeF{sz.width.getAs<float>(), sz.height.getAs<float>()}; }
        auto toNative(Color col) { return GdiColor{ col.GetAlpha<BYTE>(), col.GetRed<BYTE>(), col.GetGreen<BYTE>(), col.GetBlue<BYTE>() }; }

        void background(Color color) override
        {
            auto clearColor_ = toNative(color);
            graphics_->Clear(clearColor_);
        }

        void stroke(Appliance appliance) override
        {
            strokeEnabled_ = (appliance == Appliance::ENABLED);
        }

        void stroke(Color color) override
        {
            strokeEnabled_ = true;
            strokeColor_ = toNative(color);
            stroke_->SetColor( strokeColor_ );
        }

        void stroke(Pixel _thickness) override
        {
            strokeEnabled_ = true;
            strokeWeight_ = _thickness;
            stroke_->SetWidth(_thickness.getAs<float>());
        }

        void noFill() override
        {
            fillEnabled_ = false;
        }

        void fill(Color color) override
        {
            fillEnabled_ = true;
            fillColor_ = toNative(color);
            fillBrush_.reset( new SolidBrush( fillColor_ ) );
        }

        void point(Point pt) override
        {
            RectF rc{ pt.x.getAs<float>()-0.5f*strokeWeight_.getAs<float>(), pt.y.getAs<float>()-0.5f*strokeWeight_.getAs<float>(), strokeWeight_.getAs<float>(), strokeWeight_.getAs<float>() };
            SolidBrush b{ strokeColor_ };
            graphics_->FillRectangle( &b, rc );
        }

        void line(Point pt1, Point pt2) override
        {
            if (strokeEnabled_)
                graphics_->DrawLine( stroke_.get(), toNative(pt1), toNative(pt2) );
        }

        void rect(Point center, Direction size) override
        {
            RectF rc{ toNative(center - size * 0.5f), toNative(size) };
            if (fillEnabled_)
                graphics_->FillRectangle( fillBrush_.get(), rc );
            if (strokeEnabled_)
                graphics_->DrawRectangle( stroke_.get(), rc );
        }

        void ellipse(Point center, Direction size) override
        {
            RectF rc{ toNative(center - size * 0.5f), toNative(size) };
            if (fillEnabled_)
                graphics_->FillEllipse( fillBrush_.get(), rc );
            if (strokeEnabled_)
                graphics_->DrawEllipse( stroke_.get(), rc );
        }

        void arc(Point center, Direction size, Angle start, Angle end, ArcMode mode) override
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

        void quad(Point pt1, Point pt2, Point pt3, Point pt4) override
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

        void triangle(Point pt1, Point pt2, Point pt3) override
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

        bool shapeBuilding = false;
        std::vector<PointF> shapeVertices;

        void beginShape() override
        {
            Assert(!shapeBuilding);
            shapeVertices.clear();
            shapeBuilding = true;
        }

        void endShape() override
        {
            Assert(shapeBuilding);
            shapeBuilding = false;
            GraphicsPath shape;
            shape.AddLines(&shapeVertices.front(), (INT)shapeVertices.size());
            if (fillEnabled_)
                graphics_->FillPath( fillBrush_.get(), &shape );
            if (strokeEnabled_)
                graphics_->DrawPath( stroke_.get(), &shape);
        }

        void vertex(Point pt) override
        {
            shapeVertices.emplace_back(pt.x.getAs<float>(), pt.y.getAs<float>());
        }

        Font loadFont(std::string_view name, std::size_t size) override
        {
            auto len = name.length();
            auto wcs = (WCHAR*)_malloca((len+1)*sizeof(WCHAR));
            std::transform(std::begin(name), std::end(name), wcs, [](char c){ WCHAR wc; mbtowc(&wc, &c, 1); return wc; });
            wcs[len] = L'\0';
            return Font{ std::make_shared<GdiFont>(wcs, static_cast<REAL>(size), Gdiplus::FontStyle::FontStyleRegular, Gdiplus::Unit::UnitPixel) };
        }

        void textFont(const Font& font) override
        {
            font_ = font;
        }

        void text(std::string_view txt, Point pt, TextHAlign halign, TextVAlign valign) override
        {
            StringFormat sf;
            switch(halign)
            {
            default:
            case TextHAlign::LEFT:    sf.SetAlignment(StringAlignment::StringAlignmentNear); break;
            case TextHAlign::CENTER:  sf.SetAlignment(StringAlignment::StringAlignmentCenter); break;
            case TextHAlign::RIGHT:   sf.SetAlignment(StringAlignment::StringAlignmentFar); break;
            }
            switch(valign)
            {
            default:
            case TextVAlign::TOP:     sf.SetLineAlignment(StringAlignment::StringAlignmentNear); break;
            case TextVAlign::MIDDLE:  sf.SetLineAlignment(StringAlignment::StringAlignmentCenter); break;
            case TextVAlign::BOTTOM:  sf.SetLineAlignment(StringAlignment::StringAlignmentFar); break;
            }
            auto len = txt.length();
            auto wcs = (WCHAR*)_malloca((len+1)*sizeof(WCHAR));
            std::transform(std::begin(txt), std::end(txt), wcs, [](char c){ WCHAR wc; mbtowc(&wc, &c, 1); return wc; });
            wcs[len] = L'\0';
            auto gdiFont = font_.getNativeAs<std::shared_ptr<GdiFont>>().get();
            graphics_->DrawString(wcs, static_cast<INT>(len), gdiFont, toNative(pt), &sf, fillBrush_.get());
        }

        Direction textSize(std::string_view txt) override
        {
            auto len = txt.length();
            auto wcs = (WCHAR*)_malloca((len+1)*sizeof(WCHAR));
            std::transform(std::begin(txt), std::end(txt), wcs, [](char c){ WCHAR wc; mbtowc(&wc, &c, 1); return wc; });
            wcs[len] = L'\0';
            auto gdiFont = font_.getNativeAs<std::shared_ptr<GdiFont>>().get();
            RectF boundingBox;

            auto hdc = getDeviceContext().getNativeAs<HDC>();
            Graphics g{ hdc };
            StringFormat sf;
            sf.SetFormatFlags(StringFormatFlags::StringFormatFlagsNoClip | StringFormatFlags::StringFormatFlagsNoWrap);
            g.MeasureString(wcs,static_cast<INT>(len), gdiFont, RectF{0,0,32768,32768}, &sf, &boundingBox);
            return Direction{ Pixel{ boundingBox.Width }, Pixel{ boundingBox.Height } };
        }

        void resetMatrix() override
        {
            Matrix identity;
            graphics_->SetTransform(&identity);
        }

        void pushMatrix() override
        {
            matrixStack.emplace();
            graphics_->GetTransform(&matrixStack.top());
        }

        bool popMatrix() override
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

        void translate(Direction translation) override
        {
            graphics_->TranslateTransform( translation.width.getAs<float>(), translation.height.getAs<float>(), Gdiplus::MatrixOrder::MatrixOrderAppend);
        }

        void rotate(Angle angle) override
        {
            graphics_->RotateTransform( angle.ToDegree(), Gdiplus::MatrixOrder::MatrixOrderAppend);
        }

        void scale(float xscale, float yscale, float) override
        {
            graphics_->ScaleTransform(xscale, yscale, Gdiplus::MatrixOrder::MatrixOrderAppend);
        }

        void shearX(float slope) override
        {
            Matrix mxTransform{ 1, 0, slope, 1, 0, 0 };
            graphics_->MultiplyTransform( &mxTransform, Gdiplus::MatrixOrder::MatrixOrderAppend);
        }

        void shearY(float slope) override
        {
            Matrix mxTransform{ 1, slope, 0, 1, 0, 0 };
            graphics_->MultiplyTransform(&mxTransform, Gdiplus::MatrixOrder::MatrixOrderAppend);
        }

        Image loadImage(std::string_view filename) override
        {
            auto len = filename.length();
            auto wcs = (WCHAR*)_malloca((len+1)*sizeof(WCHAR));
            std::transform(std::begin(filename), std::end(filename), wcs, [](char c){ WCHAR wc; mbtowc(&wc, &c, 1); return wc; });
            wcs[len] = L'\0';
            //auto gdiImage = std::make_shared<GdiImage>( wcs );
            //std::cout << gdiImage->GetWidth() << " x " << gdiImage->GetHeight() << " x " << gdiImage->GetPaletteSize() << std::endl;
            //std::cout << gdiImage->GetHorizontalResolution() << " x " << gdiImage->GetVerticalResolution() << std::endl;
            //SizeF sz;
            //gdiImage->GetPhysicalDimension(&sz);
            //std::cout << sz.Width << " x " << sz.Height << std::endl;
            return Image{ std::make_shared<GdiBitmap>( wcs ) };
        }

        void image(const Image& img, Point pt) override
        {
            auto gdiImage = img.getNativeAs<std::shared_ptr<GdiBitmap>>().get();
            RectF rc{ toNative(pt), SizeF{static_cast<float>(gdiImage->GetWidth()),static_cast<float>(gdiImage->GetHeight())}};
            auto status = graphics_->DrawImage(gdiImage, rc);
            int a = 1;
            (void)a;
            (void)status;
        }

        Image createImage(Direction size) override
        {
            return Image{ std::make_shared<GdiBitmap>( size.width.getAs<INT>(), size.height.getAs<INT>()) };
        }

        BitmapData bitmapData = { };
        void loadPixels(const Image& img) override
        {
            auto gdiImage = img.getNativeAs<std::shared_ptr<GdiBitmap>>().get();
            Rect rc{ GdiPoint( 0, 0 ), Size{static_cast<INT>(gdiImage->GetWidth()),static_cast<INT>(gdiImage->GetHeight())}};
            auto status = gdiImage->LockBits(&rc, ImageLockMode::ImageLockModeRead | ImageLockMode::ImageLockModeWrite, gdiImage->GetPixelFormat(), &bitmapData);
            Assert(status == Status::Ok);
        }

        void updatePixels(const Image& img) override
        {
            auto gdiImage = img.getNativeAs<std::shared_ptr<GdiBitmap>>().get();
            auto status = gdiImage->UnlockBits(&bitmapData);
            Assert(status == Status::Ok);
        }

        Color* getPixels() override
        {
            return static_cast<Color*>(bitmapData.Scan0);
        }

    private:
        app*                    app_ = nullptr;
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

        Font                    font_;

        ULONG_PTR                       token;
        Gdiplus::GdiplusStartupInput    startupInput;
    };

    std::unique_ptr<graphicsDriverInterface> createGraphicsDriverGdiplus() { return std::make_unique<graphicsDriverGdiplus>(); }
} // namespace cuppa