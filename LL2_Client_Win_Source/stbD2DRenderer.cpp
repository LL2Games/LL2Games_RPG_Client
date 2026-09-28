#include "stbD2DRenderer.h"

bool stbD2DRenderer::Initialize(HWND hwnd)
{
    mHwnd = hwnd;

    /*
        D2D1CreateFactory : Direct2d의 팩토리 객체를 만드는 함수
        Direct2D 리소스들을 만들기 위한 출발점을 생성하는 함수
        도형, 렌더 타겟, 2D 그래픽 쪽의 시작점이다.


        D2D1_FACTORY_TYPE_SINGLE_THREADED :
        Direct2D가 동기화를 해주지 않는다. 여러 스레드에서 접근하면 개발자가 직접 락을 걸어야 한다.

        D2D1_FACTORY_TYPE_MULIT_THREADED :
        Direct2D가 팩토리와 그 팩토리에서 만든 객체들에 대해 내부적으로 동기화를 제공해서
        여러 스레드에서 안전하게 접근할 수 있게 해준다.

    */
    HRESULT hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        m_D2DFactory.GetAddressOf());

    if (FAILED(hr))
        return false;

    /*
        DWriteCreateFactory : 텍스트를 다루고 그리기위한 DirectWirte의 출발점
        폰트, 텍스트 레이아웃, 글자 렌더링 쪽의 시작점이다.

        DWRITE_FACTORY_TYPE_SHARED : 이 팩토리를 프로세스 전체에서 공유해서 사용한다는 의미

        DWRITE_FACTORY_TYPE_ISOLATED : 따로 격리해서 사용한다는 의미
    */
    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(m_DWriteFactory.GetAddressOf()));
    if (FAILED(hr))
        return false;

    /*
    CoCreateInstance : WIC Imaging Factory라는 COM 객체를 하나 생성해서 m_WicFactory에 넣어준다
                       WIC 이미지 팩토리 객체를 만드는것
    */
    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(m_WicFactory.GetAddressOf()));


    if (FAILED(hr))
    {
        OutputDebugString(L"CoCreateInstance WIC failed\n");
        return false;
    }
        

    if (!CreateTextFormats())
        return false;

    return CreateDeviceResources();
}

// 종료 / 해제
void stbD2DRenderer::ShutDown()
{
    DiscardDeviceResources();

    m_TitleTextFormat.Reset();
    m_BodyTextFormat.Reset();

    m_WicFactory.Reset();
    m_DWriteFactory.Reset();
    m_D2DFactory.Reset();

    mHwnd = nullptr;
}

// 한 프레임 렌더 시작
void stbD2DRenderer::BeginFrame()
{
    if (!m_RenderTarget)
    {
        if (!CreateDeviceResources())
            return;
    }

    m_RenderTarget->BeginDraw();
}

void stbD2DRenderer::EndFrame()
{
    if (!m_RenderTarget)
        return;

    HRESULT hr = m_RenderTarget->EndDraw();

    if (hr == D2DERR_RECREATE_TARGET)
    {
        DiscardDeviceResources();
    }
}

void stbD2DRenderer::Resize(UINT width, UINT height)
{
    if (width == 0 || height == 0)
        return;

    if (m_RenderTarget)
    {
        m_RenderTarget->Resize(D2D1::SizeU(width, height));
    }
}

bool stbD2DRenderer::LoadBitmap(const std::wstring& filePath)
{
    m_BitmapFilePath = filePath;
    m_Bitmap.Reset();

    if (filePath.empty())
        return true;

    if (!m_RenderTarget)
        return true; // 나중에 CreateDeviceResources()에서 다시 생성

    HRESULT hr = CreateBitmapFromFile(filePath.c_str(), m_Bitmap.ReleaseAndGetAddressOf());
    return SUCCEEDED(hr);
}

