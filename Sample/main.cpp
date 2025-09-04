#pragma comment( lib, "d3d9.lib" )
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment( lib, "d3dx9d.lib" )
#else
#pragma comment( lib, "d3dx9.lib" )
#endif

#pragma comment (lib, "winmm.lib")

#pragma comment( lib, "HUD.lib" )

#include "..\HUD\HUD.h"

#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include <tchar.h>

using namespace NSHUD;

#define SAFE_RELEASE(p) { if (p) { (p)->Release(); (p) = NULL; } }

class Sprite : public ISprite
{
public:

    Sprite(LPDIRECT3DDEVICE9 dev)
        : m_pD3DDevice(dev)
    {
    }

    void DrawImage(const int percent, const int x, const int y, const int transparency) override
    {
        D3DXVECTOR3 pos {(float)x, (float)y, 0.f};
        m_D3DSprite->Begin(D3DXSPRITE_ALPHABLEND);
        RECT rect { };

        rect.left = 0;
        rect.top = 0;
        rect.right = (LONG)(m_width * percent / 100);
        rect.bottom = (LONG)(m_height / 2);

        D3DXVECTOR3 center { 0, 0, 0 };

        m_D3DSprite->Draw(m_pD3DTexture,
                          &rect,
                          &center,
                          &pos,
                          D3DCOLOR_ARGB(transparency, 255, 255, 255));

        m_D3DSprite->End();

    }

    void Load(const std::wstring& filepath) override
    {
        LPD3DXSPRITE tempSprite { nullptr };
        if (FAILED(D3DXCreateSprite(m_pD3DDevice, &m_D3DSprite)))
        {
            throw std::exception("Failed to create a sprite.");
        }

        if (FAILED(D3DXCreateTextureFromFile(m_pD3DDevice, filepath.c_str(), &m_pD3DTexture)))
        {
            throw std::exception("Failed to create a texture.");
        }

        D3DSURFACE_DESC desc { };
        if (FAILED(m_pD3DTexture->GetLevelDesc(0, &desc)))
        {
            throw std::exception("Failed to create a texture.");
        }
        m_width = desc.Width;
        m_height = desc.Height;
    }

    ~Sprite()
    {
        m_D3DSprite->Release();
        m_pD3DTexture->Release();
    }

    void OnDeviceLost()
    {
        m_D3DSprite->OnLostDevice();
    }

    void OnDeviceReset()
    {
        m_D3DSprite->OnResetDevice();
    }

private:

    LPDIRECT3DDEVICE9 m_pD3DDevice = NULL;
    LPD3DXSPRITE m_D3DSprite = NULL;
    LPDIRECT3DTEXTURE9 m_pD3DTexture = NULL;
    UINT m_width = 0L;
    UINT m_height = 0L;
};

class Font : public IFont
{
public:

    Font(LPDIRECT3DDEVICE9 pD3DDevice)
        : m_pD3DDevice(pD3DDevice)
    {
    }

    void Init(const bool bEnglish)
    {
        if (!bEnglish)
        {
            HRESULT hr = D3DXCreateFont(m_pD3DDevice,
                                        20,
                                        0,
                                        FW_THIN,
                                        1,
                                        false,
                                        SHIFTJIS_CHARSET,
                                        OUT_TT_ONLY_PRECIS,
                                        ANTIALIASED_QUALITY,
                                        FF_DONTCARE,
                                        _T("游明朝"),
                                        &m_pFont);
        }
        else
        {
            HRESULT hr = D3DXCreateFont(m_pD3DDevice,
                                        20,
                                        0,
                                        FW_BOLD,
                                        1,
                                        false,
                                        DEFAULT_CHARSET,
                                        OUT_TT_ONLY_PRECIS,
                                        CLEARTYPE_QUALITY,
                                        FF_DONTCARE,
                                        _T("Courier New"),
                                        &m_pFont);
        }
    }

