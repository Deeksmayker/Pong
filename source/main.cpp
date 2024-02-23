#include <stdio.h>
#include <windows.h>
#include <stdint.h>

#define global_variable static
#define local_persist   static
#define internal        static

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

global_variable bool Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;

global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesPerPixel = 4;


internal void
RenderFunnyGradient(int XOffset, int YOffset){
    int Pitch = BitmapWidth * BytesPerPixel;
    u8 *ROW = (u8 *) BitmapMemory;
    for (int Y = 0; Y < BitmapHeight; Y++){    
        u32 *Pixel = (u32 *) ROW;
        for (int X = 0; X < BitmapWidth; X++){
            u8 blue = (X + XOffset);
            u8 green = (Y + YOffset);
            *Pixel++ = (u32)((green << 8) | blue);
        }
        ROW += Pitch;
    }

}

internal void
Win32ResizeDIBSection(int Width, int Height){
    if (BitmapMemory){
        VirtualFree(BitmapMemory, 0, MEM_RELEASE);
    }
    
    BitmapWidth = Width;
    BitmapHeight = Height;

    BitmapInfo.bmiHeader.biSize        = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth       = BitmapWidth;
    BitmapInfo.bmiHeader.biHeight      = -BitmapHeight;
    BitmapInfo.bmiHeader.biPlanes      = 1;
    BitmapInfo.bmiHeader.biBitCount    = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;
    
    int BitmapMemorySize = BytesPerPixel * BitmapWidth * BitmapHeight;
    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    
} 

internal void
Win32UpdateWindow(HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height){
    int WindowWidth = ClientRect->right - ClientRect->left;
    int WindowHeight = ClientRect->bottom - ClientRect->top;
    StretchDIBits(DeviceContext,
                  /*
                  X, Y, Width, Height,
                  X, Y, Width, Height,
                  */
                  0, 0, BitmapWidth, BitmapHeight,
                  0, 0, WindowWidth, WindowHeight,
                  BitmapMemory,
                  &BitmapInfo,
                  DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
                        UINT Message,
                        WPARAM WParam,
                        LPARAM LParam)
{
    LRESULT Result = 0;
    
    switch (Message){
        case WM_SIZE:{
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            int Width = ClientRect.right - ClientRect.left;
            int Height = ClientRect.bottom - ClientRect.top;
            Win32ResizeDIBSection(Width, Height);
        } break;
        
        case WM_CLOSE:{
            Running = false;
        } break;
        
        case WM_DESTROY:{
            Running = false;
        } break;
        
        case WM_PAINT:{
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);

            
            Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);
            
            //PatBlt(DeviceContext, X, Y, Width, Height, WHITENESS);
  
            EndPaint(Window, &Paint);
        } break;
        
        default:{
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }

    return (Result);
}

int CALLBACK WinMain(HINSTANCE Instance,
                     HINSTANCE PrevInstance,
                     LPSTR CommandLine,
                     int ShowConsole)
{
    WNDCLASS WindowClass = {0};    
    
    WindowClass.style       = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance   = Instance;
    
    WindowClass.lpszClassName = "CoolNotPongGameClass";
    
    AttachConsole(ATTACH_PARENT_PROCESS);
    //for printf to work
    freopen("CONOUT$", "w", stdout);

    if (!RegisterClass(&WindowClass)){
        printf("oshiblis");
    }
    HWND Window = CreateWindowExA(
                             0,
                             WindowClass.lpszClassName,
                             "Not Pong",
                             WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             0,
                             0,
                             Instance,
                             0);
    if (!Window){
        printf("oshibkaa");
    }
    
    Running = true;
    while(Running){
        MSG Message;
        while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)){
            if (Message.message == WM_QUIT){
                Running = false;
            }
        
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
        
        local_persist int aboba = 0;
        RenderFunnyGradient(aboba, aboba);
        
        
        HDC DeviceContext = GetDC(Window);
        RECT ClientRect;
        GetClientRect(Window, &ClientRect);
        int WindowHeight = ClientRect.bottom - ClientRect.top;
        int WindowWidth = ClientRect.right - ClientRect.left;
        Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
        ReleaseDC(Window, DeviceContext);

        aboba += 1;

    }                                 
    return (0);
}

