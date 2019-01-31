#include <math.h>
#include <stdint.h>
#include <windows.h>
#include <Xinput.h>
#include <dsound.h>

#include "handmade.cpp"
#include "win32_handmade.h"


global bool32 GlobalRunning;
global win32_offscreen_buffer GlobalBackbuffer;
global LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;


#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
	return(ERROR_DEVICE_NOT_CONNECTED);
}
global x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
	return(ERROR_DEVICE_NOT_CONNECTED);
}
global x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);


internal debug_read_file_result
DEBUGPlatformReadEntireFile(char *Filename) 
{
	debug_read_file_result Result = {};
    
	HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if(FileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER FileSize;
		if(GetFileSizeEx(FileHandle, &FileSize))
		{
			uint32 FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
			Result.Contents = VirtualAlloc(0, FileSize.QuadPart, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if(Result.Contents)
			{
				DWORD BytesRead;
				if(ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) && (FileSize32 == BytesRead))
				{
					Result.ContentSize = FileSize32;
				}
				else
				{
					DEBUGPlatformFreeFileMemory(Result.Contents);
					Result.Contents = 0;
				}
			}
			else
			{
				// TODO(hunter): Logging
			}
		}
		else
		{
			// TODO(hunter): Logging
		}
        
		CloseHandle(FileHandle);
	}
	else
	{
		// TODO(hunter): Logging
	}
    
	return(Result);
}

internal void DEBUGPlatformFreeFileMemory(void *Memory)
{
	if(Memory)
	{
		VirtualFree(Memory, 0, MEM_RELEASE);
	}
}

internal bool32 
DEBUGPlatformWriteEntireFile(char *Filename, uint64 MemorySize, void *Memory)
{
	bool32 Result = false;
    
	HANDLE FileHandle = CreateFileA(Filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if (FileHandle != INVALID_HANDLE_VALUE)
	{
		DWORD BytesWritten;
		if(WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0))
		{
			Result = (BytesWritten == MemorySize);
		}
		else
		{
			// TODO(hunter): Logging
		}
        
		CloseHandle(FileHandle);
	}
    
	return(Result);
}


internal void
Win32LoadXInput(void)
{
	HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
	if(!XInputLibrary)
	{
		XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
	}
    
	if(!XInputLibrary)
	{
		XInputLibrary = LoadLibraryA("xinput1_3.dll");
	}
    
	if(XInputLibrary)
	{
		XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
		if (!XInputGetState) { XInputGetState = XInputGetStateStub; }
        
		XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
		if (!XInputSetState) { XInputSetState = XInputSetStateStub; }
	}
	else
	{
		// TODO(hunter): Logging
	}
}

internal void
Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize)
{
	// Load Library
	HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
	if(DSoundLibrary)
	{
		direct_sound_create *DirectSoundCreate = (direct_sound_create *)GetProcAddress(DSoundLibrary, "DirectSoundCreate");
		
		LPDIRECTSOUND DirectSound;
		if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
		{
			WAVEFORMATEX WaveFormat = { };
			WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
			WaveFormat.nChannels = 2;
			WaveFormat.nSamplesPerSec = SamplesPerSecond;
			WaveFormat.wBitsPerSample = 16;
			WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
			WaveFormat.nAvgBytesPerSec = (WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign);
			WaveFormat.cbSize = 0;
            
			if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
			{
				DSBUFFERDESC BufferDescription = { };
				BufferDescription.dwSize = sizeof(BufferDescription);
				BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
                
				LPDIRECTSOUNDBUFFER PrimaryBuffer;
				if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
				{					
					if(SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat)))
					{
					}
					else
					{
						// TODO(hunter): Logging
					}
				}
				else
				{
					// TODO(hunter): Logging
				}
			}
			else
			{
				// TODO(hunter): Logging
			}
            
			DSBUFFERDESC BufferDescription = { };
			BufferDescription.dwSize = sizeof(BufferDescription);
			BufferDescription.dwFlags = 0;
			BufferDescription.dwBufferBytes = BufferSize;
			BufferDescription.lpwfxFormat = &WaveFormat;
            
			if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0)))
			{
                
			}
		}
		else
		{
			// TODO(hunter): Logging
		}
	}
	else
	{
		// TODO(hunter): Logging
	}
}

internal win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
	win32_window_dimension Result;
    
	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	Result.Width = ClientRect.right - ClientRect.left;
	Result.Height = ClientRect.bottom - ClientRect.top;
    
	return(Result);
}


internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
	if (Buffer->Memory)
	{
		VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
	}

	Buffer->Width = Width;
	Buffer->Height = Height;

	int BytesPerPixel = 4;

	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
	Buffer->Info.bmiHeader.biWidth = Buffer->Width;
	Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
	Buffer->Info.bmiHeader.biPlanes = 1;
	Buffer->Info.bmiHeader.biBitCount = 32;
	Buffer->Info.bmiHeader.biCompression = BI_RGB;

	int BitmapMemorySize = (Buffer->Width * Buffer->Height) * BytesPerPixel;
	Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	Buffer->Pitch = (Width * BytesPerPixel);
}

internal void
Win32DisplayBufferInWindow(win32_offscreen_buffer *Buffer, HDC DeviceContext,
                        int WindowWidth, 
                        int WindowHeight)
{
	StretchDIBits(DeviceContext, 
				0, 0, WindowWidth, WindowHeight,
				0, 0, Buffer->Width, Buffer->Height,
				Buffer->Memory, &Buffer->Info,
				DIB_RGB_COLORS, SRCCOPY);
}


internal LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
                    UINT Message,
                    WPARAM WParam,
                    LPARAM LParam)
{
	LRESULT Result = 0;
    
	switch(Message)
	{
		case WM_SIZE:
		{
            
		} break;
		
		case WM_DESTROY:
		{
			GlobalRunning = false;
		} break;
        
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			
		} break;
        
		case WM_CLOSE:
		{
			GlobalRunning = false;
		} break;
        
		case WM_ACTIVATEAPP:
		{
            
		} break;
        
		case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(Window, &Paint);
            
			int Height = (Paint.rcPaint.bottom - Paint.rcPaint.top);
			int Width = (Paint.rcPaint.right - Paint.rcPaint.left);
            
			RECT ClientRect;
			GetClientRect(Window, &ClientRect);
            
			win32_window_dimension Dimension = Win32GetWindowDimension(Window);
			Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext,
										Dimension.Width, Dimension.Height);
            
			EndPaint(Window, &Paint);
		} break;
        
		default:
		{
			Result = DefWindowProc(Window, Message, WParam, LParam);
		} break;
	}
    
	return(Result);
}


internal void
Win32ClearBuffer(win32_sound_output *SoundOutput)
{
	VOID *Region1;
	DWORD Region1Size;
	VOID *Region2;
	DWORD Region2Size;

	if (SUCCEEDED(GlobalSecondaryBuffer->Lock(0, SoundOutput->SecondaryBufferSize,
		&Region1, &Region1Size,
		&Region2, &Region2Size,
		0)))
	{
		uint8 *DestSample = (uint8 *)Region1;
		for (DWORD ByteIndex = 0; ByteIndex < Region1Size; ++ByteIndex)
		{
			*DestSample++ = 0;
		}

		DestSample = (uint8 *)Region2;
		for (DWORD ByteIndex = 0; ByteIndex < Region2Size; ++ByteIndex)
		{
			*DestSample++ = 0;
		}

		GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
	}
}

internal void
Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD ByteToLock, DWORD BytesToWrite,
                    game_sound_output_buffer *SourceBuffer)
{
	VOID *Region1;
	DWORD Region1Size;
	VOID *Region2;
	DWORD Region2Size;
	if (SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite,
                                            &Region1, &Region1Size,
                                            &Region2, &Region2Size,
                                            0)))
	{
		DWORD Region1SampleCount = Region1Size / SoundOutput->BytesPerSample;
		int16 *DestSample = (int16 *)Region1;
		int16 *SourceSample = SourceBuffer->Samples;
		for (DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex)
		{
			*DestSample++ = *SourceSample++;
			*DestSample++ = *SourceSample++;
			++SoundOutput->RunningSampleIndex;
		}

		DWORD Region2SampleCount = Region2Size / SoundOutput->BytesPerSample;
		DestSample = (int16 *)Region2;
		for (DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex)
		{
			*DestSample++ = *SourceSample++;
			*DestSample++ = *SourceSample++;
			++SoundOutput->RunningSampleIndex;
		}

		GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
	}
}


internal void
Win32ProcessKeyboardMessage(game_button_state *NewState, bool32 IsDown)
{   
	Assert(NewState->EndedDown != IsDown);
	NewState->EndedDown = IsDown;
	++NewState->HalfTransitionCount;
}