// 텍스트 스타일을 만드는 함수
bool stbD2DRenderer::CreateTextFormats()
{
    /*
    CreateTextFormat : 글자를 실제로 그리는 함수가 아니라, 글자를 어떤 스타일과 규칙으로 그릴지 정하는 "텍스트 포맷 객체"를 만드는 함수이다.

    text layout에 사용되는 IDWriteTextFormat 객체를 생성한다.


    */
    HRESULT hr = m_DWriteFactory->CreateTextFormat(
        L"Malgun Gothic",
        nullptr,
        DWRITE_FONT_WEIGHT_BOLD,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        32.0f,
        L"ko-KR",
        m_TitleTextFormat.GetAddressOf());

    if (FAILED(hr))
    {
        OutputDebugString(L"Create title text format failed\n");
        return false;
    }
        
    hr = m_DWriteFactory->CreateTextFormat(
        L"Malgun Gothic",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        20.0f,
        L"ko-KR",
        m_BodyTextFormat.GetAddressOf());


    if (FAILED(hr))
    {
        OutputDebugString(L"Create body text format failed\n");
        return false;
    }
       
    hr = m_DWriteFactory->CreateTextFormat(
        L"Tahoma",
        nullptr,
        DWRITE_FONT_WEIGHT_BOLD,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        12.0f,
        L"ko-kr",
        m_SmallTextFormat.GetAddressOf());

    if (FAILED(hr))
    {
        OutputDebugString(L"Create title text format failed\n");
        return false;
    }

    hr = m_DWriteFactory->CreateTextFormat(
        L"Tahoma",
        nullptr,
        DWRITE_FONT_WEIGHT_BOLD,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        9.0f,
        L"ko-kr",
        m_QuickSlotTextFormat.GetAddressOf());

    if (FAILED(hr))
    {
        OutputDebugString(L"Create title text format failed\n");
        return false;
    }
    

    hr = m_DWriteFactory->CreateTextFormat(
        L"메이플스토리",
        nullptr,
        DWRITE_FONT_WEIGHT_BOLD,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        17.0f,
        L"ko-kr",
        m_NicknameTextFormat.GetAddressOf());

    hr = m_DWriteFactory->CreateTextFormat(
        L"메이플스토리",
        nullptr,
        DWRITE_FONT_WEIGHT_BOLD,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        15.0f,
        L"ko-kr",
        m_StatTextFormat.GetAddressOf());

    hr = m_DWriteFactory->CreateTextFormat(
        L"메이플스토리",
        nullptr,
        DWRITE_FONT_WEIGHT_BOLD,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        10.0f,
        L"ko-kr",
        m_ExpTextFormat.GetAddressOf());

    hr = m_DWriteFactory->CreateTextFormat(
        L"메이플스토리",
        nullptr,
        DWRITE_FONT_WEIGHT_BOLD,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        17.0f,
        L"ko-kr",
        m_TradeTextFormat.GetAddressOf());

    hr = m_DWriteFactory->CreateTextFormat(
        L"메이플스토리",
        nullptr,
        DWRITE_FONT_WEIGHT_BOLD,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        20.0f,
        L"ko-kr",
        m_TradeButtonTextFormat.GetAddressOf());

    hr = m_DWriteFactory->CreateTextFormat(
        L"메이플스토리",
        nullptr,
        DWRITE_FONT_WEIGHT_BOLD,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        15.0f,
        L"ko-kr",
        m_ChatTextFormat.GetAddressOf());

  
   

    /*
        SetTextAlignment：가로 정렬
        DWRITE_TEXT_ALIGNMENT_LEADING : 왼쪽 정렬

        SetParagraphAlignment : 세로 정렬
    */
    m_TitleTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    m_TitleTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

    m_BodyTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    m_BodyTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

    m_SmallTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    m_SmallTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

    m_QuickSlotTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    m_QuickSlotTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

    m_NicknameTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_NicknameTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
    
    m_StatTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_StatTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    m_ExpTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_ExpTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

    m_TradeTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    m_TradeTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    m_TradeButtonTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_TradeButtonTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    m_ChatTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    m_ChatTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);


    return true;
}

