#include <stdio.h>
#include <time.h>

#include "LogUtility.h"

namespace gbr::Tank::Utilities {
    std::wofstream* LogUtility::logFile = nullptr;

    void LogUtility::Init(std::wstring charName) {
        auto fileName = L"log - " + charName + L".txt";

		logFile = new std::wofstream(fileName.c_str()); // comment out to stop logging
    }

    void LogUtility::Close() {
		if (logFile) {
			logFile->close();
			delete logFile;
		}
    }

    void LogUtility::Log(std::wstring str) {
        if (!logFile)
            return;

        time_t rawtime;
        time(&rawtime);

        struct tm timeinfo;
        localtime_s(&timeinfo, &rawtime);

        wchar_t buffer[16];
        wcsftime(buffer, sizeof(buffer), L"%H:%M:%S", &timeinfo);

		*logFile << L"[" << buffer << L"] " << str.c_str() << L"\n";

		logFile->flush();
    }
}