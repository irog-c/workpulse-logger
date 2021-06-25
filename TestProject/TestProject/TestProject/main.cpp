#include <iostream>
#include <string>
#include <fstream>

#include <Windows.h>
#include <lmcons.h>
#include <WinBase.h>
#include <WinUser.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <FunctionDiscoveryKeys_devpkey.h>
//#include <Psapi.h>

#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

#define MAX_NAME 256

BOOL microphone_is_recording()
{
	// #1 Get the audio endpoint associated with the microphone device
	HRESULT hr = S_OK;
	IMMDeviceEnumerator* pEnumerator = NULL;
	IAudioSessionManager2* pSessionManager = NULL;
	BOOL result = FALSE;

	CoInitialize(0);

	// Create the device enumerator.
	hr = CoCreateInstance(
		__uuidof(MMDeviceEnumerator),
		NULL, CLSCTX_ALL,
		__uuidof(IMMDeviceEnumerator),
		(void**)&pEnumerator);

	IMMDeviceCollection* dCol = NULL;
	hr = pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &dCol);
	UINT dCount;
	hr = dCol->GetCount(&dCount);
	for(UINT i = 0; i < dCount; i++)
	{
		IMMDevice* pCaptureDevice = NULL;
		hr = dCol->Item(i, &pCaptureDevice);

		IPropertyStore* pProps = NULL;
		hr = pCaptureDevice->OpenPropertyStore(
			STGM_READ, &pProps);

		PROPVARIANT varName;
		// Initialize container for property value.
		PropVariantInit(&varName);

		// Get the endpoint's friendly-name property.
		hr = pProps->GetValue(
			PKEY_Device_FriendlyName, &varName);

		std::wstring nameStr(varName.pwszVal);

		// #2 Determine whether it is the microphone device you are focusing on
		std::size_t found = nameStr.find(L"Microphone");
		if(found != std::string::npos)
		{
			// Print endpoint friendly name.
			//printf("Endpoint friendly name: \"%S\"\n", varName.pwszVal);

			// Get the session manager.
			hr = pCaptureDevice->Activate(
				__uuidof(IAudioSessionManager2), CLSCTX_ALL,
				NULL, (void**)&pSessionManager);
			break;
		}
	}

	// Get session state
	if(!pSessionManager)
	{
		return (result = FALSE);
	}

	int cbSessionCount = 0;
	LPWSTR pswSession = NULL;

	IAudioSessionEnumerator* pSessionList = NULL;
	IAudioSessionControl* pSessionControl = NULL;
	IAudioSessionControl2* pSessionControl2 = NULL;

	// Get the current list of sessions.
	hr = pSessionManager->GetSessionEnumerator(&pSessionList);

	// Get the session count.
	hr = pSessionList->GetCount(&cbSessionCount);
	//wprintf_s(L"Session count: %d\n", cbSessionCount);

	for(int index = 0; index < cbSessionCount; index++)
	{
		CoTaskMemFree(pswSession);
		SAFE_RELEASE(pSessionControl);

		// Get the <n>th session.
		hr = pSessionList->GetSession(index, &pSessionControl);

		hr = pSessionControl->QueryInterface(
			__uuidof(IAudioSessionControl2), (void**)&pSessionControl2);

		// Exclude system sound session
		hr = pSessionControl2->IsSystemSoundsSession();
		if(S_OK == hr)
		{
			//wprintf_s(L" this is a system sound.\n");
			continue;
		}

		// Optional. Determine which application is using Microphone for recording
		LPWSTR instId = NULL;
		hr = pSessionControl2->GetSessionInstanceIdentifier(&instId);
		if(S_OK == hr)
		{
			//wprintf_s(L"SessionInstanceIdentifier: %s\n", instId);
		}

		AudioSessionState state;
		hr = pSessionControl->GetState(&state);
		switch(state)
		{
		case AudioSessionStateInactive:
			//wprintf_s(L"Session state: Inactive\n");
			break;
		case AudioSessionStateActive:
			// #3 Active state indicates it is recording, otherwise is not recording.
			//wprintf_s(L"Session state: Active\n");
			result = TRUE;
			break;
		case AudioSessionStateExpired:
			//wprintf_s(L"Session state: Expired\n");
			break;
		}
	}

done:
	// Clean up.
	SAFE_RELEASE(pSessionControl);
	SAFE_RELEASE(pSessionList);
	SAFE_RELEASE(pSessionControl2);
	SAFE_RELEASE(pSessionManager);
	SAFE_RELEASE(pEnumerator);

	return result;
}

std::string get_active_window(void)
{
	HWND handle = GetForegroundWindow();
	const int max_count = 256 + 1;
	CHAR lpBuffer[max_count];

	if(!GetWindowTextA(handle, lpBuffer, max_count))
		//throw std::runtime_error("Could not acquire window title.");
		return std::string("Failed to acquire window title.");
	else
		return std::string(lpBuffer);
}

std::string get_username(void)
{
	CHAR lpBuffer[UNLEN + 1];
	DWORD pcbBuffer = UNLEN + 1;

	if(!GetUserNameA(lpBuffer, &pcbBuffer))
		throw std::runtime_error("Could not acquire username.");
	else
		return std::string(lpBuffer);
}

void write_app_focus(const std::string& str)
{
	static constexpr char file_name[] = "C:\\log-app.txt";
	std::ofstream logfile;
	logfile.open(file_name, std::ios_base::app);
	logfile << str << '\n';
	logfile.close();
}

void write_mic_active(const std::string& str)
{
	static constexpr char file_name[] = "C:\\log-camera.txt";
	std::ofstream logfile;
	logfile.open(file_name, std::ios_base::app);
	logfile << str << '\n';
	logfile.close();
}

int main(void) try
{
	std::string user = get_username();
	std::string last_active_window = get_active_window();
	bool last_mic_active = microphone_is_recording();

	while(true)
	{
		Sleep(5000);
		std::string active_window = get_active_window();
		bool mic_active = microphone_is_recording();

		if(active_window != last_active_window)
		{
			write_app_focus(user + ": " + active_window);
			last_active_window = active_window;
		}

		if(mic_active != last_mic_active)
		{
			if(mic_active)
				write_mic_active(user + ": microphone activated.");
			else
				write_mic_active(user + ": microphone deactivated.");

			last_mic_active = mic_active;
		}
	}

	return EXIT_SUCCESS;
}
catch(std::exception& e)
{
	std::cout << "Exception thrown: " << e.what() << '\n';
}