    virtual void DrawText_(const std::wstring& msg, const int x, const int y)
    {
        RECT rect = { x, y, 0, 0 };
        m_pFont->DrawText(NULL,
                          msg.c_str(),
                          -1,
                          &rect,
                          DT_LEFT | DT_NOCLIP,
                          D3DCOLOR_ARGB(255, 255, 255, 255));
    }

    ~Font()
    {
        m_pFont->Release();
    }

    void OnDeviceLost()
    {
        m_pFont->OnLostDevice();
    }

    void OnDeviceReset()
    {
        m_pFont->OnResetDevice();
    }

private:

    LPDIRECT3DDEVICE9 m_pD3DDevice = NULL;
    LPD3DXFONT m_pFont = NULL;
};


LPDIRECT3D9 g_pD3D = NULL;
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;
LPD3DXFONT g_pFont = NULL;
LPD3DXMESH g_pMesh = NULL;
D3DMATERIAL9* g_pMaterials = NULL;
LPDIRECT3DTEXTURE9* g_pTextures = NULL;
DWORD dwNumMaterials = 0;
LPD3DXEFFECT g_pEffect = NULL;
D3DXMATERIAL* d3dxMaterials = NULL;
float f = 0.0f;
bool bShowMenu = true;

HUD g_menu;

static void TextDraw(LPD3DXFONT pFont, wchar_t* text, int X, int Y)
{
    RECT rect = { X,Y,0,0 };
    pFont->DrawText(NULL, text, -1, &rect, DT_LEFT | DT_NOCLIP, D3DCOLOR_ARGB(255, 0, 0, 0));
}

static HRESULT InitD3D(HWND hWnd)
{
    if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
    {
        return E_FAIL;
    }

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.BackBufferCount = 1;
    d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
    d3dpp.MultiSampleQuality = 0;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    d3dpp.hDeviceWindow = hWnd;
    d3dpp.Flags = 0;
    d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

    if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice)))
    {
        if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice)))
        {
            return(E_FAIL);
        }
    }

    HRESULT hr = D3DXCreateFont(g_pd3dDevice,
                                20,
                                0,
                                FW_HEAVY,
                                1,
                                false,
                                SHIFTJIS_CHARSET,
                                OUT_TT_ONLY_PRECIS,
                                ANTIALIASED_QUALITY,
                                FF_DONTCARE,
                                _T("ＭＳ ゴシック"),
                                &g_pFont);

    if FAILED(hr)
    {
        return(E_FAIL);
    }

    LPD3DXBUFFER pD3DXMtrlBuffer = NULL;

    if (FAILED(D3DXLoadMeshFromX(_T("cube.x"), D3DXMESH_SYSTEMMEM,
        g_pd3dDevice, NULL, &pD3DXMtrlBuffer, NULL,
        &dwNumMaterials, &g_pMesh)))
    {
        MessageBox(NULL, _T("Xファイルの読み込みに失敗しました"), NULL, MB_OK);
        return E_FAIL;
    }
    d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
    g_pMaterials = new D3DMATERIAL9[dwNumMaterials];
    g_pTextures = new LPDIRECT3DTEXTURE9[dwNumMaterials];

    for (DWORD i = 0; i < dwNumMaterials; i++)
    {
        int len = MultiByteToWideChar(CP_ACP, 0, d3dxMaterials[i].pTextureFilename, -1, NULL, 0);
        std::wstring texFilename(len, 0);
        MultiByteToWideChar(CP_ACP, 0, d3dxMaterials[i].pTextureFilename, -1, &texFilename[0], len);

        g_pMaterials[i] = d3dxMaterials[i].MatD3D;
        g_pMaterials[i].Ambient = g_pMaterials[i].Diffuse;
        g_pTextures[i] = NULL;
        if (!texFilename.empty())
        {
            if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice,
                                                 texFilename.c_str(),
                                                 &g_pTextures[i])))
            {
                MessageBox(NULL, _T("テクスチャの読み込みに失敗しました"), NULL, MB_OK);
            }
        }
    }
    pD3DXMtrlBuffer->Release();

    D3DXCreateEffectFromFile(g_pd3dDevice,
                             _T("simple.fx"),
                             NULL,
                             NULL,
                             D3DXSHADER_DEBUG,
                             NULL,
                             &g_pEffect,
                             NULL );

    Sprite* sprBack = new Sprite(g_pd3dDevice);
    sprBack->Load(_T("status_back.png"));

    Sprite* sprMiddle = new Sprite(g_pd3dDevice);
    sprMiddle->Load(_T("status_middle.png"));

    Sprite* sprFront = new Sprite(g_pd3dDevice);
    sprFront->Load(_T("status_front.png"));

    IFont* pFont = new Font(g_pd3dDevice);

    g_menu.Init(pFont, sprBack, sprMiddle, sprFront, true);
    
