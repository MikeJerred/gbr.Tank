#pragma once

#include <fstream>
#include <iostream>
#include <string>

namespace gbr::Tank::Utilities {

    class LogUtility {
    private:
        static std::wofstream* logFile;
    public:
        static void Init(std::wstring charName);
        static void Close();
        static void Log(std::wstring str);
    };
}