bool stbD2DRenderer::CreateDeviceResources()
{
    if (m_RenderTarget)
        return true;

    RECT rc{};
    GetClientRect(mHwnd, &rc);

    const UINT width = static_cast<UINT>(rc.right - rc.left);
    const UINT height = static_cast<UINT>(rc.bottom - rc.top);

    /*
        윈도우(HWND)에 직접 그릴 수 있는 Direct2D 렌더 타켓 객체를 생성하는 함수이다.
        즉, 창에 렌더링하는 ID2D1HwndRenderTarget을 생성하는 함수이다.

        GDI+의 HDC랑 동일한 것

        즉, HWND 창에 직접 그릴 수 있는 Direct2D용 도화지를 생성하는 함수이다.

        D2D1::RenderTargetProperties() : 렌더 타켓의 기본 속성
        예) - 하드웨어 렌더링 쓸지, 픽셀 포멧, DPI 같은 렌더 타켓 전반 설정이다.
        아무것도 넣지 않으면 기본값을 사용

        D2D1::HwndRenderTargetProperties(mHwnd, D2D1::SizeU(width, height))
        윈도우 전용 속성이다.

        어느 창에 그릴 건지, 랜더 타켓 크기

        랜더 타켓은 한 번 만들어서 가능한 오래 유지하는게 성능상 좋다. 그리고
        D2DERR_RECRETE_TARGET이 나오면 다시 만들어야 한다.
    */
    HRESULT hr = m_D2DFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(mHwnd, D2D1::SizeU(width, height)),
        m_RenderTarget.GetAddressOf());
    if (FAILED(hr))
        return false;

    /*
    CreateSolidColorBrush : 한 가지 색만 가지는 브러시를 만든다.

    ID2D1SolidColorBrush : 단색 한 가지
    ID2D1LinearGradientBrush : 직선 방향으로 색이 바뀌는 브러시
    ID2D1RadialGradientBrush : 원형으로 퍼지면서 색이 바뀌는 브러시
    ID2D1BitmapBrush : 이미지 패턴으로 칠하는 브러시
    */
    hr = m_RenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::White),
        m_Brush.GetAddressOf());

    if (FAILED(hr))
    {
        DiscardDeviceResources();
        return false;
    }

    if (!m_BitmapFilePath.empty())
    {
        // 비트맵 로드는 실패해도 앱 전체를 죽이지 않음
        CreateBitmapFromFile(m_BitmapFilePath.c_str(), m_Bitmap.ReleaseAndGetAddressOf());
    }

    return true;
}

void stbD2DRenderer::DiscardDeviceResources()
{
    //Reset은 ComPtr이 들고 있는 COM 객체 참조를 해제한다..
    m_Bitmap.Reset();
    m_Brush.Reset();
    m_RenderTarget.Reset();
}

// 브러시가 만들어져 있는지 확인하고 없으면 새로 생성
bool stbD2DRenderer::EnsureBrush()
{
    if (!m_RenderTarget)
        return false;

    if (!m_Brush)
    {
        HRESULT hr = m_RenderTarget->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::White),
            m_Brush.GetAddressOf());

        if (FAILED(hr))
            return false;
    }

    return true;
}