internal void
Win32ProcessXInputDigitalButton(DWORD XInputButtonState, game_button_state *OldState, DWORD ButtonBit, 
							game_button_state *NewState)
{    
	NewState->EndedDown = ((XInputButtonState & ButtonBit) == ButtonBit);
	NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown) ? 1 : 0;
}


internal void
Win32ProcessPendingMessages(game_controller_input *KeyboardController)
{
	MSG Message;
	while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
	{
		if (Message.message == WM_QUIT)
		{
			GlobalRunning = false;
		}

		switch (Message.message)
		{
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYDOWN:
			case WM_KEYUP:
			{
				WPARAM VKCode = Message.wParam;
				bool32 WasDown = ((Message.lParam & (1 << 30)) != 0);
				bool32 IsDown = ((Message.lParam & (1 << 31)) == 0);

				if (WasDown != IsDown)
				{
					if (VKCode == 'W')
					{
						Win32ProcessKeyboardMessage(&KeyboardController->MoveUp, IsDown);
					}
					else if (VKCode == 'A')
					{
						Win32ProcessKeyboardMessage(&KeyboardController->MoveLeft, IsDown);
					}
					else if (VKCode == 'S')
					{
						Win32ProcessKeyboardMessage(&KeyboardController->MoveDown, IsDown);
					}
					else if (VKCode == 'D')
					{
						Win32ProcessKeyboardMessage(&KeyboardController->MoveRight, IsDown);
					}
					else if (VKCode == 'Q')
					{
						Win32ProcessKeyboardMessage(&KeyboardController->LeftShoulder, IsDown);
					}
					else if (VKCode == 'E')
					{
						Win32ProcessKeyboardMessage(&KeyboardController->RightShoulder, IsDown);
					}
					else if (VKCode == VK_UP)
					{
						Win32ProcessKeyboardMessage(&KeyboardController->ActionUp, IsDown);
					}
					else if (VKCode == VK_LEFT)
					{
						Win32ProcessKeyboardMessage(&KeyboardController->ActionLeft, IsDown);
					}
					else if (VKCode == VK_DOWN)
					{
						Win32ProcessKeyboardMessage(&KeyboardController->ActionDown, IsDown);
					}
					else if (VKCode == VK_RIGHT)
					{
						Win32ProcessKeyboardMessage(&KeyboardController->ActionRight, IsDown);
					}
					else if (VKCode == VK_ESCAPE)
					{
						Win32ProcessKeyboardMessage(&KeyboardController->Start, IsDown);
					}
					else if (VKCode == VK_SPACE)
					{
						Win32ProcessKeyboardMessage(&KeyboardController->Back, IsDown);
					}
				}

				bool32 AltKeyWasDown = ((Message.lParam &(1 << 29)));
				if ((VKCode == VK_F4) && AltKeyWasDown)
				{
					GlobalRunning = false;
				}
			}
			default:
			{
				TranslateMessage(&Message);
				DispatchMessage(&Message);
			} break;
		}
	}
}

internal real32
Win32ProcessXInputStickValue(SHORT Value, SHORT DeadZoneThreshold)
{
	real32 Result = 0;
	if (Value < -DeadZoneThreshold)
	{
		Result = (real32)Value / 32768.0f;
	}
	else if (Value > DeadZoneThreshold)
	{
		Result = (real32)Value / 32768.0f;
	}

	return(Result);
}