//     g_menu.UpsertStatus(_T("身体のスタミナ"), 100, 100, true);
//     g_menu.UpsertStatus(_T("脳のスタミナ"), 10, 20, true);
//     g_menu.UpsertStatus(_T("水分"), 10, 40, true);
//     g_menu.UpsertStatus(_T("糖分"), 50, 100, true);
//     g_menu.UpsertStatus(_T("タンパク質"), 60, 80, true);
// //    g_menu.UpsertStatus(_T("脂質"), 100, 100, true);
// //    g_menu.UpsertStatus(_T("ビタミン"), 100, 100, true);
// //    g_menu.UpsertStatus(_T("ミネラル"), 100, 100, true);
//     g_menu.UpsertStatus(_T("頭痛"), 100, 100, false);
//     g_menu.UpsertStatus(_T("腹痛"), 100, 100, false);

    g_menu.UpsertStatus(_T("Body stamina"), 100, 100, true);
    g_menu.UpsertStatus(_T("Brain stamina"), 10, 20, true);
    g_menu.UpsertStatus(_T("Hydrogen"), 10, 40, true);
    g_menu.UpsertStatus(_T("Carbo"), 50, 100, true);
    g_menu.UpsertStatus(_T("Protein"), 60, 80, true);
//    g_menu.UpsertStatus(_T("脂質"), 100, 100, true);
//    g_menu.UpsertStatus(_T("ビタミン"), 100, 100, true);
//    g_menu.UpsertStatus(_T("ミネラル"), 100, 100, true);
    g_menu.UpsertStatus(_T("Headache"), 100, 100, false);
    g_menu.UpsertStatus(_T("Stomacache"), 100, 100, false);

    return S_OK;
}

static VOID Cleanup()
{
    SAFE_RELEASE(g_pMesh);
    SAFE_RELEASE(g_pFont);
    SAFE_RELEASE(g_pd3dDevice);
    SAFE_RELEASE(g_pD3D);
}

static VOID Render()
{
    if (NULL == g_pd3dDevice)
    {
        return;
    }
    f += 0.010f;

    D3DXMATRIX mat;
    D3DXMATRIX View, Proj;
    D3DXMatrixPerspectiveFovLH(&Proj, D3DXToRadian(45), 1600.0f / 900.0f, 1.0f, 1000.0f);
    D3DXVECTOR3 vec1(3 * sinf(f), 3, -3 * cosf(f));
    D3DXVECTOR3 vec2(0, 0, 0);
    D3DXVECTOR3 vec3(0, 1, 0);
    D3DXMatrixLookAtLH(&View, &vec1, &vec2, &vec3);
    D3DXMatrixIdentity(&mat);
    mat = mat * View * Proj;
    g_pEffect->SetMatrix("matWorldViewProj", &mat);

    g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        D3DCOLOR_XRGB(70, 50, 30), 1.0f, 0);

    if (SUCCEEDED(g_pd3dDevice->BeginScene()))
    {
        wchar_t msg[128];
        wcscpy_s(msg, 128, _T("Cキーでステータスを表示"));
        TextDraw(g_pFont, msg, 0, 0);

        g_pEffect->SetTechnique("BasicTec");
        UINT numPass;
        g_pEffect->Begin(&numPass, 0);
        g_pEffect->BeginPass(0);
        for (DWORD i = 0; i < dwNumMaterials; i++)
        {
            g_pEffect->SetTexture("texture1", g_pTextures[i]);
            g_pMesh->DrawSubset(i);
        }
        if (bShowMenu)
        {
            g_menu.Draw();
        }
        g_pEffect->EndPass();
        g_pEffect->End();
        g_pd3dDevice->EndScene();
    }

    g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

