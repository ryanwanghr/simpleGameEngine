#include <windows.h>
#include <Objbase.h>
#include <stdint.h>
#include <xaudio2.h>
#include <xinput.h>


#define internal static
#define global_variable static
#define local_persist static 

struct bitmapBuffer {
    BITMAPINFO info;

    void* memory;
    int width;
    int height;
    int bytesPerPixel;
    int pitch;
};

#define X_INPUT_GET_STATE(name) DWORD name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(XInputGetState_t);
X_INPUT_GET_STATE(x_input_get_state_stub) {
    return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable XInputGetState_t *XInputGetState_ = &x_input_get_state_stub;
#define XInputGetState XInputGetState_


global_variable bool GlobalRunning = true;
global_variable struct bitmapBuffer GlobalBitmapBuffer;

internal void win32LoadAudio() {

}

internal void win32LoadXInput() {
    HMODULE libraryModule = LoadLibrary("xinput1_3.dll");
    if (libraryModule) {
        XInputGetState = (XInputGetState_t*) GetProcAddress(libraryModule, "XInputGetState");
    }
}

internal void win32InitSound() {
}

internal void win32MakePattern() {
    if (!GlobalBitmapBuffer.memory) {
        return;
    }
    uint8_t* row = (uint8_t*) GlobalBitmapBuffer.memory;
    local_persist uint8_t offset = 0;
    for (int y = 0; y < GlobalBitmapBuffer.height; y++) {
        uint32_t* pixel = (uint32_t*) row;
        for (int x = 0; x < GlobalBitmapBuffer.width; x++) {
            uint8_t blue = (uint8_t) 0x20 + x + offset;
            uint8_t green = (uint8_t) 0x10 + offset + y;
            uint8_t red = (uint8_t) offset * (0x30 + x - y);
            *pixel = (blue | (green << 8) | (red << 16));

            pixel++;
        }

        row+=GlobalBitmapBuffer.pitch;
    }
    offset++;

 }
internal void win32ResizeBitmapSize(int width, int height) {
    if (GlobalBitmapBuffer.memory) {
        VirtualFree(GlobalBitmapBuffer.memory, 0, MEM_RELEASE);
    }

    GlobalBitmapBuffer.width = width;
    GlobalBitmapBuffer.height = height;
    GlobalBitmapBuffer.info.bmiHeader.biWidth = width;
    GlobalBitmapBuffer.info.bmiHeader.biHeight = -height;
    GlobalBitmapBuffer.pitch = width*GlobalBitmapBuffer.bytesPerPixel;

    int bitmapBytesNeeded = (width*height)*GlobalBitmapBuffer.bytesPerPixel;
    GlobalBitmapBuffer.memory = VirtualAlloc(0, bitmapBytesNeeded, MEM_COMMIT, PAGE_READWRITE);
}

internal void win32PaintWindow(HDC DeviceContext, RECT *windowRect) {
    int windowX = windowRect->left;
    int windowY = windowRect->top;
    int windowHeight = windowRect->bottom - windowRect->top;
    int windowWidth = windowRect->right - windowRect->left;

    int bitmapX = 0;
    int bitmapY = 0;
    int bitmapWidth = GlobalBitmapBuffer.width;
    int bitmapHeight = GlobalBitmapBuffer.height;
    StretchDIBits(DeviceContext, windowX, windowY, windowWidth, windowHeight,
                                 bitmapX, bitmapY, bitmapWidth, bitmapHeight, 
                                 GlobalBitmapBuffer.memory, &GlobalBitmapBuffer.info, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT Wndproc(
  HWND windowHandle,
  UINT message,
  WPARAM WParam,
  LPARAM lParam
)
{
    int result = 0;
    switch (message) {
        case (WM_CLOSE): 
            GlobalRunning = false;
            break;
        case (WM_DESTROY): 
            GlobalRunning = false;
            break;
        case (WM_SIZE): 
            break;
        case (WM_KEYDOWN):
        case (WM_KEYUP):
        case (WM_SYSKEYDOWN):
        case (WM_SYSKEYUP): {
            WPARAM virtualKeyCode = WParam;

            bool keyWasDown = (lParam & (1 << 30)) != 0;
            bool keyWentUp = (lParam & (1 << 31)) != 0;
            bool altIsDown = (lParam & (1 << 29)) != 0;
            switch (virtualKeyCode) {
                case(VK_F4):
                    if (altIsDown) {
                        GlobalRunning = false;
                    }
                    break;
                case(VK_BACK): 
                    break;
                case(VK_TAB):
                    break;
                case(VK_SHIFT):
                    break;
                case(VK_MENU):
                    break;
                case(VK_ESCAPE):
                    break;
                case(VK_SPACE):
                    break;
                case(VK_END):
                    break;
                case(VK_UP):
                    break;
                case(VK_LEFT):
                    break;
                case(VK_DOWN):
                    break;
                case(VK_RIGHT):
                    break;
                case('W'):
                    break;
                case('A'):
                    break;
                case('S'):
                    break;
                case('D'):
                    break;
                default:
                    break;
            }
            break;
        }
        case (WM_ACTIVATEAPP):  
            break;
        case (WM_PAINT): {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(windowHandle, &Paint);
            win32PaintWindow(DeviceContext, &Paint.rcPaint);
            EndPaint(windowHandle, &Paint);
            break;
        }
        default: 
            result = DefWindowProc(windowHandle, message, WParam, lParam);
            break;
        
    }
    return result;
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR lpCmdLine, int nCmdShow)
{
    win32LoadXInput();
    win32LoadAudio();

    int numChannels = 2;
    int monoBitsPerSample = 16;
    int samplesPerSecond = 48000;
    int bytesPerSample = (monoBitsPerSample * numChannels)/8;
    int avgBytesPerSec = samplesPerSecond * bytesPerSample;

    int amplitude = 300;
    int frequency = 440;
    
    pAudioData = VirtualAlloc(0, avgBytesPerSec, MEM_COMMIT, PAGE_READWRITE);

    int samplesPerCycle = samplesPerSecond/frequency;
    int samplesUntilChangeSign = samplesPerCycle/2;
    bool positive = true;
    for (int sampleCount = 0; sampleCount < samplesPerSecond; sampleCount++) {
        int32_t* sample = pAudioData + sampleCount;
        if (sampleCount%samplesUntilChangeSign == 0) {
            positive = !positive;
        }
        if (positive) {
            *sample = amplitude;
        }
        if (!positive) {
            *sample = -amplitude;
        }
    }
    for (int32_t* sample = pAudioData; sample 
    IXAudio2* pAudioInterface = nullptr;
    if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED))) {
        OutputDebugStringA("CoInitializeEx Failed\n");
        GlobalRunning = false;
        return 1;
    }

    if (FAILED(XAudio2Create(&pAudioInterface, 0, XAUDIO2_DEFAULT_PROCESSOR))) {
        OutputDebugStringA("XAudio2Create Failed\n");
        GlobalRunning = false;
        return 1;
    }

    IXAudio2MasteringVoice* pMasterVoice = nullptr;
    if (FAILED(pAudioInterface->CreateMasteringVoice(&pMasterVoice))) {
        OutputDebugStringA("CreateMasteringVoice Failed\n");
        GlobalRunning = false;
        return 1;
    }

    WAVEFORMATEX waveFormat = {0}; 
    waveFormat.wFormatTag = WAVE_FORMAT_PCM; 
    waveFormat.nChannels = numChannels;       
    waveFormat.nSamplesPerSec = samplesPerSecond;
    waveFormat.nAvgBytesPerSec = avgBytesPerSec;
    waveFormat.nBlockAlign = bytesPerSample;
    waveFormat.wBitsPerSample = monoBitsPerSample;
    waveFormat.cbSize = 0;

    IXAudio2SourceVoice* pSourceVoice = nullptr;
    if (FAILED(pAudioInterface->CreateSourceVoice(&pSourceVoice, &waveFormat, 0, 1.0, 0, 0, 0))) {
        GlobalRunning = false;
        OutputDebugStringA("CreateSourceVoice Failed");
        return 1;
    }

    

    

    XAUDIO2_BUFFER audioBuffer;          
    audioBuffer.Flags = 0;
    audioBuffer.AudioBytes = avgBytesPerSec;
    audioBuffer.pAudioData = pAudioData;
    audioBuffer.
    audioBuffer.
    audioBuffer.
    audioBuffer.
    audioBuffer.
    audioBuffer.
    audioBuffer.

    
    pSourceVoice->SubmitSourceBuffer()

    WNDCLASSEX windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = Wndproc;
    windowClass.hInstance = hInstance;
    windowClass.lpszClassName = "handmadeHero";

    GlobalBitmapBuffer.info.bmiHeader.biSize = sizeof(GlobalBitmapBuffer.info.bmiHeader);
    GlobalBitmapBuffer.info.bmiHeader.biPlanes = 1;
    GlobalBitmapBuffer.info.bmiHeader.biBitCount = 32;
    GlobalBitmapBuffer.info.bmiHeader.biCompression = BI_RGB;
    GlobalBitmapBuffer.bytesPerPixel = 4;
    int registrationSuceeded = RegisterClassExA(&windowClass);

    if (!registrationSuceeded) {
        OutputDebugStringA("Class registration failed");
        return 1;
    }
    HWND windowHandle = CreateWindowEx(
        0,
        windowClass.lpszClassName,
        "Handmade Hero",
        WS_OVERLAPPEDWINDOW|WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        hInstance,
        0);

    if (!windowHandle) {
        OutputDebugStringA("Window handle creation failed");
        return 1;
    }

    win32ResizeBitmapSize(1280, 720);

    MSG Message;
    while (GlobalRunning) {
        while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&Message);
            DispatchMessage(&Message); 
        }
        
        DWORD dwResult;    
        for (DWORD i=0; i < XUSER_MAX_COUNT; i++)
        {
            XINPUT_STATE state;
            ZeroMemory( &state, sizeof(XINPUT_STATE) );
            dwResult = XInputGetState( i, &state );
            if( dwResult != ERROR_SUCCESS )
            {
                break;
            }
            // do something
            state.Gamepad.wButtons;
            state.Gamepad.bLeftTrigger;
            state.Gamepad.bRightTrigger;
            state.Gamepad.sThumbLX;
            state.Gamepad.sThumbLY;
            state.Gamepad.sThumbRX;
            state.Gamepad.sThumbRY;
        }

        win32MakePattern();
        
        HDC deviceContext = GetDC(windowHandle);
        RECT clientRect;
        GetClientRect(windowHandle, &clientRect);
        win32PaintWindow(deviceContext, &clientRect);
        ReleaseDC(windowHandle, deviceContext);
    }
    return 0;
}