// 
HRESULT stbD2DRenderer::CreateBitmapFromFile(PCWSTR filePath, ID2D1Bitmap** outBitmap)
{
    if (!m_WicFactory || !m_RenderTarget || !outBitmap)
        return E_FAIL;

    /*
    IWICBitmapDecoder : 이미지 파일 해독기 ( PNG, JPG, BMP 같은 파일을 열 수 있다.)
                        내부 이미지 데이터를 읽을 수 있게 준비함

    IWICBitmapFrameDecode : 이미지의 실제 프레임 데이터를 나타낸다.

    IWICFormatConverter : 이미지 픽셀 포멧 변환기
                          원본 이미지 포멧이 D2D가 바로 쓰기 좋은 포멧이 아닐 수 있기 때문에
                          원본 프레임을 D2D가 좋아하는 포맷으로 변환한다.

    */
    ComPtr<IWICBitmapDecoder> decoder;
    ComPtr<IWICBitmapFrameDecode> frame;
    ComPtr<IWICFormatConverter> converter;

    /*
    CreateDecoderFromFilename : 디코더를 생성하는 함수
    png, jpg 파일을 WIC가 읽을 수 없어서 읽을 수 있게 디코더를 생성한다.

    메타데이터란?
    이미지 파일안에 들어있는 크기, 포멧 정보, 색상 프로필, EXIF 정보, 기타 부가 정보이다.
    */
    HRESULT hr = m_WicFactory->CreateDecoderFromFilename(
        filePath,
        nullptr,
        GENERIC_READ, // 읽기 전용으로 열겠다.
        WICDecodeMetadataCacheOnLoad, // 메타 데이터를 파일을 열 때 캐시한다.
        decoder.GetAddressOf());


    if (FAILED(hr))
        return hr;

    // GetFrame : 실제 이미지 프레임을 가져옴 GIF 처럼 움짤이 아니면 0을 넣어 원본을 가지고 온다
    hr = decoder->GetFrame(0, frame.GetAddressOf());

    if (FAILED(hr))
        return hr;

    // CreateFormatConverter : 컨버터를 생성
    hr = m_WicFactory->CreateFormatConverter(converter.GetAddressOf());

    if (FAILED(hr))
        return hr;

    /*
    Initialize : 컨버터 설정 함수
        frame에 들어있는 원본 이미지를 dithering 없이, 32bppPBGRA 형식으로 변환해서 쓸 수 있도록
        converter를 설정
    */
    hr = converter->Initialize(
        frame.Get(),
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone, // 디더링 안함
        nullptr,                // 팔레트 지정 안함
        0.0f,                   // 알파 threshold 안 씀
        WICBitmapPaletteTypeMedianCut); //팔레트 변환 방식 관련 값

    if (FAILED(hr))
        return hr;

    /*
    CreateBitmapFromWicBitmap :
    conveter.Get()을 하면 원본 이미지를 컨버터 설정에 따라 변환한 값을 가지고 비트맵으로 생성해서 outBitmap에 저장

    */
    hr = m_RenderTarget->CreateBitmapFromWicBitmap(
        converter.Get(),
        nullptr,
        outBitmap);
    return hr;
}

void stbD2DRenderer::Clear(const D2D1::ColorF& color)
{
    if (!m_RenderTarget)
        return;

    m_RenderTarget->Clear(color);
}

// 라인 그리는 함수
void stbD2DRenderer::DrawLine(float x1, float y1, float x2, float y2, const D2D1::ColorF& color, float stroke)
{
    if (!EnsureBrush())
        return;

    m_Brush->SetColor(color);
    m_RenderTarget->DrawLine(
        D2D1::Point2F(x1, y1),
        D2D1::Point2F(x2, y2),
        m_Brush.Get(),
        stroke);// strok는 픽셀 두께를 의미
}

// 사각형 테두리를 그리는 함수
void stbD2DRenderer::DrawRect(float x, float y, float w, float h, const D2D1::ColorF& color, float stroke)
{
    if (!EnsureBrush())
        return;

    m_Brush->SetColor(color);
    /*
    DrawRectangle: 사각형 영역을 만들어서 그 테두리를 브러시로 그리는 함수
    D2D1::RectF : 사각형을 만드는 함수
    */
    m_RenderTarget->DrawRectangle(
        D2D1::RectF(x, y, x + w, y + h),
        m_Brush.Get(),
        stroke); // strok는 픽셀 두께를 의미
}

// 사각형 내부를 색으로 채우는 함수
void stbD2DRenderer::FillRect(float x, float y, float w, float h, const D2D1::ColorF& color)
{
    if (!EnsureBrush())
        return;

    m_Brush->SetColor(color);
    m_RenderTarget->FillRectangle(
        D2D1::RectF(x, y, x + w, y + h),
        m_Brush.Get());
}

void stbD2DRenderer::DrawCircle(float cx, float cy, float radius, const D2D1::ColorF& color, float stroke)
{
    if (!EnsureBrush())
        return;

    m_Brush->SetColor(color);
    m_RenderTarget->DrawEllipse(
        D2D1::Ellipse(D2D1::Point2F(cx, cy), radius, radius),
        m_Brush.Get(),
        stroke);
}