bool g_bFullScreen = false;
bool g_bSmallWindow = false;

static LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        Cleanup();
        PostQuitMessage(0);
        return 0;
    case WM_SIZE:
        InvalidateRect(hWnd, NULL, true);
        return 0;
    case WM_KEYDOWN:
        switch (wParam)
        {
        case 'C':
            if (bShowMenu == false)
            {
                bShowMenu = true;
            }
            break;
        // メニューを表示している最中にメニューに表示されている内容を変える
        case VK_F11:
        {
            if (g_bFullScreen)
            {
                g_bFullScreen = false;
            }
            else
            {
                g_bFullScreen = true;
            }

            D3DPRESENT_PARAMETERS d3dpp;
            ZeroMemory(&d3dpp, sizeof(d3dpp));

            if (g_bFullScreen)
            {
                d3dpp.Windowed = FALSE;
                d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
                d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
                d3dpp.BackBufferCount = 1;
                d3dpp.BackBufferWidth = 1920;
                d3dpp.BackBufferHeight = 1080;
                d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
                d3dpp.MultiSampleQuality = 0;
                d3dpp.EnableAutoDepthStencil = TRUE;
                d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
                d3dpp.hDeviceWindow = hWnd;
                d3dpp.Flags = 0;
                d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
                d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
            }
            else
            {
                d3dpp.Windowed = TRUE;
                d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
                d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
                d3dpp.BackBufferCount = 1;
                d3dpp.BackBufferWidth = 0;
                d3dpp.BackBufferHeight = 0;
                d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
                d3dpp.MultiSampleQuality = 0;
                d3dpp.EnableAutoDepthStencil = TRUE;
                d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
                d3dpp.hDeviceWindow = hWnd;
                d3dpp.Flags = 0;
                d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
                d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
            }

            HRESULT hr = S_OK;
            hr = g_pEffect->OnLostDevice();
            hr = g_pFont->OnLostDevice();
            g_menu.OnDeviceLost();

            hr = g_pd3dDevice->Reset(&d3dpp);

            g_menu.OnDeviceReset();
            hr = g_pFont->OnResetDevice();
            hr = g_pEffect->OnResetDevice();

            break;
        }
        case VK_ESCAPE:
            PostQuitMessage(0);
            break;
        }
        break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

extern INT WINAPI wWinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ INT);
INT WINAPI wWinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ INT)
{
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      _T("Window1"), NULL };
    RegisterClassEx(&wc);

    RECT rect;
    SetRect(&rect, 0, 0, 1600, 900);
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    rect.right = rect.right - rect.left;
    rect.bottom = rect.bottom - rect.top;
    rect.top = 0;
    rect.left = 0;

    HWND hWnd = CreateWindow(_T("Window1"),
                             _T("Hello DirectX9 World !!"),
                             WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             rect.right,
                             rect.bottom,
                             NULL,
                             NULL,
                             wc.hInstance,
                             NULL);

    if (SUCCEEDED(InitD3D(hWnd)))
    {
        ShowWindow(hWnd, SW_SHOWDEFAULT);
        UpdateWindow(hWnd);
        MSG msg = {};

        for (;;)
        {
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    return (int)msg.wParam;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            // 毎フレーム描画
            Render();
        }
    }

    UnregisterClass(_T("Window1"), wc.hInstance);
    return 0;
}
