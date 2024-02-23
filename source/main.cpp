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

struct win32_offscreen_buffer{ 
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel = 4;
    
} ;

global_variable bool Running;
global_variable win32_offscreen_buffer GlobalBackBuffer;

struct win32_window_dimension{
    int Width;
    int Height;
};

win32_window_dimension 
Win32GetWindowDimension(HWND Window){
    win32_window_dimension Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;
    return Result;  
}

internal void
RenderFunnyGradient(win32_offscreen_buffer *Buffer, int XOffset, int YOffset){
    u8 *ROW = (u8 *) Buffer->Memory;
    for (int Y = 0; Y < Buffer->Height; Y++){    
        u32 *Pixel = (u32 *) ROW;
        for (int X = 0; X < Buffer->Width; X++){
            u8 blue = (X + XOffset);
            u8 green = (Y + YOffset);
            *Pixel++ = (u32)((green << 8) | blue);
        }
        ROW += Buffer->Pitch;
    }

}

internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height){
    if (Buffer->Memory){
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }
    
    Buffer->Width  = Width;
    Buffer->Height = Height;
    
    Buffer->Info.bmiHeader.biSize        = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth       = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight      = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes      = 1;
    Buffer->Info.bmiHeader.biBitCount    = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;
    
    int BitmapMemorySize = Buffer->BytesPerPixel * Buffer->Width * Buffer->Height;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    
    Buffer->Pitch = Buffer->Width * Buffer->BytesPerPixel;
} 

internal void
Win32DisplayBufferInWindow(HDC DeviceContext, int WindowWidth, int WindowHeight,
                           win32_offscreen_buffer *Buffer,
                           int X, int Y, int Width, int Height) // Guys on this line don't used
{
    StretchDIBits(DeviceContext,
                  /*
                  X, Y, Width, Height,
                  X, Y, Width, Height,
                  */
                  0, 0, WindowWidth, WindowHeight,
                  0, 0, Buffer->Width, Buffer->Height,
                  Buffer->Memory,
                  &Buffer->Info,
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
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            u32 VKCode = WParam;
            bool WasDown = (LParam & (1 << 30));
            bool IsDown  = ((LParam & (1 << 31)) == 0);
            if (WasDown && IsDown) break;
            if (VKCode == 'W'){
                if (!WasDown && IsDown){
                    printf("WPRESSED");
                } else if (WasDown && IsDown){
                    printf(" WDOWN");
                } else if (WasDown && !IsDown){
                    printf("WUP");
                }
                printf("\n");
            }
            
            bool AltKeyWasDown = LParam & 1 << 29;
            if (VKCode == VK_F4 && AltKeyWasDown){
                Running = false;
            }
        } break;
    
        case WM_SIZE:{
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
            
            win32_window_dimension Dimension = Win32GetWindowDimension(Window);
            
            Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, &GlobalBackBuffer, X, Y, Width, Height);
            
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
                     int ShowCode)
{
    WNDCLASS WindowClass = {0};    
    
    Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);
    
    WindowClass.style       = CS_HREDRAW | CS_VREDRAW;
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
        RenderFunnyGradient(&GlobalBackBuffer, aboba, aboba);
        
        
        HDC DeviceContext = GetDC(Window);
        
        win32_window_dimension Dimension = Win32GetWindowDimension(Window);
        
        Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, &GlobalBackBuffer, 0, 0, Dimension.Width, Dimension.Height);
        ReleaseDC(Window, DeviceContext);

        aboba += 1;

    }                                 
    return (0);
}