void stbD2DRenderer::FillCircle(float cx, float cy, float radius, const D2D1::ColorF& color)
{
    if (!EnsureBrush())
        return;

    m_Brush->SetColor(color);
    m_RenderTarget->FillEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), radius, radius), m_Brush.Get());
}

// 문자열을 출력하는 함수
void stbD2DRenderer::DrawTextString(const std::wstring& text, const D2D1_RECT_F& layoutRect, const D2D1::ColorF& color, TextStyle TextStyle)
{
    if (!m_RenderTarget || !EnsureBrush()) 
    {
        OutputDebugString(L"DrawTextString : RenderTarget or Brush invalid\n");
        return;
    }
       

    IDWriteTextFormat* textFormat = nullptr;

    switch (TextStyle)
    {
    case TextStyle::Title:
        textFormat = m_TitleTextFormat.Get();
        break;

    case TextStyle::Small:
        textFormat = m_SmallTextFormat.Get();
        break;

    case TextStyle::QuickSlot:
        textFormat = m_QuickSlotTextFormat.Get();
        break;

    case TextStyle::NickName:
        textFormat = m_NicknameTextFormat.Get();
        break;

    case TextStyle::Stat:
        textFormat = m_StatTextFormat.Get();
        break;

    case TextStyle::EXP:
        textFormat = m_ExpTextFormat.Get();
        break;

    case TextStyle::Trade:
        textFormat = m_TradeTextFormat.Get();
        break;

    case TextStyle::TradeButton:
        textFormat = m_TradeButtonTextFormat.Get();
        break;
    case TextStyle::Chat:
        textFormat = m_ChatTextFormat.Get();
        break;
    case TextStyle::Body:
    default:
        textFormat = m_BodyTextFormat.Get();
        break;
    }

    if (!textFormat)
        return;
 

    m_Brush->SetColor(color);
    m_RenderTarget->DrawTextW(
        text.c_str(),
        static_cast<UINT32>(text.size()),
        textFormat,
        layoutRect,
        m_Brush.Get());
}