int CALLBACK WinMain(HINSTANCE Instance,
	HINSTANCE PrevInstance,
	LPSTR CommandLine,
	int ShowCode)
{
	LARGE_INTEGER PerfCountFrequencyResult;
	QueryPerformanceFrequency(&PerfCountFrequencyResult);
	int64 PerfCountFrequency = PerfCountFrequencyResult.QuadPart;

	Win32LoadXInput();

	WNDCLASSA WindowClass = {};

	Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);

	WindowClass.style = (CS_HREDRAW | CS_VREDRAW);
	WindowClass.lpfnWndProc = Win32MainWindowCallback;
	WindowClass.hInstance = Instance;
	//WindowClass.hIcon = ;
	WindowClass.lpszClassName = "HandmadeHeroWindowClass";

	if (RegisterClass(&WindowClass))
	{
		HWND Window = CreateWindowExA(0,
			WindowClass.lpszClassName,
			"Handmade Hero",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT,
			0, 0, Instance, 0);

		if (Window)
		{
			HDC DeviceContext = GetDC(Window);

			int XOffset = 0;
			int YOffset = 0;

			win32_sound_output SoundOutput = { };
			SoundOutput.SamplesPerSecond = 48000;
			SoundOutput.RunningSampleIndex = 0;
			SoundOutput.BytesPerSample = sizeof(int16) * 2;
			SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;
			SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond / 15;
			Win32InitDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
			Win32ClearBuffer(&SoundOutput);
			GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

			GlobalRunning = true;

			int16 *Samples = (int16 *)VirtualAlloc(0, SoundOutput.SecondaryBufferSize,
				MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);


#if HANDMADE_INTERNAL
			LPVOID BaseAddress = (LPVOID)Terabytes((uint64)2);

#else
			LPVOID BaseAddress = 0
#endif

			game_memory GameMemory = {};
			GameMemory.PermanentStorageSize = Megabytes(64);
			GameMemory.TransientStorageSize = Gigabytes(1);

			uint64 TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
			GameMemory.PermanentStorage = VirtualAlloc(BaseAddress, TotalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			GameMemory.TransientStorage = ((uint8 *)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize);

			if (Samples && GameMemory.PermanentStorage && GameMemory.TransientStorage)
			{
				game_input Input[2] = { };
				game_input *NewInput = &Input[0];
				game_input *OldInput = &Input[1];

				LARGE_INTEGER LastCounter;
				QueryPerformanceCounter(&LastCounter);
				uint64 LastCycleCount = __rdtsc();
				while (GlobalRunning)
				{
					game_controller_input *OldKeyboardController = GetController(OldInput, 0);
					game_controller_input *NewKeyboardController = GetController(NewInput, 0);
					*NewKeyboardController = { };

					NewKeyboardController->IsConnected = true;

					for(int ButtonIndex = 0; ButtonIndex < ArrayCount(NewKeyboardController->Buttons); ++ButtonIndex)
					{
						NewKeyboardController->Buttons[ButtonIndex].EndedDown = OldKeyboardController->Buttons[ButtonIndex].EndedDown;
					}

					Win32ProcessPendingMessages(NewKeyboardController);

					DWORD MaxControllerCount = XUSER_MAX_COUNT;
					if (MaxControllerCount > (ArrayCount(NewInput->Controllers) - 1))
					{
						MaxControllerCount = (ArrayCount(NewInput->Controllers)- 1);
					}

					for (DWORD ControllerIndex = 0;
						ControllerIndex < MaxControllerCount;
						++ControllerIndex)
					{
						int OurControllerIndex = ControllerIndex + 1;
						game_controller_input *OldController = GetController(OldInput, OurControllerIndex);
						game_controller_input *NewController = GetController(NewInput, OurControllerIndex);

						XINPUT_STATE ControllerState;
						if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
						{
							NewController->IsConnected = true;
							XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

							NewController->StickAverageX = Win32ProcessXInputStickValue(Pad->sThumbLX,
								XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
							NewController->StickAverageY = Win32ProcessXInputStickValue(Pad->sThumbLY,
								XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

							bool32 Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
							bool32 Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
							bool32 Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
							bool32 Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);


							if((NewController->StickAverageX != 0.0f) || (NewController->StickAverageY != 0.0f))
							{
								NewController->IsAnalog = true;
							}

							if(Up)
							{
								NewController->StickAverageY = 1.0f;
								NewController->IsAnalog = false;
							}
							if (Down)
							{
								NewController->StickAverageY = -1.0f;
								NewController->IsAnalog = false;
							}
							if (Left)
							{
								NewController->StickAverageX = -1.0f;
								NewController->IsAnalog = false;
							}
							if (Right)
							{
								NewController->StickAverageX = 1.0f;
								NewController->IsAnalog = false;
							}

							real32 Threshold = 0.5f;
							Win32ProcessXInputDigitalButton((NewController->StickAverageX < -Threshold) ? 1 : 0,
								&OldController->MoveLeft, 1,
								&NewController->MoveLeft);
							Win32ProcessXInputDigitalButton((NewController->StickAverageX > Threshold) ? 1 : 0,
								&OldController->MoveRight, 1,
								&NewController->MoveRight);
							Win32ProcessXInputDigitalButton((NewController->StickAverageY < -Threshold) ? 1 : 0,
								&OldController->MoveDown, 1,
								&NewController->MoveDown);
							Win32ProcessXInputDigitalButton((NewController->StickAverageY > Threshold) ? 1 : 0,
								&OldController->MoveUp, 1,
								&NewController->MoveUp);

							Win32ProcessXInputDigitalButton(Pad->wButtons,
								&OldController->ActionDown, XINPUT_GAMEPAD_A,
								&NewController->ActionDown);
							Win32ProcessXInputDigitalButton(Pad->wButtons,
								&OldController->ActionRight, XINPUT_GAMEPAD_B,
								&NewController->ActionRight);
							Win32ProcessXInputDigitalButton(Pad->wButtons,
								&OldController->ActionLeft, XINPUT_GAMEPAD_X,
								&NewController->ActionLeft);
							Win32ProcessXInputDigitalButton(Pad->wButtons,
								&OldController->ActionUp, XINPUT_GAMEPAD_Y,
								&NewController->ActionUp);
							Win32ProcessXInputDigitalButton(Pad->wButtons,
								&OldController->LeftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER,
								&NewController->LeftShoulder);
							Win32ProcessXInputDigitalButton(Pad->wButtons,
								&OldController->RightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER,
								&NewController->RightShoulder);
							Win32ProcessXInputDigitalButton(Pad->wButtons,
								&OldController->Start, XINPUT_GAMEPAD_START,
								&NewController->Start);
							Win32ProcessXInputDigitalButton(Pad->wButtons,
								&OldController->Back, XINPUT_GAMEPAD_BACK,
								&NewController->Back);
						}
						else
						{
							// TODO(hunter): Logging
							NewController->IsConnected = false;
						}
					}

					DWORD ByteToLock;
					DWORD PlayCursor;
					DWORD WriteCursor;
					DWORD TargetCursor;
					DWORD BytesToWrite;
					bool32 SoundIsValid = false;

					if (SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
					{
						ByteToLock = ((SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize);
						TargetCursor = ((PlayCursor + (SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample)) % SoundOutput.SecondaryBufferSize);
						BytesToWrite = 0;

						if (ByteToLock > TargetCursor)
						{
							BytesToWrite = (SoundOutput.SecondaryBufferSize - ByteToLock);
							BytesToWrite += TargetCursor;
						}
						else
						{
							BytesToWrite = (TargetCursor - ByteToLock);
						}

						SoundIsValid = true;
					}
					game_sound_output_buffer SoundBuffer = {};
					SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
					SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
					SoundBuffer.Samples = Samples;


					game_offscreen_buffer Buffer = {};
					Buffer.Memory = GlobalBackbuffer.Memory;
					Buffer.Width = GlobalBackbuffer.Width;
					Buffer.Height = GlobalBackbuffer.Height;
					Buffer.Pitch = GlobalBackbuffer.Pitch;
					GameUpdateAndRender(&GameMemory, NewInput, &Buffer, &SoundBuffer);

					if (SoundIsValid)
					{
						Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer);
					}

					win32_window_dimension Dimension = Win32GetWindowDimension(Window);
					Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext,
						Dimension.Width, Dimension.Height);

					uint64 EndCycleCount = __rdtsc();


					LARGE_INTEGER EndCounter;
					QueryPerformanceCounter(&EndCounter);

					uint64 CyclesElapsed = EndCycleCount - LastCycleCount;
					int64 CounterElapsed = (EndCounter.QuadPart - LastCounter.QuadPart);
					int64 MSPerFrame = ((1000 * CounterElapsed) / PerfCountFrequency);
					int32 FPS = (int32)PerfCountFrequency / (int32)CounterElapsed;
					int32 MCPF = (int32)(CyclesElapsed / (1000 * 1000));
#if 0
					char Buffer[256];
					wsprintf(Buffer, "%dms\f, %df/s, %dmc/f\n", MSPerFrame, FPS, MCPF);
					OutputDebugStringA(Buffer);
#endif
					LastCounter = EndCounter;
					LastCycleCount = EndCycleCount;

					game_input *Temp = NewInput;
					NewInput = OldInput;
					OldInput = Temp;
				}
			}
			else
			{
				// TODO(hunter): Logging
			}
		}
		else
		{
			// TODO(hunter): Logging
		}
	}
	else 
	{
		// TODO(hunter): Logging
	}

	return (0);
}