// 비트맵을 출력하는 함수
void stbD2DRenderer::DrawBitmap(float x, float y, float width, float height, float opacity)
{
    if (!m_RenderTarget || !m_Bitmap)
        return;

    D2D1_SIZE_F size = m_Bitmap->GetSize();

    if (width <= 0.0f)  width = size.width;
    if (height <= 0.0f) height = size.height;

    D2D1_RECT_F destRect = D2D1::RectF(x, y, x + width, y + height);

    m_RenderTarget->DrawBitmap(
        m_Bitmap.Get(),
        destRect,
        opacity,
        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
}

void stbD2DRenderer::DrawBitmap(ID2D1Bitmap* bitmap, float x, float y, float width, float height, float opacity)
{
    if (!m_RenderTarget || !bitmap)
        return;

    D2D1_SIZE_F size = bitmap->GetSize();

    if (width <= 0.0f)  width = size.width;
    if (height <= 0.0f) height = size.height;

    D2D1_RECT_F destRect = D2D1::RectF(x, y, x + width, y + height);

    m_RenderTarget->DrawBitmap(
        bitmap,
        destRect,
        opacity,
        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
}

void stbD2DRenderer::DrawBitmap(ID2D1Bitmap* bitmap, const D2D1_RECT_F& destRect, const D2D1_RECT_F& srcRect, float opacity)
{
    if (bitmap == nullptr) return;

    m_RenderTarget->DrawBitmap(
        bitmap,
        destRect,
        opacity,
        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
        srcRect
    );
}

void stbD2DRenderer::DrawBitmap(ID2D1Bitmap* bitmap, const D2D1_RECT_F& destRect, const D2D1_RECT_F& /*srcRect*/, float /*opacity*/, bool flipX)
{
    if (bitmap == nullptr)
        return;

    if (!flipX)
    {
        m_RenderTarget->DrawBitmap(bitmap, destRect);
        return;
    }

    D2D1_MATRIX_3X2_F oldTransform;
    m_RenderTarget->GetTransform(&oldTransform);

    float centerX = (destRect.left + destRect.right) * 0.5f;
    float centerY = (destRect.top + destRect.bottom) * 0.5f;

    D2D1_MATRIX_3X2_F flip =
        D2D1::Matrix3x2F::Scale(
            -1.0f,
            1.0f,
            D2D1::Point2F(centerX, centerY)
        );

    m_RenderTarget->SetTransform(flip * oldTransform);

    m_RenderTarget->DrawBitmap(bitmap, destRect);

    m_RenderTarget->SetTransform(oldTransform);
}

bool stbD2DRenderer::HasBitmap() const
{
    return m_Bitmap != nullptr;
}

void stbD2DRenderer::DrawSprite(ID2D1Bitmap* bitmap, float destX, float destY, float destW, float destH,
    float srcX, float srcY, float srcW, float srcH, float opacity)
{
    if (!m_RenderTarget || !bitmap)
        return;

    D2D1_RECT_F destRect = D2D1::RectF(destX, destY, destX + destW, destY + destH);
    D2D1_RECT_F srcRect  = D2D1::RectF(srcX,  srcY,  srcX  + srcW,  srcY  + srcH);

    m_RenderTarget->DrawBitmap(
        bitmap,
        destRect,
        opacity,
        D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
        srcRect);
}

void stbD2DRenderer::DrawSprite(ID2D1Bitmap* bitmap, float destX, float destY, float destW, float destH, float srcX, float srcY, float srcW, float srcH, bool flipX, float opacity)
{
    if (!m_RenderTarget || !bitmap)
        return;

    D2D1_RECT_F destRect = D2D1::RectF(destX,destY,destX + destW,destY + destH);
    D2D1_RECT_F srcRect = D2D1::RectF(srcX,srcY,srcX + srcW,srcY + srcH);

    if (!flipX)
    {
        m_RenderTarget->DrawBitmap(
            bitmap,
            destRect,
            opacity,
            D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
            srcRect
        );

        return;
    }

    D2D1_MATRIX_3X2_F oldTransform;
    m_RenderTarget->GetTransform(&oldTransform);

    float centerX = destX + destW * 0.5f;
    float centerY = destY + destH * 0.5f;

    D2D1_MATRIX_3X2_F flipTransform =
        D2D1::Matrix3x2F::Scale(
            -1.0f,
            1.0f,
            D2D1::Point2F(centerX, centerY)
        );

    m_RenderTarget->SetTransform(flipTransform * oldTransform);

    m_RenderTarget->DrawBitmap(
        bitmap,
        destRect,
        opacity,
        D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
        srcRect
    );

    m_RenderTarget->SetTransform(oldTransform);
}

void stbD2DRenderer::DrawSprite2(ID2D1Bitmap* bitmap, float destX, float destY, float destW, float destH, float srcX, float srcY, float srcW, float srcH, bool flipX, float opacity)
{
    if (!m_RenderTarget || !bitmap)
        return;

    D2D1_RECT_F destRect = D2D1::RectF(destX,destY,destX + destW,destY + destH);

    D2D1_RECT_F srcRect = D2D1::RectF(srcX,srcY,srcX + srcW,srcY + srcH);

    if (!flipX)
    {
        m_RenderTarget->DrawBitmap(
            bitmap,
            destRect,
            opacity,
            D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
            srcRect
        );
        return;
    }

    D2D1_MATRIX_3X2_F oldTransform;
    m_RenderTarget->GetTransform(&oldTransform);

    float pivotX = destX + destW * 0.5f;
    float pivotY = destY + destH * 0.5f;

    D2D1_MATRIX_3X2_F flipTransform =
        D2D1::Matrix3x2F::Scale(
            -1.0f,
            1.0f,
            D2D1::Point2F(pivotX, pivotY)
        );

    m_RenderTarget->SetTransform(flipTransform * oldTransform);

    m_RenderTarget->DrawBitmap(
        bitmap,
        destRect,
        opacity,
        D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
        srcRect
    );

    m_RenderTarget->SetTransform(oldTransform);
}


D2D1_SIZE_F stbD2DRenderer::GetRenderTargetSize() const
{
    if(!m_RenderTarget)
        return D2D1::SizeF(0.f, 0.f);

    return m_RenderTarget->GetSize();
}
